/**
 * @file error_handler.h
 * @brief 统一错误处理系统 / Unified Error Handling System
 * @author newLISP Binary Compiler Team
 * @date 2024
 * 
 * 提供统一的错误处理、报告和恢复机制
 * Provides unified error handling, reporting and recovery mechanisms
 */

#ifndef ERROR_HANDLER_H
#define ERROR_HANDLER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// 错误级别定义 / Error Level Definitions
typedef enum {
    ERROR_LEVEL_DEBUG = 0,    // 调试信息 / Debug information
    ERROR_LEVEL_INFO,         // 一般信息 / General information  
    ERROR_LEVEL_WARNING,      // 警告 / Warning
    ERROR_LEVEL_ERROR,        // 错误 / Error
    ERROR_LEVEL_FATAL         // 致命错误 / Fatal error
} ErrorLevel;

// 错误类型定义 / Error Type Definitions
typedef enum {
    ERROR_TYPE_NONE = 0,      // 无错误 / No error
    ERROR_TYPE_MEMORY,        // 内存错误 / Memory error
    ERROR_TYPE_PARSE,         // 解析错误 / Parse error
    ERROR_TYPE_TYPE,          // 类型错误 / Type error
    ERROR_TYPE_SYMBOL,        // 符号错误 / Symbol error
    ERROR_TYPE_COMPILE,       // 编译错误 / Compilation error
    ERROR_TYPE_RUNTIME,       // 运行时错误 / Runtime error
    ERROR_TYPE_IO,            // I/O错误 / I/O error
    ERROR_TYPE_SYSTEM,        // 系统错误 / System error
    ERROR_TYPE_USER           // 用户定义错误 / User-defined error
} ErrorType;

// 错误信息结构 / Error Information Structure
typedef struct {
    ErrorLevel level;         // 错误级别 / Error level
    ErrorType type;           // 错误类型 / Error type
    int code;                 // 错误代码 / Error code
    char* message;            // 错误消息 / Error message
    char* file;               // 源文件名 / Source file name
    int line;                 // 行号 / Line number
    char* function;           // 函数名 / Function name
    void* context;            // 上下文数据 / Context data
    double timestamp;         // 时间戳 / Timestamp
} ErrorInfo;

// 错误处理器回调函数类型 / Error Handler Callback Function Type
typedef void (*ErrorCallback)(const ErrorInfo* error, void* user_data);

// 错误恢复策略 / Error Recovery Strategy
typedef enum {
    RECOVERY_NONE = 0,        // 不恢复 / No recovery
    RECOVERY_CONTINUE,        // 继续执行 / Continue execution
    RECOVERY_RETRY,           // 重试 / Retry
    RECOVERY_FALLBACK,        // 回退 / Fallback
    RECOVERY_ABORT            // 中止 / Abort
} RecoveryStrategy;

// 错误处理器结构 / Error Handler Structure
typedef struct {
    ErrorCallback callback;   // 回调函数 / Callback function
    void* user_data;         // 用户数据 / User data
    ErrorLevel min_level;    // 最小处理级别 / Minimum handling level
    bool enabled;            // 是否启用 / Whether enabled
} ErrorHandler;

// 错误管理器结构 / Error Manager Structure
typedef struct {
    ErrorHandler* handlers;   // 处理器数组 / Handler array
    int handler_count;       // 处理器数量 / Handler count
    int max_handlers;        // 最大处理器数 / Maximum handlers
    ErrorInfo* error_history; // 错误历史 / Error history
    int history_count;       // 历史数量 / History count
    int max_history;         // 最大历史数 / Maximum history
    bool initialized;        // 是否已初始化 / Whether initialized
} ErrorManager;

// 全局错误管理器 / Global Error Manager
extern ErrorManager* g_error_manager;

// 核心API函数 / Core API Functions

/**
 * @brief 初始化错误处理系统 / Initialize error handling system
 * @param max_handlers 最大处理器数量 / Maximum number of handlers
 * @param max_history 最大错误历史数量 / Maximum error history count
 * @return 成功返回true / Returns true on success
 */
bool error_system_init(int max_handlers, int max_history);

/**
 * @brief 清理错误处理系统 / Cleanup error handling system
 */
void error_system_cleanup(void);

/**
 * @brief 注册错误处理器 / Register error handler
 * @param callback 回调函数 / Callback function
 * @param user_data 用户数据 / User data
 * @param min_level 最小处理级别 / Minimum handling level
 * @return 处理器ID，失败返回-1 / Handler ID, -1 on failure
 */
int error_register_handler(ErrorCallback callback, void* user_data, ErrorLevel min_level);

/**
 * @brief 注销错误处理器 / Unregister error handler
 * @param handler_id 处理器ID / Handler ID
 * @return 成功返回true / Returns true on success
 */
bool error_unregister_handler(int handler_id);

