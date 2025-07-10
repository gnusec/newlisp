/**
 * @file tinycc_wrapper.h
 * @brief TinyCC集成包装器 / TinyCC Integration Wrapper
 * @author newLISP Binary Compiler Team
 * @date 2024
 * 
 * 提供TinyCC编译器的简化接口
 * Provides simplified interface for TinyCC compiler
 */

#ifndef TINYCC_WRAPPER_H
#define TINYCC_WRAPPER_H

#include "../runtime/error_handler.h"
#include "../runtime/memory_manager.h"
#include <stdbool.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

// TinyCC编译配置 / TinyCC Compilation Configuration
typedef struct {
    bool enable_debug;        // 启用调试信息 / Enable debug info
    bool enable_optimization; // 启用优化 / Enable optimization
    char* output_file;        // 输出文件名 / Output filename
    char* include_paths;      // 包含路径 / Include paths
    char* library_paths;      // 库路径 / Library paths
    char* libraries;          // 链接库 / Libraries to link
    bool verbose;             // 详细输出 / Verbose output
} TinyccConfig;

// TinyCC编译结果 / TinyCC Compilation Result
typedef struct {
    bool success;             // 编译是否成功 / Whether compilation succeeded
    int exit_code;            // 退出代码 / Exit code
    char* output_message;     // 输出消息 / Output message
    char* error_message;      // 错误消息 / Error message
    char* executable_path;    // 可执行文件路径 / Executable file path
} TinyccResult;

// 核心API函数 / Core API Functions

/**
 * @brief 初始化TinyCC包装器 / Initialize TinyCC wrapper
 * @return 成功返回true / Returns true on success
 */
bool tinycc_init(void);

/**
 * @brief 清理TinyCC包装器 / Cleanup TinyCC wrapper
 */
void tinycc_cleanup(void);

/**
 * @brief 编译C代码到可执行文件 / Compile C code to executable
 * @param c_code C源代码 / C source code
 * @param config 编译配置 / Compilation configuration
 * @return 编译结果 / Compilation result
 */
TinyccResult* tinycc_compile_to_executable(const char* c_code, const TinyccConfig* config);

/**
 * @brief 编译C代码到内存 / Compile C code to memory
 * @param c_code C源代码 / C source code
 * @param config 编译配置 / Compilation configuration
 * @return 编译结果 / Compilation result
 */
TinyccResult* tinycc_compile_to_memory(const char* c_code, const TinyccConfig* config);

/**
 * @brief 从文件编译 / Compile from file
 * @param c_file C源文件路径 / C source file path
 * @param config 编译配置 / Compilation configuration
 * @return 编译结果 / Compilation result
 */
TinyccResult* tinycc_compile_file(const char* c_file, const TinyccConfig* config);

/**
 * @brief 运行编译后的可执行文件 / Run compiled executable
 * @param executable_path 可执行文件路径 / Executable file path
 * @param args 命令行参数 / Command line arguments
 * @param arg_count 参数数量 / Argument count
 * @return 程序退出代码 / Program exit code
 */
int tinycc_run_executable(const char* executable_path, char** args, int arg_count);

/**
 * @brief 创建默认配置 / Create default configuration
 * @return 默认配置 / Default configuration
 */
TinyccConfig tinycc_create_default_config(void);

/**
 * @brief 释放编译结果 / Free compilation result
 * @param result 编译结果 / Compilation result
 */
void tinycc_free_result(TinyccResult* result);

/**
 * @brief 检查TinyCC是否可用 / Check if TinyCC is available
 * @return TinyCC可用返回true / Returns true if TinyCC is available
 */
bool tinycc_is_available(void);

/**
 * @brief 获取TinyCC版本信息 / Get TinyCC version information
 * @return 版本字符串 / Version string
 */
const char* tinycc_get_version(void);

// 便利函数 / Convenience Functions

