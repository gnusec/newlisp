/**
 * @file memory_manager.c
 * @brief 统一内存管理系统实现 / Unified Memory Management System Implementation
 * @author newLISP Binary Compiler Team
 * @date 2024
 */

#include "memory_manager.h"
#include "error_handler.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <assert.h>

// 添加必要的系统头文件 / Add necessary system headers
#define _POSIX_C_SOURCE 199309L
#include <unistd.h>
#include <sys/time.h>

// 使用我们的安全字符串函数 / Use our safe string functions

// 全局内存管理器实例 / Global memory manager instance
MemoryManager* g_memory_manager = NULL;

// 内存类型字符串映射 / Memory type string mapping
static const char* memory_type_strings[] = {
    "GENERAL",  // MEM_TYPE_GENERAL
    "AST",      // MEM_TYPE_AST
    "STRING",   // MEM_TYPE_STRING
    "SYMBOL",   // MEM_TYPE_SYMBOL
    "CODE",     // MEM_TYPE_CODE
    "TEMP",     // MEM_TYPE_TEMP
    "CACHE",    // MEM_TYPE_CACHE
    "DEBUG"     // MEM_TYPE_DEBUG
};

// 内部辅助函数声明 / Internal helper function declarations
static double get_current_timestamp(void);
static MemoryBlock* find_memory_block(void* ptr);
static void add_memory_block(MemoryBlock* block);
static void remove_memory_block(MemoryBlock* block);
static void update_stats_on_alloc(size_t size, MemoryType type);
static void update_stats_on_free(size_t size, MemoryType type);
static bool check_memory_limit(size_t size);
static void* add_guard_bytes(void* ptr, size_t size);
static void* remove_guard_bytes(void* ptr, size_t* size);

/**
 * @brief 初始化内存管理系统 / Initialize memory management system
 */
bool memory_manager_init(const MemoryConfig* config) {
    // 检查是否已经初始化 / Check if already initialized
    if (g_memory_manager != NULL && g_memory_manager->initialized) {
        ERROR_REPORT_WARNING(ERROR_TYPE_MEMORY, -1, 
                            "Memory manager already initialized");
        return true;
    }
    
    // 分配内存管理器内存 / Allocate memory manager memory
    g_memory_manager = (MemoryManager*)malloc(sizeof(MemoryManager));
    if (g_memory_manager == NULL) {
        ERROR_REPORT_FATAL(ERROR_TYPE_MEMORY, -1, 
                          "Failed to allocate memory for memory manager");
        return false;
    }
    
    // 初始化内存管理器 / Initialize memory manager
    memset(g_memory_manager, 0, sizeof(MemoryManager));
    
    // 设置配置 / Set configuration
    if (config != NULL) {
        g_memory_manager->config = *config;
    } else {
        // 默认配置 / Default configuration
        g_memory_manager->config.enable_tracking = true;
        g_memory_manager->config.enable_debugging = true;
        g_memory_manager->config.enable_leak_detection = true;
        g_memory_manager->config.enable_bounds_checking = false;
        g_memory_manager->config.max_memory_limit = 0; // 无限制 / No limit
        g_memory_manager->config.alignment = sizeof(void*);
    }
    
    g_memory_manager->initialized = true;
    
    ERROR_REPORT_INFO(ERROR_TYPE_MEMORY, 0, "Memory manager initialized successfully");
    return true;
}

/**
 * @brief 清理内存管理系统 / Cleanup memory management system
 */
void memory_manager_cleanup(void) {
    if (g_memory_manager == NULL) {
        return;
    }
    
    // 检查内存泄漏 / Check memory leaks
    if (g_memory_manager->config.enable_leak_detection) {
        int leak_count = memory_check_leaks(stderr);
        if (leak_count > 0) {
            ERROR_REPORT_WARNING(ERROR_TYPE_MEMORY, leak_count, 
                                "Memory leaks detected: %d blocks", leak_count);
        }
    }
    
    // 释放所有剩余的内存块 / Free all remaining memory blocks
    MemoryBlock* current = g_memory_manager->block_list;
    while (current != NULL) {
        MemoryBlock* next = current->next;
        
        if (current->ptr != NULL) {
            free(current->ptr);
        }
        free(current);
        
        current = next;
    }
    
    // 打印最终统计信息 / Print final statistics
    ERROR_REPORT_INFO(ERROR_TYPE_MEMORY, 0, 
                     "Memory manager cleanup: %d allocations, %d deallocations, peak usage: %zu bytes",
                     g_memory_manager->stats.allocation_count,
                     g_memory_manager->stats.deallocation_count,
                     g_memory_manager->stats.peak_usage);
    
    // 释放内存管理器 / Free memory manager
    free(g_memory_manager);
    g_memory_manager = NULL;
}

