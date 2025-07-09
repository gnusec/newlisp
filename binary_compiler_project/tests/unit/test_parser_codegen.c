/**
 * @file test_parser_codegen.c
 * @brief 解析器和代码生成器集成测试 / Parser and Code Generator Integration Tests
 * @author newLISP Binary Compiler Team
 * @date 2024
 * 
 * 测试从newLISP源码到C代码的完整转换流程
 * Tests complete conversion flow from newLISP source to C code
 */

#include "../../src/runtime/error_handler.h"
#include "../../src/runtime/memory_manager.h"
#include "../../src/ast_converter/ast_nodes.h"
#include "../../src/ast_converter/simple_parser.h"
#include "../../src/tinycc_integration/c_code_generator.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>

// 简化的测试框架 / Simplified test framework
#define TEST_ASSERT(condition) \
    do { \
        if (!(condition)) { \
            printf("ASSERTION FAILED: %s at %s:%d\n", #condition, __FILE__, __LINE__); \
            return false; \
        } \
    } while(0)

#define TEST_ASSERT_NOT_NULL(ptr) TEST_ASSERT((ptr) != NULL)
#define TEST_ASSERT_NULL(ptr) TEST_ASSERT((ptr) == NULL)
#define TEST_ASSERT_STRING_CONTAINS(haystack, needle) \
    TEST_ASSERT(strstr((haystack), (needle)) != NULL)

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

// 测试词法分析器基础功能 / Test lexer basic functionality
bool test_lexer_basic(void) {
    const char* input = "(+ 1 2)";
    
    Lexer* lexer = lexer_create(input);
    TEST_ASSERT_NOT_NULL(lexer);
    
    // 测试词法单元序列 / Test token sequence
    Token token1 = lexer_next_token(lexer);
    TEST_ASSERT(token1.type == TOKEN_LPAREN);
    
    Token token2 = lexer_next_token(lexer);
    TEST_ASSERT(token2.type == TOKEN_SYMBOL);
    TEST_ASSERT(strcmp(token2.value, "+") == 0);
    
    Token token3 = lexer_next_token(lexer);
    TEST_ASSERT(token3.type == TOKEN_INTEGER);
    TEST_ASSERT(strcmp(token3.value, "1") == 0);
    
    Token token4 = lexer_next_token(lexer);
    TEST_ASSERT(token4.type == TOKEN_INTEGER);
    TEST_ASSERT(strcmp(token4.value, "2") == 0);
    
    Token token5 = lexer_next_token(lexer);
    TEST_ASSERT(token5.type == TOKEN_RPAREN);
    
    Token token6 = lexer_next_token(lexer);
    TEST_ASSERT(token6.type == TOKEN_EOF);
    
    // 清理 / Cleanup
    token_free(&token1);
    token_free(&token2);
    token_free(&token3);
    token_free(&token4);
    token_free(&token5);
    token_free(&token6);
    lexer_destroy(lexer);
    
    return true;
}

// 测试解析器基础功能 / Test parser basic functionality
bool test_parser_basic(void) {
    const char* input = "(+ 1 2)";
    
    ASTNode* ast = parse_string(input);
    TEST_ASSERT_NOT_NULL(ast);
    
    // 验证AST结构 / Verify AST structure
    TEST_ASSERT(ast->type == AST_NODE_LIST);
    TEST_ASSERT(ast->data.list.count == 1);
    
    ASTNode* expr = ast->data.list.elements[0];
    TEST_ASSERT(expr->type == AST_NODE_FUNCTION_CALL);
    TEST_ASSERT(expr->data.call.function->type == AST_NODE_SYMBOL);
    TEST_ASSERT(strcmp(expr->data.call.function->data.symbol.name, "+") == 0);
    TEST_ASSERT(expr->data.call.arg_count == 2);
    
    ASTNode* arg1 = expr->data.call.arguments[0];
    TEST_ASSERT(arg1->type == AST_NODE_INTEGER);
    TEST_ASSERT(arg1->data.literal.int_value == 1);
    
    ASTNode* arg2 = expr->data.call.arguments[1];
    TEST_ASSERT(arg2->type == AST_NODE_INTEGER);
    TEST_ASSERT(arg2->data.literal.int_value == 2);
    
    // 清理 / Cleanup
    ast_destroy_node(ast);
    
    return true;
}

// 测试代码生成器基础功能 / Test code generator basic functionality
bool test_codegen_basic(void) {
    // 创建简单的AST / Create simple AST
    ASTNode* left = ast_create_integer(1);
    ASTNode* right = ast_create_integer(2);
    ASTNode* add_op = ast_create_binary_op(BINARY_OP_ADD, left, right);
    
    TEST_ASSERT_NOT_NULL(add_op);
    
    // 创建代码生成器 / Create code generator
    CodeGenConfig config = codegen_create_default_config();
    CodeGenerator* codegen = codegen_create(&config);
    TEST_ASSERT_NOT_NULL(codegen);
    
    // 生成表达式代码 / Generate expression code
    char* expr_code = codegen_generate_expression(codegen, add_op);
    TEST_ASSERT_NOT_NULL(expr_code);
    TEST_ASSERT_STRING_CONTAINS(expr_code, "1");
    TEST_ASSERT_STRING_CONTAINS(expr_code, "2");
    TEST_ASSERT_STRING_CONTAINS(expr_code, "+");
    
    printf("Generated expression: %s\n", expr_code);
    
    // 清理 / Cleanup
    memory_free(expr_code);
    codegen_destroy(codegen);
    ast_destroy_node(add_op);
    
    return true;
}

// 测试完整的编译流程 / Test complete compilation flow
bool test_complete_compilation(void) {
    const char* input = "(+ 3 4)";
    
    // 解析 / Parse
    ASTNode* ast = parse_string(input);
    TEST_ASSERT_NOT_NULL(ast);
    
    // 代码生成 / Code generation
    CodeGenConfig config = codegen_create_default_config();
    CodeGenerator* codegen = codegen_create(&config);
    TEST_ASSERT_NOT_NULL(codegen);
    
    char* c_code = codegen_generate(codegen, ast);
    TEST_ASSERT_NOT_NULL(c_code);
    
    // 验证生成的C代码包含必要元素 / Verify generated C code contains necessary elements
    TEST_ASSERT_STRING_CONTAINS(c_code, "#include <stdio.h>");
    TEST_ASSERT_STRING_CONTAINS(c_code, "int main");
    TEST_ASSERT_STRING_CONTAINS(c_code, "3");
    TEST_ASSERT_STRING_CONTAINS(c_code, "4");
    TEST_ASSERT_STRING_CONTAINS(c_code, "+");
    TEST_ASSERT_STRING_CONTAINS(c_code, "return 0");
    
    printf("Generated C code:\n%s\n", c_code);
    
    // 清理 / Cleanup
    memory_free(c_code);
    codegen_destroy(codegen);
    ast_destroy_node(ast);
    
    return true;
}

// 测试字符串字面量 / Test string literals
bool test_string_literals(void) {
    const char* input = "\"hello world\"";
    
    ASTNode* ast = parse_string(input);
    TEST_ASSERT_NOT_NULL(ast);
    
    // 验证AST / Verify AST
    TEST_ASSERT(ast->type == AST_NODE_LIST);
    TEST_ASSERT(ast->data.list.count == 1);
    
    ASTNode* str_node = ast->data.list.elements[0];
    TEST_ASSERT(str_node->type == AST_NODE_STRING);
    TEST_ASSERT(strcmp(str_node->data.literal.string_value, "hello world") == 0);
    
    // 代码生成 / Code generation
    CodeGenConfig config = codegen_create_default_config();
    CodeGenerator* codegen = codegen_create(&config);
    
    char* expr_code = codegen_generate_expression(codegen, str_node);
    TEST_ASSERT_NOT_NULL(expr_code);
    TEST_ASSERT_STRING_CONTAINS(expr_code, "hello world");
    
    printf("Generated string: %s\n", expr_code);
    
    // 清理 / Cleanup
    memory_free(expr_code);
    codegen_destroy(codegen);
    ast_destroy_node(ast);
    
    return true;
}

// 测试嵌套表达式 / Test nested expressions
bool test_nested_expressions(void) {
    const char* input = "(+ (* 2 3) 4)";
    
    ASTNode* ast = parse_string(input);
    TEST_ASSERT_NOT_NULL(ast);
    
    // 代码生成 / Code generation
    CodeGenConfig config = codegen_create_default_config();
    CodeGenerator* codegen = codegen_create(&config);
    
    char* c_code = codegen_generate(codegen, ast);
    TEST_ASSERT_NOT_NULL(c_code);
    
    // 验证包含嵌套运算 / Verify contains nested operations
    TEST_ASSERT_STRING_CONTAINS(c_code, "2");
    TEST_ASSERT_STRING_CONTAINS(c_code, "3");
    TEST_ASSERT_STRING_CONTAINS(c_code, "4");
    TEST_ASSERT_STRING_CONTAINS(c_code, "*");
    TEST_ASSERT_STRING_CONTAINS(c_code, "+");
    
    printf("Generated nested expression C code:\n%s\n", c_code);
    
    // 清理 / Cleanup
    memory_free(c_code);
    codegen_destroy(codegen);
    ast_destroy_node(ast);
    
    return true;
}

// 测试多个表达式 / Test multiple expressions
bool test_multiple_expressions(void) {
    const char* input = "(+ 1 2) (* 3 4) (- 5 1)";
    
    ASTNode* ast = parse_string(input);
    TEST_ASSERT_NOT_NULL(ast);
    
    // 验证解析了多个表达式 / Verify multiple expressions parsed
    TEST_ASSERT(ast->type == AST_NODE_LIST);
    TEST_ASSERT(ast->data.list.count == 3);
    
    // 代码生成 / Code generation
    CodeGenConfig config = codegen_create_default_config();
    CodeGenerator* codegen = codegen_create(&config);
    
    char* c_code = codegen_generate(codegen, ast);
    TEST_ASSERT_NOT_NULL(c_code);
    
    // 验证包含所有运算 / Verify contains all operations
    TEST_ASSERT_STRING_CONTAINS(c_code, "+");
    TEST_ASSERT_STRING_CONTAINS(c_code, "*");
    TEST_ASSERT_STRING_CONTAINS(c_code, "-");
    
    printf("Generated multiple expressions C code:\n%s\n", c_code);
    
    // 清理 / Cleanup
    memory_free(c_code);
    codegen_destroy(codegen);
    ast_destroy_node(ast);
    
    return true;
}

// 测试错误处理 / Test error handling
bool test_error_handling(void) {
    // 测试无效输入 / Test invalid input
    ASTNode* ast1 = parse_string("(+ 1");  // 缺少右括号 / Missing right parenthesis
    TEST_ASSERT_NULL(ast1);
    
    ASTNode* ast2 = parse_string("+ 1 2)"); // 缺少左括号 / Missing left parenthesis
    TEST_ASSERT_NULL(ast2);
    
    // 测试空输入 / Test empty input
    ASTNode* ast3 = parse_string("");
    TEST_ASSERT_NOT_NULL(ast3); // 空程序应该返回空列表 / Empty program should return empty list
    TEST_ASSERT(ast3->type == AST_NODE_LIST);
    TEST_ASSERT(ast3->data.list.count == 0);
    
    if (ast3) ast_destroy_node(ast3);
    
    return true;
}

// 主测试函数 / Main test function
int main(void) {
    printf("=== Parser and Code Generator Integration Tests ===\n\n");
    
    // 初始化基础设施 / Initialize infrastructure
    if (!error_system_init(10, 100)) {
        printf("Failed to initialize error system\n");
        return 1;
    }
    
    MemoryConfig mem_config = {
        .enable_tracking = true,
        .enable_debugging = true,
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
    RUN_TEST(test_lexer_basic);
    RUN_TEST(test_parser_basic);
    RUN_TEST(test_codegen_basic);
    RUN_TEST(test_complete_compilation);
    RUN_TEST(test_string_literals);
    RUN_TEST(test_nested_expressions);
    RUN_TEST(test_multiple_expressions);
    RUN_TEST(test_error_handling);
    
    // 打印测试结果 / Print test results
    printf("\n=== Test Results ===\n");
    printf("Tests run: %d\n", tests_run);
    printf("Tests passed: %d\n", tests_passed);
    printf("Tests failed: %d\n", tests_failed);
    printf("Success rate: %.1f%%\n", 
           tests_run > 0 ? (double)tests_passed / tests_run * 100.0 : 0.0);
    
    // 检查内存泄漏 / Check memory leaks
    int leak_count = memory_check_leaks(stdout);
    if (leak_count > 0) {
        printf("WARNING: %d memory leaks detected\n", leak_count);
    }
    
    // 清理 / Cleanup
    memory_manager_cleanup();
    error_system_cleanup();
    
    return tests_failed == 0 ? 0 : 1;
}
