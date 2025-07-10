/**
 * @file tinycc_wrapper.c
 * @brief TinyCC集成包装器实现 / TinyCC Integration Wrapper Implementation
 * @author newLISP Binary Compiler Team
 * @date 2024
 * 
 * MVP版本：使用系统调用方式集成TinyCC，避免复杂的API依赖
 * MVP version: Use system call approach to integrate TinyCC, avoid complex API dependencies
 */

#define _GNU_SOURCE
#include "tinycc_wrapper.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <time.h>

// 全局状态 / Global state
static bool g_tinycc_initialized = false;
static char* g_last_error = NULL;
static char* g_temp_dir = "/tmp";

// 内部辅助函数 / Internal helper functions
static void set_last_error(const char* error);
// static char* read_file_content(const char* filename); // 暂时未实现
static bool file_exists(const char* filename);
static char* generate_temp_filename(const char* prefix, const char* suffix);

/**
 * @brief 初始化TinyCC包装器 / Initialize TinyCC wrapper
 */
bool tinycc_init(void) {
    if (g_tinycc_initialized) {
        return true;
    }
    
    // 检查TinyCC是否可用 / Check if TinyCC is available
    if (!tinycc_is_available()) {
        set_last_error("TinyCC not found in system PATH");
        return false;
    }
    
    g_tinycc_initialized = true;
    return true;
}

/**
 * @brief 清理TinyCC包装器 / Cleanup TinyCC wrapper
 */
void tinycc_cleanup(void) {
    if (g_last_error) {
        memory_free(g_last_error);
        g_last_error = NULL;
    }
    
    g_tinycc_initialized = false;
}

/**
 * @brief 检查TinyCC是否可用 / Check if TinyCC is available
 */
bool tinycc_is_available(void) {
    // 尝试运行 tcc --version / Try running tcc --version
    int result = system("tcc --version >/dev/null 2>&1");
    return WEXITSTATUS(result) == 0;
}

/**
 * @brief 获取TinyCC版本信息 / Get TinyCC version information
 */
const char* tinycc_get_version(void) {
    static char version_buffer[256] = {0};
    
    if (version_buffer[0] == '\0') {
        FILE* fp = popen("tcc --version 2>&1", "r");
        if (fp) {
            if (fgets(version_buffer, sizeof(version_buffer), fp)) {
                // 移除换行符 / Remove newline
                char* newline = strchr(version_buffer, '\n');
                if (newline) *newline = '\0';
            }
            pclose(fp);
        }
        
        if (version_buffer[0] == '\0') {
            strcpy(version_buffer, "Unknown");
        }
    }
    
    return version_buffer;
}

/**
 * @brief 创建默认配置 / Create default configuration
 */
TinyccConfig tinycc_create_default_config(void) {
    TinyccConfig config = {0};
    config.enable_debug = false;
    config.enable_optimization = false;
    config.output_file = NULL;
    config.include_paths = NULL;
    config.library_paths = NULL;
    config.libraries = NULL;
    config.verbose = false;
    return config;
}

/**
 * @brief 编译C代码到可执行文件 / Compile C code to executable
 */
