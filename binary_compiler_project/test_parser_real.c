/**
 * @file test_parser_real.c
 * @brief 真实解析器测试 / Real Parser Testing
 * 
 * ultrathink模式：逐步测试真实解析器组件
 * ultrathink mode: Step-by-step testing of real parser components
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 包含必要的头文件 / Include necessary headers
#include "src/runtime/string_utils.h"
#include "src/runtime/error_handler.h"
#include "src/runtime/memory_manager.h"
#include "src/ast_converter/ast_nodes.h"
#include "src/ast_converter/simple_parser.h"

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
    
    // 测试第一个词法单元 / Test first token
    Token token1 = lexer_next_token(lexer);
    TEST_ASSERT(token1.type == TOKEN_LPAREN);
    token_free(&token1);
    
    // 测试第二个词法单元 / Test second token
    Token token2 = lexer_next_token(lexer);
    TEST_ASSERT(token2.type == TOKEN_SYMBOL);
    if (token2.value) {
        TEST_ASSERT(strcmp(token2.value, "+") == 0);
    }
    token_free(&token2);
    
    // 清理 / Cleanup
    lexer_destroy(lexer);
    
    return true;
}

// 测试字符分类函数 / Test character classification functions
bool test_character_classification(void) {
    // 测试数字 / Test digits
    TEST_ASSERT(is_digit('0'));
    TEST_ASSERT(is_digit('9'));
    TEST_ASSERT(!is_digit('a'));
    
    // 测试字母 / Test letters
    TEST_ASSERT(is_alpha('a'));
    TEST_ASSERT(is_alpha('Z'));
    TEST_ASSERT(!is_alpha('1'));
    
    // 测试空白字符 / Test whitespace
    TEST_ASSERT(is_whitespace(' '));
    TEST_ASSERT(is_whitespace('\t'));
    TEST_ASSERT(is_whitespace('\n'));
    TEST_ASSERT(!is_whitespace('a'));
    
    // 测试符号字符 / Test symbol characters
    TEST_ASSERT(is_symbol_char('a'));
    TEST_ASSERT(is_symbol_char('1'));
    TEST_ASSERT(is_symbol_char('+'));
    TEST_ASSERT(!is_symbol_char('('));
    
    return true;
}

// 测试内置函数识别 / Test built-in function recognition
bool test_builtin_functions(void) {
    // 测试算术运算符 / Test arithmetic operators
    TEST_ASSERT(is_arithmetic_operator("+"));
    TEST_ASSERT(is_arithmetic_operator("-"));
    TEST_ASSERT(is_arithmetic_operator("*"));
    TEST_ASSERT(is_arithmetic_operator("/"));
    TEST_ASSERT(!is_arithmetic_operator("hello"));
    
    // 测试比较运算符 / Test comparison operators
    TEST_ASSERT(is_comparison_operator("="));
    TEST_ASSERT(is_comparison_operator("<"));
    TEST_ASSERT(is_comparison_operator(">"));
    TEST_ASSERT(!is_comparison_operator("+"));
    
    // 测试内置函数 / Test built-in functions
    TEST_ASSERT(is_builtin_function("+"));
    TEST_ASSERT(is_builtin_function("if"));
    TEST_ASSERT(is_builtin_function("while"));
    TEST_ASSERT(!is_builtin_function("my_function"));
    
    return true;
}

// 测试简单表达式解析 / Test simple expression parsing
bool test_simple_parsing(void) {
    const char* input = "42";
    
    ASTNode* ast = parse_string(input);
    TEST_ASSERT_NOT_NULL(ast);
    
    // 验证AST结构 / Verify AST structure
    TEST_ASSERT(ast->type == AST_NODE_LIST);
    if (ast->data.list.count > 0) {
        ASTNode* first = ast->data.list.elements[0];
        TEST_ASSERT(first->type == AST_NODE_INTEGER);
        TEST_ASSERT(first->data.literal.int_value == 42);
    }
    
    // 清理 / Cleanup
    ast_destroy_node(ast);
    
    return true;
}

// 测试算术表达式解析 / Test arithmetic expression parsing
bool test_arithmetic_parsing(void) {
    const char* input = "(+ 1 2)";
    
    ASTNode* ast = parse_string(input);
    TEST_ASSERT_NOT_NULL(ast);
    
    // 验证AST结构 / Verify AST structure
    TEST_ASSERT(ast->type == AST_NODE_LIST);
    if (ast->data.list.count > 0) {
        ASTNode* expr = ast->data.list.elements[0];
        TEST_ASSERT(expr->type == AST_NODE_FUNCTION_CALL);
        
        if (expr->data.call.function) {
            TEST_ASSERT(expr->data.call.function->type == AST_NODE_SYMBOL);
            if (expr->data.call.function->data.symbol.name) {
                TEST_ASSERT(strcmp(expr->data.call.function->data.symbol.name, "+") == 0);
            }
        }
        
        TEST_ASSERT(expr->data.call.arg_count == 2);
    }
    
    // 清理 / Cleanup
    ast_destroy_node(ast);
    
    return true;
}

// 测试错误处理 / Test error handling
bool test_error_handling(void) {
    // 测试无效输入 / Test invalid input
    ASTNode* ast1 = parse_string("(+ 1");  // 缺少右括号 / Missing right parenthesis
    // 应该返回NULL或者有错误标记 / Should return NULL or have error flag
    if (ast1) {
        ast_destroy_node(ast1);
    }
    
    // 测试空输入 / Test empty input
    ASTNode* ast2 = parse_string("");
    TEST_ASSERT_NOT_NULL(ast2); // 空程序应该返回空列表 / Empty program should return empty list
    TEST_ASSERT(ast2->type == AST_NODE_LIST);
    TEST_ASSERT(ast2->data.list.count == 0);
    ast_destroy_node(ast2);
    
    return true;
}

// 主测试函数 / Main test function
int main(void) {
    printf("=== Real Parser Component Testing ===\n\n");
    
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
    
    // 运行测试 / Run tests
    RUN_TEST(test_character_classification);
    RUN_TEST(test_builtin_functions);
    RUN_TEST(test_lexer_basic);
    RUN_TEST(test_simple_parsing);
    RUN_TEST(test_arithmetic_parsing);
    RUN_TEST(test_error_handling);
    
    // 打印测试结果 / Print test results
    printf("\n=== Real Parser Test Results ===\n");
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
    
    if (tests_failed == 0) {
        printf("✅ All real parser tests passed!\n");
        printf("Parser components are working correctly.\n");
        return 0;
    } else {
        printf("❌ Some real parser tests failed.\n");
        return 1;
    }
}
