/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    usart.c
  * @brief   This file provides code for the configuration
  *          of the USART instances.
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
#include "usart.h"

/* USER CODE BEGIN 0 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "gpio.h"

#define DBG_TAG "usart"
#define DBG_LVL DBG_LOG
#include "debug_log.h"

// DMA缓冲区定义
uint8_t lpuart1_rx_dma_buf[UART_BUF_SIZE];
uint8_t lpuart1_tx_dma_buf[UART_BUF_SIZE];
uint8_t usart2_rx_dma_buf[UART_BUF_SIZE];
uint8_t usart2_tx_dma_buf[UART_BUF_SIZE*8];

// 环形缓冲区用的缓冲区
uint8_t lpuart1_rx_buf[UART_BUF_SIZE];
uint8_t lpuart1_tx_buf[UART_BUF_SIZE];
uint8_t usart2_rx_buf[UART_BUF_SIZE];
uint8_t usart2_tx_buf[UART_BUF_SIZE*8];

// 环形缓冲区结构体
struct ringbuffer lpuart1_rx_ringbuf;
struct ringbuffer lpuart1_tx_ringbuf;
struct ringbuffer usart2_rx_ringbuf;
struct ringbuffer usart2_tx_ringbuf;

// 传输完成标志
volatile uint8_t lpuart1_tx_complete = 1;
volatile uint8_t usart2_tx_complete = 1;

// 内部函数声明
static void lpuart1_ringbuffer_init(void);
static void lpuart1_ringbuffer_deinit(void);
static void usart2_ringbuffer_init(void);
static void usart2_ringbuffer_deinit(void);
/* USER CODE END 0 */

UART_HandleTypeDef hlpuart1;
UART_HandleTypeDef huart2;
DMA_HandleTypeDef hdma_lpuart1_tx;
DMA_HandleTypeDef hdma_lpuart1_rx;
DMA_HandleTypeDef hdma_usart2_tx;
DMA_HandleTypeDef hdma_usart2_rx;

/* LPUART1 init function */

/* 保持原来的初始化函数，用于系统自动调用 */
void MX_LPUART1_UART_Init(void)
{

  /* USER CODE BEGIN LPUART1_Init 0 */

  /* USER CODE END LPUART1_Init 0 */

  /* USER CODE BEGIN LPUART1_Init 1 */

  /* USER CODE END LPUART1_Init 1 */
  hlpuart1.Instance = LPUART1;
  hlpuart1.Init.BaudRate = 9600;
  hlpuart1.Init.WordLength = UART_WORDLENGTH_8B;
  hlpuart1.Init.StopBits = UART_STOPBITS_1;
  hlpuart1.Init.Parity = UART_PARITY_NONE;
  hlpuart1.Init.Mode = UART_MODE_TX_RX;
  hlpuart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  hlpuart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  hlpuart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&hlpuart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN LPUART1_Init 2 */

  /* USER CODE END LPUART1_Init 2 */

}

/* 新的LPUART1初始化函数，包含环形缓冲区初始化 */
void lpuart1_init(void)
{
    MX_LPUART1_UART_Init();
    lpuart1_ringbuffer_init();
    lpuart1_start_dma();
}

/* USART2 init function */

void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/* 新的USART2初始化函数，包含环形缓冲区初始化 */
void usart2_init(void)
{
    MX_USART2_UART_Init();
    usart2_ringbuffer_init();
    usart2_start_dma();
}

