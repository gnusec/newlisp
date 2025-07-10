/**
 * @file config_manager.c
 * @brief 配置管理系统实现 / Configuration management system implementation
 * 
 * 提供统一的配置管理接口，支持多种数据类型和持久化存储
 * Provides unified configuration management interface with multiple data types and persistence
 */

#include "config_manager.h"
#include "error_handler.h"
#include "memory_manager.h"
#include "string_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

/**
 * @brief 配置项结构 / Configuration item structure
 */
typedef struct ConfigItem {
    char* key;                    // 配置键 / Configuration key
    ConfigType type;              // 值类型 / Value type
    ConfigValue value;            // 配置值 / Configuration value
    struct ConfigItem* next;      // 链表下一项 / Next item in list
} ConfigItem;

/**
 * @brief 全局配置管理器 / Global configuration manager
 */
static struct {
    ConfigItem* items;            // 配置项链表 / Configuration items list
    bool initialized;             // 初始化状态 / Initialization status
    char* config_file_path;       // 配置文件路径 / Configuration file path
} g_config_manager = {NULL, false, NULL};

/**
 * @brief 查找配置项 / Find configuration item
 */
static ConfigItem* find_config_item(const char* key) {
    if (!key) return NULL;
    
    ConfigItem* current = g_config_manager.items;
    while (current) {
        if (strcmp(current->key, key) == 0) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

/**
 * @brief 创建配置项 / Create configuration item
 */
static ConfigItem* create_config_item(const char* key, ConfigType type) {
    if (!key) return NULL;
    
    ConfigItem* item = (ConfigItem*)memory_alloc(sizeof(ConfigItem), MEM_TYPE_CONFIG);
    if (!item) {
        ERROR_REPORT_ERROR(ERROR_TYPE_MEMORY, -1, "Failed to allocate config item%s", "");
        return NULL;
    }
    
    item->key = safe_strdup(key);
    if (!item->key) {
        memory_free(item);
        return NULL;
    }
    
    item->type = type;
    memset(&item->value, 0, sizeof(ConfigValue));
    item->next = NULL;
    
    return item;
}

/**
 * @brief 释放配置项 / Free configuration item
 */
static void free_config_item(ConfigItem* item) {
    if (!item) return;
    
    if (item->key) {
        memory_free(item->key);
    }
    
    if (item->type == CONFIG_TYPE_STRING && item->value.value.string_val) {
        memory_free(item->value.value.string_val);
    }
    
    memory_free(item);
}

/**
 * @brief 初始化配置管理器 / Initialize configuration manager
 */
bool config_manager_init(const char* config_file) {
    if (g_config_manager.initialized) {
        ERROR_REPORT_WARNING(ERROR_TYPE_SYSTEM, -1, "Config manager already initialized%s", "");
        return true;
    }
    
    g_config_manager.items = NULL;
    g_config_manager.initialized = true;
    
    if (config_file) {
        g_config_manager.config_file_path = safe_strdup(config_file);
    }
    
    ERROR_REPORT_INFO(ERROR_TYPE_SYSTEM, 0, "Configuration manager initialized successfully%s", "");
    return true;
}

/**
 * @brief 清理配置管理器 / Cleanup configuration manager
 */
void config_manager_cleanup(void) {
    if (!g_config_manager.initialized) {
        return;
    }
    
    ConfigItem* current = g_config_manager.items;
    while (current) {
        ConfigItem* next = current->next;
        free_config_item(current);
        current = next;
    }
    
    if (g_config_manager.config_file_path) {
        memory_free(g_config_manager.config_file_path);
        g_config_manager.config_file_path = NULL;
    }
    
    g_config_manager.items = NULL;
    g_config_manager.initialized = false;
    
    ERROR_REPORT_INFO(ERROR_TYPE_SYSTEM, 0, "Configuration manager cleaned up%s", "");
}

/**
 * @brief 设置布尔值配置 / Set boolean configuration
 */
bool config_set_bool(const char* key, bool value) {
    if (!g_config_manager.initialized || !key) {
        return false;
    }
    
    ConfigItem* item = find_config_item(key);
    if (!item) {
        item = create_config_item(key, CONFIG_TYPE_BOOL);
        if (!item) return false;
        
        item->next = g_config_manager.items;
        g_config_manager.items = item;
    }
    
    item->type = CONFIG_TYPE_BOOL;
    item->value.type = CONFIG_TYPE_BOOL;
    item->value.value.bool_val = value;
    return true;
}

/**
 * @brief 获取布尔值配置 / Get boolean configuration
 */
bool config_get_bool(const char* key, bool default_value) {
    if (!g_config_manager.initialized || !key) {
        return default_value;
    }
    
    ConfigItem* item = find_config_item(key);
    if (!item || item->type != CONFIG_TYPE_BOOL) {
        return default_value;
    }
    
    return item->value.value.bool_val;
}

/**
 * @brief 设置整数配置 / Set integer configuration
 */
bool config_set_int(const char* key, int64_t value) {
    if (!g_config_manager.initialized || !key) {
        return false;
    }
    
    ConfigItem* item = find_config_item(key);
    if (!item) {
        item = create_config_item(key, CONFIG_TYPE_INT);
        if (!item) return false;
        
        item->next = g_config_manager.items;
        g_config_manager.items = item;
    }
    
    item->type = CONFIG_TYPE_INT;
    item->value.type = CONFIG_TYPE_INT;
    item->value.value.int_val = value;
    return true;
}

/**
 * @brief 获取整数配置 / Get integer configuration
 */
int64_t config_get_int(const char* key, int64_t default_value) {
    if (!g_config_manager.initialized || !key) {
        return default_value;
    }
    
    ConfigItem* item = find_config_item(key);
    if (!item || item->type != CONFIG_TYPE_INT) {
        return default_value;
    }
    
    return item->value.value.int_val;
}

/**
 * @brief 设置浮点数配置 / Set float configuration
 */
bool config_set_float(const char* key, double value) {
    if (!g_config_manager.initialized || !key) {
        return false;
    }
    
    ConfigItem* item = find_config_item(key);
    if (!item) {
        item = create_config_item(key, CONFIG_TYPE_FLOAT);
        if (!item) return false;
        
        item->next = g_config_manager.items;
        g_config_manager.items = item;
    }
    
    item->type = CONFIG_TYPE_FLOAT;
    item->value.type = CONFIG_TYPE_FLOAT;
    item->value.value.float_val = value;
    return true;
}

/**
 * @brief 获取浮点数配置 / Get float configuration
 */
double config_get_float(const char* key, double default_value) {
    if (!g_config_manager.initialized || !key) {
        return default_value;
    }
    
    ConfigItem* item = find_config_item(key);
    if (!item || item->type != CONFIG_TYPE_FLOAT) {
        return default_value;
    }
    
    return item->value.value.float_val;
}

/**
 * @brief 设置字符串配置 / Set string configuration
 */
bool config_set_string(const char* key, const char* value) {
    if (!g_config_manager.initialized || !key || !value) {
        return false;
    }
    
    ConfigItem* item = find_config_item(key);
    if (!item) {
        item = create_config_item(key, CONFIG_TYPE_STRING);
        if (!item) return false;
        
        item->next = g_config_manager.items;
        g_config_manager.items = item;
    } else if (item->type == CONFIG_TYPE_STRING && item->value.value.string_val) {
        memory_free(item->value.value.string_val);
    }

    item->type = CONFIG_TYPE_STRING;
    item->value.type = CONFIG_TYPE_STRING;
    item->value.value.string_val = safe_strdup(value);
    return item->value.value.string_val != NULL;
}

/**
 * @brief 获取字符串配置 / Get string configuration
 */
const char* config_get_string(const char* key, const char* default_value) {
    if (!g_config_manager.initialized || !key) {
        return default_value;
    }
    
    ConfigItem* item = find_config_item(key);
    if (!item || item->type != CONFIG_TYPE_STRING || !item->value.value.string_val) {
        return default_value;
    }

    return item->value.value.string_val;
}

/**
 * @brief 检查配置键是否存在 / Check if configuration key exists
 */
bool config_has_key(const char* key) {
    if (!g_config_manager.initialized || !key) {
        return false;
    }
    
    return find_config_item(key) != NULL;
}

/**
 * @brief 移除配置项 / Remove configuration item
 */
bool config_remove_key(const char* key) {
    if (!g_config_manager.initialized || !key) {
        return false;
    }
    
    ConfigItem* current = g_config_manager.items;
    ConfigItem* prev = NULL;
    
    while (current) {
        if (strcmp(current->key, key) == 0) {
            if (prev) {
                prev->next = current->next;
            } else {
                g_config_manager.items = current->next;
            }
            
            free_config_item(current);
            return true;
        }
        prev = current;
        current = current->next;
    }
    
    return false;
}

/**
 * @brief 创建默认编译器配置 / Create default compiler configuration
 */
CompilerConfig config_create_default_compiler_config(void) {
    CompilerConfig config;
    memset(&config, 0, sizeof(CompilerConfig));

    config.enable_compilation = true;
    config.enable_debug_info = false;
    config.enable_optimization = true;
    config.thread_count = 1;
    config.max_memory_usage = 64 * 1024 * 1024; // 64MB

    config.compilation_mode = safe_strdup("release");
    config.optimization_level = safe_strdup("O2");
    config.target_architecture = safe_strdup("x86_64");

    return config;
}

/**
 * @brief 保存编译器配置 / Save compiler configuration
 */
bool config_save_compiler_config(const CompilerConfig* config) {
    if (!config) return false;

    bool success = true;
    success &= config_set_bool("compiler.enable_compilation", config->enable_compilation);
    success &= config_set_bool("compiler.enable_debug_info", config->enable_debug_info);
    success &= config_set_bool("compiler.enable_optimization", config->enable_optimization);
    success &= config_set_int("compiler.thread_count", config->thread_count);
    success &= config_set_int("compiler.max_memory_usage", (int)config->max_memory_usage);

    if (config->compilation_mode) {
        success &= config_set_string("compiler.compilation_mode", config->compilation_mode);
    }
    if (config->optimization_level) {
        success &= config_set_string("compiler.optimization_level", config->optimization_level);
    }
    if (config->target_architecture) {
        success &= config_set_string("compiler.target_architecture", config->target_architecture);
    }

    return success;
}

/**
 * @brief 加载编译器配置 / Load compiler configuration
 */
bool config_load_compiler_config(CompilerConfig* config) {
    if (!config) return false;

    memset(config, 0, sizeof(CompilerConfig));

    config->enable_compilation = config_get_bool("compiler.enable_compilation", true);
    config->enable_debug_info = config_get_bool("compiler.enable_debug_info", false);
    config->enable_optimization = config_get_bool("compiler.enable_optimization", true);
    config->thread_count = config_get_int("compiler.thread_count", 1);
    config->max_memory_usage = (size_t)config_get_int("compiler.max_memory_usage", 64 * 1024 * 1024);

    const char* mode = config_get_string("compiler.compilation_mode", "release");
    config->compilation_mode = safe_strdup(mode);

    const char* opt_level = config_get_string("compiler.optimization_level", "O2");
    config->optimization_level = safe_strdup(opt_level);

    const char* arch = config_get_string("compiler.target_architecture", "x86_64");
    config->target_architecture = safe_strdup(arch);

    return true;
}

/**
 * @brief 释放编译器配置 / Free compiler configuration
 */
void config_free_compiler_config(CompilerConfig* config) {
    if (!config) return;

    if (config->compilation_mode) {
        memory_free(config->compilation_mode);
        config->compilation_mode = NULL;
    }
    if (config->optimization_level) {
        memory_free(config->optimization_level);
        config->optimization_level = NULL;
    }
    if (config->target_architecture) {
        memory_free(config->target_architecture);
        config->target_architecture = NULL;
    }
}
