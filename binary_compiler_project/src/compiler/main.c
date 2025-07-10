/**
 * @file main.c
 * @brief newLISP二进制编译器主程序 / newLISP Binary Compiler Main Program
 * 
 * 这是newLISP二进制编译器的主入口点，负责：
 * - 命令行参数解析
 * - 基础设施初始化
 * - 编译流程控制
 * - 资源清理
 * 
 * This is the main entry point for the newLISP binary compiler, responsible for:
 * - Command line argument parsing
 * - Infrastructure initialization
 * - Compilation process control
 * - Resource cleanup
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <getopt.h>

// 基础设施组件 / Infrastructure components
#include "../runtime/memory_manager.h"
#include "../runtime/error_handler.h"
#include "../runtime/logger.h"
#include "../runtime/config_manager.h"

// 编译器组件 / Compiler components
#include "../ast_converter/simple_parser.h"
#include "../tinycc_integration/c_code_generator.h"
#include "../tinycc_integration/tinycc_wrapper.h"

/**
 * @brief 编译器版本信息 / Compiler version information
 */
#define COMPILER_VERSION "0.1.0"
#define COMPILER_NAME "newlisp-compiler"

/**
 * @brief 命令行选项 / Command line options
 */
typedef struct {
    char* input_file;           // 输入文件 / Input file
    char* output_file;          // 输出文件 / Output file
    bool verbose;               // 详细输出 / Verbose output
    bool debug;                 // 调试模式 / Debug mode
    bool show_help;             // 显示帮助 / Show help
    bool show_version;          // 显示版本 / Show version
    bool compile_only;          // 仅编译不链接 / Compile only, don't link
    char* optimization_level;   // 优化级别 / Optimization level
} CompilerOptions;

/**
 * @brief 显示帮助信息 / Show help information
 */
void show_help(const char* program_name) {
    printf("Usage: %s [OPTIONS] INPUT_FILE\n\n", program_name);
    printf("newLISP Binary Compiler v%s\n", COMPILER_VERSION);
    printf("Compiles newLISP source code to native binary executables.\n\n");
    
    printf("Options:\n");
    printf("  -o, --output FILE     Output file name (default: a.out)\n");
    printf("  -c, --compile-only    Compile only, don't link\n");
    printf("  -O, --optimize LEVEL  Optimization level (0, 1, 2, 3)\n");
    printf("  -v, --verbose         Enable verbose output\n");
    printf("  -d, --debug           Enable debug mode\n");
    printf("  -h, --help            Show this help message\n");
    printf("  -V, --version         Show version information\n\n");
    
    printf("Examples:\n");
    printf("  %s hello.lsp                    # Compile to a.out\n", program_name);
    printf("  %s -o hello hello.lsp           # Compile to hello\n", program_name);
    printf("  %s -O2 -v program.lsp           # Compile with optimization\n", program_name);
    printf("  %s -c -o program.o program.lsp  # Compile only\n", program_name);
}

/**
 * @brief 显示版本信息 / Show version information
 */
void show_version(void) {
    printf("%s version %s\n", COMPILER_NAME, COMPILER_VERSION);
    printf("Built with TinyCC integration\n");
    printf("Copyright (c) 2024 newLISP Binary Compiler Project\n");
}

/**
 * @brief 解析命令行参数 / Parse command line arguments
 */
