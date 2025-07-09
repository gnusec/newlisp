/**
 * @file compiler_main.c
 * @brief newLISP二进制编译器主程序 / newLISP Binary Compiler Main Program
 * @author newLISP Binary Compiler Team
 * @date 2024
 * 
 * MVP版本：实现基础的命令行接口和编译流程
 * MVP version: Implement basic command line interface and compilation flow
 */

#include "../runtime/error_handler.h"
#include "../runtime/memory_manager.h"
#include "../runtime/config_manager.h"
#include "../ast_converter/ast_nodes.h"
#include "../ast_converter/simple_parser.h"
#include "../tinycc_integration/c_code_generator.h"
#include "../tinycc_integration/tinycc_wrapper.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <unistd.h>

// 编译器版本信息 / Compiler version information
#define NLCC_VERSION "0.1.0-MVP"
#define NLCC_BUILD_DATE __DATE__

// 编译器选项 / Compiler options
typedef struct {
    char* input_file;         // 输入文件 / Input file
    char* output_file;        // 输出文件 / Output file
    bool verbose;             // 详细输出 / Verbose output
    bool debug;               // 调试模式 / Debug mode
    bool optimize;            // 优化模式 / Optimization mode
    bool compile_only;        // 仅编译不运行 / Compile only, don't run
    bool run_immediately;     // 编译后立即运行 / Run immediately after compilation
    bool show_ast;            // 显示AST / Show AST
    bool show_c_code;         // 显示生成的C代码 / Show generated C code
    bool keep_temp_files;     // 保留临时文件 / Keep temporary files
} CompilerOptions;

// 函数声明 / Function declarations
static void print_version(void);
static void print_help(const char* program_name);
static bool parse_command_line(int argc, char* argv[], CompilerOptions* options);
static bool initialize_compiler(void);
static void cleanup_compiler(void);
static bool compile_file(const CompilerOptions* options);
static bool compile_string(const char* source_code, const CompilerOptions* options);
static char* read_input_file(const char* filename);

/**
 * @brief 主函数 / Main function
 */
int main(int argc, char* argv[]) {
    printf("newLISP Binary Compiler (nlcc) v%s\n", NLCC_VERSION);
    printf("Build Date: %s\n\n", NLCC_BUILD_DATE);
    
    // 解析命令行参数 / Parse command line arguments
    CompilerOptions options = {0};
    if (!parse_command_line(argc, argv, &options)) {
        return 1;
    }
    
    // 初始化编译器 / Initialize compiler
    if (!initialize_compiler()) {
        fprintf(stderr, "Failed to initialize compiler\n");
        return 1;
    }
    
    // 执行编译 / Execute compilation
    bool success = false;
    
    if (options.input_file) {
        success = compile_file(&options);
    } else {
        // 从标准输入读取 / Read from standard input
        printf("Enter newLISP code (Ctrl+D to finish):\n");
        
        char* source_code = (char*)memory_alloc(4096, MEM_TYPE_TEMP);
        if (source_code) {
            size_t pos = 0;
            int c;
            
            while ((c = getchar()) != EOF && pos < 4095) {
                source_code[pos++] = c;
            }
            source_code[pos] = '\0';
            
            if (pos > 0) {
                success = compile_string(source_code, &options);
            }
            
            memory_free(source_code);
        }
    }
    
    // 清理 / Cleanup
    cleanup_compiler();
    
    return success ? 0 : 1;
}

/**
 * @brief 打印版本信息 / Print version information
 */
static void print_version(void) {
    printf("newLISP Binary Compiler (nlcc) v%s\n", NLCC_VERSION);
    printf("Build Date: %s\n", NLCC_BUILD_DATE);
    printf("TinyCC Version: %s\n", tinycc_get_version());
    printf("\nCopyright (c) 2024 newLISP Binary Compiler Team\n");
    printf("This is free software; see the source for copying conditions.\n");
}

/**
 * @brief 打印帮助信息 / Print help information
 */
static void print_help(const char* program_name) {
    printf("Usage: %s [OPTIONS] [INPUT_FILE]\n\n", program_name);
    
    printf("Options:\n");
    printf("  -o, --output FILE     Output executable file\n");
    printf("  -v, --verbose         Enable verbose output\n");
    printf("  -d, --debug           Enable debug mode\n");
    printf("  -O, --optimize        Enable optimization\n");
    printf("  -c, --compile-only    Compile only, don't run\n");
    printf("  -r, --run             Compile and run immediately\n");
    printf("  --show-ast            Show generated AST\n");
    printf("  --show-c-code         Show generated C code\n");
    printf("  --keep-temp-files     Keep temporary files\n");
    printf("  -h, --help            Show this help message\n");
    printf("  --version             Show version information\n");
    
    printf("\nExamples:\n");
    printf("  %s hello.lsp                    # Compile hello.lsp to executable\n", program_name);
    printf("  %s -o hello hello.lsp           # Compile to specific output file\n", program_name);
    printf("  %s -r hello.lsp                 # Compile and run immediately\n", program_name);
    printf("  %s --show-c-code hello.lsp      # Show generated C code\n", program_name);
    printf("  echo '(+ 1 2)' | %s             # Compile from standard input\n", program_name);
    
    printf("\nSupported newLISP features (MVP):\n");
    printf("  - Basic arithmetic: +, -, *, /, mod\n");
    printf("  - Comparison: =, !=, <, <=, >, >=\n");
    printf("  - Logical: and, or, not\n");
    printf("  - Literals: integers, floats, strings, booleans\n");
    printf("  - Function calls and nested expressions\n");
}

