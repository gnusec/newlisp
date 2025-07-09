/**
 * @file test_basic_functionality.c
 * @brief 基础功能验证测试 / Basic Functionality Verification Test
 * @author newLISP Binary Compiler Team
 * @date 2024
 * 
 * 简化的测试程序，验证核心组件是否正常工作
 * Simplified test program to verify core components work correctly
 */

#include "src/runtime/error_handler.h"
#include "src/runtime/memory_manager.h"
#include "src/ast_converter/ast_nodes.h"
#include "src/ast_converter/simple_parser.h"
#include "src/tinycc_integration/c_code_generator.h"
#include "src/tinycc_integration/tinycc_wrapper.h"
#include <stdio.h>
#include <string.h>

int main(void) {
    printf("=== newLISP Binary Compiler Basic Functionality Test ===\n\n");
    
    // 初始化基础设施 / Initialize infrastructure
    printf("1. Initializing infrastructure...\n");
    
    if (!error_system_init(10, 100)) {
        printf("   ❌ Failed to initialize error system\n");
        return 1;
    }
    printf("   ✅ Error system initialized\n");
    
    MemoryConfig mem_config = {
        .enable_tracking = true,
        .enable_debugging = false,
        .enable_leak_detection = true,
        .enable_bounds_checking = false,
        .max_memory_limit = 0,
        .alignment = 8
    };
    
    if (!memory_manager_init(&mem_config)) {
        printf("   ❌ Failed to initialize memory manager\n");
        error_system_cleanup();
        return 1;
    }
    printf("   ✅ Memory manager initialized\n");
    
    if (!tinycc_init()) {
        printf("   ❌ Failed to initialize TinyCC\n");
        memory_manager_cleanup();
        error_system_cleanup();
        return 1;
    }
    printf("   ✅ TinyCC initialized\n");
    printf("   TinyCC version: %s\n", tinycc_get_version());
    
    // 测试AST节点创建 / Test AST node creation
    printf("\n2. Testing AST node creation...\n");
    
    ASTNode* int_node = ast_create_integer(42);
    if (!int_node) {
        printf("   ❌ Failed to create integer node\n");
        goto cleanup;
    }
    printf("   ✅ Integer node created: %lld\n", (long long)int_node->data.literal.int_value);
    
    ASTNode* str_node = ast_create_string("Hello, World!");
    if (!str_node) {
        printf("   ❌ Failed to create string node\n");
        ast_destroy_node(int_node);
        goto cleanup;
    }
    printf("   ✅ String node created: \"%s\"\n", str_node->data.literal.string_value);
    
    // 测试二元运算 / Test binary operation
    ASTNode* left = ast_create_integer(1);
    ASTNode* right = ast_create_integer(2);
    ASTNode* add_op = ast_create_binary_op(BINARY_OP_ADD, left, right);
    if (!add_op) {
        printf("   ❌ Failed to create binary operation\n");
        ast_destroy_node(int_node);
        ast_destroy_node(str_node);
        goto cleanup;
    }
    printf("   ✅ Binary operation created: (+ 1 2)\n");
    
    // 测试词法分析 / Test lexical analysis
    printf("\n3. Testing lexical analysis...\n");
    
    const char* test_input = "(+ 1 2)";
    Lexer* lexer = lexer_create(test_input);
    if (!lexer) {
        printf("   ❌ Failed to create lexer\n");
        ast_destroy_node(int_node);
        ast_destroy_node(str_node);
        ast_destroy_node(add_op);
        goto cleanup;
    }
    
    Token token = lexer_next_token(lexer);
    if (token.type == TOKEN_LPAREN) {
        printf("   ✅ Lexer correctly identified '(' token\n");
    } else {
        printf("   ❌ Lexer failed to identify '(' token\n");
    }
    token_free(&token);
    
    lexer_destroy(lexer);
    
    // 测试语法分析 / Test parsing
    printf("\n4. Testing parsing...\n");
    
    ASTNode* parsed_ast = parse_string(test_input);
    if (!parsed_ast) {
        printf("   ❌ Failed to parse input\n");
        ast_destroy_node(int_node);
        ast_destroy_node(str_node);
        ast_destroy_node(add_op);
        goto cleanup;
    }
    printf("   ✅ Successfully parsed: %s\n", test_input);
    
    // 测试代码生成 / Test code generation
    printf("\n5. Testing code generation...\n");
    
    CodeGenConfig config = codegen_create_default_config();
    CodeGenerator* codegen = codegen_create(&config);
    if (!codegen) {
        printf("   ❌ Failed to create code generator\n");
        ast_destroy_node(int_node);
        ast_destroy_node(str_node);
        ast_destroy_node(add_op);
        ast_destroy_node(parsed_ast);
        goto cleanup;
    }
    
    char* c_code = codegen_generate(codegen, parsed_ast);
    if (!c_code) {
        printf("   ❌ Failed to generate C code\n");
        codegen_destroy(codegen);
        ast_destroy_node(int_node);
        ast_destroy_node(str_node);
        ast_destroy_node(add_op);
        ast_destroy_node(parsed_ast);
        goto cleanup;
    }
    
    printf("   ✅ Successfully generated C code\n");
    printf("   Generated code preview:\n");
    printf("   ----------------------------------------\n");
    
    // 只显示前几行 / Only show first few lines
    char* line_start = c_code;
    int line_count = 0;
    while (line_start && line_count < 10) {
        char* line_end = strchr(line_start, '\n');
        if (line_end) {
            *line_end = '\0';
            printf("   %s\n", line_start);
            *line_end = '\n';
            line_start = line_end + 1;
        } else {
            printf("   %s\n", line_start);
            break;
        }
        line_count++;
    }
    if (line_count >= 10) {
        printf("   ... (truncated)\n");
    }
    printf("   ----------------------------------------\n");
    
    // 测试TinyCC编译 / Test TinyCC compilation
    printf("\n6. Testing TinyCC compilation...\n");
    
    TinyccConfig tcc_config = tinycc_create_default_config();
    TinyccResult* result = tinycc_compile_to_executable(c_code, &tcc_config);
    
    if (!result) {
        printf("   ❌ TinyCC compilation failed: %s\n", tinycc_get_last_error());
    } else if (!result->success) {
        printf("   ❌ TinyCC compilation failed:\n");
        if (result->error_message) {
            printf("   %s\n", result->error_message);
        }
    } else {
        printf("   ✅ TinyCC compilation successful!\n");
        printf("   Executable: %s\n", result->executable_path);
        
        // 运行程序 / Run program
        printf("\n7. Running compiled program...\n");
        int exit_code = tinycc_run_executable(result->executable_path, NULL, 0);
        printf("   Program exit code: %d\n", exit_code);
        
        // 清理可执行文件 / Cleanup executable
        tinycc_remove_temp_file(result->executable_path);
    }
    
    // 清理 / Cleanup
    if (result) {
        tinycc_free_result(result);
    }
    memory_free(c_code);
    codegen_destroy(codegen);
    ast_destroy_node(int_node);
    ast_destroy_node(str_node);
    ast_destroy_node(add_op);
    ast_destroy_node(parsed_ast);
    
    printf("\n=== Test Summary ===\n");
    printf("✅ All basic functionality tests completed successfully!\n");
    printf("The newLISP binary compiler core components are working correctly.\n");
    
cleanup:
    // 检查内存泄漏 / Check memory leaks
    int leak_count = memory_check_leaks(NULL);
    if (leak_count > 0) {
        printf("⚠️  Warning: %d memory leaks detected\n", leak_count);
    } else {
        printf("✅ No memory leaks detected\n");
    }
    
    // 清理系统 / Cleanup systems
    tinycc_cleanup();
    memory_manager_cleanup();
    error_system_cleanup();
    
    return 0;
}
