/**
 * @file config_manager.h
 * @brief 配置管理系统 / Configuration Management System
 * @author newLISP Binary Compiler Team
 * @date 2024
 * 
 * 提供统一的配置加载、保存和管理功能
 * Provides unified configuration loading, saving and management functionality
 */

#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// 配置值类型 / Configuration Value Types
typedef enum {
    CONFIG_TYPE_BOOL = 0,     // 布尔值 / Boolean value
    CONFIG_TYPE_INT,          // 整数值 / Integer value
    CONFIG_TYPE_FLOAT,        // 浮点值 / Float value
    CONFIG_TYPE_STRING,       // 字符串值 / String value
    CONFIG_TYPE_ARRAY,        // 数组值 / Array value
    CONFIG_TYPE_OBJECT        // 对象值 / Object value
} ConfigType;

// 配置值结构 / Configuration Value Structure
typedef struct ConfigValue {
    ConfigType type;          // 值类型 / Value type
    union {
        bool bool_val;        // 布尔值 / Boolean value
        int64_t int_val;      // 整数值 / Integer value
        double float_val;     // 浮点值 / Float value
        char* string_val;     // 字符串值 / String value
        struct {
            struct ConfigValue** items; // 数组项 / Array items
            size_t count;     // 数组大小 / Array size
        } array_val;
        struct ConfigNode* object_val; // 对象值 / Object value
    } value;
} ConfigValue;

// 配置节点结构 / Configuration Node Structure
typedef struct ConfigNode {
    char* key;                // 键名 / Key name
    ConfigValue* value;       // 配置值 / Configuration value
    char* comment;            // 注释 / Comment
    struct ConfigNode* next;  // 链表下一个 / Next in list
    struct ConfigNode* parent; // 父节点 / Parent node
} ConfigNode;

// 配置管理器结构 / Configuration Manager Structure
typedef struct {
    ConfigNode* root;         // 根节点 / Root node
    char* config_file;        // 配置文件路径 / Configuration file path
    bool auto_save;           // 自动保存 / Auto save
    bool case_sensitive;      // 大小写敏感 / Case sensitive
    char* default_section;    // 默认节 / Default section
    bool initialized;         // 是否已初始化 / Whether initialized
} ConfigManager;

// 全局配置管理器 / Global Configuration Manager
extern ConfigManager* g_config_manager;

// 编译器配置结构 / Compiler Configuration Structure
typedef struct {
    // 基本设置 / Basic Settings
    bool enable_compilation;  // 启用编译 / Enable compilation
    char* compilation_mode;   // 编译模式 / Compilation mode
    char* optimization_level; // 优化级别 / Optimization level
    char* target_architecture; // 目标架构 / Target architecture
    
    // 路径设置 / Path Settings
    char* source_directory;   // 源码目录 / Source directory
    char* output_directory;   // 输出目录 / Output directory
    char* temp_directory;     // 临时目录 / Temporary directory
    char* include_paths;      // 包含路径 / Include paths
    char* library_paths;      // 库路径 / Library paths
    
    // 编译选项 / Compilation Options
    bool enable_debug_info;   // 启用调试信息 / Enable debug info
    bool enable_optimization; // 启用优化 / Enable optimization
    bool enable_warnings;     // 启用警告 / Enable warnings
    bool treat_warnings_as_errors; // 警告视为错误 / Treat warnings as errors
    
    // 内存设置 / Memory Settings
    size_t max_memory_usage;  // 最大内存使用 / Maximum memory usage
    bool enable_memory_tracking; // 启用内存跟踪 / Enable memory tracking
    bool enable_leak_detection; // 启用泄漏检测 / Enable leak detection
    
    // 日志设置 / Logging Settings
    char* log_level;          // 日志级别 / Log level
    char* log_file;           // 日志文件 / Log file
    bool enable_console_log;  // 启用控制台日志 / Enable console log
    bool enable_file_log;     // 启用文件日志 / Enable file log
    
    // 性能设置 / Performance Settings
    int thread_count;         // 线程数量 / Thread count
    bool enable_parallel_compilation; // 启用并行编译 / Enable parallel compilation
    double compilation_timeout; // 编译超时 / Compilation timeout
} CompilerConfig;