/**
 * @brief 解析命令行参数 / Parse command line arguments
 */
static bool parse_command_line(int argc, char* argv[], CompilerOptions* options) {
    static struct option long_options[] = {
        {"output",          required_argument, 0, 'o'},
        {"verbose",         no_argument,       0, 'v'},
        {"debug",           no_argument,       0, 'd'},
        {"optimize",        no_argument,       0, 'O'},
        {"compile-only",    no_argument,       0, 'c'},
        {"run",             no_argument,       0, 'r'},
        {"show-ast",        no_argument,       0, 1001},
        {"show-c-code",     no_argument,       0, 1002},
        {"keep-temp-files", no_argument,       0, 1003},
        {"help",            no_argument,       0, 'h'},
        {"version",         no_argument,       0, 1000},
        {0, 0, 0, 0}
    };
    
    int c;
    int option_index = 0;
    
    while ((c = getopt_long(argc, argv, "o:vdOcrh", long_options, &option_index)) != -1) {
        switch (c) {
            case 'o':
                options->output_file = optarg;
                break;
            case 'v':
                options->verbose = true;
                break;
            case 'd':
                options->debug = true;
                break;
            case 'O':
                options->optimize = true;
                break;
            case 'c':
                options->compile_only = true;
                break;
            case 'r':
                options->run_immediately = true;
                break;
            case 1001:
                options->show_ast = true;
                break;
            case 1002:
                options->show_c_code = true;
                break;
            case 1003:
                options->keep_temp_files = true;
                break;
            case 'h':
                print_help(argv[0]);
                exit(0);
            case 1000:
                print_version();
                exit(0);
            case '?':
                fprintf(stderr, "Try '%s --help' for more information.\n", argv[0]);
                return false;
            default:
                return false;
        }
    }
    
    // 获取输入文件 / Get input file
    if (optind < argc) {
        options->input_file = argv[optind];
    }
    
    // 验证选项组合 / Validate option combinations
    if (options->compile_only && options->run_immediately) {
        fprintf(stderr, "Error: --compile-only and --run cannot be used together\n");
        return false;
    }
    
    return true;
}

/**
 * @brief 初始化编译器 / Initialize compiler
 */
static bool initialize_compiler(void) {
    // 初始化错误处理系统 / Initialize error handling system
    if (!error_system_init(10, 100)) {
        fprintf(stderr, "Failed to initialize error system\n");
        return false;
    }
    
    // 初始化内存管理系统 / Initialize memory management system
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
    
    // 初始化TinyCC包装器 / Initialize TinyCC wrapper
    if (!tinycc_init()) {
        fprintf(stderr, "Failed to initialize TinyCC: %s\n", tinycc_get_last_error());
        memory_manager_cleanup();
        error_system_cleanup();
        return false;
    }
    
    return true;
}

/**
 * @brief 清理编译器 / Cleanup compiler
 */
static void cleanup_compiler(void) {
    // 检查内存泄漏 / Check memory leaks
    int leak_count = memory_check_leaks(NULL);
    if (leak_count > 0) {
        fprintf(stderr, "Warning: %d memory leaks detected\n", leak_count);
    }
    
    // 清理各个系统 / Cleanup systems
    tinycc_cleanup();
    memory_manager_cleanup();
    error_system_cleanup();
}

/**
 * @brief 读取输入文件 / Read input file
 */
static char* read_input_file(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "Error: Cannot open input file '%s'\n", filename);
        return NULL;
    }
    
    // 获取文件大小 / Get file size
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    if (file_size < 0) {
        fprintf(stderr, "Error: Cannot determine file size for '%s'\n", filename);
        fclose(file);
        return NULL;
    }
    
    // 分配缓冲区 / Allocate buffer
    char* content = (char*)memory_alloc(file_size + 1, MEM_TYPE_TEMP);
    if (!content) {
        fprintf(stderr, "Error: Cannot allocate memory for file content\n");
        fclose(file);
        return NULL;
    }
    
    // 读取文件内容 / Read file content
    size_t bytes_read = fread(content, 1, file_size, file);
    content[bytes_read] = '\0';
    
    fclose(file);
    
    return content;
}

/**
 * @brief 编译文件 / Compile file
 */