/**
 * @brief 报告错误 / Report error
 * @param level 错误级别 / Error level
 * @param type 错误类型 / Error type
 * @param code 错误代码 / Error code
 * @param file 源文件名 / Source file name
 * @param line 行号 / Line number
 * @param function 函数名 / Function name
 * @param format 格式化字符串 / Format string
 * @param ... 可变参数 / Variable arguments
 */
void error_report(ErrorLevel level, ErrorType type, int code,
                 const char* file, int line, const char* function,
                 const char* format, ...);

/**
 * @brief 报告错误（带上下文） / Report error with context
 * @param level 错误级别 / Error level
 * @param type 错误类型 / Error type
 * @param code 错误代码 / Error code
 * @param file 源文件名 / Source file name
 * @param line 行号 / Line number
 * @param function 函数名 / Function name
 * @param context 上下文数据 / Context data
 * @param format 格式化字符串 / Format string
 * @param ... 可变参数 / Variable arguments
 */
void error_report_with_context(ErrorLevel level, ErrorType type, int code,
                              const char* file, int line, const char* function,
                              void* context, const char* format, ...);

/**
 * @brief 获取最后一个错误 / Get last error
 * @return 错误信息指针，无错误返回NULL / Error info pointer, NULL if no error
 */
const ErrorInfo* error_get_last(void);

/**
 * @brief 获取错误历史 / Get error history
 * @param count 输出参数，历史数量 / Output parameter, history count
 * @return 错误历史数组 / Error history array
 */
const ErrorInfo* error_get_history(int* count);

/**
 * @brief 清除错误历史 / Clear error history
 */
void error_clear_history(void);

/**
 * @brief 检查是否有错误 / Check if there are errors
 * @param min_level 最小检查级别 / Minimum check level
 * @return 有错误返回true / Returns true if there are errors
 */
bool error_has_errors(ErrorLevel min_level);

/**
 * @brief 获取错误级别字符串 / Get error level string
 * @param level 错误级别 / Error level
 * @return 级别字符串 / Level string
 */
const char* error_level_to_string(ErrorLevel level);

/**
 * @brief 获取错误类型字符串 / Get error type string
 * @param type 错误类型 / Error type
 * @return 类型字符串 / Type string
 */
const char* error_type_to_string(ErrorType type);

// 便利宏定义 / Convenience Macros

#define ERROR_REPORT_DEBUG(type, code, format, ...) \
    error_report(ERROR_LEVEL_DEBUG, type, code, __FILE__, __LINE__, __FUNCTION__, format, ##__VA_ARGS__)

#define ERROR_REPORT_INFO(type, code, format, ...) \
    error_report(ERROR_LEVEL_INFO, type, code, __FILE__, __LINE__, __FUNCTION__, format, ##__VA_ARGS__)

#define ERROR_REPORT_WARNING(type, code, format, ...) \
    error_report(ERROR_LEVEL_WARNING, type, code, __FILE__, __LINE__, __FUNCTION__, format, ##__VA_ARGS__)

#define ERROR_REPORT_ERROR(type, code, format, ...) \
    error_report(ERROR_LEVEL_ERROR, type, code, __FILE__, __LINE__, __FUNCTION__, format, ##__VA_ARGS__)

#define ERROR_REPORT_FATAL(type, code, format, ...) \
    error_report(ERROR_LEVEL_FATAL, type, code, __FILE__, __LINE__, __FUNCTION__, format, ##__VA_ARGS__)

// 带上下文的便利宏 / Convenience macros with context
#define ERROR_REPORT_WITH_CONTEXT(level, type, code, context, format, ...) \
    error_report_with_context(level, type, code, __FILE__, __LINE__, __FUNCTION__, context, format, ##__VA_ARGS__)

// 条件错误报告宏 / Conditional error reporting macros
#define ERROR_CHECK_NULL(ptr, type, code, format, ...) \
    do { \
        if ((ptr) == NULL) { \
            ERROR_REPORT_ERROR(type, code, format, ##__VA_ARGS__); \
            return false; \
        } \
    } while(0)

#define ERROR_CHECK_CONDITION(condition, type, code, format, ...) \
    do { \
        if (!(condition)) { \
            ERROR_REPORT_ERROR(type, code, format, ##__VA_ARGS__); \
            return false; \
        } \
    } while(0)

// 内存分配检查宏 / Memory allocation check macro
#define ERROR_CHECK_MALLOC(ptr, size) \
    ERROR_CHECK_NULL(ptr, ERROR_TYPE_MEMORY, -1, "Failed to allocate %zu bytes", (size_t)(size))

// 函数返回值检查宏 / Function return value check macro
#define ERROR_CHECK_RETURN(func_call, expected, type, code, format, ...) \
    do { \
        if ((func_call) != (expected)) { \
            ERROR_REPORT_ERROR(type, code, format, ##__VA_ARGS__); \
            return false; \
        } \
    } while(0)

#ifdef __cplusplus
}
#endif

#endif // ERROR_HANDLER_H