bool parse_arguments(int argc, char* argv[], CompilerOptions* options) {
    // 初始化选项 / Initialize options
    memset(options, 0, sizeof(CompilerOptions));
    options->output_file = "a.out";
    options->optimization_level = "1";
    
    static struct option long_options[] = {
        {"output",       required_argument, 0, 'o'},
        {"compile-only", no_argument,       0, 'c'},
        {"optimize",     required_argument, 0, 'O'},
        {"verbose",      no_argument,       0, 'v'},
        {"debug",        no_argument,       0, 'd'},
        {"help",         no_argument,       0, 'h'},
        {"version",      no_argument,       0, 'V'},
        {0, 0, 0, 0}
    };
    
    int option_index = 0;
    int c;
    
    while ((c = getopt_long(argc, argv, "o:cO:vdhV", long_options, &option_index)) != -1) {
        switch (c) {
            case 'o':
                options->output_file = optarg;
                break;
            case 'c':
                options->compile_only = true;
                break;
            case 'O':
                options->optimization_level = optarg;
                break;
            case 'v':
                options->verbose = true;
                break;
            case 'd':
                options->debug = true;
                break;
            case 'h':
                options->show_help = true;
                return true;
            case 'V':
                options->show_version = true;
                return true;
            case '?':
                return false;
            default:
                return false;
        }
    }
    
    // 检查输入文件 / Check input file
    if (optind < argc) {
        options->input_file = argv[optind];
    } else if (!options->show_help && !options->show_version) {
        fprintf(stderr, "Error: No input file specified\n");
        return false;
    }
    
    return true;
}

/**
 * @brief 初始化编译器基础设施 / Initialize compiler infrastructure
 */
bool initialize_infrastructure(const CompilerOptions* options) {
    // 初始化内存管理器 / Initialize memory manager
    MemoryConfig mem_config;
    mem_config.max_memory_limit = 64 * 1024 * 1024;  // 64MB
    mem_config.enable_tracking = true;
    mem_config.enable_debugging = options->debug;
    mem_config.enable_leak_detection = true;
    mem_config.enable_bounds_checking = options->debug;
    mem_config.alignment = 8;
    
    if (!memory_manager_init(&mem_config)) {
        fprintf(stderr, "Error: Failed to initialize memory manager\n");
        return false;
    }
    
    // 初始化错误处理系统 / Initialize error handling system
    if (!error_system_init(100, 1000)) {
        fprintf(stderr, "Error: Failed to initialize error handling system\n");
        return false;
    }
    
    // 初始化日志系统 / Initialize logging system
    LogConfig log_config;
    log_config.min_level = options->verbose ? LOG_LEVEL_DEBUG : LOG_LEVEL_INFO;
    log_config.targets = LOG_TARGET_CONSOLE;
    log_config.format = LOG_FORMAT_SIMPLE;
    log_config.enable_timestamps = false;
    log_config.enable_colors = true;
    log_config.max_file_size = 0;
    log_config.log_file_path = NULL;
    
    if (!logger_init(&log_config)) {
        fprintf(stderr, "Error: Failed to initialize logging system\n");
        return false;
    }
    
    // 初始化配置管理器 / Initialize configuration manager
    if (!config_manager_init(NULL)) {
        fprintf(stderr, "Error: Failed to initialize configuration manager\n");
        return false;
    }
    
    // 设置编译器配置 / Set compiler configuration
    config_set_string("compiler.optimization_level", options->optimization_level);
    config_set_bool("compiler.debug_mode", options->debug);
    config_set_bool("compiler.verbose", options->verbose);
    
    return true;
}

/**
 * @brief 清理编译器基础设施 / Cleanup compiler infrastructure
 */
void cleanup_infrastructure(void) {
    config_manager_cleanup();
    logger_cleanup();
    error_system_cleanup();
    memory_manager_cleanup();
}

/**
 * @brief 编译newLISP源文件 / Compile newLISP source file
 */
