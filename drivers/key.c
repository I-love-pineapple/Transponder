/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    key.c
  * @brief   This file provides code for the key driver.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "key.h"
#include "gpio.h"

/* USER CODE BEGIN 0 */

/* 按键控制数组 */
static key_ctrl_t key_ctrl[KEY_MAX];

/* 按键配置 */
static key_config_t key_config = {
    .filter_time = 50,       // 默认滤波时间50ms
    .long_press_time = 1000, // 默认长按时间1000ms
    .repeat_time = 200       // 默认重复时间200ms
};

/* 按键GPIO映射表 */
static const struct {
    GPIO_TypeDef* port;
    uint16_t pin;
} key_gpio_map[KEY_MAX] = {
    {GPIOA, GPIO_PIN_8},   // KEY_1
    {GPIOA, GPIO_PIN_15},  // KEY_2
    {GPIOA, GPIO_PIN_5},   // KEY_3
    {GPIOB, GPIO_PIN_6},   // KEY_4
    {GPIOA, GPIO_PIN_4},   // KEY_5
    {GPIOB, GPIO_PIN_7}    // KEY_6
};

/* USER CODE END 0 */

/* USER CODE BEGIN 1 */

/**
 * @brief 读取按键原始状态
 * @param key_id 按键ID
 * @return 0:按下 1:松开
 */
static uint8_t key_read_raw(key_id_t key_id)
{
    uint8_t index = (uint8_t)key_id;
    if (index >= KEY_MAX) {
        return 1; // 默认松开状态
    }
    
    GPIO_PinState pin_state = HAL_GPIO_ReadPin(key_gpio_map[index].port, key_gpio_map[index].pin);
    return (pin_state == GPIO_PIN_SET) ? 0 : 1; // 高电平为松开，低电平为按下
}

/**
 * @brief 获取系统时间戳(ms)
 * @return 时间戳
 */
static uint32_t key_get_tick(void)
{
    return HAL_GetTick();
}

/* USER CODE END 1 */

/**
 * @brief 按键初始化
 * @param config 按键配置参数，如果为NULL则使用默认配置
 * @return 0:成功 其他:失败
 */
int key_init(key_config_t *config)
{
    uint8_t i;
    
    // 使用默认配置或用户配置
    if (config != NULL) {
        key_config = *config;
    }
    
    // 初始化按键控制结构体
    for (i = 0; i < (uint8_t)KEY_MAX; i++) {
        key_ctrl[i].port = key_gpio_map[i].port;
        key_ctrl[i].pin = key_gpio_map[i].pin;
        key_ctrl[i].state = KEY_STATE_RELEASED;
        key_ctrl[i].event = KEY_EVENT_NONE;
        key_ctrl[i].press_time = 0;
        key_ctrl[i].last_time = 0;
        key_ctrl[i].raw_state = 1;    // 默认松开状态
        key_ctrl[i].stable_state = 1; // 默认松开状态
    }
    
    return 0;
}

/**
 * @brief 按键处理函数，需要在主循环中周期性调用
 * @param None
 * @return None
 */
void key_process(void)
{
    uint8_t i;
    uint32_t current_time = key_get_tick();
    
    for (i = 0; i < (uint8_t)KEY_MAX; i++) {
        // 读取原始状态
        key_ctrl[i].raw_state = key_read_raw((key_id_t)i);
        
        // 状态发生变化时进行滤波处理
        if (key_ctrl[i].raw_state != key_ctrl[i].stable_state) {
            // 检查滤波时间
            if ((current_time - key_ctrl[i].last_time) >= key_config.filter_time) {
                key_ctrl[i].stable_state = key_ctrl[i].raw_state;
                key_ctrl[i].last_time = current_time;
                
                // 状态变化处理
                if (key_ctrl[i].stable_state == 0) {
                    // 按键按下
                    if (key_ctrl[i].state == KEY_STATE_RELEASED) {
                        key_ctrl[i].state = KEY_STATE_PRESSED;
                        key_ctrl[i].event = KEY_EVENT_PRESS;
                        key_ctrl[i].press_time = current_time;
                    }
                } else {
                    // 按键松开
                    if (key_ctrl[i].state != KEY_STATE_RELEASED) {
                        key_ctrl[i].state = KEY_STATE_RELEASED;
                        key_ctrl[i].event = KEY_EVENT_RELEASE;
                    }
                }
            }
        } else {
            // 状态稳定，检查长按和重复
            if (key_ctrl[i].state == KEY_STATE_PRESSED) {
                uint32_t press_duration = current_time - key_ctrl[i].press_time;
                
                if (press_duration >= key_config.long_press_time) {
                    // 进入长按状态
                    key_ctrl[i].state = KEY_STATE_LONG_PRESS;
                    key_ctrl[i].event = KEY_EVENT_LONG_PRESS;
                    key_ctrl[i].last_time = current_time; // 重置时间戳，避免立即触发重复
                }
            } else if (key_ctrl[i].state == KEY_STATE_LONG_PRESS) {
                // 长按状态下的重复检测
                uint32_t time_since_last = current_time - key_ctrl[i].last_time;
                
                if (time_since_last >= key_config.repeat_time) {
                    key_ctrl[i].event = KEY_EVENT_REPEAT;
                    key_ctrl[i].last_time = current_time; // 更新上次重复时间
                }
            }
        }
    }
}

/**
 * @brief 获取按键事件
 * @param key_id 按键ID
 * @return 按键事件
 */
key_event_t key_get_event(key_id_t key_id)
{
    uint8_t index = (uint8_t)key_id;
    if (index >= KEY_MAX) {
        return KEY_EVENT_NONE;
    }
    
    return key_ctrl[index].event;
}

/**
 * @brief 清除按键事件
 * @param key_id 按键ID
 * @return None
 */
void key_clear_event(key_id_t key_id)
{
    uint8_t index = (uint8_t)key_id;
    if (index < KEY_MAX) {
        key_ctrl[index].event = KEY_EVENT_NONE;
    }
}

/**
 * @brief 获取按键状态
 * @param key_id 按键ID
 * @return 按键状态
 */
key_state_t key_get_state(key_id_t key_id)
{
    uint8_t index = (uint8_t)key_id;
    if (index >= KEY_MAX) {
        return KEY_STATE_RELEASED;
    }
    
    return key_ctrl[index].state;
}

/**
 * @brief 设置按键滤波时间
 * @param filter_time 滤波时间(ms)
 * @return None
 */
void key_set_filter_time(uint32_t filter_time)
{
    key_config.filter_time = filter_time;
}



/* USER CODE BEGIN 2 */

/* USER CODE END 2 */ 