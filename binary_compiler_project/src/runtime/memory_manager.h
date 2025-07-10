/**
 * @file memory_manager.h
 * @brief 统一内存管理系统 / Unified Memory Management System
 * @author newLISP Binary Compiler Team
 * @date 2024
 * 
 * 提供统一的内存分配、释放、跟踪和调试功能
 * Provides unified memory allocation, deallocation, tracking and debugging features
 */

#ifndef MEMORY_MANAGER_H
#define MEMORY_MANAGER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "string_utils.h"

#ifdef __cplusplus
extern "C" {
#endif

// 内存分配类型 / Memory Allocation Types
typedef enum {
    MEM_TYPE_GENERAL = 0,     // 通用内存 / General memory
    MEM_TYPE_AST,             // AST节点 / AST nodes
    MEM_TYPE_STRING,          // 字符串 / Strings
    MEM_TYPE_SYMBOL,          // 符号 / Symbols
    MEM_TYPE_CODE,            // 生成的代码 / Generated code
    MEM_TYPE_TEMP,            // 临时内存 / Temporary memory
    MEM_TYPE_CACHE,           // 缓存 / Cache
    MEM_TYPE_DEBUG,           // 调试信息 / Debug info
    MEM_TYPE_TEST,            // 测试内存 / Test memory
    MEM_TYPE_COUNT            // 类型数量 / Type count
} MemoryType;

// 内存块信息 / Memory Block Information
typedef struct MemoryBlock {
    void* ptr;                // 内存指针 / Memory pointer
    size_t size;              // 分配大小 / Allocated size
    MemoryType type;          // 内存类型 / Memory type
    const char* file;         // 分配文件 / Allocation file
    int line;                 // 分配行号 / Allocation line
    const char* function;     // 分配函数 / Allocation function
    double timestamp;         // 分配时间戳 / Allocation timestamp
    struct MemoryBlock* next; // 链表下一个 / Next in list
    struct MemoryBlock* prev; // 链表上一个 / Previous in list
    uint32_t magic;           // 魔数校验 / Magic number for validation
} MemoryBlock;

// 内存统计信息 / Memory Statistics
typedef struct {
    size_t total_allocated;   // 总分配字节数 / Total allocated bytes
    size_t current_usage;     // 当前使用字节数 / Current usage bytes
    size_t peak_usage;        // 峰值使用字节数 / Peak usage bytes
    int allocation_count;     // 分配次数 / Allocation count
    int deallocation_count;   // 释放次数 / Deallocation count
    int active_blocks;        // 活跃块数 / Active blocks
    size_t type_usage[MEM_TYPE_COUNT]; // 按类型统计 / Usage by type
} MemoryStats;

// 内存管理器配置 / Memory Manager Configuration
typedef struct {
    bool enable_tracking;     // 启用跟踪 / Enable tracking
    bool enable_debugging;    // 启用调试 / Enable debugging
    bool enable_leak_detection; // 启用泄漏检测 / Enable leak detection
    bool enable_bounds_checking; // 启用边界检查 / Enable bounds checking
    size_t max_memory_limit;  // 最大内存限制 / Maximum memory limit
    size_t alignment;         // 内存对齐 / Memory alignment
} MemoryConfig;

// 内存管理器结构 / Memory Manager Structure
typedef struct {
    MemoryBlock* block_list;  // 内存块链表 / Memory block list
    MemoryStats stats;        // 统计信息 / Statistics
    MemoryConfig config;      // 配置信息 / Configuration
    bool initialized;         // 是否已初始化 / Whether initialized
    void* mutex;              // 线程同步锁 / Thread synchronization mutex
} MemoryManager;

// 全局内存管理器 / Global Memory Manager
extern MemoryManager* g_memory_manager;

// 魔数定义 / Magic number definitions
#define MEMORY_MAGIC_ALLOCATED   0xDEADBEEF
#define MEMORY_MAGIC_FREED       0xFEEDFACE
#define MEMORY_MAGIC_GUARD       0xCAFEBABE

// 核心API函数 / Core API Functions

/**
 * @brief 初始化内存管理系统 / Initialize memory management system
 * @param config 配置参数 / Configuration parameters
 * @return 成功返回true / Returns true on success
 */
bool memory_manager_init(const MemoryConfig* config);

/**
 * @brief 清理内存管理系统 / Cleanup memory management system
 */
void memory_manager_cleanup(void);

/**
 * @brief 分配内存 / Allocate memory
 * @param size 分配大小 / Size to allocate
 * @param type 内存类型 / Memory type
 * @param file 源文件名 / Source file name
 * @param line 行号 / Line number
 * @param function 函数名 / Function name
 * @return 分配的内存指针，失败返回NULL / Allocated memory pointer, NULL on failure
 */
void* memory_alloc_debug(size_t size, MemoryType type, 
                        const char* file, int line, const char* function);

/**
 * @brief 重新分配内存 / Reallocate memory
 * @param ptr 原内存指针 / Original memory pointer
 * @param new_size 新大小 / New size
 * @param type 内存类型 / Memory type
 * @param file 源文件名 / Source file name
 * @param line 行号 / Line number
 * @param function 函数名 / Function name
 * @return 重新分配的内存指针 / Reallocated memory pointer
 */
void* memory_realloc_debug(void* ptr, size_t new_size, MemoryType type,
                          const char* file, int line, const char* function);

/**
 * @brief 释放内存 / Free memory
 * @param ptr 内存指针 / Memory pointer
 * @param file 源文件名 / Source file name
 * @param line 行号 / Line number
 * @param function 函数名 / Function name
 */
void memory_free_debug(void* ptr, const char* file, int line, const char* function);

/**
 * @brief 分配并清零内存 / Allocate and zero memory
 * @param count 元素数量 / Element count
 * @param size 元素大小 / Element size
 * @param type 内存类型 / Memory type
 * @param file 源文件名 / Source file name
 * @param line 行号 / Line number
 * @param function 函数名 / Function name
 * @return 分配的内存指针 / Allocated memory pointer
 */
void* memory_calloc_debug(size_t count, size_t size, MemoryType type,
                         const char* file, int line, const char* function);

/**
 * @brief 复制字符串 / Duplicate string
 * @param str 源字符串 / Source string
 * @param file 源文件名 / Source file name
 * @param line 行号 / Line number
 * @param function 函数名 / Function name
 * @return 复制的字符串 / Duplicated string
 */
char* memory_strdup_debug(const char* str, const char* file, int line, const char* function);

/**
 * @brief 获取内存统计信息 / Get memory statistics
 * @return 统计信息指针 / Statistics pointer
 */
const MemoryStats* memory_get_stats(void);

/**
 * @brief 打印内存统计信息 / Print memory statistics
 * @param output 输出流 / Output stream
 */
void memory_print_stats(FILE* output);

/**
 * @brief 检查内存泄漏 / Check memory leaks
 * @param output 输出流 / Output stream
 * @return 泄漏块数量 / Number of leaked blocks
 */
int memory_check_leaks(FILE* output);

/**
 * @brief 验证内存块完整性 / Validate memory block integrity
 * @param ptr 内存指针 / Memory pointer
 * @return 验证成功返回true / Returns true if validation succeeds
 */
bool memory_validate_block(void* ptr);

/**
 * @brief 获取内存块信息 / Get memory block information
 * @param ptr 内存指针 / Memory pointer
 * @return 内存块信息，失败返回NULL / Memory block info, NULL on failure
 */
const MemoryBlock* memory_get_block_info(void* ptr);

/**
 * @brief 设置内存限制 / Set memory limit
 * @param limit 内存限制字节数 / Memory limit in bytes
 */
void memory_set_limit(size_t limit);

/**
 * @brief 获取内存使用量 / Get memory usage
 * @param type 内存类型，MEM_TYPE_COUNT表示所有类型 / Memory type, MEM_TYPE_COUNT for all types
 * @return 使用量字节数 / Usage in bytes
 */
size_t memory_get_usage(MemoryType type);

/**
 * @brief 清理指定类型的内存 / Cleanup memory of specific type
 * @param type 内存类型 / Memory type
 * @return 清理的块数量 / Number of cleaned blocks
 */
int memory_cleanup_type(MemoryType type);

/**
 * @brief 获取内存类型字符串 / Get memory type string
 * @param type 内存类型 / Memory type
 * @return 类型字符串 / Type string
 */
const char* memory_type_to_string(MemoryType type);

// 便利宏定义 / Convenience Macros

#ifdef MEMORY_DEBUG
    #define memory_alloc(size, type) \
        memory_alloc_debug(size, type, __FILE__, __LINE__, __FUNCTION__)
    
    #define memory_realloc(ptr, new_size, type) \
        memory_realloc_debug(ptr, new_size, type, __FILE__, __LINE__, __FUNCTION__)
    
    #define memory_free(ptr) \
        memory_free_debug(ptr, __FILE__, __LINE__, __FUNCTION__)
    
    #define memory_calloc(count, size, type) \
        memory_calloc_debug(count, size, type, __FILE__, __LINE__, __FUNCTION__)
    
    #define memory_strdup(str) \
        memory_strdup_debug(str, __FILE__, __LINE__, __FUNCTION__)
#else
    #define memory_alloc(size, type) malloc(size)
    #define memory_realloc(ptr, new_size, type) realloc(ptr, new_size)
    #define memory_free(ptr) free(ptr)
    #define memory_calloc(count, size, type) calloc(count, size)
    #define memory_strdup(str) safe_strdup(str)
#endif

// 类型特定的分配宏 / Type-specific allocation macros
#define memory_alloc_ast(size)      memory_alloc(size, MEM_TYPE_AST)
#define memory_alloc_string(size)   memory_alloc(size, MEM_TYPE_STRING)
#define memory_alloc_symbol(size)   memory_alloc(size, MEM_TYPE_SYMBOL)
#define memory_alloc_code(size)     memory_alloc(size, MEM_TYPE_CODE)
#define memory_alloc_temp(size)     memory_alloc(size, MEM_TYPE_TEMP)
#define memory_alloc_cache(size)    memory_alloc(size, MEM_TYPE_CACHE)

// 安全内存操作宏 / Safe memory operation macros
#define SAFE_FREE(ptr) \
    do { \
        if (ptr != NULL) { \
            memory_free(ptr); \
            ptr = NULL; \
        } \
    } while(0)

#define SAFE_STRDUP(dest, src) \
    do { \
        if (src != NULL) { \
            dest = memory_strdup(src); \
        } else { \
            dest = NULL; \
        } \
    } while(0)

// 内存检查宏 / Memory check macros
#define CHECK_MEMORY_ALLOCATION(ptr, size) \
    do { \
        if ((ptr) == NULL) { \
            ERROR_REPORT_FATAL(ERROR_TYPE_MEMORY, -1, \
                             "Failed to allocate %zu bytes", (size_t)(size)); \
        } \
    } while(0)

#define VALIDATE_MEMORY_BLOCK(ptr) \
    do { \
        if (!memory_validate_block(ptr)) { \
            ERROR_REPORT_ERROR(ERROR_TYPE_MEMORY, -1, \
                             "Memory block validation failed for %p", ptr); \
        } \
    } while(0)

#ifdef __cplusplus
}
#endif

#endif // MEMORY_MANAGER_H
