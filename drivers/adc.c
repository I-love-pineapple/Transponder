/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    adc.c
  * @brief   This file provides code for the configuration
  *          of the ADC instances.
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
#include "adc.h"

/* USER CODE BEGIN 0 */

/* ADC句柄 */
static ADC_HandleTypeDef hadc1;

/* ADC转换结果 */
static uint16_t adc_raw_value = 0;

/* ADC状态 */
static adc_state_t adc_current_state = ADC_STATE_RESET;

/* USER CODE END 0 */

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

/**
 * @brief ADC1 Initialization Function
 * @param None
 * @retval HAL_StatusTypeDef
 */
static HAL_StatusTypeDef MX_ADC1_Init(void)
{
    ADC_ChannelConfTypeDef sConfig = {0};

    /* ADC1 configuration */
    hadc1.Instance = ADC1;
    hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV2;
    hadc1.Init.Resolution = ADC_RESOLUTION_12B;
    hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
    hadc1.Init.ScanConvMode = ADC_SCAN_DIRECTION_FORWARD;
    hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
    hadc1.Init.LowPowerAutoWait = DISABLE;
    hadc1.Init.LowPowerAutoPowerOff = DISABLE;
    hadc1.Init.ContinuousConvMode = DISABLE;
    hadc1.Init.DiscontinuousConvMode = DISABLE;
    hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
    hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
    hadc1.Init.DMAContinuousRequests = DISABLE;
    hadc1.Init.Overrun = ADC_OVR_DATA_PRESERVED;
    hadc1.Init.LowPowerFrequencyMode = DISABLE;
    hadc1.Init.SamplingTime = ADC_SAMPLETIME_79CYCLES_5; // 增加采样时间提高精度
    hadc1.Init.OversamplingMode = DISABLE;
    
    if (HAL_ADC_Init(&hadc1) != HAL_OK)
    {
        return HAL_ERROR;
    }

    /* ADC1 regular channel1 configuration */
    sConfig.Channel = ADC_CHANNEL_1;  // PA1对应ADC通道1
    sConfig.Rank = ADC_RANK_CHANNEL_NUMBER;
    
    if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
    {
        return HAL_ERROR;
    }
    
    return HAL_OK;
}

/**
 * @brief ADC MSP Initialization
 * @param hadc: ADC handle pointer
 * @retval None
 */
void HAL_ADC_MspInit(ADC_HandleTypeDef* hadc)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    if(hadc->Instance == ADC1)
    {
        /* ADC1 clock enable */
        __HAL_RCC_ADC1_CLK_ENABLE();
        
        /* ADC1 GPIO Configuration */
        /* PA1     ------> ADC1_IN1 */
        __HAL_RCC_GPIOA_CLK_ENABLE();
        
        GPIO_InitStruct.Pin = GPIO_PIN_1;
        GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    }
}

/**
 * @brief ADC MSP De-Initialization
 * @param hadc: ADC handle pointer
 * @retval None
 */
void HAL_ADC_MspDeInit(ADC_HandleTypeDef* hadc)
{
    if(hadc->Instance == ADC1)
    {
        /* Peripheral clock disable */
        __HAL_RCC_ADC1_CLK_DISABLE();
        
        /* ADC1 GPIO Configuration */
        /* PA1     ------> ADC1_IN1 */
        HAL_GPIO_DeInit(GPIOA, GPIO_PIN_1);
    }
}

/* USER CODE BEGIN 2 */

/**
 * @brief ADC初始化
 * @param None
 * @return adc_error_t: ADC_OK成功，其他为错误代码
 */
adc_error_t adc_init(void)
{
    /* 设置状态为忙碌 */
    adc_current_state = ADC_STATE_BUSY;
    
    /* 初始化ADC */
    if (MX_ADC1_Init() != HAL_OK)
    {
        adc_current_state = ADC_STATE_ERROR;
        return ADC_ERROR_INIT;
    }
    
    /* 启动ADC校准 */
    if (HAL_ADCEx_Calibration_Start(&hadc1, ADC_SINGLE_ENDED) != HAL_OK)
    {
        adc_current_state = ADC_STATE_ERROR;
        return ADC_ERROR_CALIBRATION;
    }
    
    /* 重置状态 */
    adc_raw_value = 0;
    adc_current_state = ADC_STATE_READY;
    
    return ADC_OK;
}

/**
 * @brief 启动ADC转换
 * @param None
 * @return adc_error_t: ADC_OK成功，其他为错误代码
 */
adc_error_t adc_start_conversion(void)
{
    if (adc_current_state != ADC_STATE_READY)
    {
        return ADC_ERROR_INIT;
    }
    
    if (HAL_ADC_Start(&hadc1) != HAL_OK)
    {
        adc_current_state = ADC_STATE_ERROR;
        return ADC_ERROR_START;
    }
    
    adc_current_state = ADC_STATE_BUSY;
    return ADC_OK;
}