// 核心API函数 / Core API Functions

/**
 * @brief 初始化配置管理器 / Initialize configuration manager
 * @param config_file 配置文件路径 / Configuration file path
 * @return 成功返回true / Returns true on success
 */
bool config_manager_init(const char* config_file);

/**
 * @brief 清理配置管理器 / Cleanup configuration manager
 */
void config_manager_cleanup(void);

/**
 * @brief 加载配置文件 / Load configuration file
 * @param filename 文件名 / Filename
 * @return 成功返回true / Returns true on success
 */
bool config_load_file(const char* filename);

/**
 * @brief 保存配置文件 / Save configuration file
 * @param filename 文件名 / Filename
 * @return 成功返回true / Returns true on success
 */
bool config_save_file(const char* filename);

/**
 * @brief 从字符串加载配置 / Load configuration from string
 * @param config_string 配置字符串 / Configuration string
 * @return 成功返回true / Returns true on success
 */
bool config_load_string(const char* config_string);

/**
 * @brief 设置布尔值 / Set boolean value
 * @param key 键名 / Key name
 * @param value 布尔值 / Boolean value
 * @return 成功返回true / Returns true on success
 */
bool config_set_bool(const char* key, bool value);

/**
 * @brief 设置整数值 / Set integer value
 * @param key 键名 / Key name
 * @param value 整数值 / Integer value
 * @return 成功返回true / Returns true on success
 */
bool config_set_int(const char* key, int64_t value);

/**
 * @brief 设置浮点值 / Set float value
 * @param key 键名 / Key name
 * @param value 浮点值 / Float value
 * @return 成功返回true / Returns true on success
 */
bool config_set_float(const char* key, double value);

/**
 * @brief 设置字符串值 / Set string value
 * @param key 键名 / Key name
 * @param value 字符串值 / String value
 * @return 成功返回true / Returns true on success
 */
bool config_set_string(const char* key, const char* value);

/**
 * @brief 获取布尔值 / Get boolean value
 * @param key 键名 / Key name
 * @param default_value 默认值 / Default value
 * @return 布尔值 / Boolean value
 */
bool config_get_bool(const char* key, bool default_value);

/**
 * @brief 获取整数值 / Get integer value
 * @param key 键名 / Key name
 * @param default_value 默认值 / Default value
 * @return 整数值 / Integer value
 */
int64_t config_get_int(const char* key, int64_t default_value);

/**
 * @brief 获取浮点值 / Get float value
 * @param key 键名 / Key name
 * @param default_value 默认值 / Default value
 * @return 浮点值 / Float value
 */
double config_get_float(const char* key, double default_value);

/**
 * @brief 获取字符串值 / Get string value
 * @param key 键名 / Key name
 * @param default_value 默认值 / Default value
 * @return 字符串值 / String value
 */
const char* config_get_string(const char* key, const char* default_value);

/**
 * @brief 检查键是否存在 / Check if key exists
 * @param key 键名 / Key name
 * @return 存在返回true / Returns true if exists
 */
bool config_has_key(const char* key);

/**
 * @brief 删除配置项 / Remove configuration item
 * @param key 键名 / Key name
 * @return 成功返回true / Returns true on success
 */
bool config_remove_key(const char* key);

/**
 * @brief 设置注释 / Set comment
 * @param key 键名 / Key name
 * @param comment 注释内容 / Comment content
 * @return 成功返回true / Returns true on success
 */
bool config_set_comment(const char* key, const char* comment);

/**
 * @brief 获取所有键名 / Get all key names
 * @param count 输出参数，键数量 / Output parameter, key count
 * @return 键名数组 / Key name array
 */
char** config_get_all_keys(int* count);

/**
 * @brief 打印配置信息 / Print configuration
 * @param output 输出流 / Output stream
 */
void config_print(FILE* output);

