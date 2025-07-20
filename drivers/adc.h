/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    adc.h
  * @brief   This file contains all the function prototypes for
  *          the adc.c file
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
#ifndef __ADC_H__
#define __ADC_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* USER CODE BEGIN Private defines */

/* ADC配置参数 */
#define ADC_VREF_MV          3300    // 参考电压3.3V
#define ADC_RESOLUTION       4096    // 12位ADC分辨率 (2^12)
#define ADC_MAX_VALUE        4095    // 12位ADC最大值 (2^12 - 1)
#define ADC_CHANNEL_COUNT    1       // ADC通道数量

/* ADC采样配置 */
#define ADC_SAMPLING_COUNT   10      // 多次采样数量，用于滤波
#define ADC_MAX_SAMPLING_COUNT 100   // 最大采样次数限制
#define ADC_CONVERSION_TIMEOUT_MS  100  // 转换超时时间(ms)

/* ADC状态定义 */
typedef enum {
    ADC_STATE_RESET = 0,
    ADC_STATE_READY,
    ADC_STATE_BUSY,
    ADC_STATE_ERROR
} adc_state_t;

/* ADC错误代码定义 */
typedef enum {
    ADC_OK = 0,
    ADC_ERROR_INIT = -1,
    ADC_ERROR_CALIBRATION = -2,
    ADC_ERROR_START = -3,
    ADC_ERROR_TIMEOUT = -4,
    ADC_ERROR_INVALID_PARAM = -5
} adc_error_t;

/* USER CODE END Private defines */

/* USER CODE BEGIN Prototypes */

/**
 * @brief ADC初始化
 * @param None
 * @return adc_error_t: ADC_OK成功，其他为错误代码
 */
adc_error_t adc_init(void);

/**
 * @brief 启动ADC转换
 * @param None
 * @return adc_error_t: ADC_OK成功，其他为错误代码
 */
adc_error_t adc_start_conversion(void);

/**
 * @brief 获取ADC原始值（单次读取）
 * @param None
 * @return ADC原始值(0-4095)
 */
uint16_t adc_get_raw_value(void);

/**
 * @brief 获取ADC原始值（多次采样平均）
 * @param sample_count: 采样次数(1-100)，0表示使用默认值
 * @return ADC原始值(0-4095)
 */
uint16_t adc_get_raw_value_average(uint8_t sample_count);

/**
 * @brief 获取ADC电压值(mV)
 * @param None
 * @return 电压值(mV)
 */
uint32_t adc_get_voltage_mv(void);

/**
 * @brief 获取ADC电压值(V)
 * @param None
 * @return 电压值(V)，浮点数
 */
float adc_get_voltage_v(void);

/**
 * @brief 获取ADC电压值(mV) - 多次采样平均
 * @param sample_count: 采样次数(1-100)，0表示使用默认值
 * @return 电压值(mV)
 */
uint32_t adc_get_voltage_mv_average(uint8_t sample_count);

/**
 * @brief 检查ADC转换是否完成
 * @param timeout_ms: 超时时间(ms)
 * @return 1:完成 0:未完成/超时
 */
uint8_t adc_is_conversion_complete(uint32_t timeout_ms);

/**
 * @brief 停止ADC转换
 * @param None
 * @return None
 */
void adc_stop_conversion(void);

/**
 * @brief 获取ADC状态
 * @param None
 * @return adc_state_t: 当前ADC状态
 */
adc_state_t adc_get_state(void);

/**
 * @brief ADC自检
 * @param None
 * @return adc_error_t: ADC_OK成功，其他为错误代码
 */
adc_error_t adc_self_test(void);

/**
 * @brief ADC反初始化
 * @param None
 * @return None
 */
void adc_deinit(void);

/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /*__ ADC_H__ */