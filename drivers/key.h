/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    key.h
  * @brief   This file contains all the function prototypes for
  *          the key.c file
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
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __KEY_H__
#define __KEY_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* USER CODE BEGIN Private defines */

/* 按键定义 */
typedef enum {
    KEY_1 = 0,  // PA8
    KEY_2,      // PA15
    KEY_3,      // PA5
    KEY_4,      // PB6
    KEY_5,      // PA4
    KEY_6,      // PB7
    KEY_MAX
} key_id_t;

/* 按键状态 */
typedef enum {
    KEY_STATE_RELEASED = 0,  // 松开状态
    KEY_STATE_PRESSED,       // 按下状态
    KEY_STATE_LONG_PRESS,    // 长按状态
    KEY_STATE_REPEAT         // 重复状态
} key_state_t;

/* 按键事件 */
typedef enum {
    KEY_EVENT_NONE = 0,      // 无事件
    KEY_EVENT_PRESS,         // 按下事件
    KEY_EVENT_RELEASE,       // 松开事件
    KEY_EVENT_LONG_PRESS,    // 长按事件
    KEY_EVENT_REPEAT         // 重复事件
} key_event_t;

/* 按键配置结构体 */
typedef struct {
    uint32_t filter_time;    // 滤波时间(ms)，默认50ms
    uint32_t long_press_time; // 长按时间(ms)，默认1000ms
    uint32_t repeat_time;    // 重复时间(ms)，默认200ms
} key_config_t;

/* 按键控制结构体 */
typedef struct {
    GPIO_TypeDef* port;      // GPIO端口
    uint16_t pin;           // GPIO引脚
    key_state_t state;      // 当前状态
    key_event_t event;      // 当前事件
    uint32_t press_time;    // 按下时间戳
    uint32_t last_time;     // 上次处理时间戳
    uint8_t raw_state;      // 原始状态
    uint8_t stable_state;   // 稳定状态
} key_ctrl_t;

/* USER CODE END Private defines */

/* USER CODE BEGIN Prototypes */

/**
 * @brief 按键初始化
 * @param config 按键配置参数
 * @return 0:成功 其他:失败
 */
int key_init(key_config_t *config);

/**
 * @brief 按键处理函数，需要在主循环中周期性调用
 * @param None
 * @return None
 */
void key_process(void);

/**
 * @brief 获取按键事件
 * @param key_id 按键ID
 * @return 按键事件
 */
key_event_t key_get_event(key_id_t key_id);

/**
 * @brief 清除按键事件
 * @param key_id 按键ID
 * @return None
 */
void key_clear_event(key_id_t key_id);

/**
 * @brief 获取按键状态
 * @param key_id 按键ID
 * @return 按键状态
 */
key_state_t key_get_state(key_id_t key_id);

/**
 * @brief 设置按键滤波时间
 * @param filter_time 滤波时间(ms)
 * @return None
 */
void key_set_filter_time(uint32_t filter_time);



/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif
#endif /*__ KEY_H__ */ 