TinyccResult* tinycc_compile_to_executable(const char* c_code, const TinyccConfig* config) {
    if (!c_code || !config) {
        set_last_error("Invalid parameters");
        return NULL;
    }
    
    if (!g_tinycc_initialized) {
        set_last_error("TinyCC wrapper not initialized");
        return NULL;
    }
    
    TinyccResult* result = (TinyccResult*)memory_alloc(sizeof(TinyccResult), MEM_TYPE_CODE);
    if (!result) {
        set_last_error("Failed to allocate result structure");
        return NULL;
    }
    
    memset(result, 0, sizeof(TinyccResult));
    
    // 生成临时源文件 / Generate temporary source file
    char* temp_c_file = tinycc_save_to_temp_file(c_code, "nlcc_temp");
    if (!temp_c_file) {
        tinycc_free_result(result);
        return NULL;
    }
    
    // 确定输出文件名 / Determine output filename
    char* output_file = NULL;
    if (config->output_file) {
        output_file = memory_strdup(config->output_file);
    } else {
        output_file = generate_temp_filename("nlcc_out", "");
    }
    
    if (!output_file) {
        tinycc_remove_temp_file(temp_c_file);
        memory_free(temp_c_file);
        tinycc_free_result(result);
        return NULL;
    }
    
    // 构建编译命令 / Build compilation command
    size_t cmd_len = strlen(temp_c_file) + strlen(output_file) + 256;
    char* cmd = (char*)memory_alloc(cmd_len, MEM_TYPE_TEMP);
    if (!cmd) {
        memory_free(output_file);
        tinycc_remove_temp_file(temp_c_file);
        memory_free(temp_c_file);
        tinycc_free_result(result);
        return NULL;
    }
    
    snprintf(cmd, cmd_len, "tcc");
    
    // 添加调试选项 / Add debug options
    if (config->enable_debug) {
        strcat(cmd, " -g");
    }
    
    // 添加优化选项 / Add optimization options
    if (config->enable_optimization) {
        strcat(cmd, " -O2");
    }
    
    // 添加包含路径 / Add include paths
    if (config->include_paths) {
        strcat(cmd, " -I");
        strcat(cmd, config->include_paths);
    }
    
    // 添加库路径 / Add library paths
    if (config->library_paths) {
        strcat(cmd, " -L");
        strcat(cmd, config->library_paths);
    }
    
    // 添加链接库 / Add libraries
    if (config->libraries) {
        strcat(cmd, " -l");
        strcat(cmd, config->libraries);
    }
    
    // 添加输出文件和源文件 / Add output file and source file
    strcat(cmd, " -o ");
    strcat(cmd, output_file);
    strcat(cmd, " ");
    strcat(cmd, temp_c_file);
    
    // 重定向错误输出 / Redirect error output
    strcat(cmd, " 2>&1");
    
    if (config->verbose) {
        printf("Executing: %s\n", cmd);
    }
    
    // 执行编译命令 / Execute compilation command
    clock_t start_time = clock();
    (void)start_time; // 标记未使用，保留用于性能测量
    FILE* fp = popen(cmd, "r");
    
    if (!fp) {
        set_last_error("Failed to execute TinyCC");
        memory_free(cmd);
        memory_free(output_file);
        tinycc_remove_temp_file(temp_c_file);
        memory_free(temp_c_file);
        tinycc_free_result(result);
        return NULL;
    }
    
    // 读取输出 / Read output
    char output_buffer[4096] = {0};
    size_t output_pos = 0;
    char line[256];
    
    while (fgets(line, sizeof(line), fp) && output_pos < sizeof(output_buffer) - 1) {
        size_t line_len = strlen(line);
        if (output_pos + line_len < sizeof(output_buffer)) {
            strcpy(output_buffer + output_pos, line);
            output_pos += line_len;
        }
    }
    
    int exit_code = pclose(fp);
    clock_t end_time = clock();
    (void)end_time; // 标记未使用，保留用于性能测量
    
    // 填充结果 / Fill result
    result->success = (exit_code == 0);
    result->exit_code = exit_code;
    
    if (output_buffer[0] != '\0') {
        if (result->success) {
            result->output_message = memory_strdup(output_buffer);
        } else {
            result->error_message = memory_strdup(output_buffer);
        }
    }
    
    if (result->success && file_exists(output_file)) {
        result->executable_path = memory_strdup(output_file);
    }
    
    // 清理临时文件 / Cleanup temporary files
    tinycc_remove_temp_file(temp_c_file);
    memory_free(temp_c_file);
    memory_free(output_file);
    memory_free(cmd);
    
    return result;
}

/**
 * @brief 运行编译后的可执行文件 / Run compiled executable
 */
