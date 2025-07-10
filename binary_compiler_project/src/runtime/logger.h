/**
 * @file logger.h
 * @brief 统一日志系统 / Unified Logging System
 * @author newLISP Binary Compiler Team
 * @date 2024
 * 
 * 提供统一的日志记录、格式化和输出功能
 * Provides unified logging, formatting and output functionality
 */

#ifndef LOGGER_H
#define LOGGER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <time.h>

// ISO C99兼容性宏 / ISO C99 compatibility macro
#ifndef FUNCTION_NAME
#ifdef __STDC_VERSION__
    #if __STDC_VERSION__ >= 199901L
        #define FUNCTION_NAME __func__
    #else
        #define FUNCTION_NAME "unknown"
    #endif
#else
    #define FUNCTION_NAME "unknown"
#endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

// 日志级别定义 / Log Level Definitions
typedef enum {
    LOG_LEVEL_TRACE = 0,      // 跟踪信息 / Trace information
    LOG_LEVEL_DEBUG,          // 调试信息 / Debug information
    LOG_LEVEL_INFO,           // 一般信息 / General information
    LOG_LEVEL_WARNING,        // 警告信息 / Warning information
    LOG_LEVEL_ERROR,          // 错误信息 / Error information
    LOG_LEVEL_FATAL,          // 致命错误 / Fatal error
    LOG_LEVEL_OFF             // 关闭日志 / Turn off logging
} LogLevel;

// 日志输出目标 / Log Output Target
typedef enum {
    LOG_TARGET_CONSOLE = 1,   // 控制台输出 / Console output
    LOG_TARGET_FILE = 2,      // 文件输出 / File output
    LOG_TARGET_SYSLOG = 4,    // 系统日志 / System log
    LOG_TARGET_CALLBACK = 8   // 回调函数 / Callback function
} LogTarget;

// 日志格式选项 / Log Format Options
typedef enum {
    LOG_FORMAT_SIMPLE = 0,    // 简单格式 / Simple format
    LOG_FORMAT_DETAILED,      // 详细格式 / Detailed format
    LOG_FORMAT_JSON,          // JSON格式 / JSON format
    LOG_FORMAT_CUSTOM         // 自定义格式 / Custom format
} LogFormat;

// 日志记录结构 / Log Record Structure
typedef struct {
    LogLevel level;           // 日志级别 / Log level
    double timestamp;         // 时间戳 / Timestamp
    const char* file;         // 源文件名 / Source file name
    int line;                 // 行号 / Line number
    const char* function;     // 函数名 / Function name
    const char* category;     // 日志分类 / Log category
    char* message;            // 日志消息 / Log message
    void* context;            // 上下文数据 / Context data
} LogRecord;

// 日志回调函数类型 / Log Callback Function Type
typedef void (*LogCallback)(const LogRecord* record, void* user_data);

// 日志格式化函数类型 / Log Formatter Function Type
typedef char* (*LogFormatter)(const LogRecord* record);

// 日志配置结构 / Log Configuration Structure
typedef struct {
    LogLevel min_level;       // 最小日志级别 / Minimum log level
    int targets;              // 输出目标（位掩码） / Output targets (bitmask)
    LogFormat format;         // 日志格式 / Log format
    char* log_file_path;      // 日志文件路径 / Log file path
    size_t max_file_size;     // 最大文件大小 / Maximum file size
    int max_backup_files;     // 最大备份文件数 / Maximum backup files
    bool enable_colors;       // 启用颜色输出 / Enable color output
    bool enable_timestamps;   // 启用时间戳 / Enable timestamps
    bool enable_thread_id;    // 启用线程ID / Enable thread ID
    LogCallback callback;     // 回调函数 / Callback function
    void* callback_data;      // 回调数据 / Callback data
    LogFormatter formatter;   // 自定义格式化器 / Custom formatter
} LogConfig;

// 日志系统结构 / Log System Structure
typedef struct {
    LogConfig config;         // 配置信息 / Configuration
    FILE* log_file;          // 日志文件句柄 / Log file handle
    size_t current_file_size; // 当前文件大小 / Current file size
    bool initialized;         // 是否已初始化 / Whether initialized
    void* mutex;             // 线程同步锁 / Thread synchronization mutex
} LogSystem;

// 全局日志系统 / Global Log System
extern LogSystem* g_log_system;

