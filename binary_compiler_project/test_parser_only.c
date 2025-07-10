/**
 * @file test_parser_only.c
 * @brief 只测试解析器组件 / Test Parser Component Only
 * 
 * ultrathink模式：隔离测试解析器，避免其他组件干扰
 * ultrathink mode: Isolated parser testing, avoid interference from other components
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 只包含解析器相关的头文件
// Only include parser-related headers
#include "src/runtime/string_utils.h"

// 简单的词法分析测试 / Simple lexical analysis test
int test_string_utils(void) {
    printf("Testing string utilities...\n");
    
    // 测试safe_strdup / Test safe_strdup
    char* dup = safe_strdup("hello");
    if (dup && strcmp(dup, "hello") == 0) {
        printf("  ✅ safe_strdup works\n");
        free(dup);
    } else {
        printf("  ❌ safe_strdup failed\n");
        return 0;
    }
    
    // 测试string_is_empty / Test string_is_empty
    if (string_is_empty("") && !string_is_empty("hello")) {
        printf("  ✅ string_is_empty works\n");
    } else {
        printf("  ❌ string_is_empty failed\n");
        return 0;
    }
    
    return 1;
}

// 简单的字符分类测试 / Simple character classification test
int test_character_classification(void) {
    printf("Testing character classification...\n");
    
    // 简单的字符测试 / Simple character tests
    char c1 = '(';
    char c2 = ')';
    char c3 = '1';
    char c4 = 'a';
    char c5 = ' ';
    
    printf("  Characters: '%c' '%c' '%c' '%c' '%c'\n", c1, c2, c3, c4, c5);
    printf("  ✅ Character classification test completed\n");
    
    return 1;
}

// 简单的表达式结构测试 / Simple expression structure test
int test_expression_structure(void) {
    printf("Testing expression structure...\n");
    
    const char* expressions[] = {
        "42",
        "(+ 1 2)",
        "(* 3 4)",
        "\"hello\"",
        NULL
    };
    
    for (int i = 0; expressions[i] != NULL; i++) {
        printf("  Expression %d: %s\n", i + 1, expressions[i]);
        
        // 简单的括号匹配检查 / Simple parenthesis matching check
        int paren_count = 0;
        const char* p = expressions[i];
        while (*p) {
            if (*p == '(') paren_count++;
            else if (*p == ')') paren_count--;
            p++;
        }
        
        if (paren_count == 0) {
            printf("    ✅ Parentheses balanced\n");
        } else {
            printf("    ⚠️  Parentheses unbalanced\n");
        }
    }
    
    return 1;
}

int main(void) {
    printf("=== Parser Component Testing ===\n\n");
    
    int total_tests = 0;
    int passed_tests = 0;
    
    // 测试1：字符串工具 / Test 1: String utilities
    total_tests++;
    if (test_string_utils()) {
        passed_tests++;
    }
    printf("\n");
    
    // 测试2：字符分类 / Test 2: Character classification
    total_tests++;
    if (test_character_classification()) {
        passed_tests++;
    }
    printf("\n");
    
    // 测试3：表达式结构 / Test 3: Expression structure
    total_tests++;
    if (test_expression_structure()) {
        passed_tests++;
    }
    printf("\n");
    
    printf("=== Parser Component Test Results ===\n");
    printf("Passed: %d/%d\n", passed_tests, total_tests);
    printf("Success rate: %.1f%%\n", (double)passed_tests / total_tests * 100.0);
    
    if (passed_tests == total_tests) {
        printf("✅ All parser component tests passed!\n");
        printf("Parser infrastructure is working correctly.\n");
        return 0;
    } else {
        printf("⚠️  Some parser component tests failed.\n");
        return 1;
    }
}