int tinycc_run_executable(const char* executable_path, char** args, int arg_count) {
    if (!executable_path || !file_exists(executable_path)) {
        set_last_error("Executable file not found");
        return -1;
    }
    
    // 构建命令 / Build command
    size_t cmd_len = strlen(executable_path) + 256;
    for (int i = 0; i < arg_count; i++) {
        if (args[i]) {
            cmd_len += strlen(args[i]) + 3; // 空格和可能的引号 / Space and possible quotes
        }
    }
    
    char* cmd = (char*)memory_alloc(cmd_len, MEM_TYPE_TEMP);
    if (!cmd) {
        set_last_error("Failed to allocate command buffer");
        return -1;
    }
    
    strcpy(cmd, executable_path);
    
    for (int i = 0; i < arg_count; i++) {
        if (args[i]) {
            strcat(cmd, " ");
            strcat(cmd, args[i]);
        }
    }
    
    // 执行程序 / Execute program
    int exit_code = system(cmd);
    
    memory_free(cmd);
    
    return WEXITSTATUS(exit_code);
}

/**
 * @brief 快速编译和运行 / Quick compile and run
 */
int tinycc_quick_compile_and_run(const char* c_code, const char* output_file) {
    if (!tinycc_init()) {
        return -1;
    }
    
    TinyccConfig config = tinycc_create_default_config();
    if (output_file) {
        config.output_file = (char*)output_file;
    }
    
    TinyccResult* result = tinycc_compile_to_executable(c_code, &config);
    if (!result || !result->success) {
        if (result) {
            if (result->error_message) {
                fprintf(stderr, "Compilation error: %s\n", result->error_message);
            }
            tinycc_free_result(result);
        }
        return -1;
    }
    
    int exit_code = tinycc_run_executable(result->executable_path, NULL, 0);
    
    // 清理可执行文件 / Cleanup executable
    if (!output_file) {
        tinycc_remove_temp_file(result->executable_path);
    }
    
    tinycc_free_result(result);
    
    return exit_code;
}

/**
 * @brief 释放编译结果 / Free compilation result
 */
void tinycc_free_result(TinyccResult* result) {
    if (result) {
        if (result->output_message) {
            memory_free(result->output_message);
        }
        if (result->error_message) {
            memory_free(result->error_message);
        }
        if (result->executable_path) {
            memory_free(result->executable_path);
        }
        memory_free(result);
    }
}

// 内部辅助函数实现 / Internal helper function implementations

/**
 * @brief 设置最后的错误消息 / Set last error message
 */
static void set_last_error(const char* error) {
    if (g_last_error) {
        memory_free(g_last_error);
    }

    g_last_error = error ? memory_strdup(error) : NULL;
}

/**
 * @brief 获取最后的错误消息 / Get last error message
 */
const char* tinycc_get_last_error(void) {
    return g_last_error;
}

/**
 * @brief 清除错误状态 / Clear error state
 */
void tinycc_clear_error(void) {
    if (g_last_error) {
        memory_free(g_last_error);
        g_last_error = NULL;
    }
}

/**
 * @brief 检查文件是否存在 / Check if file exists
 */
static bool file_exists(const char* filename) {
    if (!filename) {
        return false;
    }

    struct stat st;
    return stat(filename, &st) == 0;
}

/**
 * @brief 生成临时文件名 / Generate temporary filename
 */
static char* generate_temp_filename(const char* prefix, const char* suffix) {
    if (!prefix) {
        prefix = "temp";
    }
    if (!suffix) {
        suffix = "";
    }

    static int counter = 0;
    char* filename = (char*)memory_alloc(256, MEM_TYPE_TEMP);
    if (!filename) {
        return NULL;
    }

    snprintf(filename, 256, "%s/%s_%d_%ld%s",
             g_temp_dir, prefix, ++counter, time(NULL), suffix);

    return filename;
}

/**
 * @brief 保存C代码到临时文件 / Save C code to temporary file
 */
