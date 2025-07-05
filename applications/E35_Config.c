#include "E35_Config.h"
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

// 配置常量
#define E35_CMD_TIMEOUT_MS      1000
#define E35_RX_BUFFER_SIZE      64      // 小缓冲区以节省RAM
#define E35_CMD_BUFFER_SIZE     32
#define E35_MODE_SWITCH_DELAY   100

// 全局变量(节省栈空间)
static const E35_HAL_t* g_hal = NULL;
static uint8_t g_rx_buffer[E35_RX_BUFFER_SIZE];
static char g_cmd_buffer[E35_CMD_BUFFER_SIZE];

// AT命令字符串(存储在ROM中)
static const char* const AT_COMMANDS[] = {
    "AT+MODE=?\r\n",
    "AT+MODE=%d\r\n",
    "AT+ADDR=?\r\n", 
    "AT+ADDR=%d,%d\r\n",
    "AT+UART=?\r\n",
    "AT+UART=%d,0,%d\r\n",
    "AT+RATE=?\r\n",
    "AT+RATE=%d\r\n",
    "AT+POWER=?\r\n",
    "AT+POWER=%d\r\n",
    "AT+CHANNEL=?\r\n",
    "AT+CHANNEL=%d\r\n",
    "AT+TRANS=?\r\n",
    "AT+TRANS=%d\r\n",
    "AT+PACKET=?\r\n",
    "AT+PACKET=%d\r\n",
    "AT+DRSSI=?\r\n",
    "AT+DRSSI=%d\r\n",
    "AT+ENCRYPT=?\r\n",
    "AT+ENCRYPT=%d\r\n",
    "AT+KEY=?\r\n",
    "AT+KEY=%d,%d\r\n",
    "AT+LPWR=?\r\n",
    "AT+LPWR=%d\r\n",
    "AT+RESET\r\n",
    "AT+DEFAULT\r\n",
    "AT+FWCODE=?\r\n",
    "AT+UID=?\r\n"
};

// 命令索引枚举
typedef enum {
    CMD_MODE_QUERY = 0,
    CMD_MODE_SET,
    CMD_ADDR_QUERY,
    CMD_ADDR_SET,
    CMD_UART_QUERY,
    CMD_UART_SET,
    CMD_RATE_QUERY,
    CMD_RATE_SET,
    CMD_POWER_QUERY,
    CMD_POWER_SET,
    CMD_CHANNEL_QUERY,
    CMD_CHANNEL_SET,
    CMD_TRANS_QUERY,
    CMD_TRANS_SET,
    CMD_PACKET_QUERY,
    CMD_PACKET_SET,
    CMD_DRSSI_QUERY,
    CMD_DRSSI_SET,
    CMD_ENCRYPT_QUERY,
    CMD_ENCRYPT_SET,
    CMD_KEY_QUERY,
    CMD_KEY_SET,
    CMD_LPWR_QUERY,
    CMD_LPWR_SET,
    CMD_RESET,
    CMD_DEFAULT,
    CMD_VERSION,
    CMD_UID
} AT_Cmd_Index_t;

// 内部函数声明
static E35_Result_t send_at_command(AT_Cmd_Index_t cmd_idx, ...);
static bool parse_response_value(const char* response, const char* prefix, int* value);
static bool parse_response_values(const char* response, const char* prefix, int* val1, int* val2);

/**
 * @brief 初始化E35模块
 */
E35_Result_t E35_Init(const E35_HAL_t* hal) {
    if (!hal || !hal->uart_send || !hal->uart_receive || !hal->delay_ms) {
        return E35_INVALID_PARAM;
    }
    
    g_hal = hal;
    g_hal->delay_ms(500); // 等待模块启动
    return E35_OK;
}

/**
 * @brief 切换工作模式
 */
E35_Result_t E35_SwitchMode(E35_Mode_t mode) {
    if (!g_hal) return E35_ERROR;
    
    E35_Result_t result = send_at_command(CMD_MODE_SET, (int)mode);
    if (result == E35_OK) {
        g_hal->delay_ms(E35_MODE_SWITCH_DELAY);
    }
    return result;
}

/**
 * @brief 读取当前配置
 */
