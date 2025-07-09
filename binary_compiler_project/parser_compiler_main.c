/**
 * @file parser_compiler_main.c
 * @brief 带解析器的编译器主程序 / Compiler Main Program with Parser
 * 
 * ultrathink模式：在稳定基础上添加解析器功能
 * ultrathink mode: Add parser functionality on stable foundation
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

// 包含基础组件 / Include basic components
#include "src/runtime/error_handler.h"
#include "src/runtime/memory_manager.h"
#include "src/runtime/string_utils.h"

// 包含解析器组件 / Include parser components
#include "src/ast_converter/ast_nodes.h"
#include "src/ast_converter/simple_parser.h"

// 编译器版本信息 / Compiler version information
#define NLCC_VERSION "0.1.0-MVP-Parser"
#define NLCC_BUILD_DATE __DATE__

// 编译器选项 / Compiler options
typedef struct {
    char* input_file;
    char* output_file;
    bool verbose;
    bool show_help;
    bool show_version;
    bool show_ast;
} ParserOptions;

// 函数声明 / Function declarations
static void print_version(void);
static void print_help(const char* program_name);
static bool parse_args(int argc, char* argv[], ParserOptions* options);
static bool init_systems(void);
static void cleanup_systems(void);
static bool compile_newlisp(const char* source_code, const ParserOptions* options);
static void print_ast(const ASTNode* node, int indent);

/**
 * @brief 打印版本信息 / Print version information
 */
static void print_version(void) {
    printf("newLISP Binary Compiler (nlcc) v%s\n", NLCC_VERSION);
    printf("Build Date: %s\n", NLCC_BUILD_DATE);
    printf("With newLISP parser support\n");
}

/**
 * @brief 打印帮助信息 / Print help information
 */
static void print_help(const char* program_name) {
    printf("Usage: %s [OPTIONS] [INPUT_FILE]\n\n", program_name);
    printf("Options:\n");
    printf("  -o, --output FILE     Output file\n");
    printf("  -v, --verbose         Verbose output\n");
    printf("  --show-ast            Show generated AST\n");
    printf("  -h, --help            Show this help\n");
    printf("  --version             Show version\n");
    printf("\nSupported newLISP features:\n");
    printf("  - Integer literals: 42\n");
    printf("  - Arithmetic: (+ 1 2), (* 3 4)\n");
    printf("  - Nested expressions: (+ (* 2 3) 4)\n");
    printf("  - String literals: \"Hello\"\n");
}

/**
 * @brief 解析命令行参数 / Parse command line arguments
 */
