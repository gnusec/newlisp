/**
 * @file test_end_to_end.c
 * @brief 端到端集成测试 / End-to-End Integration Tests
 * @author newLISP Binary Compiler Team
 * @date 2024
 * 
 * 测试从newLISP源码到可执行文件的完整编译流程
 * Tests complete compilation flow from newLISP source to executable
 */

#include "../../src/runtime/error_handler.h"
#include "../../src/runtime/memory_manager.h"
#include "../../src/ast_converter/simple_parser.h"
#include "../../src/tinycc_integration/c_code_generator.h"
#include "../../src/tinycc_integration/tinycc_wrapper.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

// 简化的测试框架 / Simplified test framework
#define TEST_ASSERT(condition) \
    do { \
        if (!(condition)) { \
            printf("ASSERTION FAILED: %s at %s:%d\n", #condition, __FILE__, __LINE__); \
            return false; \
        } \
    } while(0)

#define TEST_ASSERT_NOT_NULL(ptr) TEST_ASSERT((ptr) != NULL)
#define TEST_ASSERT_EQUAL(expected, actual) TEST_ASSERT((expected) == (actual))

// 测试计数器 / Test counters
static int tests_run = 0;
static int tests_passed = 0;
static int tests_failed = 0;

// 运行测试宏 / Run test macro
#define RUN_TEST(test_func) \
    do { \
        printf("Running %s... ", #test_func); \
        tests_run++; \
        if (test_func()) { \
            printf("PASS\n"); \
            tests_passed++; \
        } else { \
            printf("FAIL\n"); \
            tests_failed++; \
        } \
    } while(0)

// 辅助函数：完整编译流程 / Helper function: complete compilation flow
static bool compile_and_run_newlisp(const char* source_code, int expected_exit_code) {
    // 第一步：解析 / Step 1: Parse
    ASTNode* ast = parse_string(source_code);
    if (!ast) {
        printf("Parse failed\n");
        return false;
    }
    
    // 第二步：代码生成 / Step 2: Code generation
    CodeGenConfig config = codegen_create_default_config();
    CodeGenerator* codegen = codegen_create(&config);
    if (!codegen) {
        ast_destroy_node(ast);
        return false;
    }
    
    char* c_code = codegen_generate(codegen, ast);
    if (!c_code) {
        codegen_destroy(codegen);
        ast_destroy_node(ast);
        return false;
    }
    
    // 第三步：TinyCC编译 / Step 3: TinyCC compilation
    TinyccConfig tcc_config = tinycc_create_default_config();
    TinyccResult* result = tinycc_compile_to_executable(c_code, &tcc_config);
    
    bool success = false;
    
    if (result && result->success) {
        // 第四步：运行可执行文件 / Step 4: Run executable
        int exit_code = tinycc_run_executable(result->executable_path, NULL, 0);
        success = (exit_code == expected_exit_code);
        
        // 清理可执行文件 / Cleanup executable
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

// 测试简单算术表达式 / Test simple arithmetic expression
bool test_simple_arithmetic(void) {
    const char* source = "(+ 1 2)";
    
    // 解析 / Parse
    ASTNode* ast = parse_string(source);
    TEST_ASSERT_NOT_NULL(ast);
    
    // 代码生成 / Code generation
    CodeGenConfig config = codegen_create_default_config();
    CodeGenerator* codegen = codegen_create(&config);
    TEST_ASSERT_NOT_NULL(codegen);
    
    char* c_code = codegen_generate(codegen, ast);
    TEST_ASSERT_NOT_NULL(c_code);
    
    printf("Generated C code for '%s':\n%s\n", source, c_code);
    
    // TinyCC编译 / TinyCC compilation
    TinyccConfig tcc_config = tinycc_create_default_config();
    TinyccResult* result = tinycc_compile_to_executable(c_code, &tcc_config);
    
    bool success = false;
    if (result) {
        success = result->success;
        if (result->error_message) {
            printf("TinyCC error: %s\n", result->error_message);
        }
        
        if (success && result->executable_path) {
            printf("Executable created: %s\n", result->executable_path);
            
            // 运行程序 / Run program
            int exit_code = tinycc_run_executable(result->executable_path, NULL, 0);
            printf("Program exit code: %d\n", exit_code);
            
            // 清理 / Cleanup
            tinycc_remove_temp_file(result->executable_path);
        }
        
        tinycc_free_result(result);
    }
    
    // 清理 / Cleanup
    memory_free(c_code);
    codegen_destroy(codegen);
    ast_destroy_node(ast);
    
    return success;
}

// 测试嵌套表达式 / Test nested expression
bool test_nested_expression(void) {
    const char* source = "(+ (* 2 3) 4)";
    return compile_and_run_newlisp(source, 0); // 期望正常退出 / Expect normal exit
}

// 测试多个表达式 / Test multiple expressions
bool test_multiple_expressions(void) {
    const char* source = "(+ 1 2) (* 3 4) (- 10 5)";
    return compile_and_run_newlisp(source, 0);
}

// 测试字符串字面量 / Test string literal
bool test_string_literal(void) {
    const char* source = "\"Hello, World!\"";
    return compile_and_run_newlisp(source, 0);
}

// 测试比较运算 / Test comparison operations
bool test_comparison_operations(void) {
    const char* source = "(< 1 2)";
    return compile_and_run_newlisp(source, 0);
}

// 测试布尔值 / Test boolean values
bool test_boolean_values(void) {
    const char* source = "true";
    return compile_and_run_newlisp(source, 0);
}

// 测试浮点数 / Test floating point numbers
bool test_floating_point(void) {
    const char* source = "(+ 1.5 2.5)";
    return compile_and_run_newlisp(source, 0);
}

// 测试错误处理 / Test error handling
bool test_error_handling(void) {
    // 测试语法错误 / Test syntax error
    const char* invalid_source = "(+ 1 2";  // 缺少右括号 / Missing right parenthesis
    
    ASTNode* ast = parse_string(invalid_source);
    TEST_ASSERT(ast == NULL);  // 应该解析失败 / Should fail to parse
    
    return true;
}

// 测试TinyCC可用性 / Test TinyCC availability
bool test_tinycc_availability(void) {
    if (!tinycc_init()) {
        printf("TinyCC not available, skipping compilation tests\n");
        return true; // 不算失败，只是跳过 / Not a failure, just skip
    }
    
    TEST_ASSERT(tinycc_is_available());
    
    printf("TinyCC version: %s\n", tinycc_get_version());
    
    return true;
}

// 测试完整的编译器工作流程 / Test complete compiler workflow
bool test_complete_workflow(void) {
    printf("\n=== Testing Complete Compiler Workflow ===\n");
    
    const char* test_programs[] = {
        "(+ 1 2)",
        "(* 3 4)",
        "(- 10 5)",
        "(/ 8 2)",
        "(+ (* 2 3) 4)",
        "42",
        "3.14",
        "\"test string\"",
        "true",
        "false"
    };
    
    int program_count = sizeof(test_programs) / sizeof(test_programs[0]);
    int successful_compilations = 0;
    
    for (int i = 0; i < program_count; i++) {
        printf("Testing program %d: %s\n", i + 1, test_programs[i]);
        
        if (compile_and_run_newlisp(test_programs[i], 0)) {
            printf("  ✓ Success\n");
            successful_compilations++;
        } else {
            printf("  ✗ Failed\n");
        }
    }
    
    printf("\nWorkflow test results: %d/%d programs compiled successfully\n", 
           successful_compilations, program_count);
    
    // 至少80%的程序应该编译成功 / At least 80% of programs should compile successfully
    return (successful_compilations >= (program_count * 4 / 5));
}

// 性能基准测试 / Performance benchmark test
bool test_performance_benchmark(void) {
    printf("\n=== Performance Benchmark ===\n");
    
    const char* source = "(+ (* 2 3) (- 10 5))";
    
    clock_t start_time = clock();
    
    // 编译10次来测试性能 / Compile 10 times to test performance
    int iterations = 10;
    int successful_iterations = 0;
    
    for (int i = 0; i < iterations; i++) {
        if (compile_and_run_newlisp(source, 0)) {
            successful_iterations++;
        }
    }
    
    clock_t end_time = clock();
    double total_time = ((double)(end_time - start_time)) / CLOCKS_PER_SEC;
    double avg_time = total_time / iterations;
    
    printf("Performance results:\n");
    printf("  Total time: %.3f seconds\n", total_time);
    printf("  Average time per compilation: %.3f seconds\n", avg_time);
    printf("  Successful compilations: %d/%d\n", successful_iterations, iterations);
    
    // 平均编译时间应该少于5秒 / Average compilation time should be less than 5 seconds
    return (avg_time < 5.0) && (successful_iterations >= iterations * 4 / 5);
}

// 主测试函数 / Main test function
int main(void) {
    printf("=== newLISP Binary Compiler End-to-End Tests ===\n\n");
    
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
        return 1;
    }
    
    // 运行测试 / Run tests
    RUN_TEST(test_tinycc_availability);
    RUN_TEST(test_simple_arithmetic);
    RUN_TEST(test_nested_expression);
    RUN_TEST(test_multiple_expressions);
    RUN_TEST(test_string_literal);
    RUN_TEST(test_comparison_operations);
    RUN_TEST(test_boolean_values);
    RUN_TEST(test_floating_point);
    RUN_TEST(test_error_handling);
    RUN_TEST(test_complete_workflow);
    RUN_TEST(test_performance_benchmark);
    
    // 打印测试结果 / Print test results
    printf("\n=== End-to-End Test Results ===\n");
    printf("Tests run: %d\n", tests_run);
    printf("Tests passed: %d\n", tests_passed);
    printf("Tests failed: %d\n", tests_failed);
    printf("Success rate: %.1f%%\n", 
           tests_run > 0 ? (double)tests_passed / tests_run * 100.0 : 0.0);
    
    // 检查内存泄漏 / Check memory leaks
    int leak_count = memory_check_leaks(NULL);
    if (leak_count > 0) {
        printf("WARNING: %d memory leaks detected\n", leak_count);
    }
    
    // 清理 / Cleanup
    memory_manager_cleanup();
    error_system_cleanup();
    
    return tests_failed == 0 ? 0 : 1;
}
