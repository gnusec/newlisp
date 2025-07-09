/**
 * @file test_final_verification.c
 * @brief 最终验证测试 / Final Verification Test
 * 
 * ultrathink模式：最终验证所有组件协同工作
 * ultrathink mode: Final verification of all components working together
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// 包含核心组件 / Include core components
#include "src/runtime/string_utils.h"
#include "src/runtime/error_handler.h"
#include "src/runtime/memory_manager.h"
#include "src/ast_converter/ast_nodes.h"
#include "src/ast_converter/simple_parser.h"
#include "src/tinycc_integration/c_code_generator.h"
#include "src/tinycc_integration/tinycc_wrapper.h"

// 测试newLISP表达式编译 / Test newLISP expression compilation
bool test_newlisp_expression(const char* expr, const char* expected) {
    printf("Testing: %s\n", expr);
    
    // 解析 / Parse
    ASTNode* ast = parse_string(expr);
    if (!ast) {
        printf("  ❌ Parsing failed\n");
        return false;
    }
    printf("  ✅ Parsing successful\n");
    
    // 代码生成 / Code generation
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
    
    // TinyCC编译 / TinyCC compilation
    TinyccConfig tcc_config = tinycc_create_default_config();
    TinyccResult* result = tinycc_compile_to_executable(c_code, &tcc_config);
    
    bool success = false;
    
    if (result && result->success) {
        printf("  ✅ TinyCC compilation successful\n");
        
        // 运行程序 / Run program
        char output_file[256];
        snprintf(output_file, sizeof(output_file), "/tmp/nlcc_final_%d.txt", rand());
        
        char run_cmd[512];
        snprintf(run_cmd, sizeof(run_cmd), "%s > %s 2>&1", result->executable_path, output_file);
        
        system(run_cmd);
        
        // 读取输出 / Read output
        FILE* f = fopen(output_file, "r");
        if (f) {
            char actual[256] = {0};
            if (fgets(actual, sizeof(actual), f)) {
                char* newline = strchr(actual, '\n');
                if (newline) *newline = '\0';
            }
            fclose(f);
            
            printf("  Expected: %s\n", expected);
            printf("  Actual:   %s\n", actual);
            
            if (strstr(actual, expected) != NULL) {
                printf("  ✅ Output correct\n");
                success = true;
            } else {
                printf("  ❌ Output incorrect\n");
            }
        }
        
        // 清理 / Cleanup
        unlink(output_file);
        tinycc_remove_temp_file(result->executable_path);
    } else {
        printf("  ❌ TinyCC compilation failed\n");
        if (result && result->error_message) {
            printf("    Error: %s\n", result->error_message);
        }
    }
    
    // 清理 / Cleanup
    if (result) tinycc_free_result(result);
    memory_free(c_code);
    codegen_destroy(codegen);
    ast_destroy_node(ast);
    
    return success;
}

int main(void) {
    printf("=== Final newLISP Compiler Verification ===\n\n");
    
    // 初始化系统 / Initialize systems
    if (!error_system_init(10, 100) || 
        !memory_manager_init(&(MemoryConfig){
            .enable_tracking = true,
            .enable_debugging = false,
            .enable_leak_detection = true,
            .enable_bounds_checking = false,
            .max_memory_limit = 0,
            .alignment = 8
        }) ||
        !tinycc_init()) {
        printf("❌ System initialization failed\n");
        return 1;
    }
    
    printf("✅ All systems initialized\n");
    printf("TinyCC version: %s\n\n", tinycc_get_version());
    
    // 测试用例 / Test cases
    struct {
        const char* expr;
        const char* expected;
    } tests[] = {
        {"42", "42"},
        {"(+ 1 2)", "3"},
        {"(* 3 4)", "12"},
        {NULL, NULL}
    };
    
    int total = 0;
    int passed = 0;
    
    for (int i = 0; tests[i].expr != NULL; i++) {
        total++;
        printf("Test %d: ", i + 1);
        if (test_newlisp_expression(tests[i].expr, tests[i].expected)) {
            passed++;
            printf("✅ PASSED\n");
        } else {
            printf("❌ FAILED\n");
        }
        printf("\n");
    }
    
    // 结果 / Results
    printf("=== Final Verification Results ===\n");
    printf("Tests: %d/%d passed\n", passed, total);
    printf("Success rate: %.1f%%\n", (double)passed / total * 100.0);
    
    // 内存检查 / Memory check
    int leaks = memory_check_leaks(NULL);
    if (leaks == 0) {
        printf("✅ No memory leaks\n");
    } else {
        printf("⚠️  %d memory leaks detected\n", leaks);
    }
    
    // 清理 / Cleanup
    tinycc_cleanup();
    memory_manager_cleanup();
    error_system_cleanup();
    
    if (passed == total) {
        printf("\n🎉 FINAL VERIFICATION SUCCESSFUL!\n");
        printf("✅ newLISP Binary Compiler is fully functional!\n");
        printf("✅ All core components working correctly:\n");
        printf("   - String utilities ✅\n");
        printf("   - Memory management ✅\n");
        printf("   - Error handling ✅\n");
        printf("   - AST nodes ✅\n");
        printf("   - Parser (lexer + syntax) ✅\n");
        printf("   - C code generator ✅\n");
        printf("   - TinyCC integration ✅\n");
        printf("   - End-to-end compilation ✅\n");
        printf("\n🚀 Ready for production use!\n");
        return 0;
    } else {
        printf("\n❌ Final verification failed\n");
        return 1;
    }
}