void HAL_UART_MspInit(UART_HandleTypeDef* uartHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(uartHandle->Instance==LPUART1)
  {
  /* USER CODE BEGIN LPUART1_MspInit 0 */

  /* USER CODE END LPUART1_MspInit 0 */
    /* LPUART1 clock enable */
    __HAL_RCC_LPUART1_CLK_ENABLE();

    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**LPUART1 GPIO Configuration
    PA2     ------> LPUART1_TX
    PA3     ------> LPUART1_RX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_2|GPIO_PIN_3;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF6_LPUART1;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* LPUART1 DMA Init */
    /* LPUART1_TX Init */
    hdma_lpuart1_tx.Instance = DMA1_Channel2;
    hdma_lpuart1_tx.Init.Request = DMA_REQUEST_5;
    hdma_lpuart1_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
    hdma_lpuart1_tx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_lpuart1_tx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_lpuart1_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_lpuart1_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_lpuart1_tx.Init.Mode = DMA_NORMAL;
    hdma_lpuart1_tx.Init.Priority = DMA_PRIORITY_VERY_HIGH;
    if (HAL_DMA_Init(&hdma_lpuart1_tx) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_LINKDMA(uartHandle,hdmatx,hdma_lpuart1_tx);

    /* LPUART1_RX Init */
    hdma_lpuart1_rx.Instance = DMA1_Channel3;
    hdma_lpuart1_rx.Init.Request = DMA_REQUEST_5;
    hdma_lpuart1_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_lpuart1_rx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_lpuart1_rx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_lpuart1_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_lpuart1_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_lpuart1_rx.Init.Mode = DMA_NORMAL;
    hdma_lpuart1_rx.Init.Priority = DMA_PRIORITY_VERY_HIGH;
    if (HAL_DMA_Init(&hdma_lpuart1_rx) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_LINKDMA(uartHandle,hdmarx,hdma_lpuart1_rx);

    /* LPUART1 interrupt Init */
    HAL_NVIC_SetPriority(LPUART1_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(LPUART1_IRQn);
  /* USER CODE BEGIN LPUART1_MspInit 1 */

  /* USER CODE END LPUART1_MspInit 1 */
  }
  else if(uartHandle->Instance==USART2)
  {
  /* USER CODE BEGIN USART2_MspInit 0 */

  /* USER CODE END USART2_MspInit 0 */
    /* USART2 clock enable */
    __HAL_RCC_USART2_CLK_ENABLE();

    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**USART2 GPIO Configuration
    PA9     ------> USART2_TX
    PA10     ------> USART2_RX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_9|GPIO_PIN_10;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF4_USART2;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* USART2 DMA Init */
    /* USART2_TX Init */
    hdma_usart2_tx.Instance = DMA1_Channel4;
    hdma_usart2_tx.Init.Request = DMA_REQUEST_4;
    hdma_usart2_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
    hdma_usart2_tx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_usart2_tx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_usart2_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_usart2_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_usart2_tx.Init.Mode = DMA_NORMAL;
    hdma_usart2_tx.Init.Priority = DMA_PRIORITY_LOW;
    if (HAL_DMA_Init(&hdma_usart2_tx) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_LINKDMA(uartHandle,hdmatx,hdma_usart2_tx);

    /* USART2_RX Init */
    hdma_usart2_rx.Instance = DMA1_Channel5;
    hdma_usart2_rx.Init.Request = DMA_REQUEST_4;
    hdma_usart2_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_usart2_rx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_usart2_rx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_usart2_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_usart2_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_usart2_rx.Init.Mode = DMA_NORMAL;
    hdma_usart2_rx.Init.Priority = DMA_PRIORITY_LOW;
    if (HAL_DMA_Init(&hdma_usart2_rx) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_LINKDMA(uartHandle,hdmarx,hdma_usart2_rx);

    /* USART2 interrupt Init */
    HAL_NVIC_SetPriority(USART2_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(USART2_IRQn);
  /* USER CODE BEGIN USART2_MspInit 1 */

  /* USER CODE END USART2_MspInit 1 */
  }
}

void HAL_UART_MspDeInit(UART_HandleTypeDef* uartHandle)
{

  if(uartHandle->Instance==LPUART1)
  {
  /* USER CODE BEGIN LPUART1_MspDeInit 0 */

  /* USER CODE END LPUART1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_LPUART1_CLK_DISABLE();

    /**LPUART1 GPIO Configuration
    PA2     ------> LPUART1_TX
    PA3     ------> LPUART1_RX
    */
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_2|GPIO_PIN_3);

    /* LPUART1 DMA DeInit */
    HAL_DMA_DeInit(uartHandle->hdmatx);
    HAL_DMA_DeInit(uartHandle->hdmarx);

    /* LPUART1 interrupt Deinit */
    HAL_NVIC_DisableIRQ(LPUART1_IRQn);
  /* USER CODE BEGIN LPUART1_MspDeInit 1 */

  /* USER CODE END LPUART1_MspDeInit 1 */
  }
  else if(uartHandle->Instance==USART2)
  {
  /* USER CODE BEGIN USART2_MspDeInit 0 */

  /* USER CODE END USART2_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_USART2_CLK_DISABLE();

    /**USART2 GPIO Configuration
    PA9     ------> USART2_TX
    PA10     ------> USART2_RX
    */
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_9|GPIO_PIN_10);

    /* USART2 DMA DeInit */
    HAL_DMA_DeInit(uartHandle->hdmatx);
    HAL_DMA_DeInit(uartHandle->hdmarx);

    /* USART2 interrupt Deinit */
    HAL_NVIC_DisableIRQ(USART2_IRQn);
  /* USER CODE BEGIN USART2_MspDeInit 1 */

  /* USER CODE END USART2_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */

/**
 * @brief lpuart1 环形缓冲区初始化
 */
static void lpuart1_ringbuffer_init(void)
{
    ringbuffer_init(&lpuart1_rx_ringbuf, lpuart1_rx_buf, UART_BUF_SIZE);
    ringbuffer_init(&lpuart1_tx_ringbuf, lpuart1_tx_buf, UART_BUF_SIZE);
}

/**
 * @brief lpuart1 环形缓冲区反初始化
 */
static void lpuart1_ringbuffer_deinit(void)
{
    ringbuffer_reset(&lpuart1_rx_ringbuf);
    ringbuffer_reset(&lpuart1_tx_ringbuf);
}

/**
 * @brief usart2 环形缓冲区初始化
 */
static void usart2_ringbuffer_init(void)
{
    ringbuffer_init(&usart2_rx_ringbuf, usart2_rx_buf, UART_BUF_SIZE);
    ringbuffer_init(&usart2_tx_ringbuf, usart2_tx_buf, UART_BUF_SIZE*8);
}

/**
 * @brief usart2 环形缓冲区反初始化
 */
static void usart2_ringbuffer_deinit(void)
{
    ringbuffer_reset(&usart2_rx_ringbuf);
    ringbuffer_reset(&usart2_tx_ringbuf);
}

/**
 * @brief 启动LPUART1的DMA接收
 */
void lpuart1_start_dma(void)
{
    HAL_UARTEx_ReceiveToIdle_DMA(&hlpuart1, lpuart1_rx_dma_buf, UART_BUF_SIZE);
    __HAL_DMA_DISABLE_IT(&hdma_lpuart1_rx, DMA_IT_HT);
}

/**
 * @brief 启动USART2的DMA接收
 */
void usart2_start_dma(void)
{
    HAL_UARTEx_ReceiveToIdle_DMA(&huart2, usart2_rx_dma_buf, UART_BUF_SIZE);
    __HAL_DMA_DISABLE_IT(&hdma_usart2_rx, DMA_IT_HT);
}

/**
 * @brief LPUART1反初始化
 */
void lpuart1_deinit(void)
{
    HAL_UART_DeInit(&hlpuart1);
    lpuart1_ringbuffer_deinit();
}

/**
 * @brief USART2反初始化
 */
void usart2_deinit(void)
{
    HAL_UART_DeInit(&huart2);
    usart2_ringbuffer_deinit();
}

/**
 * @brief UART接收事件回调函数
 * @param huart UART句柄
 * @param Size 接收到的数据长度
 */
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
    if(huart->Instance == LPUART1)
    {
        // 将接收到的数据放入环形缓冲区
        ringbuffer_put(&lpuart1_rx_ringbuf, lpuart1_rx_dma_buf, Size);
        // 重新启动DMA接收
        lpuart1_start_dma();
    }
    else if(huart->Instance == USART2)
    {
        // 将接收到的数据放入环形缓冲区
        ringbuffer_put(&usart2_tx_ringbuf, usart2_rx_dma_buf, Size);
        ringbuffer_put(&usart2_rx_ringbuf, usart2_rx_dma_buf, Size);
        // 重新启动DMA接收
        usart2_start_dma();
    }
}

/**
 * @brief UART错误回调函数
 * @param huart UART句柄
 */
void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    if(huart->Instance == LPUART1)
    {
        // 重新启动DMA接收
        lpuart1_start_dma();
    }
    else if(huart->Instance == USART2)
    {
        // 重新启动DMA接收
        usart2_start_dma();
    }
}

/**
 * @brief UART发送完成回调函数
 * @param huart UART句柄
 */
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    if(huart->Instance == LPUART1)
    {
        lpuart1_tx_complete = 1;
    }
    else if(huart->Instance == USART2)
    {
        usart2_tx_complete = 1;
    }
}

// 数据处理缓冲区
uint8_t lpuart1_parse_cache[UART_BUF_SIZE] = {0};
uint16_t lpuart1_parse_cache_len = 0;
uint8_t lpuart1_to_be_send[UART_BUF_SIZE] = {0};

uint8_t usart2_parse_cache[UART_BUF_SIZE] = {0};
uint16_t usart2_parse_cache_len = 0;
uint8_t usart2_to_be_send[UART_BUF_SIZE*8] = {0};

// 异步命令处理相关变量
typedef enum {
    CMD_STATE_IDLE = 0,
    CMD_STATE_PENDING,
    CMD_STATE_PROCESSING
} cmd_state_t;

volatile cmd_state_t usart2_cmd_state = CMD_STATE_IDLE;
uint8_t pending_cmd_buffer[UART_BUF_SIZE] = {0};

// 蓝牙配置状态机相关变量
typedef enum {
    BLE_CMD_IDLE = 0,
    BLE_CMD_ENTER_CONFIG,
    BLE_CMD_EXIT_CONFIG, 
    BLE_CMD_READ_CONFIG,
    BLE_CMD_WRITE_CONFIG,
    BLE_CMD_READ_VERSION,
    BLE_CMD_RESET_MODULE,
    BLE_CMD_WAIT_RESPONSE,
    BLE_CMD_TIMEOUT
} ble_cmd_state_t;

typedef struct {
    ble_cmd_state_t state;
    ble_cmd_state_t original_cmd;  // 记录原始命令类型
    uint32_t start_time;
    uint32_t timeout_ms;
    uint8_t cmd_data[10];
    uint8_t cmd_len;
    uint8_t response_data[10];
    uint8_t response_len;
    uint8_t expected_response_len;
    ble_config_result_t result;
    ble_config_t *config_ptr;  // 用于读取/写入配置
    uint8_t save_to_flash;     // 用于写入配置
    uint8_t *version_info_ptr; // 用于版本信息
    uint8_t *version_len_ptr;  // 用于版本信息长度
} ble_cmd_context_t;

volatile ble_cmd_context_t ble_cmd_ctx = {BLE_CMD_IDLE, BLE_CMD_IDLE, 0, 0, {0}, 0, {0}, 0, 0, BLE_CONFIG_SUCCESS, NULL, 0, NULL, NULL};

void debug_func_call(uint8_t *data)
{
    char *delim = " \r";
    char *p;

    p = strtok((char *)data, delim);

    // 设置
    if (strcmp(p, "set") == 0)
    {
        p = strtok(NULL, delim);
        if (strcmp(p, "m0") == 0)
        {
            ble_config_result_t result = ble_exit_config_mode_async();
            if (result == BLE_CONFIG_SUCCESS) {
                LOG_I("BLE退出配置模式命令已发送(模式0-定频模式)");
            } else {
                LOG_E("BLE退出配置模式失败: %d", result);
            }
        }
        else if (strcmp(p, "m1") == 0)
        {
            set_m0_state(1);
            set_m1_state(0);
            LOG_I("设置为模式1(跳频模式)");
        }
        else if (strcmp(p, "m2") == 0)
        {
            set_m0_state(0);
            set_m1_state(1);
            LOG_I("设置为模式2(预留模式)");
        }
        else if (strcmp(p, "m3") == 0)
        {
            ble_config_result_t result = ble_enter_config_mode_async();
            if (result == BLE_CONFIG_SUCCESS) {
                LOG_I("enter sent");
            } else {
                LOG_E("enter fail: %d", result);
            }
        }
        else{
            LOG_I("command error");
        }
    }
    else if (strcmp(p, "get") == 0)
    {
        p = strtok(NULL, delim);
        if (strcmp(p, "m") == 0)
        {
            LOG_I("m0 %d m1 %d", get_m0_state(), get_m1_state());
        }
        else if (strcmp(p, "ble") == 0)
        {
            uint8_t data[3] = {0xC3, 0xC3, 0xC3};
            ringbuffer_put(&lpuart1_tx_ringbuf, data, 3);
        }
        else{
            LOG_I("command error");
        }
    }
    else if (strcmp(p, "send") == 0)
    {
        p = strtok(NULL, delim);
        if (strcmp(p, "str") == 0)
        {
            p = strtok(NULL, delim);
            ringbuffer_put(&lpuart1_tx_ringbuf, (uint8_t *)p, strlen(p));
            LOG_I("send str %s", p);
        }
        else if (strcmp(p, "hex") == 0)
        {
            p = strtok(NULL, delim);
            uint8_t data[100] = {0};
            int len = strlen(p);
            if (len % 2 != 0) {
                LOG_I("hex string length must be even");
                return;
            }
            for(int i = 0; i < len; i += 2)
            {
                char hex_byte[3] = {p[i], p[i+1], '\0'};
                data[i / 2] = (uint8_t)strtol(hex_byte, NULL, 16);
            }
            ringbuffer_put(&lpuart1_tx_ringbuf, data, len / 2);
            LOG_I("send hex %s", p);
        }
        else{
            LOG_I("command error");
        }
    }
    else if (strcmp(p, "aux") == 0)
    {
        LOG_I("aux %d", get_aux_state());
    }
    else if (strcmp(p, "status") == 0)
    {
        LOG_I("系统状态:");
        LOG_I("  M0: %d, M1: %d", get_m0_state(), get_m1_state());
        LOG_I("  AUX: %d", get_aux_state());
        ble_cmd_state_t ble_state = ble_cmd_ctx.state;
        LOG_I("  BLE状态: %d", ble_state);
        if (ble_state != BLE_CMD_IDLE) {
            uint32_t start_time = ble_cmd_ctx.start_time;
            uint8_t expected_len = ble_cmd_ctx.expected_response_len;
            uint8_t response_len = ble_cmd_ctx.response_len;
            LOG_I("  执行时间: %dms", HAL_GetTick() - start_time);
            LOG_I("  期望响应长度: %d", expected_len);
            LOG_I("  实际响应长度: %d", response_len);
        }
    }
    else if (strcmp(p, "reboot") == 0)
    {
        HAL_NVIC_SystemReset();
    }
    // 蓝牙配置命令 - 异步处理
    else if (strcmp(p, "ble") == 0)
    {
        p = strtok(NULL, delim);
        if (strcmp(p, "enter") == 0)
        {
            // 检查当前是否有命令在执行
            if (ble_cmd_ctx.state != BLE_CMD_IDLE) {
                LOG_E("有BLE命令正在执行，状态: %d", ble_cmd_ctx.state);
                return;
            }
            
            ble_config_result_t result = ble_enter_config_mode_async();
            if (result == BLE_CONFIG_SUCCESS) {
                LOG_I("enter sent");
            } else {
                LOG_E("enter fail: %d", result);
            }
        }
        else if (strcmp(p, "exit") == 0)
        {
            // 检查当前是否有命令在执行
            if (ble_cmd_ctx.state != BLE_CMD_IDLE) {
                LOG_E("有BLE命令正在执行，状态: %d", ble_cmd_ctx.state);
                return;
            }
            
            ble_config_result_t result = ble_exit_config_mode_async();
            if (result == BLE_CONFIG_SUCCESS) {
                LOG_I("BLE退出配置模式命令已发送");
            } else {
                LOG_E("BLE退出配置模式失败: %d", result);
            }
        }
        else if (strcmp(p, "read") == 0)
        {
            // 检查当前是否有命令在执行
            if (ble_cmd_ctx.state != BLE_CMD_IDLE) {
                LOG_E("有BLE命令正在执行，状态: %d", ble_cmd_ctx.state);
                return;
            }
            
            ble_config_result_t result = ble_read_config_async();
            if (result == BLE_CONFIG_SUCCESS) {
                LOG_I("read sent");
            } else {
                LOG_E("read fail: %d", result);
            }
        }
        else if (strcmp(p, "write") == 0)
        {
            // 示例：ble write 15 0 0 3 1 0 0 0
            // 参数：重发次数 地址高 地址低 串口波特率 空中速率 信道 发射功率 定点传输
            static ble_config_t config = {0};  // 使用静态变量保证异步处理时数据有效
            
            p = strtok(NULL, delim);
            if (p) config.retransmit_times = atoi(p);
            p = strtok(NULL, delim);
            if (p) config.addr_high = atoi(p);
            p = strtok(NULL, delim);
            if (p) config.addr_low = atoi(p);
            p = strtok(NULL, delim);
            if (p) config.uart_baud = atoi(p);
            p = strtok(NULL, delim);
            if (p) config.air_rate = atoi(p);
            p = strtok(NULL, delim);
            if (p) config.channel = atoi(p);
            p = strtok(NULL, delim);
            if (p) config.tx_power = atoi(p);
            p = strtok(NULL, delim);
            if (p) config.fixed_trans = atoi(p);
            
            ble_config_result_t result = ble_write_config_async(&config, 1);
            if (result == BLE_CONFIG_SUCCESS) {
                LOG_I("BLE写入配置命令已发送");
            } else {
                LOG_E("BLE写入配置失败: %d", result);
            }
        }
        else if (strcmp(p, "version") == 0)
        {
            ble_config_result_t result = ble_read_version_async();
            if (result == BLE_CONFIG_SUCCESS) {
                LOG_I("BLE版本信息读取命令已发送");
            } else {
                LOG_E("BLE版本信息读取失败: %d", result);
            }
        }
        else if (strcmp(p, "reset") == 0)
        {
            ble_config_result_t result = ble_reset_module_async();
            if (result == BLE_CONFIG_SUCCESS) {
                LOG_I("BLE复位命令已发送");
            } else {
                LOG_E("BLE复位失败: %d", result);
            }
        }
        else if (strcmp(p, "status") == 0)
        {
            ble_cmd_state_t ble_state = ble_cmd_ctx.state;
            LOG_I("BLE命令状态: %d", ble_state);
            if (ble_state != BLE_CMD_IDLE) {
                uint32_t start_time = ble_cmd_ctx.start_time;
                LOG_I("命令执行时间: %d ms", HAL_GetTick() - start_time);
            }
        }
        else if (strcmp(p, "help") == 0)
        {
            LOG_I("BLE配置命令帮助:");
            LOG_I("  ble enter         - 进入配置模式");
            LOG_I("  ble exit          - 退出配置模式");
            LOG_I("  ble read          - 读取配置参数");
            LOG_I("  ble write <参数>  - 写入配置参数");
            LOG_I("    参数格式: 重发次数 地址高 地址低 串口波特率 空中速率 信道 发射功率 定点传输");
            LOG_I("    示例: ble write 15 0 0 3 1 0 0 0");
            LOG_I("  ble version       - 读取版本信息");
            LOG_I("  ble reset         - 复位模组");
            LOG_I("  ble status        - 查看命令状态");
        }
        else{
            LOG_I("BLE命令错误，输入 'ble help' 查看帮助");
        }
    }
    else
    {
        LOG_I("command error");
    }
}

/**
 * @brief UART数据处理函数，需要在主循环中周期性调用
 */
void uart_proc(void)
{
    uint32_t len = 0;

    // 处理异步命令
    process_async_commands();
    
    // 处理蓝牙配置状态机
    ble_config_state_machine();

    // LPUART1 接收处理
    if(ringbuffer_data_len(&lpuart1_rx_ringbuf) > 0)
    {
        len = ringbuffer_get(&lpuart1_rx_ringbuf, &lpuart1_parse_cache[lpuart1_parse_cache_len], 
                            UART_BUF_SIZE - lpuart1_parse_cache_len);
        if(len > 0)
        {
            lpuart1_parse_cache_len += len;
            
            // 如果有蓝牙命令在等待响应，处理响应数据
            if (ble_cmd_ctx.state == BLE_CMD_WAIT_RESPONSE) {
                // 检查是否收到了足够的响应数据
                if (lpuart1_parse_cache_len >= ble_cmd_ctx.expected_response_len) {
                    // 复制响应数据
                    memcpy((void*)ble_cmd_ctx.response_data, lpuart1_parse_cache, 
                           lpuart1_parse_cache_len > sizeof(ble_cmd_ctx.response_data) ? 
                           sizeof(ble_cmd_ctx.response_data) : lpuart1_parse_cache_len);
                    ble_cmd_ctx.response_len = lpuart1_parse_cache_len;
                    
                    // 清空解析缓存
                    memset(lpuart1_parse_cache, 0, UART_BUF_SIZE);
                    lpuart1_parse_cache_len = 0;
                } else {
                    // 数据不够，继续等待
                }
            } else {
                // 正常模式，显示接收到的数据
                LOG_RAW("BLE:");
                for(int i = 0; i < lpuart1_parse_cache_len; i++)
                {
                    LOG_RAW("%02X ", lpuart1_parse_cache[i]);
                }
                LOG_RAW("\r\n");
                
                // 清空解析缓存
                memset(lpuart1_parse_cache, 0, UART_BUF_SIZE);
                lpuart1_parse_cache_len = 0;
            }
        }
    }

    // LPUART1 发送处理
    if((lpuart1_tx_complete) && (ringbuffer_data_len(&lpuart1_tx_ringbuf) > 0))
    {
        len = ringbuffer_get(&lpuart1_tx_ringbuf, lpuart1_to_be_send, UART_BUF_SIZE);
        if(len > 0)
        {
            lpuart1_tx_complete = 0;
            HAL_UART_Transmit_DMA(&hlpuart1, lpuart1_to_be_send, len);
        }
    }

    // USART2 接收处理
    if(ringbuffer_data_len(&usart2_rx_ringbuf) > 0)
    {
        len = ringbuffer_get(&usart2_rx_ringbuf, &usart2_parse_cache[usart2_parse_cache_len], 
                            UART_BUF_SIZE - usart2_parse_cache_len);
        if(len > 0)
        {
            usart2_parse_cache_len += len;
            // 检查是否收到完整的命令（以换行符结尾）
            if(strstr((const char *)usart2_parse_cache, "\r") || 
               strstr((const char *)usart2_parse_cache, "\n"))
            {
                // 设置异步命令处理标志
                if (usart2_cmd_state == CMD_STATE_IDLE) {
                    memcpy(pending_cmd_buffer, usart2_parse_cache, usart2_parse_cache_len);
                    usart2_cmd_state = CMD_STATE_PENDING;
                }
                
                // 清空解析缓存
                memset(usart2_parse_cache, 0, UART_BUF_SIZE);
                usart2_parse_cache_len = 0;
            }
        }
    }

    // USART2 发送处理
    if((usart2_tx_complete) && (ringbuffer_data_len(&usart2_tx_ringbuf) > 0))
    {
        len = ringbuffer_get(&usart2_tx_ringbuf, usart2_to_be_send, UART_BUF_SIZE*8);
        if(len > 0)
        {
            usart2_tx_complete = 0;
            HAL_UART_Transmit_DMA(&huart2, usart2_to_be_send, len);
        }
    }
}

/**
 * @brief 向LPUART1发送数据
 * @param data 要发送的数据
 * @param len 数据长度
 */
void send_to_lpuart1(uint8_t *data, uint16_t len)
{
    ringbuffer_put(&lpuart1_tx_ringbuf, data, len);
}

/**
 * @brief 处理异步命令
 */
void process_async_commands(void)
{
    if (usart2_cmd_state == CMD_STATE_PENDING) {
        usart2_cmd_state = CMD_STATE_PROCESSING;
        printf("\r\n");
        debug_func_call(pending_cmd_buffer);
        memset(pending_cmd_buffer, 0, sizeof(pending_cmd_buffer));
        usart2_cmd_state = CMD_STATE_IDLE;
    }
}

/**
 * @brief 蓝牙配置状态机处理
 */
void ble_config_state_machine(void)
{
    uint32_t current_time = HAL_GetTick();
    
    switch (ble_cmd_ctx.state) {
        case BLE_CMD_IDLE:
            // 空闲状态，什么都不做
            break;
            
        case BLE_CMD_ENTER_CONFIG:
            // 设置M0=1, M1=1进入休眠模式(模式3)
            LOG_I("enter cfg");
            ble_cmd_ctx.original_cmd = BLE_CMD_ENTER_CONFIG;
            set_m0_state(1);
            set_m1_state(1);
            ble_cmd_ctx.state = BLE_CMD_WAIT_RESPONSE;
            ble_cmd_ctx.start_time = current_time;
            ble_cmd_ctx.timeout_ms = 2000;
            ble_cmd_ctx.expected_response_len = 0; // 不期望数据响应，只等待AUX状态
            break;
            
        case BLE_CMD_EXIT_CONFIG:
            // 设置M0=0, M1=0进入定频模式(模式0)
            ble_cmd_ctx.original_cmd = BLE_CMD_EXIT_CONFIG;
            set_m0_state(0);
            set_m1_state(0);
            ble_cmd_ctx.state = BLE_CMD_WAIT_RESPONSE;
            ble_cmd_ctx.start_time = current_time;
            ble_cmd_ctx.timeout_ms = 2000;
            ble_cmd_ctx.expected_response_len = 0; // 不期望数据响应，只等待AUX状态
            break;
            
        case BLE_CMD_READ_CONFIG:
            // 发送读取配置命令
            LOG_I("read cfg");
            ble_cmd_ctx.original_cmd = BLE_CMD_READ_CONFIG;
            ble_cmd_ctx.cmd_data[0] = 0xC1;
            ble_cmd_ctx.cmd_data[1] = 0xC1;
            ble_cmd_ctx.cmd_data[2] = 0xC1;
            ble_cmd_ctx.cmd_len = 3;
            {
                uint8_t cmd_len = ble_cmd_ctx.cmd_len;
                ringbuffer_put(&lpuart1_tx_ringbuf, (uint8_t*)ble_cmd_ctx.cmd_data, cmd_len);
            }
            ble_cmd_ctx.state = BLE_CMD_WAIT_RESPONSE;
            ble_cmd_ctx.start_time = current_time;
            ble_cmd_ctx.timeout_ms = 1000;
            ble_cmd_ctx.expected_response_len = 6; // 期望6字节响应
            // 清空接收缓冲区
            ringbuffer_reset(&lpuart1_rx_ringbuf);
            memset(lpuart1_parse_cache, 0, UART_BUF_SIZE);
            lpuart1_parse_cache_len = 0;
            break;
            
        case BLE_CMD_WRITE_CONFIG:
            // 构造并发送写入配置命令
            ble_cmd_ctx.original_cmd = BLE_CMD_WRITE_CONFIG;
            {
                uint8_t save_to_flash = ble_cmd_ctx.save_to_flash;
                ble_cmd_ctx.cmd_data[0] = save_to_flash ? 0xC0 : 0xC2;
                ble_config_to_bytes(ble_cmd_ctx.config_ptr, (uint8_t*)ble_cmd_ctx.cmd_data);
                ble_cmd_ctx.cmd_data[0] = save_to_flash ? 0xC0 : 0xC2; // 重新设置控制字
            }
            ble_cmd_ctx.cmd_len = 6;
            {
                uint8_t cmd_len = ble_cmd_ctx.cmd_len;
                ringbuffer_put(&lpuart1_tx_ringbuf, (uint8_t*)ble_cmd_ctx.cmd_data, cmd_len);
            }
            ble_cmd_ctx.state = BLE_CMD_WAIT_RESPONSE;
            ble_cmd_ctx.start_time = current_time;
            ble_cmd_ctx.timeout_ms = 2000;
            ble_cmd_ctx.expected_response_len = 0; // 写入命令不期望数据响应，只等待AUX状态
            break;
            
        case BLE_CMD_READ_VERSION:
            // 发送读取版本命令
            ble_cmd_ctx.original_cmd = BLE_CMD_READ_VERSION;
            ble_cmd_ctx.cmd_data[0] = 0xC3;
            ble_cmd_ctx.cmd_data[1] = 0xC3;
            ble_cmd_ctx.cmd_data[2] = 0xC3;
            ble_cmd_ctx.cmd_len = 3;
            {
                uint8_t cmd_len = ble_cmd_ctx.cmd_len;
                ringbuffer_put(&lpuart1_tx_ringbuf, (uint8_t*)ble_cmd_ctx.cmd_data, cmd_len);
            }
            ble_cmd_ctx.state = BLE_CMD_WAIT_RESPONSE;
            ble_cmd_ctx.start_time = current_time;
            ble_cmd_ctx.timeout_ms = 1000;
            ble_cmd_ctx.expected_response_len = 3; // 期望至少3字节响应
            // 清空接收缓冲区
            ringbuffer_reset(&lpuart1_rx_ringbuf);
            memset(lpuart1_parse_cache, 0, UART_BUF_SIZE);
            lpuart1_parse_cache_len = 0;
            break;
            
        case BLE_CMD_RESET_MODULE:
            // 发送复位命令
            ble_cmd_ctx.original_cmd = BLE_CMD_RESET_MODULE;
            ble_cmd_ctx.cmd_data[0] = 0xC4;
            ble_cmd_ctx.cmd_data[1] = 0xC4;
            ble_cmd_ctx.cmd_data[2] = 0xC4;
            ble_cmd_ctx.cmd_len = 3;
            {
                uint8_t cmd_len = ble_cmd_ctx.cmd_len;
                ringbuffer_put(&lpuart1_tx_ringbuf, (uint8_t*)ble_cmd_ctx.cmd_data, cmd_len);
            }
            ble_cmd_ctx.state = BLE_CMD_WAIT_RESPONSE;
            ble_cmd_ctx.start_time = current_time;
            ble_cmd_ctx.timeout_ms = 3000;
            ble_cmd_ctx.expected_response_len = 0; // 复位命令不期望数据响应
            break;
            
        case BLE_CMD_WAIT_RESPONSE:
            // 等待响应或超时
            {
                // 为了避免volatile访问顺序警告，先读取volatile变量到临时变量
                uint8_t expected_len = ble_cmd_ctx.expected_response_len;
                uint32_t start_time = ble_cmd_ctx.start_time;
                uint32_t timeout_ms = ble_cmd_ctx.timeout_ms;
                uint8_t response_len = ble_cmd_ctx.response_len;
                
                if (expected_len == 0) {
                    // 不期望数据响应，只等待AUX状态或超时
                    if (get_aux_state() == 1) {
                        // AUX为高电平，命令执行成功
                        LOG_I("AUX high");
                        ble_cmd_ctx.result = BLE_CONFIG_SUCCESS;
                        ble_config_command_complete();
                    } else if (current_time - start_time >= timeout_ms) {
                        // 超时
                        LOG_E("AUX timeout");
                        ble_cmd_ctx.result = BLE_CONFIG_TIMEOUT;
                        ble_cmd_ctx.state = BLE_CMD_TIMEOUT;
                    }
                } else {
                    // 期望数据响应
                    if (response_len >= expected_len) {
                        // 收到足够的响应数据
                        LOG_I("got resp: %d", response_len);
                        ble_cmd_ctx.result = BLE_CONFIG_SUCCESS;
                        ble_config_command_complete();
                    } else if (current_time - start_time >= timeout_ms) {
                        // 超时
                        LOG_E("resp timeout: %d/%d", response_len, expected_len);
                        ble_cmd_ctx.result = BLE_CONFIG_TIMEOUT;
                        ble_cmd_ctx.state = BLE_CMD_TIMEOUT;
                    }
                }
            }
            break;
            
        case BLE_CMD_TIMEOUT:
            // 命令超时，报告错误并重置状态
            LOG_E("timeout");
            ble_cmd_ctx.result = BLE_CONFIG_TIMEOUT;
            ble_config_command_complete();
            break;
            
        default:
            // 未知状态，重置
            ble_cmd_ctx.state = BLE_CMD_IDLE;
            break;
    }
}

/**
 * @brief 蓝牙配置命令完成处理
 */
void ble_config_command_complete(void)
{
    LOG_I("complete: %d", ble_cmd_ctx.original_cmd);
    switch (ble_cmd_ctx.original_cmd) {
        case BLE_CMD_ENTER_CONFIG:
            if (ble_cmd_ctx.result == BLE_CONFIG_SUCCESS) {
                LOG_I("enter OK");
            } else {
                LOG_E("enter fail: %d", ble_cmd_ctx.result);
            }
            break;
            
        case BLE_CMD_EXIT_CONFIG:
            if (ble_cmd_ctx.result == BLE_CONFIG_SUCCESS) {
                LOG_I("exit OK");
            } else {
                LOG_E("exit fail: %d", ble_cmd_ctx.result);
            }
            break;
            
        case BLE_CMD_READ_CONFIG:
            {
                ble_config_result_t result = ble_cmd_ctx.result;
                uint8_t response_len = ble_cmd_ctx.response_len;
                
                LOG_I("rd: %d,%d", result, response_len);
                if (result == BLE_CONFIG_SUCCESS && response_len == 6) {
                    // 先打印接收到的原始数据
                    LOG_I("data:");
                    for (int i = 0; i < ble_cmd_ctx.response_len; i++) {
                        LOG_RAW("%02X ", ble_cmd_ctx.response_data[i]);
                    }
                    LOG_RAW("\r\n");
                    
                    // 检查配置指针
                    if (ble_cmd_ctx.config_ptr == NULL) {
                        LOG_E("ptr NULL");
                        break;
                    }
                    
                    if (ble_cmd_ctx.response_data[0] == 0xC0) {
                        ble_bytes_to_config((const uint8_t*)ble_cmd_ctx.response_data, ble_cmd_ctx.config_ptr);
                        ble_config_t cfg = *ble_cmd_ctx.config_ptr;
                        LOG_I("cfg OK");
                        LOG_I("tx:%d a:%X%02X b:%d", 
                              cfg.retransmit_times,
                              cfg.addr_high, 
                              cfg.addr_low,
                              cfg.uart_baud);
                        LOG_I("air:%d ch:%d pw:%d fix:%d", 
                              cfg.air_rate,
                              cfg.channel,
                              cfg.tx_power,
                              cfg.fixed_trans);
                    } else {
                        LOG_E("bad fmt: %02X", ble_cmd_ctx.response_data[0]);
                    }
                } else {
                    ble_config_result_t result = ble_cmd_ctx.result;
                    uint8_t response_len = ble_cmd_ctx.response_len;
                    LOG_E("rd fail: %d,%d", result, response_len);
                }
            }
            break;
            
        case BLE_CMD_WRITE_CONFIG:
            if (ble_cmd_ctx.result == BLE_CONFIG_SUCCESS) {
                LOG_I("write OK");
            } else {
                LOG_E("write fail: %d", ble_cmd_ctx.result);
            }
            break;
            
        case BLE_CMD_READ_VERSION:
            {
                ble_config_result_t result = ble_cmd_ctx.result;
                uint8_t response_len = ble_cmd_ctx.response_len;
                
                if (result == BLE_CONFIG_SUCCESS && response_len >= 3) {
                    LOG_I("ver:");
                    for (int i = 0; i < ble_cmd_ctx.response_len; i++) {
                        LOG_RAW("%02X ", ble_cmd_ctx.response_data[i]);
                    }
                    LOG_RAW("\r\n");
                    
                    // 如果用户提供了缓冲区，复制数据
                    if (ble_cmd_ctx.version_info_ptr && ble_cmd_ctx.version_len_ptr) {
                        uint8_t copy_len = response_len > *ble_cmd_ctx.version_len_ptr ? 
                                          *ble_cmd_ctx.version_len_ptr : response_len;
                        memcpy(ble_cmd_ctx.version_info_ptr, (const void*)ble_cmd_ctx.response_data, copy_len);
                        *ble_cmd_ctx.version_len_ptr = copy_len;
                    }
                } else {
                    ble_config_result_t result = ble_cmd_ctx.result;
                    uint8_t response_len = ble_cmd_ctx.response_len;
                    LOG_E("ver fail: %d,%d", result, response_len);
                }
            }
            break;
            
        case BLE_CMD_RESET_MODULE:
            if (ble_cmd_ctx.result == BLE_CONFIG_SUCCESS) {
                LOG_I("reset OK");
            } else {
                LOG_E("reset fail: %d", ble_cmd_ctx.result);
            }
            break;
            
        default:
            LOG_E("bad state: %d", ble_cmd_ctx.state);
            break;
    }
    
    // 重置状态机
    ble_cmd_ctx.state = BLE_CMD_IDLE;
    ble_cmd_ctx.original_cmd = BLE_CMD_IDLE;
    ble_cmd_ctx.start_time = 0;
    ble_cmd_ctx.timeout_ms = 0;
    ble_cmd_ctx.cmd_len = 0;
    ble_cmd_ctx.response_len = 0;
    ble_cmd_ctx.expected_response_len = 0;
    ble_cmd_ctx.config_ptr = NULL;
    ble_cmd_ctx.version_info_ptr = NULL;
    ble_cmd_ctx.version_len_ptr = NULL;
    memset((void*)ble_cmd_ctx.cmd_data, 0, sizeof(ble_cmd_ctx.cmd_data));
    memset((void*)ble_cmd_ctx.response_data, 0, sizeof(ble_cmd_ctx.response_data));
}

int putchar(int ch)
{
    ringbuffer_put(&usart2_tx_ringbuf, (uint8_t *)&ch, 1);
    return (ch);
}

int _write (int fd, char *pBuffer, int size)
{
    ringbuffer_put(&usart2_tx_ringbuf, (uint8_t *)pBuffer, size);
	return size;
}

int _read (int fd, char *pBuffer, int size)
{
    return size;
}

/* 蓝牙模组配置实现 */

/**
 * @brief 等待AUX引脚变为高电平
 * @param timeout_ms 超时时间(毫秒)
 * @return 1-成功, 0-超时
 */
uint8_t ble_wait_aux_high(uint32_t timeout_ms)
{
    uint32_t start_time = HAL_GetTick();
    while (HAL_GetTick() - start_time < timeout_ms) {
        if (get_aux_state() == 1) {
            return 1;
        }
        HAL_Delay(1);
    }
    return 0;
}

/**
 * @brief 将配置结构体转换为字节数组
 * @param config 配置结构体
 * @param bytes 输出字节数组(6字节)
 */
void ble_config_to_bytes(const ble_config_t *config, uint8_t *bytes)
{
    bytes[0] = 0xC0;  // 控制字，保存到flash
    bytes[1] = (config->retransmit_times << 4) | (config->addr_high & 0x0F);
    bytes[2] = config->addr_low;
    bytes[3] = (config->parity << 6) | (config->uart_baud << 3) | 
               (config->reserved1 << 2) | (config->air_rate & 0x03);
    bytes[4] = (config->reserved2 << 4) | (config->channel & 0x0F);
    bytes[5] = (config->fixed_trans << 7) | (config->reserved3 << 2) | 
               (config->tx_power & 0x03);
}

/**
 * @brief 将字节数组转换为配置结构体
 * @param bytes 输入字节数组(6字节)
 * @param config 输出配置结构体
 */
void ble_bytes_to_config(const uint8_t *bytes, ble_config_t *config)
{
    config->retransmit_times = (bytes[1] >> 4) & 0x0F;
    config->addr_high = bytes[1] & 0x0F;
    config->addr_low = bytes[2];
    config->parity = (bytes[3] >> 6) & 0x03;
    config->uart_baud = (bytes[3] >> 3) & 0x07;
    config->reserved1 = (bytes[3] >> 2) & 0x01;
    config->air_rate = bytes[3] & 0x03;
    config->reserved2 = (bytes[4] >> 4) & 0x0F;
    config->channel = bytes[4] & 0x0F;
    config->fixed_trans = (bytes[5] >> 7) & 0x01;
    config->reserved3 = (bytes[5] >> 2) & 0x1F;
    config->tx_power = bytes[5] & 0x03;
}

/**
 * @brief 进入蓝牙模组配置模式
 * @return 配置结果
 */
ble_config_result_t ble_enter_config_mode(void)
{
    // 设置M0=1, M1=1进入休眠模式(模式3)
    set_m0_state(1);
    set_m1_state(1);
    
    // 等待AUX变为高电平，表示模式切换完成
    if (!ble_wait_aux_high(2000)) {
        LOG_E("BLE进入配置模式超时");
        return BLE_CONFIG_TIMEOUT;
    }
    
    // 再等待2ms确保模式切换稳定
    HAL_Delay(2);
    
    LOG_I("BLE已进入配置模式");
    return BLE_CONFIG_SUCCESS;
}

/**
 * @brief 退出蓝牙模组配置模式
 * @return 配置结果
 */
ble_config_result_t ble_exit_config_mode(void)
{
    // 设置M0=0, M1=0进入定频模式(模式0)
    set_m0_state(0);
    set_m1_state(0);
    
    // 等待AUX变为高电平，表示模式切换完成
    if (!ble_wait_aux_high(2000)) {
        LOG_E("BLE退出配置模式超时");
        return BLE_CONFIG_TIMEOUT;
    }
    
    LOG_I("BLE已退出配置模式");
    return BLE_CONFIG_SUCCESS;
}

/**
 * @brief 读取蓝牙模组配置参数
 * @param config 输出配置结构体
 * @return 配置结果
 */
ble_config_result_t ble_read_config(ble_config_t *config)
{
    uint8_t cmd[3] = {0xC1, 0xC1, 0xC1};
    uint8_t response[6];
    uint32_t start_time;
    uint16_t response_len = 0;
    
    // 确保在配置模式
    if (get_m0_state() != 1 || get_m1_state() != 1) {
        LOG_E("BLE不在配置模式");
        return BLE_CONFIG_MODE_ERROR;
    }
    
    // 清空接收缓冲区
    ringbuffer_reset(&lpuart1_rx_ringbuf);
    
    // 发送读取配置命令
    ringbuffer_put(&lpuart1_tx_ringbuf, cmd, 3);
    
    // 等待响应
    start_time = HAL_GetTick();
    while (HAL_GetTick() - start_time < 1000) {
        if (ringbuffer_data_len(&lpuart1_rx_ringbuf) >= 6) {
            response_len = ringbuffer_get(&lpuart1_rx_ringbuf, response, 6);
            break;
        }
        HAL_Delay(10);
    }
    
    if (response_len != 6 || response[0] != 0xC0) {
        LOG_E("BLE读取配置失败，响应长度:%d", response_len);
        return BLE_CONFIG_ERROR;
    }
    
    // 解析配置参数
    ble_bytes_to_config(response, config);
    
    LOG_I("BLE配置读取成功");
    return BLE_CONFIG_SUCCESS;
}

/**
 * @brief 写入蓝牙模组配置参数
 * @param config 配置结构体
 * @param save_to_flash 是否保存到flash (1-保存, 0-不保存)
 * @return 配置结果
 */
ble_config_result_t ble_write_config(const ble_config_t *config, uint8_t save_to_flash)
{
    uint8_t cmd[6];
    uint32_t start_time;
    
    // 确保在配置模式
    if (get_m0_state() != 1 || get_m1_state() != 1) {
        LOG_E("BLE不在配置模式");
        return BLE_CONFIG_MODE_ERROR;
    }
    
    // 构造配置命令
    cmd[0] = save_to_flash ? 0xC0 : 0xC2;
    ble_config_to_bytes(config, cmd);
    cmd[0] = save_to_flash ? 0xC0 : 0xC2;  // 重新设置控制字
    
    // 等待AUX为高电平
    if (!ble_wait_aux_high(1000)) {
        LOG_E("BLE写配置前AUX状态错误");
        return BLE_CONFIG_ERROR;
    }
    
    // 发送配置命令
    ringbuffer_put(&lpuart1_tx_ringbuf, cmd, 6);
    
    // 等待配置完成(AUX可能会变低然后变高)
    HAL_Delay(50);  // 等待命令处理
    start_time = HAL_GetTick();
    while (HAL_GetTick() - start_time < 2000) {
        if (get_aux_state() == 1) {
            break;
        }
        HAL_Delay(10);
    }
    
    if (get_aux_state() != 1) {
        LOG_E("BLE写入配置超时");
        return BLE_CONFIG_TIMEOUT;
    }
    
    LOG_I("BLE配置写入成功");
    return BLE_CONFIG_SUCCESS;
}

/**
 * @brief 读取蓝牙模组版本信息
 * @param version_info 输出版本信息缓冲区
 * @param len 输入输出参数，输入缓冲区大小，输出实际长度
 * @return 配置结果
 */
ble_config_result_t ble_read_version(uint8_t *version_info, uint8_t *len)
{
    uint8_t cmd[3] = {0xC3, 0xC3, 0xC3};
    uint32_t start_time;
    uint16_t response_len = 0;
    uint8_t max_len = *len;
    
    // 确保在配置模式
    if (get_m0_state() != 1 || get_m1_state() != 1) {
        LOG_E("BLE不在配置模式");
        return BLE_CONFIG_MODE_ERROR;
    }
    
    // 清空接收缓冲区
    ringbuffer_reset(&lpuart1_rx_ringbuf);
    
    // 发送读取版本命令
    ringbuffer_put(&lpuart1_tx_ringbuf, cmd, 3);
    
    // 等待响应
    start_time = HAL_GetTick();
    while (HAL_GetTick() - start_time < 1000) {
        uint16_t available = ringbuffer_data_len(&lpuart1_rx_ringbuf);
        if (available > 0) {
            response_len = ringbuffer_get(&lpuart1_rx_ringbuf, version_info, 
                                        available > max_len ? max_len : available);
            break;
        }
        HAL_Delay(10);
    }
    
    if (response_len == 0) {
        LOG_E("BLE读取版本信息失败");
        return BLE_CONFIG_ERROR;
    }
    
    *len = response_len;
    LOG_I("BLE版本信息读取成功，长度:%d", response_len);
    return BLE_CONFIG_SUCCESS;
}

/**
 * @brief 复位蓝牙模组
 * @return 配置结果
 */
ble_config_result_t ble_reset_module(void)
{
    uint8_t cmd[3] = {0xC4, 0xC4, 0xC4};
    uint32_t start_time;
    
    // 确保在配置模式
    if (get_m0_state() != 1 || get_m1_state() != 1) {
        LOG_E("BLE不在配置模式");
        return BLE_CONFIG_MODE_ERROR;
    }
    
    // 发送复位命令
    ringbuffer_put(&lpuart1_tx_ringbuf, cmd, 3);
    
    LOG_I("BLE复位命令已发送");
    
    // 等待复位完成(AUX会变低然后变高)
    HAL_Delay(100);  // 等待命令处理
    start_time = HAL_GetTick();
    while (HAL_GetTick() - start_time < 3000) {
        if (get_aux_state() == 1) {
            LOG_I("BLE复位完成");
            return BLE_CONFIG_SUCCESS;
        }
        HAL_Delay(10);
    }
    
    LOG_E("BLE复位超时");
    return BLE_CONFIG_TIMEOUT;
}

/* 异步蓝牙配置接口 */

/**
 * @brief 异步进入蓝牙模组配置模式
 * @return 配置结果
 */
ble_config_result_t ble_enter_config_mode_async(void)
{
    if (ble_cmd_ctx.state != BLE_CMD_IDLE) {
        return BLE_CONFIG_ERROR; // 有其他命令在执行
    }
    
    ble_cmd_ctx.state = BLE_CMD_ENTER_CONFIG;
    ble_cmd_ctx.result = BLE_CONFIG_SUCCESS;
    return BLE_CONFIG_SUCCESS;
}

/**
 * @brief 异步退出蓝牙模组配置模式
 * @return 配置结果
 */
ble_config_result_t ble_exit_config_mode_async(void)
{
    if (ble_cmd_ctx.state != BLE_CMD_IDLE) {
        return BLE_CONFIG_ERROR; // 有其他命令在执行
    }
    
    ble_cmd_ctx.state = BLE_CMD_EXIT_CONFIG;
    ble_cmd_ctx.result = BLE_CONFIG_SUCCESS;
    return BLE_CONFIG_SUCCESS;
}

/**
 * @brief 异步读取蓝牙模组配置参数
 * @return 配置结果
 */
ble_config_result_t ble_read_config_async(void)
{
    static ble_config_t static_config; // 使用静态变量存储配置
    
    if (ble_cmd_ctx.state != BLE_CMD_IDLE) {
        return BLE_CONFIG_ERROR; // 有其他命令在执行
    }
    
    // 确保在配置模式
    if (get_m0_state() != 1 || get_m1_state() != 1) {
        LOG_E("BLE不在配置模式");
        return BLE_CONFIG_MODE_ERROR;
    }
    
    ble_cmd_ctx.state = BLE_CMD_READ_CONFIG;
    ble_cmd_ctx.config_ptr = &static_config;
    ble_cmd_ctx.result = BLE_CONFIG_SUCCESS;
    return BLE_CONFIG_SUCCESS;
}

/**
 * @brief 异步写入蓝牙模组配置参数
 * @param config 配置结构体
 * @param save_to_flash 是否保存到flash (1-保存, 0-不保存)
 * @return 配置结果
 */
ble_config_result_t ble_write_config_async(const ble_config_t *config, uint8_t save_to_flash)
{
    if (ble_cmd_ctx.state != BLE_CMD_IDLE) {
        return BLE_CONFIG_ERROR; // 有其他命令在执行
    }
    
    // 确保在配置模式
    if (get_m0_state() != 1 || get_m1_state() != 1) {
        LOG_E("BLE不在配置模式");
        return BLE_CONFIG_MODE_ERROR;
    }
    
    // 等待AUX为高电平
    if (get_aux_state() != 1) {
        LOG_E("BLE写配置前AUX状态错误");
        return BLE_CONFIG_ERROR;
    }
    
    ble_cmd_ctx.state = BLE_CMD_WRITE_CONFIG;
    ble_cmd_ctx.config_ptr = (ble_config_t*)config;
    ble_cmd_ctx.save_to_flash = save_to_flash;
    ble_cmd_ctx.result = BLE_CONFIG_SUCCESS;
    return BLE_CONFIG_SUCCESS;
}

/**
 * @brief 异步读取蓝牙模组版本信息
 * @return 配置结果
 */
ble_config_result_t ble_read_version_async(void)
{
    if (ble_cmd_ctx.state != BLE_CMD_IDLE) {
        return BLE_CONFIG_ERROR; // 有其他命令在执行
    }
    
    // 确保在配置模式
    if (get_m0_state() != 1 || get_m1_state() != 1) {
        LOG_E("BLE不在配置模式");
        return BLE_CONFIG_MODE_ERROR;
    }
    
    ble_cmd_ctx.state = BLE_CMD_READ_VERSION;
    ble_cmd_ctx.result = BLE_CONFIG_SUCCESS;
    return BLE_CONFIG_SUCCESS;
}

/**
 * @brief 异步复位蓝牙模组
 * @return 配置结果
 */
ble_config_result_t ble_reset_module_async(void)
{
    if (ble_cmd_ctx.state != BLE_CMD_IDLE) {
        return BLE_CONFIG_ERROR; // 有其他命令在执行
    }
    
    // 确保在配置模式
    if (get_m0_state() != 1 || get_m1_state() != 1) {
        LOG_E("BLE不在配置模式");
        return BLE_CONFIG_MODE_ERROR;
    }
    
    ble_cmd_ctx.state = BLE_CMD_RESET_MODULE;
    ble_cmd_ctx.result = BLE_CONFIG_SUCCESS;
    return BLE_CONFIG_SUCCESS;
}

/**
 * @brief 检查蓝牙配置命令是否完成
 * @return 1-命令完成, 0-命令执行中
 */
uint8_t ble_config_is_command_complete(void)
{
    return (ble_cmd_ctx.state == BLE_CMD_IDLE) ? 1 : 0;
}

/**
 * @brief 获取蓝牙配置命令结果
 * @return 配置结果
 */
ble_config_result_t ble_config_get_command_result(void)
{
    return ble_cmd_ctx.result;
}

/* USER CODE END 1 */