/**
 * @brief 快速编译和运行 / Quick compile and run
 * @param c_code C源代码 / C source code
 * @param output_file 输出文件名 / Output filename
 * @return 程序退出代码 / Program exit code
 */
int tinycc_quick_compile_and_run(const char* c_code, const char* output_file);

/**
 * @brief 编译到临时文件并运行 / Compile to temporary file and run
 * @param c_code C源代码 / C source code
 * @return 程序退出代码 / Program exit code
 */
int tinycc_compile_and_run_temp(const char* c_code);

/**
 * @brief 验证C代码语法 / Validate C code syntax
 * @param c_code C源代码 / C source code
 * @return 语法正确返回true / Returns true if syntax is correct
 */
bool tinycc_validate_syntax(const char* c_code);

// 错误处理 / Error Handling

/**
 * @brief 获取最后的错误消息 / Get last error message
 * @return 错误消息 / Error message
 */
const char* tinycc_get_last_error(void);

/**
 * @brief 清除错误状态 / Clear error state
 */
void tinycc_clear_error(void);

// 调试和诊断 / Debug and Diagnostics

/**
 * @brief 打印编译结果 / Print compilation result
 * @param result 编译结果 / Compilation result
 * @param output 输出流 / Output stream
 */
void tinycc_print_result(const TinyccResult* result, FILE* output);

/**
 * @brief 保存C代码到临时文件 / Save C code to temporary file
 * @param c_code C源代码 / C source code
 * @param prefix 文件前缀 / File prefix
 * @return 临时文件路径 / Temporary file path
 */
char* tinycc_save_to_temp_file(const char* c_code, const char* prefix);

/**
 * @brief 删除临时文件 / Remove temporary file
 * @param file_path 文件路径 / File path
 * @return 成功返回true / Returns true on success
 */
bool tinycc_remove_temp_file(const char* file_path);

// 高级功能 / Advanced Features

/**
 * @brief 设置编译器选项 / Set compiler options
 * @param config 配置 / Configuration
 * @param option 选项名 / Option name
 * @param value 选项值 / Option value
 * @return 成功返回true / Returns true on success
 */
bool tinycc_set_option(TinyccConfig* config, const char* option, const char* value);

/**
 * @brief 添加包含路径 / Add include path
 * @param config 配置 / Configuration
 * @param path 路径 / Path
 * @return 成功返回true / Returns true on success
 */
bool tinycc_add_include_path(TinyccConfig* config, const char* path);

/**
 * @brief 添加库路径 / Add library path
 * @param config 配置 / Configuration
 * @param path 路径 / Path
 * @return 成功返回true / Returns true on success
 */
bool tinycc_add_library_path(TinyccConfig* config, const char* path);

/**
 * @brief 添加链接库 / Add library to link
 * @param config 配置 / Configuration
 * @param library 库名 / Library name
 * @return 成功返回true / Returns true on success
 */
bool tinycc_add_library(TinyccConfig* config, const char* library);

// 性能和统计 / Performance and Statistics

/**
 * @brief 获取编译时间 / Get compilation time
 * @param result 编译结果 / Compilation result
 * @return 编译时间（毫秒） / Compilation time in milliseconds
 */
double tinycc_get_compile_time(const TinyccResult* result);

/**
 * @brief 获取生成的可执行文件大小 / Get generated executable size
 * @param executable_path 可执行文件路径 / Executable file path
 * @return 文件大小（字节） / File size in bytes
 */
size_t tinycc_get_executable_size(const char* executable_path);

// 测试支持 / Test Support

/**
 * @brief 测试TinyCC基础功能 / Test TinyCC basic functionality
 * @return 测试成功返回true / Returns true if test succeeds
 */
bool tinycc_test_basic(void);

/**
 * @brief 测试编译简单程序 / Test compiling simple program
 * @return 测试成功返回true / Returns true if test succeeds
 */
bool tinycc_test_simple_program(void);

#ifdef __cplusplus
}
#endif

#endif // TINYCC_WRAPPER_H
