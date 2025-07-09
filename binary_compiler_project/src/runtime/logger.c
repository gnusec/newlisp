/**
 * @file logger.c
 * @brief 日志系统实现 / Logging system implementation
 * 
 * 提供多级别日志记录功能，支持文件输出和性能监控
 * Provides multi-level logging functionality with file output and performance monitoring
 */

#include "logger.h"
#include "error_handler.h"
#include "memory_manager.h"
#include "string_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdarg.h>
#include <sys/time.h>

/**
 * @brief 日志级别名称 / Log level names
 */
static const char* LOG_LEVEL_NAMES[] = {
    "TRACE", "DEBUG", "INFO", "WARN", "ERROR", "FATAL"
};

/**
 * @brief 全局日志系统 / Global logging system
 */
static struct {
    LogConfig config;             // 日志配置 / Log configuration
    FILE* log_file;               // 日志文件句柄 / Log file handle
    bool initialized;             // 初始化状态 / Initialization status
    uint64_t start_time;          // 启动时间 / Start time
} g_logger = {0};

/**
 * @brief 获取当前时间戳 / Get current timestamp
 */
static uint64_t get_timestamp_ms(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (uint64_t)tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

/**
 * @brief 格式化时间字符串 / Format time string
 */
static void format_timestamp(char* buffer, size_t size) {
    time_t now = time(NULL);
    struct tm* tm_info = localtime(&now);
    strftime(buffer, size, "%Y-%m-%d %H:%M:%S", tm_info);
}

/**
 * @brief 初始化日志系统 / Initialize logging system
 */
bool logger_init(const LogConfig* config) {
    if (g_logger.initialized) {
        return true;
    }
    
    if (config) {
        g_logger.config = *config;
    } else {
        // 默认配置 / Default configuration
        g_logger.config.min_level = LOG_LEVEL_INFO;
        g_logger.config.targets = LOG_TARGET_CONSOLE;
        g_logger.config.format = LOG_FORMAT_DETAILED;
        g_logger.config.enable_timestamps = true;
        g_logger.config.enable_colors = true;
        g_logger.config.max_file_size = 10 * 1024 * 1024; // 10MB
        g_logger.config.log_file_path = NULL;
    }
    
    // 打开日志文件 / Open log file
    if ((g_logger.config.targets & LOG_TARGET_FILE) && g_logger.config.log_file_path) {
        g_logger.log_file = fopen(g_logger.config.log_file_path, "a");
        if (!g_logger.log_file) {
            fprintf(stderr, "Failed to open log file: %s\n", g_logger.config.log_file_path);
            return false;
        }
    }
    
    g_logger.start_time = get_timestamp_ms();
    g_logger.initialized = true;
    
    // 初始化完成日志 / Initialization complete log
    if (g_logger.initialized) {
        logger_log(LOG_LEVEL_INFO, __FILE__, __LINE__, FUNCTION_NAME,
                   "system", "Logger initialized successfully");
    }
    return true;
}

/**
 * @brief 清理日志系统 / Cleanup logging system
 */
void logger_cleanup(void) {
    if (!g_logger.initialized) {
        return;
    }
    
    // 关闭日志 / Shutdown log
    if (g_logger.initialized) {
        logger_log(LOG_LEVEL_INFO, __FILE__, __LINE__, FUNCTION_NAME,
                   "system", "Logger shutting down");
    }
    
    if (g_logger.log_file) {
        fclose(g_logger.log_file);
        g_logger.log_file = NULL;
    }
    
    g_logger.initialized = false;
}

/**
 * @brief 检查日志级别是否启用 / Check if log level is enabled
 */
bool logger_is_level_enabled(LogLevel level) {
    return g_logger.initialized && level >= g_logger.config.min_level;
}

/**
 * @brief 写入日志消息 / Write log message
 */
void logger_log(LogLevel level, const char* file, int line, const char* function,
               const char* category, const char* format, ...) {
    if (!g_logger.initialized || level < g_logger.config.min_level) {
        return;
    }
    
    // 格式化消息 / Format message
    va_list args;
    va_start(args, format);

    char message[1024];
    vsnprintf(message, sizeof(message), format, args);
    va_end(args);

    // 格式化时间戳 / Format timestamp
    char timestamp[64] = "";
    if (g_logger.config.enable_timestamps) {
        format_timestamp(timestamp, sizeof(timestamp));
    }

    // 构建完整日志行 / Build complete log line
    char log_line[1280];
    if (g_logger.config.enable_timestamps) {
        snprintf(log_line, sizeof(log_line), "[%s] [%s] [%s] %s:%d:%s - %s\n",
                timestamp, LOG_LEVEL_NAMES[level], category, file, line, function, message);
    } else {
        snprintf(log_line, sizeof(log_line), "[%s] [%s] %s:%d:%s - %s\n",
                LOG_LEVEL_NAMES[level], category, file, line, function, message);
    }

    // 输出到控制台 / Output to console
    if (g_logger.config.targets & LOG_TARGET_CONSOLE) {
        FILE* output = (level >= LOG_LEVEL_ERROR) ? stderr : stdout;
        
        if (g_logger.config.enable_colors) {
            // 添加颜色代码 / Add color codes
            const char* color_start = "";
            const char* color_end = "\033[0m";
            
            switch (level) {
                case LOG_LEVEL_TRACE: color_start = "\033[37m"; break; // White
                case LOG_LEVEL_DEBUG: color_start = "\033[36m"; break; // Cyan
                case LOG_LEVEL_INFO:  color_start = "\033[32m"; break; // Green
                case LOG_LEVEL_WARNING: color_start = "\033[33m"; break; // Yellow
                case LOG_LEVEL_ERROR: color_start = "\033[31m"; break; // Red
                case LOG_LEVEL_FATAL: color_start = "\033[35m"; break; // Magenta
                case LOG_LEVEL_OFF: color_start = "\033[0m"; break; // Default
            }
            
            fprintf(output, "%s%s%s", color_start, log_line, color_end);
        } else {
            fprintf(output, "%s", log_line);
        }
        
        fflush(output);
    }
    
    // 输出到文件 / Output to file
    if ((g_logger.config.targets & LOG_TARGET_FILE) && g_logger.log_file) {
        fprintf(g_logger.log_file, "%s", log_line);
        fflush(g_logger.log_file);
        
        // 检查文件大小 / Check file size
        if (g_logger.config.max_file_size > 0) {
            long file_size = ftell(g_logger.log_file);
            if (file_size > (long)g_logger.config.max_file_size) {
                // 轮转日志文件 / Rotate log file
                logger_rotate_file();
            }
        }
    }
}

/**
 * @brief 轮转日志文件 / Rotate log file
 */
bool logger_rotate_file(void) {
    if (!g_logger.log_file || !g_logger.config.log_file_path) {
        return false;
    }
    
    fclose(g_logger.log_file);
    
    // 创建备份文件名 / Create backup filename
    char backup_path[512];
    time_t now = time(NULL);
    struct tm* tm_info = localtime(&now);
    
    snprintf(backup_path, sizeof(backup_path), "%s.%04d%02d%02d_%02d%02d%02d",
            g_logger.config.log_file_path,
            tm_info->tm_year + 1900, tm_info->tm_mon + 1, tm_info->tm_mday,
            tm_info->tm_hour, tm_info->tm_min, tm_info->tm_sec);
    
    // 重命名当前文件 / Rename current file
    if (rename(g_logger.config.log_file_path, backup_path) != 0) {
        fprintf(stderr, "Failed to rotate log file\n");
        return false;
    }
    
    // 重新打开日志文件 / Reopen log file
    g_logger.log_file = fopen(g_logger.config.log_file_path, "w");
    if (!g_logger.log_file) {
        fprintf(stderr, "Failed to reopen log file after rotation\n");
        return false;
    }
    
    logger_log(LOG_LEVEL_INFO, __FILE__, __LINE__, FUNCTION_NAME,
               "system", "Log file rotated to: %s", backup_path);
    return true;
}

/**
 * @brief 设置日志级别 / Set log level
 */
void logger_set_level(LogLevel level) {
    if (g_logger.initialized) {
        g_logger.config.min_level = level;
        logger_log(LOG_LEVEL_INFO, __FILE__, __LINE__, FUNCTION_NAME,
                   "system", "Log level changed to: %s", LOG_LEVEL_NAMES[level]);
    }
}

/**
 * @brief 获取当前日志级别 / Get current log level
 */
LogLevel logger_get_level(void) {
    return g_logger.initialized ? g_logger.config.min_level : LOG_LEVEL_INFO;
}

/**
 * @brief 刷新日志缓冲区 / Flush log buffers
 */
void logger_flush(void) {
    if (g_logger.initialized) {
        if (g_logger.config.targets & LOG_TARGET_CONSOLE) {
            fflush(stdout);
            fflush(stderr);
        }

        if (g_logger.log_file) {
            fflush(g_logger.log_file);
        }
    }
}

/**
 * @brief 获取日志运行时间 / Get logger uptime
 */
uint64_t logger_get_uptime_ms(void) {
    if (g_logger.initialized) {
        return get_timestamp_ms() - g_logger.start_time;
    }
    return 0;
}

/**
 * @brief 检查文件日志是否启用 / Check if file logging is enabled
 */
bool logger_is_file_enabled(void) {
    return g_logger.initialized && (g_logger.config.targets & LOG_TARGET_FILE) != 0;
}

/**
 * @brief 检查控制台日志是否启用 / Check if console logging is enabled
 */
bool logger_is_console_enabled(void) {
    return g_logger.initialized && (g_logger.config.targets & LOG_TARGET_CONSOLE) != 0;
}