E35_Result_t E35_ReadConfig(E35_Config_t* config) {
    if (!config || !g_hal) return E35_INVALID_PARAM;
    
    // 切换到配置模式
    if (E35_SwitchMode(E35_MODE_CONFIG) != E35_OK) {
        return E35_ERROR;
    }
    
    int val1, val2;
    
    // 读取地址
    if (send_at_command(CMD_ADDR_QUERY) != E35_OK) return E35_ERROR;
    if (parse_response_values((char*)g_rx_buffer, "AT+ADDR=", &val1, &val2)) {
        config->addr_high = (uint8_t)val1;
        config->addr_low = (uint8_t)val2;
    }
    
    // 读取串口参数
    if (send_at_command(CMD_UART_QUERY) != E35_OK) return E35_ERROR;
    if (parse_response_values((char*)g_rx_buffer, "AT+UART=", &val1, &val2)) {
        config->baud = (uint8_t)val1;
        config->parity = (uint8_t)val2; // 跳过停止位
    }
    
    // 读取空中速率
    if (send_at_command(CMD_RATE_QUERY) != E35_OK) return E35_ERROR;
    if (parse_response_value((char*)g_rx_buffer, "AT+RATE=", &val1)) {
        config->rate = (uint8_t)val1;
    }
    
    // 读取发射功率
    if (send_at_command(CMD_POWER_QUERY) != E35_OK) return E35_ERROR;
    if (parse_response_value((char*)g_rx_buffer, "AT+POWER=", &val1)) {
        config->power = (uint8_t)val1;
    }
    
    // 读取信道
    if (send_at_command(CMD_CHANNEL_QUERY) != E35_OK) return E35_ERROR;
    if (parse_response_value((char*)g_rx_buffer, "AT+CHANNEL=", &val1)) {
        config->channel = (uint8_t)val1;
    }
    
    // 读取传输方式
    if (send_at_command(CMD_TRANS_QUERY) != E35_OK) return E35_ERROR;
    if (parse_response_value((char*)g_rx_buffer, "AT+TRANS=", &val1)) {
        config->trans = (uint8_t)val1;
    }
    
    // 读取分包大小
    if (send_at_command(CMD_PACKET_QUERY) != E35_OK) return E35_ERROR;
    if (parse_response_value((char*)g_rx_buffer, "AT+PACKET=", &val1)) {
        config->packet_size = (uint8_t)val1;
    }
    
    return E35_OK;
}

/**
 * @brief 写入配置
 */
E35_Result_t E35_WriteConfig(const E35_Config_t* config) {
    if (!config || !g_hal) return E35_INVALID_PARAM;
    
    // 切换到配置模式
    if (E35_SwitchMode(E35_MODE_CONFIG) != E35_OK) {
        return E35_ERROR;
    }
    
    // 配置地址
    if (send_at_command(CMD_ADDR_SET, config->addr_high, config->addr_low) != E35_OK) {
        return E35_ERROR;
    }
    
    // 配置串口参数
    if (send_at_command(CMD_UART_SET, config->baud, config->parity) != E35_OK) {
        return E35_ERROR;
    }
    
    // 配置空中速率
    if (send_at_command(CMD_RATE_SET, config->rate) != E35_OK) {
        return E35_ERROR;
    }
    
    // 配置发射功率
    if (send_at_command(CMD_POWER_SET, config->power) != E35_OK) {
        return E35_ERROR;
    }
    
    // 配置信道
    if (send_at_command(CMD_CHANNEL_SET, config->channel) != E35_OK) {
        return E35_ERROR;
    }
    
    // 配置传输方式
    if (send_at_command(CMD_TRANS_SET, config->trans) != E35_OK) {
        return E35_ERROR;
    }
    
    // 配置分包大小
    if (send_at_command(CMD_PACKET_SET, config->packet_size) != E35_OK) {
        return E35_ERROR;
    }
    
    // 配置RSSI
    if (send_at_command(CMD_DRSSI_SET, config->drssi) != E35_OK) {
        return E35_ERROR;
    }
    
    // 配置加密
    if (send_at_command(CMD_ENCRYPT_SET, config->encrypt) != E35_OK) {
        return E35_ERROR;
    }
    
    // 如果启用加密，配置密钥
    if (config->encrypt) {
        if (send_at_command(CMD_KEY_SET, config->key0, config->key1) != E35_OK) {
            return E35_ERROR;
        }
    }
    
    // 配置低功耗
    if (send_at_command(CMD_LPWR_SET, config->lpwr) != E35_OK) {
        return E35_ERROR;
    }
    
    return E35_OK;
}

/**
 * @brief 复位模块
 */
E35_Result_t E35_Reset(void) {
    if (!g_hal) return E35_ERROR;
    
    return send_at_command(CMD_RESET);
}

/**
 * @brief 保存配置并重启
 */
E35_Result_t E35_SaveAndRestart(void) {
    if (!g_hal) return E35_ERROR;
    
    E35_Result_t result = send_at_command(CMD_RESET);
    if (result == E35_OK) {
        g_hal->delay_ms(500); // 等待重启
    }
    return result;
}

/**
 * @brief 获取固件版本
 */
E35_Result_t E35_GetVersion(char* version, uint8_t max_len) {
    if (!version || !g_hal || max_len == 0) return E35_INVALID_PARAM;
    
    if (send_at_command(CMD_VERSION) != E35_OK) {
        return E35_ERROR;
    }
    
    // 简单复制响应(节省代码空间)
    uint8_t copy_len = (max_len - 1 < E35_RX_BUFFER_SIZE - 1) ? max_len - 1 : E35_RX_BUFFER_SIZE - 1;
    memcpy(version, g_rx_buffer, copy_len);
    version[copy_len] = '\0';
    
    return E35_OK;
}

/**
 * @brief 获取UID
 */