/**
 * @brief 分配内存 / Allocate memory
 */
void* memory_alloc_debug(size_t size, MemoryType type, 
                        const char* file, int line, const char* function) {
    if (g_memory_manager == NULL || !g_memory_manager->initialized) {
        ERROR_REPORT_ERROR(ERROR_TYPE_MEMORY, -1, "Memory manager not initialized%s", "");
        return malloc(size); // 回退到标准malloc / Fallback to standard malloc
    }
    
    if (size == 0) {
        ERROR_REPORT_WARNING(ERROR_TYPE_MEMORY, -1, "Attempting to allocate 0 bytes%s", "");
        return NULL;
    }
    
    // 检查内存限制 / Check memory limit
    if (!check_memory_limit(size)) {
        ERROR_REPORT_ERROR(ERROR_TYPE_MEMORY, -1, 
                          "Memory allocation would exceed limit: %zu bytes", size);
        return NULL;
    }
    
    // 计算实际分配大小（包含对齐和保护字节） / Calculate actual allocation size
    size_t aligned_size = (size + g_memory_manager->config.alignment - 1) & 
                         ~(g_memory_manager->config.alignment - 1);
    size_t actual_size = aligned_size;
    
    if (g_memory_manager->config.enable_bounds_checking) {
        actual_size += 2 * sizeof(uint32_t); // 前后保护字节 / Guard bytes
    }
    
    // 分配内存 / Allocate memory
    void* ptr = malloc(actual_size);
    if (ptr == NULL) {
        ERROR_REPORT_ERROR(ERROR_TYPE_MEMORY, -1, 
                          "Failed to allocate %zu bytes", actual_size);
        return NULL;
    }
    
    // 添加保护字节 / Add guard bytes
    void* user_ptr = ptr;
    if (g_memory_manager->config.enable_bounds_checking) {
        user_ptr = add_guard_bytes(ptr, size);
    }
    
    // 创建内存块记录 / Create memory block record
    if (g_memory_manager->config.enable_tracking) {
        MemoryBlock* block = (MemoryBlock*)malloc(sizeof(MemoryBlock));
        if (block != NULL) {
            block->ptr = user_ptr;
            block->size = size;
            block->type = type;
            block->file = file;
            block->line = line;
            block->function = function;
            block->timestamp = get_current_timestamp();
            block->magic = MEMORY_MAGIC_ALLOCATED;
            block->next = NULL;
            block->prev = NULL;
            
            add_memory_block(block);
        }
    }
    
    // 更新统计信息 / Update statistics
    update_stats_on_alloc(size, type);
    
    return user_ptr;
}

/**
 * @brief 释放内存 / Free memory
 */
void memory_free_debug(void* ptr, const char* file, int line, const char* function) {
    if (ptr == NULL) {
        return; // 允许释放NULL指针 / Allow freeing NULL pointer
    }
    
    if (g_memory_manager == NULL || !g_memory_manager->initialized) {
        ERROR_REPORT_WARNING(ERROR_TYPE_MEMORY, -1, 
                            "Memory manager not initialized, using standard free");
        free(ptr);
        return;
    }
    
    // 查找内存块记录 / Find memory block record
    MemoryBlock* block = find_memory_block(ptr);
    if (block == NULL) {
        ERROR_REPORT_ERROR(ERROR_TYPE_MEMORY, -1, 
                          "Attempting to free untracked memory at %p (%s:%d)", 
                          ptr, file, line);
        free(ptr); // 仍然尝试释放 / Still try to free
        return;
    }
    
    // 验证内存块 / Validate memory block
    if (block->magic != MEMORY_MAGIC_ALLOCATED) {
        ERROR_REPORT_ERROR(ERROR_TYPE_MEMORY, -1, 
                          "Memory corruption detected: invalid magic number at %p", ptr);
        return;
    }
    
    // 检查边界保护 / Check bounds protection
    if (g_memory_manager->config.enable_bounds_checking) {
        if (!memory_validate_block(ptr)) {
            ERROR_REPORT_ERROR(ERROR_TYPE_MEMORY, -1, 
                              "Memory bounds violation detected at %p", ptr);
        }
    }
    
    // 更新统计信息 / Update statistics
    update_stats_on_free(block->size, block->type);
    
    // 标记为已释放 / Mark as freed
    block->magic = MEMORY_MAGIC_FREED;
    
    // 从链表中移除 / Remove from list
    remove_memory_block(block);
    
    // 释放实际内存 / Free actual memory
    void* actual_ptr = ptr;
    if (g_memory_manager->config.enable_bounds_checking) {
        size_t original_size;
        actual_ptr = remove_guard_bytes(ptr, &original_size);
    }
    
    free(actual_ptr);
    free(block);
}