// 核心API函数 / Core API Functions

/**
 * @brief 初始化日志系统 / Initialize logging system
 * @param config 配置参数 / Configuration parameters
 * @return 成功返回true / Returns true on success
 */
bool logger_init(const LogConfig* config);

/**
 * @brief 清理日志系统 / Cleanup logging system
 */
void logger_cleanup(void);

/**
 * @brief 记录日志 / Log message
 * @param level 日志级别 / Log level
 * @param file 源文件名 / Source file name
 * @param line 行号 / Line number
 * @param function 函数名 / Function name
 * @param category 日志分类 / Log category
 * @param format 格式化字符串 / Format string
 * @param ... 可变参数 / Variable arguments
 */
void logger_log(LogLevel level, const char* file, int line, const char* function,
               const char* category, const char* format, ...);

/**
 * @brief 记录日志（带上下文） / Log message with context
 * @param level 日志级别 / Log level
 * @param file 源文件名 / Source file name
 * @param line 行号 / Line number
 * @param function 函数名 / Function name
 * @param category 日志分类 / Log category
 * @param context 上下文数据 / Context data
 * @param format 格式化字符串 / Format string
 * @param ... 可变参数 / Variable arguments
 */
void logger_log_with_context(LogLevel level, const char* file, int line, const char* function,
                            const char* category, void* context, const char* format, ...);

/**
 * @brief 设置日志级别 / Set log level
 * @param level 日志级别 / Log level
 */
void logger_set_level(LogLevel level);

/**
 * @brief 获取日志级别 / Get log level
 * @return 当前日志级别 / Current log level
 */
LogLevel logger_get_level(void);

/**
 * @brief 启用/禁用日志输出目标 / Enable/disable log output target
 * @param target 输出目标 / Output target
 * @param enable 是否启用 / Whether to enable
 */
void logger_set_target(LogTarget target, bool enable);

/**
 * @brief 设置日志文件 / Set log file
 * @param file_path 文件路径 / File path
 * @return 成功返回true / Returns true on success
 */
bool logger_set_file(const char* file_path);

/**
 * @brief 检查日志级别是否启用 / Check if log level is enabled
 * @param level 日志级别 / Log level
 * @return 启用返回true / Returns true if enabled
 */
bool logger_is_level_enabled(LogLevel level);

/**
 * @brief 检查控制台日志是否启用 / Check if console logging is enabled
 * @return 启用返回true / Returns true if enabled
 */
bool logger_is_console_enabled(void);

/**
 * @brief 检查文件日志是否启用 / Check if file logging is enabled
 * @return 启用返回true / Returns true if enabled
 */
bool logger_is_file_enabled(void);

/**
 * @brief 获取日志运行时间 / Get logger uptime
 * @return 运行时间(毫秒) / Uptime in milliseconds
 */
uint64_t logger_get_uptime_ms(void);

/**
 * @brief 刷新日志缓冲区 / Flush log buffers
 */
void logger_flush(void);

/**
 * @brief 轮转日志文件 / Rotate log file
 * @return 成功返回true / Returns true on success
 */
bool logger_rotate_file(void);

/**
 * @brief 获取日志级别字符串 / Get log level string
 * @param level 日志级别 / Log level
 * @return 级别字符串 / Level string
 */
const char* logger_level_to_string(LogLevel level);

/**
 * @brief 从字符串解析日志级别 / Parse log level from string
 * @param level_str 级别字符串 / Level string
 * @return 日志级别 / Log level
 */
LogLevel logger_level_from_string(const char* level_str);

/**
 * @brief 格式化时间戳 / Format timestamp
 * @param timestamp 时间戳 / Timestamp
 * @param buffer 输出缓冲区 / Output buffer
 * @param buffer_size 缓冲区大小 / Buffer size
 * @return 格式化的字符串长度 / Length of formatted string
 */
int logger_format_timestamp(double timestamp, char* buffer, size_t buffer_size);

// 便利宏定义 / Convenience Macros