/**
 * @brief 获取ADC原始值（单次读取）
 * @param None
 * @return ADC原始值(0-4095)
 */
uint16_t adc_get_raw_value(void)
{
    return adc_raw_value;
}

/**
 * @brief 获取ADC原始值（多次采样平均）
 * @param sample_count: 采样次数(1-255)
 * @return ADC原始值(0-4095)
 */
uint16_t adc_get_raw_value_average(uint8_t sample_count)
{
    if (sample_count == 0 || sample_count > ADC_MAX_SAMPLING_COUNT)
    {
        sample_count = ADC_SAMPLING_COUNT;
    }
    
    uint32_t sum = 0;
    uint8_t valid_samples = 0;
    
    for (uint8_t i = 0; i < sample_count; i++)
    {
        if (adc_start_conversion() == ADC_OK)
        {
            if (adc_is_conversion_complete(ADC_CONVERSION_TIMEOUT_MS))
            {
                sum += adc_raw_value;
                valid_samples++;
            }
        }
        HAL_Delay(1); // 短暂延时，避免连续采样过快
    }
    
    if (valid_samples > 0)
    {
        return (uint16_t)(sum / valid_samples);
    }
    
    return 0;
}

/**
 * @brief 获取ADC电压值(mV)
 * @param None
 * @return 电压值(mV)
 */
uint32_t adc_get_voltage_mv(void)
{
    return (uint32_t)((adc_raw_value * ADC_VREF_MV) / ADC_RESOLUTION);
}

/**
 * @brief 获取ADC电压值(V)
 * @param None
 * @return 电压值(V)，浮点数
 */
float adc_get_voltage_v(void)
{
    return (float)(adc_raw_value * ADC_VREF_MV) / (ADC_RESOLUTION * 1000.0f);
}

/**
 * @brief 获取ADC电压值(mV) - 多次采样平均
 * @param sample_count: 采样次数(1-255)
 * @return 电压值(mV)
 */
uint32_t adc_get_voltage_mv_average(uint8_t sample_count)
{
    uint16_t raw_avg = adc_get_raw_value_average(sample_count);
    return (uint32_t)((raw_avg * ADC_VREF_MV) / ADC_RESOLUTION);
}

/**
 * @brief 检查ADC转换是否完成
 * @param timeout_ms: 超时时间(ms)
 * @return 1:完成 0:未完成/超时
 */
uint8_t adc_is_conversion_complete(uint32_t timeout_ms)
{
    if (HAL_ADC_PollForConversion(&hadc1, timeout_ms) == HAL_OK)
    {
        adc_raw_value = HAL_ADC_GetValue(&hadc1);
        HAL_ADC_Stop(&hadc1);
        adc_current_state = ADC_STATE_READY;
        return 1;
    }
    
    // 超时处理
    HAL_ADC_Stop(&hadc1);
    adc_current_state = ADC_STATE_ERROR;
    return 0;
}

/**
 * @brief 停止ADC转换
 * @param None
 * @return None
 */
void adc_stop_conversion(void)
{
    HAL_ADC_Stop(&hadc1);
    adc_current_state = ADC_STATE_READY;
}

/**
 * @brief 获取ADC状态
 * @param None
 * @return adc_state_t: 当前ADC状态
 */
adc_state_t adc_get_state(void)
{
    return adc_current_state;
}

/**
 * @brief ADC自检
 * @param None
 * @return adc_error_t: ADC_OK成功，其他为错误代码
 */
adc_error_t adc_self_test(void)
{
    if (adc_current_state == ADC_STATE_RESET)
    {
        return ADC_ERROR_INIT;
    }
    
    // 进行几次采样测试
    uint16_t test_values[5];
    uint8_t valid_readings = 0;
    
    for (uint8_t i = 0; i < 5; i++)
    {
        if (adc_start_conversion() == ADC_OK)
        {
            if (adc_is_conversion_complete(ADC_CONVERSION_TIMEOUT_MS))
            {
                test_values[valid_readings] = adc_raw_value;
                valid_readings++;
            }
        }
        HAL_Delay(10);
    }
    
    // 检查是否有有效读数
    if (valid_readings < 3)
    {
        return ADC_ERROR_TIMEOUT;
    }
    
    // 检查读数是否在合理范围内
    for (uint8_t i = 0; i < valid_readings; i++)
    {
        if (test_values[i] > ADC_MAX_VALUE)
        {
            return ADC_ERROR_INVALID_PARAM;
        }
    }
    
    return ADC_OK;
}

/**
 * @brief ADC反初始化
 * @param None
 * @return None
 */
void adc_deinit(void)
{
    HAL_ADC_Stop(&hadc1);
    HAL_ADC_DeInit(&hadc1);
    adc_raw_value = 0;
    adc_current_state = ADC_STATE_RESET;
}

/* USER CODE END 2 */