E35_Result_t E35_GetUID(uint8_t* uid, uint8_t max_len) {
    if (!uid || !g_hal || max_len == 0) return E35_INVALID_PARAM;
    
    if (send_at_command(CMD_UID) != E35_OK) {
        return E35_ERROR;
    }
    
    // 简单复制响应
    uint8_t copy_len = (max_len < E35_RX_BUFFER_SIZE) ? max_len : E35_RX_BUFFER_SIZE;
    memcpy(uid, g_rx_buffer, copy_len);
    
    return E35_OK;
}

/**
 * @brief 快速配置函数
 */
E35_Result_t E35_QuickConfig(uint8_t addr_h, uint8_t addr_l, uint8_t channel, 
                             E35_Rate_t rate, uint8_t power) {
    E35_Config_t config = {0};
    
    // 设置基本参数
    config.addr_high = addr_h;
    config.addr_low = addr_l;
    config.channel = channel;
    config.rate = rate;
    config.power = power;
    
    // 设置默认值
    config.baud = E35_BAUD_9600;
    config.parity = E35_PARITY_NONE;
    config.trans = E35_TRANS_TRANSPARENT;
    config.packet_size = 23;
    config.encrypt = 0;
    config.drssi = 0;
    config.lpwr = 0;
    
    return E35_WriteConfig(&config);
}

// ================== 内部函数实现 ==================

/**
 * @brief 发送AT命令并等待响应
 */
static E35_Result_t send_at_command(AT_Cmd_Index_t cmd_idx, ...) {
    if (!g_hal || cmd_idx >= sizeof(AT_COMMANDS)/sizeof(AT_COMMANDS[0])) {
        return E35_INVALID_PARAM;
    }
    
    // 构造命令
    va_list args;
    va_start(args, cmd_idx);
    
    int len;
    if (cmd_idx == CMD_MODE_SET || cmd_idx == CMD_RATE_SET || 
        cmd_idx == CMD_POWER_SET || cmd_idx == CMD_CHANNEL_SET ||
        cmd_idx == CMD_TRANS_SET || cmd_idx == CMD_PACKET_SET ||
        cmd_idx == CMD_DRSSI_SET || cmd_idx == CMD_ENCRYPT_SET ||
        cmd_idx == CMD_LPWR_SET) {
        int val = va_arg(args, int);
        len = snprintf(g_cmd_buffer, E35_CMD_BUFFER_SIZE, AT_COMMANDS[cmd_idx], val);
    } else if (cmd_idx == CMD_ADDR_SET || cmd_idx == CMD_UART_SET || cmd_idx == CMD_KEY_SET) {
        int val1 = va_arg(args, int);
        int val2 = va_arg(args, int);
        len = snprintf(g_cmd_buffer, E35_CMD_BUFFER_SIZE, AT_COMMANDS[cmd_idx], val1, val2);
    } else {
        len = snprintf(g_cmd_buffer, E35_CMD_BUFFER_SIZE, "%s", AT_COMMANDS[cmd_idx]);
    }
    
    va_end(args);
    
    if (len >= E35_CMD_BUFFER_SIZE) return E35_ERROR;
    
    // 发送命令
    g_hal->uart_send((uint8_t*)g_cmd_buffer, len);
    
    // 等待响应
    uint16_t rx_len = g_hal->uart_receive(g_rx_buffer, E35_RX_BUFFER_SIZE - 1, E35_CMD_TIMEOUT_MS);
    if (rx_len == 0) return E35_TIMEOUT;
    
    g_rx_buffer[rx_len] = '\0'; // 确保字符串结束
    
    // 检查是否包含OK或ERROR
    if (strstr((char*)g_rx_buffer, "=OK") != NULL) {
        return E35_OK;
    } else if (strstr((char*)g_rx_buffer, "=ERR") != NULL) {
        return E35_ERROR;
    } else if (strstr((char*)g_rx_buffer, "AT+") != NULL) {
        return E35_OK; // 查询命令返回值
    }
    
    return E35_ERROR;
}

/**
 * @brief 解析响应中的单个数值
 */
static bool parse_response_value(const char* response, const char* prefix, int* value) {
    char* pos = strstr(response, prefix);
    if (!pos) return false;
    
    pos += strlen(prefix);
    *value = 0;
    
    // 简单的数字解析
    while (*pos >= '0' && *pos <= '9') {
        *value = *value * 10 + (*pos - '0');
        pos++;
    }
    
    return true;
}

/**
 * @brief 解析响应中的两个数值
 */
static bool parse_response_values(const char* response, const char* prefix, int* val1, int* val2) {
    char* pos = strstr(response, prefix);
    if (!pos) return false;
    
    pos += strlen(prefix);
    
    // 解析第一个值
    *val1 = 0;
    while (*pos >= '0' && *pos <= '9') {
        *val1 = *val1 * 10 + (*pos - '0');
        pos++;
    }
    
    // 跳过逗号
    if (*pos != ',') return false;
    pos++;
    
    // 解析第二个值
    *val2 = 0;
    while (*pos >= '0' && *pos <= '9') {
        *val2 = *val2 * 10 + (*pos - '0');
        pos++;
    }
    
    return true;
}