/**
 * @brief 验证配置 / Validate configuration
 * @return 验证成功返回true / Returns true if validation succeeds
 */
bool config_validate(void);

// 编译器配置专用函数 / Compiler Configuration Specific Functions

/**
 * @brief 加载编译器配置 / Load compiler configuration
 * @param config 配置结构 / Configuration structure
 * @return 成功返回true / Returns true on success
 */
bool config_load_compiler_config(CompilerConfig* config);

/**
 * @brief 保存编译器配置 / Save compiler configuration
 * @param config 配置结构 / Configuration structure
 * @return 成功返回true / Returns true on success
 */
bool config_save_compiler_config(const CompilerConfig* config);

/**
 * @brief 创建默认编译器配置 / Create default compiler configuration
 * @return 默认配置 / Default configuration
 */
CompilerConfig config_create_default_compiler_config(void);

/**
 * @brief 释放编译器配置 / Free compiler configuration
 * @param config 配置结构 / Configuration structure
 */
void config_free_compiler_config(CompilerConfig* config);

// 便利宏定义 / Convenience Macros

#define CONFIG_SET_BOOL(key, value) config_set_bool(key, value)
#define CONFIG_SET_INT(key, value) config_set_int(key, value)
#define CONFIG_SET_FLOAT(key, value) config_set_float(key, value)
#define CONFIG_SET_STRING(key, value) config_set_string(key, value)

#define CONFIG_GET_BOOL(key, default) config_get_bool(key, default)
#define CONFIG_GET_INT(key, default) config_get_int(key, default)
#define CONFIG_GET_FLOAT(key, default) config_get_float(key, default)
#define CONFIG_GET_STRING(key, default) config_get_string(key, default)

// 编译器配置键名常量 / Compiler Configuration Key Constants
#define CONFIG_KEY_ENABLE_COMPILATION     "compiler.enable_compilation"
#define CONFIG_KEY_COMPILATION_MODE       "compiler.compilation_mode"
#define CONFIG_KEY_OPTIMIZATION_LEVEL     "compiler.optimization_level"
#define CONFIG_KEY_TARGET_ARCHITECTURE    "compiler.target_architecture"
#define CONFIG_KEY_SOURCE_DIRECTORY       "paths.source_directory"
#define CONFIG_KEY_OUTPUT_DIRECTORY       "paths.output_directory"
#define CONFIG_KEY_TEMP_DIRECTORY         "paths.temp_directory"
#define CONFIG_KEY_INCLUDE_PATHS          "paths.include_paths"
#define CONFIG_KEY_LIBRARY_PATHS          "paths.library_paths"
#define CONFIG_KEY_ENABLE_DEBUG_INFO      "options.enable_debug_info"
#define CONFIG_KEY_ENABLE_OPTIMIZATION    "options.enable_optimization"
#define CONFIG_KEY_ENABLE_WARNINGS        "options.enable_warnings"
#define CONFIG_KEY_WARNINGS_AS_ERRORS     "options.treat_warnings_as_errors"
#define CONFIG_KEY_MAX_MEMORY_USAGE       "memory.max_memory_usage"
#define CONFIG_KEY_ENABLE_MEMORY_TRACKING "memory.enable_memory_tracking"
#define CONFIG_KEY_ENABLE_LEAK_DETECTION  "memory.enable_leak_detection"
#define CONFIG_KEY_LOG_LEVEL              "logging.log_level"
#define CONFIG_KEY_LOG_FILE               "logging.log_file"
#define CONFIG_KEY_ENABLE_CONSOLE_LOG     "logging.enable_console_log"
#define CONFIG_KEY_ENABLE_FILE_LOG        "logging.enable_file_log"
#define CONFIG_KEY_THREAD_COUNT           "performance.thread_count"
#define CONFIG_KEY_ENABLE_PARALLEL        "performance.enable_parallel_compilation"
#define CONFIG_KEY_COMPILATION_TIMEOUT    "performance.compilation_timeout"

#ifdef __cplusplus
}
#endif

#endif // CONFIG_MANAGER_H