bool compile_file(const CompilerOptions* options) {
    bool success = false;
    
    logger_log(LOG_LEVEL_INFO, __FILE__, __LINE__, FUNCTION_NAME, 
               "compiler", "Starting compilation of %s", options->input_file);
    
    // 解析源文件 / Parse source file
    ASTNode* ast = parse_file(options->input_file);
    if (!ast) {
        logger_log(LOG_LEVEL_ERROR, __FILE__, __LINE__, FUNCTION_NAME, 
                   "compiler", "Failed to parse input file: %s", options->input_file);
        return false;
    }
    
    logger_log(LOG_LEVEL_INFO, __FILE__, __LINE__, FUNCTION_NAME, 
               "compiler", "Successfully parsed input file");
    
    // 初始化TinyCC包装器 / Initialize TinyCC wrapper
    if (!tinycc_init()) {
        logger_log(LOG_LEVEL_ERROR, __FILE__, __LINE__, FUNCTION_NAME,
                   "compiler", "Failed to initialize TinyCC wrapper");
        ast_destroy_node(ast);
        return false;
    }

    // 生成C代码 / Generate C code
    CodeGenConfig codegen_config = {0};
    CodeGenerator* codegen = codegen_create(&codegen_config);
    if (!codegen) {
        logger_log(LOG_LEVEL_ERROR, __FILE__, __LINE__, FUNCTION_NAME,
                   "compiler", "Failed to create code generator");
        ast_destroy_node(ast);
        return false;
    }

    char* c_code = codegen_generate(codegen, ast);
    if (!c_code) {
        logger_log(LOG_LEVEL_ERROR, __FILE__, __LINE__, FUNCTION_NAME,
                   "compiler", "Failed to generate C code");
        codegen_destroy(codegen);
        ast_destroy_node(ast);
        return false;
    }
    
    logger_log(LOG_LEVEL_INFO, __FILE__, __LINE__, FUNCTION_NAME, 
               "compiler", "Successfully generated C code");
    
    if (options->verbose) {
        printf("Generated C code:\n%s\n", c_code);
    }
    
    // 编译C代码 / Compile C code
    TinyccConfig tinycc_config;
    tinycc_config.output_file = options->output_file;
    tinycc_config.enable_optimization = (atoi(options->optimization_level) > 0);
    tinycc_config.enable_debug = options->debug;
    tinycc_config.verbose = options->verbose;
    tinycc_config.include_paths = NULL;
    tinycc_config.library_paths = NULL;
    tinycc_config.libraries = "m"; // 链接数学库 / Link math library

    TinyccResult* result = tinycc_compile_to_executable(c_code, &tinycc_config);
    if (result && result->success) {
        logger_log(LOG_LEVEL_INFO, __FILE__, __LINE__, FUNCTION_NAME,
                   "compiler", "Successfully compiled to %s", options->output_file);
        success = true;
    } else {
        const char* error_msg = "Unknown error";
        if (result) {
            if (result->error_message) {
                error_msg = result->error_message;
            } else if (result->output_message) {
                error_msg = result->output_message;
            } else {
                error_msg = "TinyCC compilation failed with no error message";
            }
        } else {
            error_msg = "Failed to create TinyCC result";
        }
        logger_log(LOG_LEVEL_ERROR, __FILE__, __LINE__, FUNCTION_NAME,
                   "compiler", "Compilation failed: %s", error_msg);
    }

    // 清理资源 / Cleanup resources
    if (result) {
        memory_free(result);
    }
    memory_free(c_code);
    codegen_destroy(codegen);
    ast_destroy_node(ast);
    
    return success;
}

/**
 * @brief 主函数 / Main function
 */
int main(int argc, char* argv[]) {
    CompilerOptions options;
    int exit_code = 0;
    
    // 解析命令行参数 / Parse command line arguments
    if (!parse_arguments(argc, argv, &options)) {
        show_help(argv[0]);
        return 1;
    }
    
    // 处理帮助和版本选项 / Handle help and version options
    if (options.show_help) {
        show_help(argv[0]);
        return 0;
    }
    
    if (options.show_version) {
        show_version();
        return 0;
    }
    
    // 初始化基础设施 / Initialize infrastructure
    if (!initialize_infrastructure(&options)) {
        return 1;
    }
    
    // 编译文件 / Compile file
    if (!compile_file(&options)) {
        exit_code = 1;
    }
    
    // 检查内存泄漏 / Check memory leaks
    int leak_count = memory_check_leaks(NULL);
    if (leak_count > 0) {
        fprintf(stderr, "Warning: %d memory leaks detected\n", leak_count);
    }
    
    // 清理基础设施 / Cleanup infrastructure
    cleanup_infrastructure();
    
    return exit_code;
}