static bool parse_args(int argc, char* argv[], ParserOptions* options) {
    static struct option long_options[] = {
        {"output",   required_argument, 0, 'o'},
        {"verbose",  no_argument,       0, 'v'},
        {"show-ast", no_argument,       0, 1001},
        {"help",     no_argument,       0, 'h'},
        {"version",  no_argument,       0, 1000},
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
            case 1001:
                options->show_ast = true;
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
 * @brief 打印AST / Print AST
 */
static void print_ast(const ASTNode* node, int indent) {
    if (!node) return;
    
    // 打印缩进 / Print indentation
    for (int i = 0; i < indent; i++) {
        printf("  ");
    }
    
    // 打印节点信息 / Print node information
    switch (node->type) {
        case AST_NODE_INTEGER:
            printf("INTEGER: %lld\n", (long long)node->data.literal.int_value);
            break;
        case AST_NODE_FLOAT:
            printf("FLOAT: %f\n", node->data.literal.float_value);
            break;
        case AST_NODE_STRING:
            printf("STRING: \"%s\"\n", node->data.literal.string_value ? node->data.literal.string_value : "");
            break;
        case AST_NODE_SYMBOL:
            printf("SYMBOL: %s\n", node->data.symbol.name ? node->data.symbol.name : "");
            break;
        case AST_NODE_LIST:
            printf("LIST (%d elements):\n", node->data.list.count);
            for (int i = 0; i < node->data.list.count; i++) {
                print_ast(node->data.list.elements[i], indent + 1);
            }
            break;
        case AST_NODE_FUNCTION_CALL:
            printf("FUNCTION_CALL:\n");
            if (node->data.call.function) {
                for (int i = 0; i < indent + 1; i++) printf("  ");
                printf("Function:\n");
                print_ast(node->data.call.function, indent + 2);
            }
            if (node->data.call.arg_count > 0) {
                for (int i = 0; i < indent + 1; i++) printf("  ");
                printf("Arguments (%d):\n", node->data.call.arg_count);
                for (int i = 0; i < node->data.call.arg_count; i++) {
                    print_ast(node->data.call.arguments[i], indent + 2);
                }
            }
            break;
        default:
            printf("UNKNOWN_NODE_TYPE: %d\n", node->type);
            break;
    }
}

/**
 * @brief 编译newLISP代码 / Compile newLISP code
 */
static bool compile_newlisp(const char* source_code, const ParserOptions* options) {
    if (options->verbose) {
        printf("Parsing newLISP code...\n");
    }
    
    // 解析源代码 / Parse source code
    ASTNode* ast = parse_string(source_code);
    if (!ast) {
        fprintf(stderr, "❌ Parsing failed\n");
        return false;
    }
    
    if (options->verbose) {
        printf("✅ Parsing successful\n");
    }
    
    // 显示AST / Show AST
    if (options->show_ast) {
        printf("\n=== Generated AST ===\n");
        print_ast(ast, 0);
        printf("===================\n\n");
    }
    
    // 简单的代码生成模拟 / Simple code generation simulation
    if (options->verbose) {
        printf("Generating C code...\n");
    }
    
    printf("Generated C code (simulated):\n");
    printf("#include <stdio.h>\n");
    printf("typedef long long nl_int;\n");
    printf("int main() {\n");
    
    // 简单的代码生成逻辑 / Simple code generation logic
    if (ast->type == AST_NODE_LIST && ast->data.list.count > 0) {
        ASTNode* first = ast->data.list.elements[0];
        if (first->type == AST_NODE_INTEGER) {
            printf("    printf(\"%%lld\\n\", %lldLL);\n", (long long)first->data.literal.int_value);
        } else if (first->type == AST_NODE_FUNCTION_CALL) {
            // 简单的算术表达式处理 / Simple arithmetic expression handling
            printf("    // Function call detected\n");
            printf("    printf(\"Result: TODO\\n\");\n");
        }
    }
    
    printf("    return 0;\n");
    printf("}\n");
    
    if (options->verbose) {
        printf("✅ Code generation completed\n");
    }
    
    // 清理AST / Cleanup AST
    ast_destroy_node(ast);
    
    return true;
}

/**
 * @brief 主函数 / Main function
 */
int main(int argc, char* argv[]) {
    // 解析命令行参数 / Parse command line arguments
    ParserOptions options = {0};
    
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
    
    // 读取输入 / Read input
    char* source_code = NULL;
    bool success = false;
    
    if (options.input_file) {
        // 从文件读取 / Read from file
        FILE* f = fopen(options.input_file, "r");
        if (!f) {
            fprintf(stderr, "Cannot open file: %s\n", options.input_file);
            cleanup_systems();
            return 1;
        }
        
        fseek(f, 0, SEEK_END);
        long size = ftell(f);
        fseek(f, 0, SEEK_SET);
        
        source_code = (char*)memory_alloc(size + 1, MEM_TYPE_TEMP);
        if (source_code) {
            fread(source_code, 1, size, f);
            source_code[size] = '\0';
        }
        fclose(f);
        
    } else {
        // 从标准输入读取 / Read from stdin
        printf("Enter newLISP code (Ctrl+D to finish):\n");
        
        source_code = (char*)memory_alloc(4096, MEM_TYPE_TEMP);
        if (source_code) {
            size_t pos = 0;
            int c;
            while ((c = getchar()) != EOF && pos < 4095) {
                source_code[pos++] = c;
            }
            source_code[pos] = '\0';
        }
    }
    
    // 编译代码 / Compile code
    if (source_code && strlen(source_code) > 0) {
        success = compile_newlisp(source_code, &options);
        memory_free(source_code);
    } else {
        fprintf(stderr, "No input provided\n");
    }
    
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
