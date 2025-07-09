/**
 * @file error_handler.c
 * @brief 统一错误处理系统实现 / Unified Error Handling System Implementation
 * @author newLISP Binary Compiler Team
 * @date 2024
 */

#include "error_handler.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include <errno.h>
#include <assert.h>

// 添加必要的系统头文件 / Add necessary system headers
#define _POSIX_C_SOURCE 199309L
#include <unistd.h>
#include <sys/time.h>

#include "string_utils.h"

// 全局错误管理器实例 / Global error manager instance
ErrorManager* g_error_manager = NULL;

// 错误级别字符串映射 / Error level string mapping
static const char* error_level_strings[] = {
    "DEBUG",    // ERROR_LEVEL_DEBUG
    "INFO",     // ERROR_LEVEL_INFO
    "WARNING",  // ERROR_LEVEL_WARNING
    "ERROR",    // ERROR_LEVEL_ERROR
    "FATAL"     // ERROR_LEVEL_FATAL
};

// 错误类型字符串映射 / Error type string mapping
static const char* error_type_strings[] = {
    "NONE",     // ERROR_TYPE_NONE
    "MEMORY",   // ERROR_TYPE_MEMORY
    "PARSE",    // ERROR_TYPE_PARSE
    "TYPE",     // ERROR_TYPE_TYPE
    "SYMBOL",   // ERROR_TYPE_SYMBOL
    "COMPILE",  // ERROR_TYPE_COMPILE
    "RUNTIME",  // ERROR_TYPE_RUNTIME
    "IO",       // ERROR_TYPE_IO
    "SYSTEM",   // ERROR_TYPE_SYSTEM
    "USER"      // ERROR_TYPE_USER
};

// 内部辅助函数声明 / Internal helper function declarations
static double get_current_timestamp(void);
static void free_error_info(ErrorInfo* error);
static void add_to_history(const ErrorInfo* error);
static void default_error_handler(const ErrorInfo* error, void* user_data);

/**
 * @brief 初始化错误处理系统 / Initialize error handling system
 */
bool error_system_init(int max_handlers, int max_history) {
    // 检查是否已经初始化 / Check if already initialized
    if (g_error_manager != NULL && g_error_manager->initialized) {
        ERROR_REPORT_WARNING(ERROR_TYPE_SYSTEM, -1, 
                            "Error system already initialized");
        return true;
    }
    
    // 分配错误管理器内存 / Allocate error manager memory
    g_error_manager = (ErrorManager*)malloc(sizeof(ErrorManager));
    if (g_error_manager == NULL) {
        fprintf(stderr, "FATAL: Failed to allocate memory for error manager\n");
        return false;
    }
    
    // 初始化错误管理器 / Initialize error manager
    memset(g_error_manager, 0, sizeof(ErrorManager));
    g_error_manager->max_handlers = max_handlers > 0 ? max_handlers : 10;
    g_error_manager->max_history = max_history > 0 ? max_history : 100;
    
    // 分配处理器数组 / Allocate handler array
    g_error_manager->handlers = (ErrorHandler*)calloc(
        g_error_manager->max_handlers, sizeof(ErrorHandler));
    if (g_error_manager->handlers == NULL) {
        free(g_error_manager);
        g_error_manager = NULL;
        fprintf(stderr, "FATAL: Failed to allocate memory for error handlers\n");
        return false;
    }
    
    // 分配错误历史数组 / Allocate error history array
    g_error_manager->error_history = (ErrorInfo*)calloc(
        g_error_manager->max_history, sizeof(ErrorInfo));
    if (g_error_manager->error_history == NULL) {
        free(g_error_manager->handlers);
        free(g_error_manager);
        g_error_manager = NULL;
        fprintf(stderr, "FATAL: Failed to allocate memory for error history\n");
        return false;
    }
    
    // 注册默认错误处理器 / Register default error handler
    g_error_manager->initialized = true;
    error_register_handler(default_error_handler, NULL, ERROR_LEVEL_WARNING);
    
    return true;
}

/**
 * @brief 清理错误处理系统 / Cleanup error handling system
 */
void error_system_cleanup(void) {
    if (g_error_manager == NULL) {
        return;
    }
    
    // 清理错误历史 / Cleanup error history
    if (g_error_manager->error_history != NULL) {
        for (int i = 0; i < g_error_manager->history_count; i++) {
            free_error_info(&g_error_manager->error_history[i]);
        }
        free(g_error_manager->error_history);
    }
    
    // 清理处理器数组 / Cleanup handler array
    if (g_error_manager->handlers != NULL) {
        free(g_error_manager->handlers);
    }
    
    // 释放错误管理器 / Free error manager
    free(g_error_manager);
    g_error_manager = NULL;
}

/**
 * @brief 注册错误处理器 / Register error handler
 */