char* tinycc_save_to_temp_file(const char* c_code, const char* prefix) {
    if (!c_code) {
        set_last_error("C code cannot be NULL");
        return NULL;
    }

    char* temp_file = generate_temp_filename(prefix, ".c");
    if (!temp_file) {
        set_last_error("Failed to generate temporary filename");
        return NULL;
    }

    FILE* fp = fopen(temp_file, "w");
    if (!fp) {
        set_last_error("Failed to create temporary file");
        memory_free(temp_file);
        return NULL;
    }

    size_t written = fwrite(c_code, 1, strlen(c_code), fp);
    fclose(fp);

    if (written != strlen(c_code)) {
        set_last_error("Failed to write C code to temporary file");
        tinycc_remove_temp_file(temp_file);
        memory_free(temp_file);
        return NULL;
    }

    return temp_file;
}

/**
 * @brief 删除临时文件 / Remove temporary file
 */
bool tinycc_remove_temp_file(const char* file_path) {
    if (!file_path) {
        return false;
    }

    return unlink(file_path) == 0;
}

/**
 * @brief 编译到临时文件并运行 / Compile to temporary file and run
 */
int tinycc_compile_and_run_temp(const char* c_code) {
    return tinycc_quick_compile_and_run(c_code, NULL);
}

/**
 * @brief 验证C代码语法 / Validate C code syntax
 */
bool tinycc_validate_syntax(const char* c_code) {
    if (!tinycc_init()) {
        return false;
    }

    TinyccConfig config = tinycc_create_default_config();
    config.output_file = "/dev/null"; // 不生成实际文件 / Don't generate actual file

    TinyccResult* result = tinycc_compile_to_executable(c_code, &config);
    bool valid = result && result->success;

    tinycc_free_result(result);

    return valid;
}

/**
 * @brief 打印编译结果 / Print compilation result
 */
void tinycc_print_result(const TinyccResult* result, FILE* output) {
    if (!result || !output) {
        return;
    }

    fprintf(output, "=== TinyCC Compilation Result ===\n");
    fprintf(output, "Success: %s\n", result->success ? "Yes" : "No");
    fprintf(output, "Exit Code: %d\n", result->exit_code);

    if (result->executable_path) {
        fprintf(output, "Executable: %s\n", result->executable_path);
    }

    if (result->output_message) {
        fprintf(output, "Output: %s\n", result->output_message);
    }

    if (result->error_message) {
        fprintf(output, "Error: %s\n", result->error_message);
    }

    fprintf(output, "================================\n");
}

/**
 * @brief 获取生成的可执行文件大小 / Get generated executable size
 */
size_t tinycc_get_executable_size(const char* executable_path) {
    if (!executable_path || !file_exists(executable_path)) {
        return 0;
    }

    struct stat st;
    if (stat(executable_path, &st) == 0) {
        return st.st_size;
    }

    return 0;
}

/**
 * @brief 测试TinyCC基础功能 / Test TinyCC basic functionality
 */
bool tinycc_test_basic(void) {
    if (!tinycc_init()) {
        printf("TinyCC initialization failed\n");
        return false;
    }

    printf("TinyCC Version: %s\n", tinycc_get_version());
    printf("TinyCC Available: %s\n", tinycc_is_available() ? "Yes" : "No");

    return tinycc_is_available();
}

/**
 * @brief 测试编译简单程序 / Test compiling simple program
 */
bool tinycc_test_simple_program(void) {
    const char* simple_c_code =
        "#include <stdio.h>\n"
        "int main() {\n"
        "    printf(\"Hello from TinyCC!\\n\");\n"
        "    return 42;\n"
        "}\n";

    printf("Testing simple program compilation...\n");

    TinyccConfig config = tinycc_create_default_config();
    TinyccResult* result = tinycc_compile_to_executable(simple_c_code, &config);

    if (!result) {
        printf("Failed to compile simple program\n");
        return false;
    }

    tinycc_print_result(result, stdout);

    bool success = result->success;

    if (success && result->executable_path) {
        printf("Running compiled program...\n");
        int exit_code = tinycc_run_executable(result->executable_path, NULL, 0);
        printf("Program exit code: %d\n", exit_code);

        // 清理可执行文件 / Cleanup executable
        tinycc_remove_temp_file(result->executable_path);

        success = (exit_code == 42);
    }

    tinycc_free_result(result);

    return success;
}
