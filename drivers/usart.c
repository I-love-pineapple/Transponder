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

// DMA缓冲区定义
uint8_t lpuart1_rx_dma_buf[UART_BUF_SIZE];
uint8_t lpuart1_tx_dma_buf[UART_BUF_SIZE];
uint8_t usart2_rx_dma_buf[UART_BUF_SIZE];
uint8_t usart2_tx_dma_buf[UART_BUF_SIZE];

// 环形缓冲区用的缓冲区
uint8_t lpuart1_rx_buf[UART_BUF_SIZE];
uint8_t lpuart1_tx_buf[UART_BUF_SIZE * 2];  // 发送缓冲区可以大一些
uint8_t usart2_rx_buf[UART_BUF_SIZE];
uint8_t usart2_tx_buf[UART_BUF_SIZE * 2];

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
  hlpuart1.Init.BaudRate = 115200;
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
    ringbuffer_init(&usart2_tx_ringbuf, usart2_tx_buf, UART_BUF_SIZE);
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
uint8_t usart2_to_be_send[UART_BUF_SIZE] = {0};

void debug_func_call(uint8_t *data)
{
  char *delim=" \r";
  char *p;

  p = strtok((char *)data, delim);

  //设置
  if(strcmp(p, "set") == 0)
  {
    p = strtok(NULL, delim);
    if(strcmp(p, "join") == 0)
    {
        printf("join\r\n");
        
    }
  }
  else if(strcmp(p, "reboot") == 0)
  {
    HAL_NVIC_SystemReset();
  }
  else
  {
    printf("command error\r\n");
  }
}

/**
 * @brief UART数据处理函数，需要在主循环中周期性调用
 */
void uart_proc(void)
{
    uint32_t len = 0;

    // LPUART1 接收处理
    if(ringbuffer_data_len(&lpuart1_rx_ringbuf) > 0)
    {
        len = ringbuffer_get(&lpuart1_rx_ringbuf, &lpuart1_parse_cache[lpuart1_parse_cache_len], 
                            UART_BUF_SIZE - lpuart1_parse_cache_len);
        if(len > 0)
        {
            lpuart1_parse_cache_len += len;
            // 这里可以添加具体的数据解析逻辑
            // 例如检查是否收到完整的命令（以换行符结尾）
            if(strstr((const char *)lpuart1_parse_cache, "\r") || 
               strstr((const char *)lpuart1_parse_cache, "\n"))
            {
                // 处理接收到的命令
                // 可以在这里添加具体的命令处理逻辑
                
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
            // 这里可以添加具体的数据解析逻辑
            if(strstr((const char *)usart2_parse_cache, "\r") || 
               strstr((const char *)usart2_parse_cache, "\n"))
            {
                // 处理接收到的命令
                printf("\r\n");
                debug_func_call(usart2_parse_cache);

                
                // 清空解析缓存
                memset(usart2_parse_cache, 0, UART_BUF_SIZE);
                usart2_parse_cache_len = 0;
            }
        }
    }

    // USART2 发送处理
    if((usart2_tx_complete) && (ringbuffer_data_len(&usart2_tx_ringbuf) > 0))
    {
        len = ringbuffer_get(&usart2_tx_ringbuf, usart2_to_be_send, UART_BUF_SIZE);
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

/* USER CODE END 1 */