int error_register_handler(ErrorCallback callback, void* user_data, ErrorLevel min_level) {
    if (g_error_manager == NULL || !g_error_manager->initialized) {
        fprintf(stderr, "ERROR: Error system not initialized\n");
        return -1;
    }
    
    if (callback == NULL) {
        ERROR_REPORT_ERROR(ERROR_TYPE_SYSTEM, -1, "Callback function is NULL%s", "");
        return -1;
    }
    
    // 查找空闲的处理器槽位 / Find free handler slot
    for (int i = 0; i < g_error_manager->max_handlers; i++) {
        if (!g_error_manager->handlers[i].enabled) {
            g_error_manager->handlers[i].callback = callback;
            g_error_manager->handlers[i].user_data = user_data;
            g_error_manager->handlers[i].min_level = min_level;
            g_error_manager->handlers[i].enabled = true;
            g_error_manager->handler_count++;
            return i;
        }
    }
    
    ERROR_REPORT_ERROR(ERROR_TYPE_SYSTEM, -1, 
                      "No free handler slots available (max: %d)", 
                      g_error_manager->max_handlers);
    return -1;
}

/**
 * @brief 注销错误处理器 / Unregister error handler
 */
bool error_unregister_handler(int handler_id) {
    if (g_error_manager == NULL || !g_error_manager->initialized) {
        return false;
    }
    
    if (handler_id < 0 || handler_id >= g_error_manager->max_handlers) {
        ERROR_REPORT_ERROR(ERROR_TYPE_SYSTEM, -1, 
                          "Invalid handler ID: %d", handler_id);
        return false;
    }
    
    if (g_error_manager->handlers[handler_id].enabled) {
        memset(&g_error_manager->handlers[handler_id], 0, sizeof(ErrorHandler));
        g_error_manager->handler_count--;
        return true;
    }
    
    return false;
}

/**
 * @brief 报告错误 / Report error
 */
void error_report(ErrorLevel level, ErrorType type, int code,
                 const char* file, int line, const char* function,
                 const char* format, ...) {
    va_list args;
    va_start(args, format);
    
    // 创建错误信息 / Create error info
    ErrorInfo error = {0};
    error.level = level;
    error.type = type;
    error.code = code;
    error.file = file ? safe_strdup(file) : NULL;
    error.line = line;
    error.function = function ? safe_strdup(function) : NULL;
    error.context = NULL;
    error.timestamp = get_current_timestamp();
    
    // 格式化错误消息 / Format error message
    char buffer[1024];
    vsnprintf(buffer, sizeof(buffer), format, args);
    error.message = safe_strdup(buffer);
    
    va_end(args);
    
    // 添加到历史记录 / Add to history
    add_to_history(&error);
    
    // 调用所有注册的处理器 / Call all registered handlers
    if (g_error_manager != NULL && g_error_manager->initialized) {
        for (int i = 0; i < g_error_manager->max_handlers; i++) {
            ErrorHandler* handler = &g_error_manager->handlers[i];
            if (handler->enabled && handler->callback != NULL && 
                level >= handler->min_level) {
                handler->callback(&error, handler->user_data);
            }
        }
    }
    
    // 如果是致命错误，终止程序 / If fatal error, terminate program
    if (level == ERROR_LEVEL_FATAL) {
        fprintf(stderr, "FATAL ERROR: %s\n", error.message);
        exit(EXIT_FAILURE);
    }
}

/**
 * @brief 报告错误（带上下文） / Report error with context
 */
void error_report_with_context(ErrorLevel level, ErrorType type, int code,
                              const char* file, int line, const char* function,
                              void* context, const char* format, ...) {
    va_list args;
    va_start(args, format);
    
    // 创建错误信息 / Create error info
    ErrorInfo error = {0};
    error.level = level;
    error.type = type;
    error.code = code;
    error.file = file ? safe_strdup(file) : NULL;
    error.line = line;
    error.function = function ? safe_strdup(function) : NULL;
    error.context = context;
    error.timestamp = get_current_timestamp();
    
    // 格式化错误消息 / Format error message
    char buffer[1024];
    vsnprintf(buffer, sizeof(buffer), format, args);
    error.message = safe_strdup(buffer);
    
    va_end(args);
    
    // 添加到历史记录 / Add to history
    add_to_history(&error);
    
    // 调用所有注册的处理器 / Call all registered handlers
    if (g_error_manager != NULL && g_error_manager->initialized) {
        for (int i = 0; i < g_error_manager->max_handlers; i++) {
            ErrorHandler* handler = &g_error_manager->handlers[i];
            if (handler->enabled && handler->callback != NULL && 
                level >= handler->min_level) {
                handler->callback(&error, handler->user_data);
            }
        }
    }
    
    // 如果是致命错误，终止程序 / If fatal error, terminate program
    if (level == ERROR_LEVEL_FATAL) {
        fprintf(stderr, "FATAL ERROR: %s\n", error.message);
        exit(EXIT_FAILURE);
    }
}