/**
 * @brief 获取内存统计信息 / Get memory statistics
 */
const MemoryStats* memory_get_stats(void) {
    if (g_memory_manager == NULL) {
        return NULL;
    }
    
    return &g_memory_manager->stats;
}

/**
 * @brief 打印内存统计信息 / Print memory statistics
 */
void memory_print_stats(FILE* output) {
    if (g_memory_manager == NULL || output == NULL) {
        return;
    }
    
    const MemoryStats* stats = &g_memory_manager->stats;
    
    fprintf(output, "\n=== Memory Statistics ===\n");
    fprintf(output, "Total allocated: %zu bytes\n", stats->total_allocated);
    fprintf(output, "Current usage: %zu bytes\n", stats->current_usage);
    fprintf(output, "Peak usage: %zu bytes\n", stats->peak_usage);
    fprintf(output, "Allocation count: %d\n", stats->allocation_count);
    fprintf(output, "Deallocation count: %d\n", stats->deallocation_count);
    fprintf(output, "Active blocks: %d\n", stats->active_blocks);
    
    fprintf(output, "\nUsage by type:\n");
    for (int i = 0; i < MEM_TYPE_COUNT; i++) {
        if (stats->type_usage[i] > 0) {
            fprintf(output, "  %s: %zu bytes\n", 
                   memory_type_to_string((MemoryType)i), 
                   stats->type_usage[i]);
        }
    }
    fprintf(output, "========================\n\n");
}

/**
 * @brief 调试版本的字符串复制 / Debug version of string duplication
 */
char* memory_strdup_debug(const char* str, const char* file, int line, const char* function) {
    if (!str) {
        return NULL;
    }

    size_t len = strlen(str);
    char* dup = memory_alloc_debug(len + 1, MEM_TYPE_STRING, file, line, function);
    if (dup) {
        memcpy(dup, str, len + 1);
    }

    return dup;
}

/**
 * @brief 检查内存泄漏 / Check memory leaks
 */
int memory_check_leaks(FILE* output) {
    if (g_memory_manager == NULL) {
        return 0;
    }
    
    int leak_count = 0;
    MemoryBlock* current = g_memory_manager->block_list;
    
    if (current != NULL && output != NULL) {
        fprintf(output, "\n=== Memory Leak Report ===\n");
    }
    
    while (current != NULL) {
        if (current->magic == MEMORY_MAGIC_ALLOCATED) {
            leak_count++;
            
            if (output != NULL) {
                fprintf(output, "LEAK: %zu bytes at %p (%s:%d in %s) - %s\n",
                       current->size, current->ptr,
                       current->file ? current->file : "unknown",
                       current->line,
                       current->function ? current->function : "unknown",
                       memory_type_to_string(current->type));
            }
        }
        current = current->next;
    }
    
    if (output != NULL) {
        if (leak_count == 0) {
            fprintf(output, "No memory leaks detected.\n");
        } else {
            fprintf(output, "Total leaks: %d blocks\n", leak_count);
        }
        fprintf(output, "=========================\n\n");
    }
    
    return leak_count;
}

/**
 * @brief 获取内存类型字符串 / Get memory type string
 */
