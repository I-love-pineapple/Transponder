#ifndef E35_CONFIG_H
#define E35_CONFIG_H

#include <stdint.h>
#include <stdbool.h>

// 配置结果枚举
typedef enum {
    E35_OK = 0,
    E35_ERROR,
    E35_TIMEOUT,
    E35_INVALID_PARAM
} E35_Result_t;

// 工作模式枚举
typedef enum {
    E35_MODE_TRANSMISSION = 0,  // 传输模式
    E35_MODE_CONFIG = 1         // 配置模式
} E35_Mode_t;

// 串口波特率枚举
typedef enum {
    E35_BAUD_9600 = 0,
    E35_BAUD_19200 = 1,
    E35_BAUD_38400 = 2,
    E35_BAUD_57600 = 3,
    E35_BAUD_115200 = 4
} E35_Baud_t;

// 串口校验位枚举
typedef enum {
    E35_PARITY_NONE = 0,
    E35_PARITY_EVEN = 1,
    E35_PARITY_ODD = 2
} E35_Parity_t;

// 空中速率枚举
typedef enum {
    E35_RATE_250K = 0,
    E35_RATE_500K = 1,
    E35_RATE_1M = 2,
    E35_RATE_2M = 3
} E35_Rate_t;

// 传输方式枚举
typedef enum {
    E35_TRANS_TRANSPARENT = 0,  // 透明传输
    E35_TRANS_FIXED = 1         // 定点传输
} E35_Trans_t;

// 模块配置结构体(紧凑设计以节省内存)
typedef struct {
    uint8_t addr_high;      // 地址高字节 0-255
    uint8_t addr_low;       // 地址低字节 0-255
    uint8_t channel;        // 信道 0-80
    uint8_t baud    : 3;    // 波特率 (E35_Baud_t)
    uint8_t parity  : 2;    // 校验位 (E35_Parity_t)
    uint8_t rate    : 2;    // 空中速率 (E35_Rate_t)
    uint8_t trans   : 1;    // 传输方式 (E35_Trans_t)
    uint8_t power;          // 发射功率 0-26
    uint8_t packet_size;    // 分包长度 23-48
    uint8_t encrypt : 1;    // 加密开关
    uint8_t drssi   : 1;    // RSSI开关
    uint8_t lpwr    : 1;    // 低功耗开关
    uint8_t reserved: 5;    // 保留位
    uint8_t key0;           // 密钥0
    uint8_t key1;           // 密钥1
} E35_Config_t;

// 用户需要实现的硬件接口函数
typedef struct {
    void (*uart_send)(const uint8_t* data, uint16_t len);
    uint16_t (*uart_receive)(uint8_t* buffer, uint16_t max_len, uint32_t timeout_ms);
    void (*delay_ms)(uint32_t ms);
    void (*mode_pin_set)(uint8_t level);  // 可选：如果有硬件模式切换引脚
} E35_HAL_t;

// 函数声明
E35_Result_t E35_Init(const E35_HAL_t* hal);
E35_Result_t E35_SwitchMode(E35_Mode_t mode);
E35_Result_t E35_ReadConfig(E35_Config_t* config);
E35_Result_t E35_WriteConfig(const E35_Config_t* config);
E35_Result_t E35_Reset(void);
E35_Result_t E35_SaveAndRestart(void);
E35_Result_t E35_GetVersion(char* version, uint8_t max_len);
E35_Result_t E35_GetUID(uint8_t* uid, uint8_t max_len);

// 快速配置函数
E35_Result_t E35_QuickConfig(uint8_t addr_h, uint8_t addr_l, uint8_t channel, 
                             E35_Rate_t rate, uint8_t power);

#endif // E35_CONFIG_H