static bool compile_file(const CompilerOptions* options) {
    if (!options || !options->input_file) {
        fprintf(stderr, "Error: No input file specified\n");
        return false;
    }

    if (options->verbose) {
        printf("Compiling file: %s\n", options->input_file);
    }

    // 读取输入文件 / Read input file
    char* source_code = read_input_file(options->input_file);
    if (!source_code) {
        return false;
    }

    bool success = compile_string(source_code, options);

    memory_free(source_code);

    return success;
}

/**
 * @brief 编译字符串 / Compile string
 */
static bool compile_string(const char* source_code, const CompilerOptions* options) {
    if (!source_code || !options) {
        return false;
    }

    if (options->verbose) {
        printf("Source code:\n%s\n\n", source_code);
    }

    // 第一步：解析源代码 / Step 1: Parse source code
    if (options->verbose) {
        printf("Step 1: Parsing newLISP source code...\n");
    }

    ASTNode* ast = parse_string(source_code);
    if (!ast) {
        fprintf(stderr, "Error: Failed to parse source code\n");
        return false;
    }

    if (options->show_ast) {
        printf("Generated AST:\n");
        ast_print_node(ast, 0);
        printf("\n");
    }

    // 第二步：生成C代码 / Step 2: Generate C code
    if (options->verbose) {
        printf("Step 2: Generating C code...\n");
    }

    CodeGenConfig codegen_config = codegen_create_default_config();
    codegen_config.enable_debug_info = options->debug;
    codegen_config.enable_optimization = options->optimize;

    CodeGenerator* codegen = codegen_create(&codegen_config);
    if (!codegen) {
        fprintf(stderr, "Error: Failed to create code generator\n");
        ast_destroy_node(ast);
        return false;
    }

    char* c_code = codegen_generate(codegen, ast);
    if (!c_code) {
        fprintf(stderr, "Error: Failed to generate C code\n");
        codegen_destroy(codegen);
        ast_destroy_node(ast);
        return false;
    }

    if (options->show_c_code) {
        printf("Generated C code:\n");
        printf("================\n");
        printf("%s\n", c_code);
        printf("================\n\n");
    }

    // 第三步：编译C代码 / Step 3: Compile C code
    if (options->verbose) {
        printf("Step 3: Compiling C code with TinyCC...\n");
    }

    TinyccConfig tcc_config = tinycc_create_default_config();
    tcc_config.enable_debug = options->debug;
    tcc_config.enable_optimization = options->optimize;
    tcc_config.verbose = options->verbose;

    // 确定输出文件名 / Determine output filename
    if (options->output_file) {
        tcc_config.output_file = (char*)options->output_file;
    } else if (options->input_file) {
        // 从输入文件名生成输出文件名 / Generate output filename from input filename
        char* base_name = safe_strdup(options->input_file);
        char* dot = strrchr(base_name, '.');
        if (dot) *dot = '\0';

        char* output_name = (char*)memory_alloc(strlen(base_name) + 10, MEM_TYPE_TEMP);
        strcpy(output_name, base_name);

        tcc_config.output_file = output_name;
        free(base_name);
    }

    TinyccResult* result = tinycc_compile_to_executable(c_code, &tcc_config);

    bool compilation_success = false;

    if (!result) {
        fprintf(stderr, "Error: TinyCC compilation failed: %s\n", tinycc_get_last_error());
    } else if (!result->success) {
        fprintf(stderr, "Error: TinyCC compilation failed:\n%s\n",
                result->error_message ? result->error_message : "Unknown error");
    } else {
        compilation_success = true;

        if (options->verbose) {
            printf("Compilation successful!\n");
            printf("Executable: %s\n", result->executable_path);

            size_t exe_size = tinycc_get_executable_size(result->executable_path);
            if (exe_size > 0) {
                printf("Executable size: %zu bytes\n", exe_size);
            }
        }

        // 第四步：运行可执行文件（如果需要） / Step 4: Run executable (if needed)
        if (options->run_immediately && !options->compile_only) {
            if (options->verbose) {
                printf("\nStep 4: Running compiled program...\n");
                printf("=====================================\n");
            }

            int exit_code = tinycc_run_executable(result->executable_path, NULL, 0);

            if (options->verbose) {
                printf("=====================================\n");
                printf("Program exit code: %d\n", exit_code);
            }

            // 如果是临时文件且不需要保留，删除它 / If temporary file and not keeping, delete it
            if (!options->output_file && !options->keep_temp_files) {
                tinycc_remove_temp_file(result->executable_path);
            }
        }
    }

    // 清理 / Cleanup
    if (!options->keep_temp_files) {
        // 清理临时C代码文件等 / Cleanup temporary C code files etc.
    }

    if (result) {
        tinycc_free_result(result);
    }

    memory_free(c_code);
    codegen_destroy(codegen);
    ast_destroy_node(ast);

    return compilation_success;
}