#define LOG_TRACE(category, format, ...) \
    logger_log(LOG_LEVEL_TRACE, __FILE__, __LINE__, FUNCTION_NAME, category, format, ##__VA_ARGS__)

#define LOG_DEBUG(category, format, ...) \
    logger_log(LOG_LEVEL_DEBUG, __FILE__, __LINE__, FUNCTION_NAME, category, format, ##__VA_ARGS__)

#define LOG_INFO(category, format, ...) \
    logger_log(LOG_LEVEL_INFO, __FILE__, __LINE__, FUNCTION_NAME, category, format, ##__VA_ARGS__)

#define LOG_WARNING(category, format, ...) \
    logger_log(LOG_LEVEL_WARNING, __FILE__, __LINE__, FUNCTION_NAME, category, format, ##__VA_ARGS__)

#define LOG_ERROR(category, format, ...) \
    logger_log(LOG_LEVEL_ERROR, __FILE__, __LINE__, FUNCTION_NAME, category, format, ##__VA_ARGS__)

#define LOG_FATAL(category, format, ...) \
    logger_log(LOG_LEVEL_FATAL, __FILE__, __LINE__, FUNCTION_NAME, category, format, ##__VA_ARGS__)

// 带上下文的日志宏 / Log macros with context
#define LOG_WITH_CONTEXT(level, category, context, format, ...) \
    logger_log_with_context(level, __FILE__, __LINE__, FUNCTION_NAME, category, context, format, ##__VA_ARGS__)

// 分类日志宏 / Category-specific log macros
#define LOG_COMPILER_TRACE(format, ...)   LOG_TRACE("COMPILER", format, ##__VA_ARGS__)
#define LOG_COMPILER_DEBUG(format, ...)   LOG_DEBUG("COMPILER", format, ##__VA_ARGS__)
#define LOG_COMPILER_INFO(format, ...)    LOG_INFO("COMPILER", format, ##__VA_ARGS__)
#define LOG_COMPILER_WARNING(format, ...) LOG_WARNING("COMPILER", format, ##__VA_ARGS__)
#define LOG_COMPILER_ERROR(format, ...)   LOG_ERROR("COMPILER", format, ##__VA_ARGS__)

#define LOG_MEMORY_TRACE(format, ...)     LOG_TRACE("MEMORY", format, ##__VA_ARGS__)
#define LOG_MEMORY_DEBUG(format, ...)     LOG_DEBUG("MEMORY", format, ##__VA_ARGS__)
#define LOG_MEMORY_INFO(format, ...)      LOG_INFO("MEMORY", format, ##__VA_ARGS__)
#define LOG_MEMORY_WARNING(format, ...)   LOG_WARNING("MEMORY", format, ##__VA_ARGS__)
#define LOG_MEMORY_ERROR(format, ...)     LOG_ERROR("MEMORY", format, ##__VA_ARGS__)

#define LOG_AST_TRACE(format, ...)        LOG_TRACE("AST", format, ##__VA_ARGS__)
#define LOG_AST_DEBUG(format, ...)        LOG_DEBUG("AST", format, ##__VA_ARGS__)
#define LOG_AST_INFO(format, ...)         LOG_INFO("AST", format, ##__VA_ARGS__)
#define LOG_AST_WARNING(format, ...)      LOG_WARNING("AST", format, ##__VA_ARGS__)
#define LOG_AST_ERROR(format, ...)        LOG_ERROR("AST", format, ##__VA_ARGS__)

// 条件日志宏 / Conditional log macros
#define LOG_IF(condition, level, category, format, ...) \
    do { \
        if (condition) { \
            logger_log(level, __FILE__, __LINE__, __FUNCTION__, category, format, ##__VA_ARGS__); \
        } \
    } while(0)

// 性能日志宏 / Performance log macros
#define LOG_PERFORMANCE_START(name) \
    double _perf_start_##name = logger_get_timestamp()

#define LOG_PERFORMANCE_END(name, category) \
    do { \
        double _perf_end_##name = logger_get_timestamp(); \
        LOG_INFO(category, "Performance: %s took %.3f ms", #name, \
                (_perf_end_##name - _perf_start_##name) * 1000.0); \
    } while(0)

// 辅助函数 / Helper Functions

/**
 * @brief 获取当前时间戳 / Get current timestamp
 * @return 时间戳 / Timestamp
 */
double logger_get_timestamp(void);

/**
 * @brief 创建默认日志配置 / Create default log configuration
 * @return 默认配置 / Default configuration
 */
LogConfig logger_create_default_config(void);

#ifdef __cplusplus
}
#endif

#endif // LOGGER_H
