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
extern uint8_t usart2_tx_dma_buf[UART_BUF_SIZE*8];

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

// 蓝牙模组配置相关定义和函数
typedef struct {
    uint8_t retransmit_times : 4;  // 重发次数(0-15)
    uint8_t addr_high : 4;         // 地址高4位
    uint8_t addr_low;              // 地址低8位
    uint8_t parity : 2;            // 校验位: 00=8N1, 01=8O1, 10=8E1, 11=8N1
    uint8_t uart_baud : 3;         // 串口波特率
    uint8_t reserved1 : 1;         // 保留位
    uint8_t air_rate : 2;          // 空中速率: 00=250k, 01=1M, 10=2M, 11=2M
    uint8_t reserved2 : 4;         // 保留位
    uint8_t channel : 4;           // 通信信道(0-11)
    uint8_t fixed_trans : 1;       // 定点传输使能: 0=透明传输, 1=定点传输
    uint8_t reserved3 : 5;         // 保留位
    uint8_t tx_power : 2;          // 发射功率: 00=11dBm, 01=7dBm, 10=3dBm, 11=-1dBm
} __attribute__((packed)) ble_config_t;

// 蓝牙模组配置函数
typedef enum {
    BLE_CONFIG_SUCCESS = 0,
    BLE_CONFIG_TIMEOUT,
    BLE_CONFIG_ERROR,
    BLE_CONFIG_MODE_ERROR
} ble_config_result_t;

// 蓝牙模组配置接口
ble_config_result_t ble_enter_config_mode(void);
ble_config_result_t ble_exit_config_mode(void);
ble_config_result_t ble_read_config(ble_config_t *config);
ble_config_result_t ble_write_config(const ble_config_t *config, uint8_t save_to_flash);
ble_config_result_t ble_read_version(uint8_t *version_info, uint8_t *len);
ble_config_result_t ble_reset_module(void);

// 辅助函数
uint8_t ble_wait_aux_high(uint32_t timeout_ms);
void ble_config_to_bytes(const ble_config_t *config, uint8_t *bytes);
void ble_bytes_to_config(const uint8_t *bytes, ble_config_t *config);

// 异步蓝牙配置接口
ble_config_result_t ble_enter_config_mode_async(void);
ble_config_result_t ble_exit_config_mode_async(void);
ble_config_result_t ble_read_config_async(void);
ble_config_result_t ble_write_config_async(const ble_config_t *config, uint8_t save_to_flash);
ble_config_result_t ble_read_version_async(void);
ble_config_result_t ble_reset_module_async(void);
uint8_t ble_config_is_command_complete(void);
ble_config_result_t ble_config_get_command_result(void);

// 内部处理函数
void process_async_commands(void);
void ble_config_state_machine(void);
void ble_config_command_complete(void);
/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __USART_H__ */

