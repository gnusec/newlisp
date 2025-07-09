/**
 * @file test_complete_compiler.c
 * @brief 完整编译器测试 / Complete Compiler Testing
 * 
 * ultrathink模式：端到端测试完整编译流程
 * ultrathink mode: End-to-end testing of complete compilation flow
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// 包含所有必要的组件 / Include all necessary components
#include "src/runtime/string_utils.h"
#include "src/runtime/error_handler.h"
#include "src/runtime/memory_manager.h"
#include "src/ast_converter/ast_nodes.h"
#include "src/ast_converter/simple_parser.h"
#include "src/tinycc_integration/c_code_generator.h"
#include "src/tinycc_integration/tinycc_wrapper.h"

// 测试用例结构 / Test case structure
typedef struct {
    const char* name;
    const char* newlisp_code;
    const char* expected_output_pattern;
    bool should_succeed;
} CompilerTestCase;

// 测试用例列表 / Test case list
CompilerTestCase test_cases[] = {
    {
        "Simple Integer",
        "42",
        "42",
        true
    },
    {
        "Simple Addition", 
        "(+ 1 2)",
        "3",
        true
    },
    {
        "Simple Multiplication",
        "(* 3 4)", 
        "12",
        true
    },
    {
        "Nested Expression",
        "(+ (* 2 3) 4)",
        "10", 
        true
    },
    {
        "String Literal",
        "\"Hello\"",
        "Hello",
        true
    },
    {NULL, NULL, NULL, false} // 结束标记 / End marker
};

// 简化的测试框架 / Simplified test framework
#define TEST_ASSERT(condition) \
    do { \
        if (!(condition)) { \
            printf("ASSERTION FAILED: %s at %s:%d\n", #condition, __FILE__, __LINE__); \
            return false; \
        } \
    } while(0)

#define TEST_ASSERT_NOT_NULL(ptr) TEST_ASSERT((ptr) != NULL)

// 测试计数器 / Test counters
static int tests_run = 0;
static int tests_passed = 0;
static int tests_failed = 0;

// 运行单个编译测试 / Run single compilation test
bool run_compiler_test(const CompilerTestCase* test) {
    printf("Testing: %s\n", test->name);
    printf("  newLISP: %s\n", test->newlisp_code);
    
    // 第一步：解析 / Step 1: Parse
    ASTNode* ast = parse_string(test->newlisp_code);
    if (!ast) {
        printf("  ❌ Parsing failed\n");
        return false;
    }
    printf("  ✅ Parsing successful\n");
    
    // 第二步：代码生成 / Step 2: Code generation
    CodeGenConfig config = codegen_create_default_config();
    CodeGenerator* codegen = codegen_create(&config);
    if (!codegen) {
        printf("  ❌ Code generator creation failed\n");
        ast_destroy_node(ast);
        return false;
    }
    
    char* c_code = codegen_generate(codegen, ast);
    if (!c_code) {
        printf("  ❌ C code generation failed\n");
        codegen_destroy(codegen);
        ast_destroy_node(ast);
        return false;
    }
    printf("  ✅ C code generation successful\n");
    
    // 显示生成的C代码片段 / Show generated C code snippet
    printf("  Generated C code (first 3 lines):\n");
    char* line_start = c_code;
    int line_count = 0;
    while (line_start && line_count < 3) {
        char* line_end = strchr(line_start, '\n');
        if (line_end) {
            *line_end = '\0';
            printf("    %s\n", line_start);
            *line_end = '\n';
            line_start = line_end + 1;
        } else {
            printf("    %s\n", line_start);
            break;
        }
        line_count++;
    }
    
    // 第三步：TinyCC编译 / Step 3: TinyCC compilation
    TinyccConfig tcc_config = tinycc_create_default_config();
    TinyccResult* result = tinycc_compile_to_executable(c_code, &tcc_config);
    
    bool success = false;
    
    if (!result) {
        printf("  ❌ TinyCC compilation failed: %s\n", tinycc_get_last_error());
    } else if (!result->success) {
        printf("  ❌ TinyCC compilation failed:\n");
        if (result->error_message) {
            printf("    %s\n", result->error_message);
        }
    } else {
        printf("  ✅ TinyCC compilation successful\n");
        printf("  Executable: %s\n", result->executable_path);
        
        // 第四步：运行程序 / Step 4: Run program
        printf("  Running program: ");
        fflush(stdout);
        
        // 捕获输出 / Capture output
        char output_file[256];
        snprintf(output_file, sizeof(output_file), "/tmp/nlcc_test_output_%d.txt", rand());
        
        char run_cmd[512];
        snprintf(run_cmd, sizeof(run_cmd), "%s > %s 2>&1", result->executable_path, output_file);
        
        int exit_code = system(run_cmd);
        
        // 读取输出 / Read output
        FILE* f = fopen(output_file, "r");
        if (f) {
            char actual_output[256] = {0};
            if (fgets(actual_output, sizeof(actual_output), f)) {
                // 移除换行符 / Remove newline
                char* newline = strchr(actual_output, '\n');
                if (newline) *newline = '\0';
            }
            fclose(f);
            
            printf("%s\n", actual_output);
            printf("  Expected pattern: %s\n", test->expected_output_pattern);
            
            // 简单的模式匹配 / Simple pattern matching
            if (strstr(actual_output, test->expected_output_pattern) != NULL) {
                printf("  ✅ Output matches expected pattern\n");
                success = true;
            } else {
                printf("  ❌ Output doesn't match expected pattern\n");
            }
        } else {
            printf("Failed to read output\n");
        }
        
        // 清理 / Cleanup
        unlink(output_file);
        tinycc_remove_temp_file(result->executable_path);
    }
    
    // 清理 / Cleanup
    if (result) {
        tinycc_free_result(result);
    }
    memory_free(c_code);
    codegen_destroy(codegen);
    ast_destroy_node(ast);
    
    return success;
}

// 主测试函数 / Main test function
int main(void) {
    printf("=== Complete newLISP Compiler Testing ===\n\n");
    
    // 初始化基础设施 / Initialize infrastructure
    if (!error_system_init(10, 100)) {
        printf("Failed to initialize error system\n");
        return 1;
    }
    
    MemoryConfig mem_config = {
        .enable_tracking = true,
        .enable_debugging = false,
        .enable_leak_detection = true,
        .enable_bounds_checking = false,
        .max_memory_limit = 0,
        .alignment = 8
    };
    
    if (!memory_manager_init(&mem_config)) {
        printf("Failed to initialize memory manager\n");
        error_system_cleanup();
        return 1;
    }
    
    if (!tinycc_init()) {
        printf("Failed to initialize TinyCC: %s\n", tinycc_get_last_error());
        memory_manager_cleanup();
        error_system_cleanup();
        return 1;
    }
    
    printf("✅ All systems initialized successfully\n");
    printf("TinyCC version: %s\n\n", tinycc_get_version());
    
    // 运行所有测试 / Run all tests
    for (int i = 0; test_cases[i].name != NULL; i++) {
        tests_run++;
        printf("Test %d: ", i + 1);
        if (run_compiler_test(&test_cases[i])) {
            tests_passed++;
            printf("  ✅ PASSED\n");
        } else {
            tests_failed++;
            printf("  ❌ FAILED\n");
        }
        printf("\n");
    }
    
    // 打印最终结果 / Print final results
    printf("=== Complete Compiler Test Results ===\n");
    printf("Total tests: %d\n", tests_run);
    printf("Passed: %d\n", tests_passed);
    printf("Failed: %d\n", tests_failed);
    printf("Success rate: %.1f%%\n", (double)tests_passed / tests_run * 100.0);
    
    // 检查内存泄漏 / Check memory leaks
    int leak_count = memory_check_leaks(NULL);
    if (leak_count > 0) {
        printf("WARNING: %d memory leaks detected\n", leak_count);
    } else {
        printf("✅ No memory leaks detected\n");
    }
    
    // 清理 / Cleanup
    tinycc_cleanup();
    memory_manager_cleanup();
    error_system_cleanup();
    
    if (tests_passed == tests_run) {
        printf("\n🎉 ALL TESTS PASSED!\n");
        printf("✅ newLISP Binary Compiler is working correctly!\n");
        printf("✅ Complete compilation flow: newLISP → AST → C → Executable → Run\n");
        return 0;
    } else {
        printf("\n⚠️  Some tests failed.\n");
        return 1;
    }
}
