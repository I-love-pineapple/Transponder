/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    usart.h
  * @brief   This file contains all the function prototypes for
  *          the usart.c file
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
#ifndef __USART_H__
#define __USART_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */
#include "ringbuffer.h"
/* USER CODE END Includes */

extern UART_HandleTypeDef hlpuart1;

extern UART_HandleTypeDef huart2;

/* USER CODE BEGIN Private defines */
#define UART_BUF_SIZE    128    // UART缓冲区大小

// UART DMA缓冲区声明
extern uint8_t lpuart1_rx_dma_buf[UART_BUF_SIZE];
extern uint8_t lpuart1_tx_dma_buf[UART_BUF_SIZE];
extern uint8_t usart2_rx_dma_buf[UART_BUF_SIZE];
extern uint8_t usart2_tx_dma_buf[UART_BUF_SIZE];

// 环形缓冲区声明
extern struct ringbuffer lpuart1_rx_ringbuf;
extern struct ringbuffer lpuart1_tx_ringbuf;
extern struct ringbuffer usart2_rx_ringbuf;
extern struct ringbuffer usart2_tx_ringbuf;

// 传输完成标志
extern volatile uint8_t lpuart1_tx_complete;
extern volatile uint8_t usart2_tx_complete;
/* USER CODE END Private defines */

void MX_LPUART1_UART_Init(void);
void MX_USART2_UART_Init(void);

/* USER CODE BEGIN Prototypes */
// UART初始化和反初始化函数
void lpuart1_init(void);
void usart2_init(void);
void lpuart1_deinit(void);
void usart2_deinit(void);

// DMA启动函数
void lpuart1_start_dma(void);
void usart2_start_dma(void);

// UART处理函数
void uart_proc(void);

// 数据发送函数
void send_to_lpuart1(uint8_t *data, uint16_t len);
void send_to_usart2(uint8_t *data, uint16_t len);
/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __USART_H__ */