const char* memory_type_to_string(MemoryType type) {
    if (type >= 0 && type < MEM_TYPE_COUNT) {
        return memory_type_strings[type];
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
 * @brief 查找内存块 / Find memory block
 */
static MemoryBlock* find_memory_block(void* ptr) {
    if (g_memory_manager == NULL || ptr == NULL) {
        return NULL;
    }

    MemoryBlock* current = g_memory_manager->block_list;
    while (current != NULL) {
        if (current->ptr == ptr) {
            return current;
        }
        current = current->next;
    }

    return NULL;
}

/**
 * @brief 添加内存块到链表 / Add memory block to list
 */
static void add_memory_block(MemoryBlock* block) {
    if (g_memory_manager == NULL || block == NULL) {
        return;
    }

    // 添加到链表头部 / Add to head of list
    block->next = g_memory_manager->block_list;
    block->prev = NULL;

    if (g_memory_manager->block_list != NULL) {
        g_memory_manager->block_list->prev = block;
    }

    g_memory_manager->block_list = block;
    g_memory_manager->stats.active_blocks++;
}

/**
 * @brief 从链表中移除内存块 / Remove memory block from list
 */
static void remove_memory_block(MemoryBlock* block) {
    if (g_memory_manager == NULL || block == NULL) {
        return;
    }

    // 从链表中移除 / Remove from list
    if (block->prev != NULL) {
        block->prev->next = block->next;
    } else {
        g_memory_manager->block_list = block->next;
    }

    if (block->next != NULL) {
        block->next->prev = block->prev;
    }

    g_memory_manager->stats.active_blocks--;
}

/**
 * @brief 更新分配统计信息 / Update allocation statistics
 */
static void update_stats_on_alloc(size_t size, MemoryType type) {
    if (g_memory_manager == NULL) {
        return;
    }

    MemoryStats* stats = &g_memory_manager->stats;

    stats->total_allocated += size;
    stats->current_usage += size;
    stats->allocation_count++;

    if (type < MEM_TYPE_COUNT) {
        stats->type_usage[type] += size;
    }

    // 更新峰值使用量 / Update peak usage
    if (stats->current_usage > stats->peak_usage) {
        stats->peak_usage = stats->current_usage;
    }
}

/**
 * @brief 更新释放统计信息 / Update deallocation statistics
 */
static void update_stats_on_free(size_t size, MemoryType type) {
    if (g_memory_manager == NULL) {
        return;
    }

    MemoryStats* stats = &g_memory_manager->stats;

    stats->current_usage -= size;
    stats->deallocation_count++;

    if (type < MEM_TYPE_COUNT) {
        stats->type_usage[type] -= size;
    }
}

/**
 * @brief 检查内存限制 / Check memory limit
 */
static bool check_memory_limit(size_t size) {
    if (g_memory_manager == NULL) {
        return true;
    }

    // 如果没有设置限制，总是允许 / If no limit set, always allow
    if (g_memory_manager->config.max_memory_limit == 0) {
        return true;
    }

    size_t new_usage = g_memory_manager->stats.current_usage + size;
    return new_usage <= g_memory_manager->config.max_memory_limit;
}

/**
 * @brief 添加保护字节 / Add guard bytes
 */
static void* add_guard_bytes(void* ptr, size_t size) {
    if (ptr == NULL) {
        return NULL;
    }

    uint32_t* guard_ptr = (uint32_t*)ptr;

    // 前保护字节 / Front guard bytes
    *guard_ptr = MEMORY_MAGIC_GUARD;

    // 用户数据区域 / User data area
    void* user_ptr = (char*)ptr + sizeof(uint32_t);

    // 后保护字节 / Rear guard bytes
    uint32_t* rear_guard = (uint32_t*)((char*)user_ptr + size);
    *rear_guard = MEMORY_MAGIC_GUARD;

    return user_ptr;
}

/**
 * @brief 移除保护字节 / Remove guard bytes
 */
static void* remove_guard_bytes(void* ptr, size_t* size) {
    if (ptr == NULL) {
        return NULL;
    }

    // 获取原始指针 / Get original pointer
    void* original_ptr = (char*)ptr - sizeof(uint32_t);

    return original_ptr;
}

/**
 * @brief 验证内存块完整性 / Validate memory block integrity
 */
bool memory_validate_block(void* ptr) {
    if (ptr == NULL || g_memory_manager == NULL) {
        return false;
    }

    // 查找内存块记录 / Find memory block record
    MemoryBlock* block = find_memory_block(ptr);
    if (block == NULL) {
        return false;
    }

    // 检查魔数 / Check magic number
    if (block->magic != MEMORY_MAGIC_ALLOCATED) {
        return false;
    }

    // 如果启用了边界检查，验证保护字节 / If bounds checking enabled, validate guard bytes
    if (g_memory_manager->config.enable_bounds_checking) {
        uint32_t* front_guard = (uint32_t*)((char*)ptr - sizeof(uint32_t));
        uint32_t* rear_guard = (uint32_t*)((char*)ptr + block->size);

        if (*front_guard != MEMORY_MAGIC_GUARD || *rear_guard != MEMORY_MAGIC_GUARD) {
            return false;
        }
    }

    return true;
}