/**
 * @brief 获取最后一个错误 / Get last error
 */
const ErrorInfo* error_get_last(void) {
    if (g_error_manager == NULL || g_error_manager->history_count == 0) {
        return NULL;
    }
    
    int last_index = (g_error_manager->history_count - 1) % g_error_manager->max_history;
    return &g_error_manager->error_history[last_index];
}

/**
 * @brief 获取错误历史 / Get error history
 */
const ErrorInfo* error_get_history(int* count) {
    if (count != NULL) {
        *count = g_error_manager ? g_error_manager->history_count : 0;
    }
    
    return g_error_manager ? g_error_manager->error_history : NULL;
}

/**
 * @brief 清除错误历史 / Clear error history
 */
void error_clear_history(void) {
    if (g_error_manager == NULL) {
        return;
    }
    
    // 释放所有错误信息的内存 / Free memory of all error info
    for (int i = 0; i < g_error_manager->history_count; i++) {
        free_error_info(&g_error_manager->error_history[i]);
    }
    
    g_error_manager->history_count = 0;
}

/**
 * @brief 检查是否有错误 / Check if there are errors
 */
bool error_has_errors(ErrorLevel min_level) {
    if (g_error_manager == NULL || g_error_manager->history_count == 0) {
        return false;
    }
    
    for (int i = 0; i < g_error_manager->history_count; i++) {
        if (g_error_manager->error_history[i].level >= min_level) {
            return true;
        }
    }
    
    return false;
}

/**
 * @brief 获取错误级别字符串 / Get error level string
 */
const char* error_level_to_string(ErrorLevel level) {
    if (level >= 0 && level < sizeof(error_level_strings) / sizeof(error_level_strings[0])) {
        return error_level_strings[level];
    }
    return "UNKNOWN";
}

/**
 * @brief 获取错误类型字符串 / Get error type string
 */
const char* error_type_to_string(ErrorType type) {
    if (type >= 0 && type < sizeof(error_type_strings) / sizeof(error_type_strings[0])) {
        return error_type_strings[type];
    }
    return "UNKNOWN";
}

// 内部辅助函数实现 / Internal helper function implementations

/**
 * @brief 获取当前时间戳 / Get current timestamp
 */
static double get_current_timestamp(void) {
    // 使用更兼容的时间获取方式 / Use more compatible time getting method
    time_t now = time(NULL);
    return (double)now;
}

/**
 * @brief 释放错误信息内存 / Free error info memory
 */
static void free_error_info(ErrorInfo* error) {
    if (error == NULL) {
        return;
    }
    
    if (error->message != NULL) {
        free(error->message);
        error->message = NULL;
    }
    
    if (error->file != NULL) {
        free(error->file);
        error->file = NULL;
    }
    
    if (error->function != NULL) {
        free(error->function);
        error->function = NULL;
    }
}

/**
 * @brief 添加错误到历史记录 / Add error to history
 */
static void add_to_history(const ErrorInfo* error) {
    if (g_error_manager == NULL || error == NULL) {
        return;
    }
    
    int index = g_error_manager->history_count % g_error_manager->max_history;
    
    // 如果历史记录已满，释放最旧的记录 / If history is full, free oldest record
    if (g_error_manager->history_count >= g_error_manager->max_history) {
        free_error_info(&g_error_manager->error_history[index]);
    }
    
    // 复制错误信息 / Copy error info
    g_error_manager->error_history[index] = *error;
    g_error_manager->error_history[index].message = error->message ? safe_strdup(error->message) : NULL;
    g_error_manager->error_history[index].file = error->file ? safe_strdup(error->file) : NULL;
    g_error_manager->error_history[index].function = error->function ? safe_strdup(error->function) : NULL;
    
    if (g_error_manager->history_count < g_error_manager->max_history) {
        g_error_manager->history_count++;
    }
}

/**
 * @brief 默认错误处理器 / Default error handler
 */
static void default_error_handler(const ErrorInfo* error, void* user_data) {
    (void)user_data; // 未使用参数 / Unused parameter
    
    if (error == NULL) {
        return;
    }
    
    // 根据错误级别选择输出流 / Choose output stream based on error level
    FILE* output = (error->level >= ERROR_LEVEL_ERROR) ? stderr : stdout;
    
    // 格式化输出错误信息 / Format and output error info
    fprintf(output, "[%s] %s:%s:%d - %s (%s:%d)\n",
            error_level_to_string(error->level),
            error_type_to_string(error->type),
            error->function ? error->function : "unknown",
            error->code,
            error->message ? error->message : "No message",
            error->file ? error->file : "unknown",
            error->line);
    
    // 刷新输出缓冲区 / Flush output buffer
    fflush(output);
}
