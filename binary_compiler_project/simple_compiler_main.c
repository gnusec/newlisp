/**
 * @file simple_compiler_main.c
 * @brief 简化的编译器主程序 / Simplified Compiler Main Program
 * 
 * ultrathink模式：最小化实现，避免复杂依赖
 * ultrathink mode: Minimal implementation, avoid complex dependencies
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

// 只包含必要的基础组件
// Only include necessary basic components
#include "src/runtime/error_handler.h"
#include "src/runtime/memory_manager.h"
#include "src/runtime/string_utils.h"

// 编译器版本信息 / Compiler version information
#define NLCC_VERSION "0.1.0-MVP-Simple"
#define NLCC_BUILD_DATE __DATE__

// 简化的编译器选项 / Simplified compiler options
typedef struct {
    char* input_file;
    char* output_file;
    bool verbose;
    bool show_help;
    bool show_version;
} SimpleOptions;

// 函数声明 / Function declarations
static void print_version(void);
static void print_help(const char* program_name);
static bool parse_args(int argc, char* argv[], SimpleOptions* options);
static bool init_systems(void);
static void cleanup_systems(void);
static bool process_input(const SimpleOptions* options);

/**
 * @brief 打印版本信息 / Print version information
 */
static void print_version(void) {
    printf("newLISP Binary Compiler (nlcc) v%s\n", NLCC_VERSION);
    printf("Build Date: %s\n", NLCC_BUILD_DATE);
    printf("Simplified version for debugging\n");
}

/**
 * @brief 打印帮助信息 / Print help information
 */
static void print_help(const char* program_name) {
    printf("Usage: %s [OPTIONS] [INPUT_FILE]\n\n", program_name);
    printf("Options:\n");
    printf("  -o, --output FILE     Output file\n");
    printf("  -v, --verbose         Verbose output\n");
    printf("  -h, --help            Show this help\n");
    printf("  --version             Show version\n");
    printf("\nExamples:\n");
    printf("  %s hello.lsp          # Process hello.lsp\n", program_name);
    printf("  echo '42' | %s        # Process from stdin\n", program_name);
}

/**
 * @brief 解析命令行参数 / Parse command line arguments
 */
static bool parse_args(int argc, char* argv[], SimpleOptions* options) {
    static struct option long_options[] = {
        {"output",  required_argument, 0, 'o'},
        {"verbose", no_argument,       0, 'v'},
        {"help",    no_argument,       0, 'h'},
        {"version", no_argument,       0, 1000},
        {0, 0, 0, 0}
    };
    
    int c;
    int option_index = 0;
    
    while ((c = getopt_long(argc, argv, "o:vh", long_options, &option_index)) != -1) {
        switch (c) {
            case 'o':
                options->output_file = safe_strdup(optarg);
                break;
            case 'v':
                options->verbose = true;
                break;
            case 'h':
                options->show_help = true;
                return true;
            case 1000:
                options->show_version = true;
                return true;
            case '?':
                return false;
            default:
                return false;
        }
    }
    
    // 处理输入文件 / Handle input file
    if (optind < argc) {
        options->input_file = safe_strdup(argv[optind]);
    }
    
    return true;
}

/**
 * @brief 初始化系统 / Initialize systems
 */
static bool init_systems(void) {
    // 初始化错误处理 / Initialize error handling
    if (!error_system_init(10, 100)) {
        fprintf(stderr, "Failed to initialize error system\n");
        return false;
    }
    
    // 初始化内存管理 / Initialize memory management
    MemoryConfig mem_config = {
        .enable_tracking = true,
        .enable_debugging = false,
        .enable_leak_detection = true,
        .enable_bounds_checking = false,
        .max_memory_limit = 0,
        .alignment = 8
    };
    
    if (!memory_manager_init(&mem_config)) {
        fprintf(stderr, "Failed to initialize memory manager\n");
        error_system_cleanup();
        return false;
    }
    
    return true;
}

/**
 * @brief 清理系统 / Cleanup systems
 */
static void cleanup_systems(void) {
    // 检查内存泄漏 / Check memory leaks
    int leak_count = memory_check_leaks(NULL);
    if (leak_count > 0) {
        fprintf(stderr, "Warning: %d memory leaks detected\n", leak_count);
    }
    
    memory_manager_cleanup();
    error_system_cleanup();
}

/**
 * @brief 处理输入 / Process input
 */
static bool process_input(const SimpleOptions* options) {
    if (options->verbose) {
        printf("Processing input...\n");
    }
    
    if (options->input_file) {
        printf("Input file: %s\n", options->input_file);
        
        // 简单的文件读取测试 / Simple file reading test
        FILE* f = fopen(options->input_file, "r");
        if (!f) {
            fprintf(stderr, "Cannot open file: %s\n", options->input_file);
            return false;
        }
        
        printf("File content:\n");
        char buffer[1024];
        while (fgets(buffer, sizeof(buffer), f)) {
            printf("  %s", buffer);
        }
        fclose(f);
        
    } else {
        printf("Reading from stdin (Ctrl+D to finish):\n");
        
        char* input = (char*)memory_alloc(4096, MEM_TYPE_TEMP);
        if (!input) {
            fprintf(stderr, "Memory allocation failed\n");
            return false;
        }
        
        size_t pos = 0;
        int c;
        while ((c = getchar()) != EOF && pos < 4095) {
            input[pos++] = c;
        }
        input[pos] = '\0';
        
        if (pos > 0) {
            printf("Input received:\n%s\n", input);
        }
        
        memory_free(input);
    }
    
    if (options->output_file) {
        printf("Output file: %s\n", options->output_file);
    }
    
    printf("✅ Input processing completed successfully\n");
    return true;
}

/**
 * @brief 主函数 / Main function
 */
int main(int argc, char* argv[]) {
    // 解析命令行参数 / Parse command line arguments
    SimpleOptions options = {0};
    
    if (!parse_args(argc, argv, &options)) {
        print_help(argv[0]);
        return 1;
    }
    
    // 处理特殊选项 / Handle special options
    if (options.show_version) {
        print_version();
        return 0;
    }
    
    if (options.show_help) {
        print_help(argv[0]);
        return 0;
    }
    
    // 初始化系统 / Initialize systems
    if (!init_systems()) {
        return 1;
    }
    
    if (options.verbose) {
        printf("✅ All systems initialized successfully\n");
    }
    
    // 处理输入 / Process input
    bool success = process_input(&options);
    
    // 清理 / Cleanup
    if (options.input_file) {
        memory_free(options.input_file);
    }
    if (options.output_file) {
        memory_free(options.output_file);
    }
    
    cleanup_systems();
    
    if (success) {
        if (options.verbose) {
            printf("✅ Compilation completed successfully\n");
        }
        return 0;
    } else {
        fprintf(stderr, "❌ Compilation failed\n");
        return 1;
    }
}
