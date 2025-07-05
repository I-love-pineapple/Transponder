/**
 * @file debug_log.h
 * @brief 调试日志宏定义头文件
 * @details 提供分级调试日志输出功能，支持颜色显示和格式化输出
 * 
 * 使用方法：
 * 1. 在C/C++文件中定义调试标签和级别：
 *    #define DBG_TAG "MOD_TAG"
 *    #define DBG_LVL DBG_INFO
 * 2. 包含此头文件：
 *    #include "debug_log.h"
 * 3. 使用日志宏：
 *    LOG_D("这是调试日志");
 *    LOG_E("这是错误日志");
 * 
 * @author ODM Team
 * @date 2024
 * @version 1.0
 */

#ifndef DEBUG_LOG_H__
#define DEBUG_LOG_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

/** @defgroup DEBUG_LOG_LEVELS 调试日志级别定义
 * @brief 定义不同级别的调试日志
 * @{
 */
#define DBG_ERROR           0  ///< 错误级别
#define DBG_WARNING         1  ///< 警告级别
#define DBG_INFO            2  ///< 信息级别
#define DBG_LOG             3  ///< 调试级别
/** @} */

/** @brief 强制启用调试日志 */
#define DBG_ENABLE

/** @brief 启用彩色输出 */
#define DBG_COLOR

/**
 * @brief 设置调试标签
 * @details 如果未定义DBG_TAG，则使用默认标签
 */
#ifdef DBG_TAG
#ifndef DBG_SECTION_NAME
#define DBG_SECTION_NAME    DBG_TAG
#endif
#else
#ifndef DBG_SECTION_NAME
#define DBG_SECTION_NAME    "DBG"
#endif
#endif /* DBG_TAG */

/**
 * @brief 设置调试级别
 * @details 如果未定义DBG_LVL，则使用默认警告级别
 */
#ifdef DBG_LVL
#ifndef DBG_LEVEL
#define DBG_LEVEL         DBG_LVL
#endif
#else
#ifndef DBG_LEVEL
#define DBG_LEVEL         DBG_WARNING
#endif
#endif /* DBG_LVL */

#ifdef DBG_ENABLE

/** @defgroup COLOR_CODES 终端颜色代码
 * @brief 定义终端输出的颜色代码
 * @{
 */
#ifdef DBG_COLOR
#define _DBG_COLOR(n)        printf("\033["#n"m")  ///< 设置颜色
#define _DBG_LOG_HDR(lvl_name, color_n)                    \
    printf("\033["#color_n"m[" lvl_name "/" DBG_SECTION_NAME "] ")  ///< 带颜色的日志头
#define _DBG_LOG_X_END                                     \
    printf("\033[0m\r\n")  ///< 重置颜色并换行
#else
#define _DBG_COLOR(n)
#define _DBG_LOG_HDR(lvl_name, color_n)                    \
    printf("[" lvl_name "/" DBG_SECTION_NAME "] ")  ///< 普通日志头
#define _DBG_LOG_X_END                                     \
    printf("\r\n")  ///< 换行
#endif /* DBG_COLOR */
/** @} */

/**
 * @brief 调试日志输出宏
 * @param level 日志级别
 * @param fmt 格式化字符串
 * @param ... 可变参数
 */
#define dbg_log(level, fmt, ...)                            \
    if ((level) <= DBG_LEVEL)                               \
    {                                                       \
        switch(level)                                       \
        {                                                   \
            case DBG_ERROR:   _DBG_LOG_HDR("E", 31); break; \
            case DBG_WARNING: _DBG_LOG_HDR("W", 33); break; \
            case DBG_INFO:    _DBG_LOG_HDR("I", 32); break; \
            case DBG_LOG:     _DBG_LOG_HDR("D", 0); break;  \
            default: break;                                 \
        }                                                   \
        printf(fmt, ##__VA_ARGS__);                         \
        _DBG_LOG_X_END;                                     \
    }

/**
 * @brief 显示当前位置信息
 * @details 输出当前函数名和行号
 */
#define dbg_here                                            \
    if ((DBG_LEVEL) <= DBG_LOG){                            \
        printf(DBG_SECTION_NAME " Here %s:%d\r\n",         \
            __FUNCTION__, __LINE__);                        \
    }

/**
 * @brief 格式化日志输出
 * @param lvl 级别标识
 * @param color_n 颜色代码
 * @param fmt 格式化字符串
 * @param ... 可变参数
 */
#define dbg_log_line(lvl, color_n, fmt, ...)               \
    do                                                      \
    {                                                       \
        _DBG_LOG_HDR(lvl, color_n);                         \
        printf(fmt, ##__VA_ARGS__);                         \
        _DBG_LOG_X_END;                                     \
    }                                                       \
    while (0)

/**
 * @brief 原始输出（不带格式）
 * @param ... 可变参数
 */
#define dbg_raw(...)         printf(__VA_ARGS__);

#else
/* 调试功能禁用时的空定义 */
#define dbg_log(level, fmt, ...)
#define dbg_here
#define dbg_log_line(lvl, color_n, fmt, ...)
#define dbg_raw(...)
#endif /* DBG_ENABLE */

/** @defgroup LOG_MACROS 日志输出宏
 * @brief 提供简化的日志输出接口
 * @{
 */

/**
 * @brief 调试级别日志
 * @param fmt 格式化字符串
 * @param ... 可变参数
 */
#if (DBG_LEVEL >= DBG_LOG)
#define LOG_D(fmt, ...)      dbg_log_line("D", 0, fmt, ##__VA_ARGS__)
#else
#define LOG_D(...)
#endif

/**
 * @brief 信息级别日志
 * @param fmt 格式化字符串
 * @param ... 可变参数
 */
#if (DBG_LEVEL >= DBG_INFO)
#define LOG_I(fmt, ...)      dbg_log_line("I", 32, fmt, ##__VA_ARGS__)
#else
#define LOG_I(...)
#endif

/**
 * @brief 警告级别日志
 * @param fmt 格式化字符串
 * @param ... 可变参数
 */
#if (DBG_LEVEL >= DBG_WARNING)
#define LOG_W(fmt, ...)      dbg_log_line("W", 33, fmt, ##__VA_ARGS__)
#else
#define LOG_W(...)
#endif

/**
 * @brief 错误级别日志
 * @param fmt 格式化字符串
 * @param ... 可变参数
 */
#if (DBG_LEVEL >= DBG_ERROR)
#define LOG_E(fmt, ...)      dbg_log_line("E", 31, fmt, ##__VA_ARGS__)
#else
#define LOG_E(...)
#endif

/**
 * @brief 原始输出宏
 * @param ... 可变参数
 */
#define LOG_RAW(...)         dbg_raw(__VA_ARGS__)

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* DEBUG_LOG_H__ */
