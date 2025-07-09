/**
 * @file test_string_utils.c
 * @brief 字符串工具测试 / String Utilities Test
 * @author newLISP Binary Compiler Team
 * @date 2024
 * 
 * ultrathink模式：彻底测试基础设施组件
 * ultrathink mode: Thoroughly test infrastructure components
 */

#include "../../src/runtime/string_utils.h"
#include <stdio.h>
#include <stdlib.h>
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
#define TEST_ASSERT_EQUAL(expected, actual) TEST_ASSERT((expected) == (actual))
#define TEST_ASSERT_STRING_EQUAL(expected, actual) \
    TEST_ASSERT(strcmp((expected), (actual)) == 0)

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

// 测试safe_strdup / Test safe_strdup
bool test_safe_strdup(void) {
    // 正常情况 / Normal case
    char* dup = safe_strdup("hello");
    TEST_ASSERT_NOT_NULL(dup);
    TEST_ASSERT_STRING_EQUAL("hello", dup);
    free(dup);
    
    // 空字符串 / Empty string
    dup = safe_strdup("");
    TEST_ASSERT_NOT_NULL(dup);
    TEST_ASSERT_STRING_EQUAL("", dup);
    free(dup);
    
    // NULL输入 / NULL input
    dup = safe_strdup(NULL);
    TEST_ASSERT_NULL(dup);
    
    return true;
}

// 测试safe_strcmp / Test safe_strcmp
bool test_safe_strcmp(void) {
    // 正常比较 / Normal comparison
    TEST_ASSERT_EQUAL(0, safe_strcmp("hello", "hello"));
    TEST_ASSERT(safe_strcmp("abc", "def") < 0);
    TEST_ASSERT(safe_strcmp("def", "abc") > 0);
    
    // NULL处理 / NULL handling
    TEST_ASSERT_EQUAL(0, safe_strcmp(NULL, NULL));
    TEST_ASSERT(safe_strcmp(NULL, "hello") < 0);
    TEST_ASSERT(safe_strcmp("hello", NULL) > 0);
    
    return true;
}

// 测试safe_strcpy / Test safe_strcpy
bool test_safe_strcpy(void) {
    char buffer[20];
    
    // 正常复制 / Normal copy
    TEST_ASSERT(safe_strcpy(buffer, "hello", sizeof(buffer)));
    TEST_ASSERT_STRING_EQUAL("hello", buffer);
    
    // 缓冲区太小 / Buffer too small
    TEST_ASSERT(!safe_strcpy(buffer, "this is a very long string", 10));
    
    // NULL处理 / NULL handling
    TEST_ASSERT(!safe_strcpy(NULL, "hello", 10));
    TEST_ASSERT(!safe_strcpy(buffer, NULL, sizeof(buffer)));
    
    return true;
}

// 测试safe_sprintf / Test safe_sprintf
bool test_safe_sprintf(void) {
    // 简单格式化 / Simple formatting
    char* result = safe_sprintf("Hello %s", "World");
    TEST_ASSERT_NOT_NULL(result);
    TEST_ASSERT_STRING_EQUAL("Hello World", result);
    free(result);
    
    // 数字格式化 / Number formatting
    result = safe_sprintf("Number: %d", 42);
    TEST_ASSERT_NOT_NULL(result);
    TEST_ASSERT_STRING_EQUAL("Number: 42", result);
    free(result);
    
    // NULL格式 / NULL format
    result = safe_sprintf(NULL);
    TEST_ASSERT_NULL(result);
    
    return true;
}

// 测试safe_str_to_int / Test safe_str_to_int
bool test_safe_str_to_int(void) {
    long long result;
    
    // 正常转换 / Normal conversion
    TEST_ASSERT(safe_str_to_int("42", &result));
    TEST_ASSERT_EQUAL(42, result);
    
    TEST_ASSERT(safe_str_to_int("-123", &result));
    TEST_ASSERT_EQUAL(-123, result);
    
    // 无效输入 / Invalid input
    TEST_ASSERT(!safe_str_to_int("abc", &result));
    TEST_ASSERT(!safe_str_to_int("12abc", &result));
    TEST_ASSERT(!safe_str_to_int("", &result));
    
    // NULL处理 / NULL handling
    TEST_ASSERT(!safe_str_to_int(NULL, &result));
    TEST_ASSERT(!safe_str_to_int("42", NULL));
    
    return true;
}

// 测试safe_str_to_float / Test safe_str_to_float
bool test_safe_str_to_float(void) {
    double result;
    
    // 正常转换 / Normal conversion
    TEST_ASSERT(safe_str_to_float("3.14", &result));
    TEST_ASSERT(result > 3.13 && result < 3.15); // 浮点数比较 / Float comparison
    
    TEST_ASSERT(safe_str_to_float("-2.5", &result));
    TEST_ASSERT(result > -2.51 && result < -2.49);
    
    // 整数 / Integer
    TEST_ASSERT(safe_str_to_float("42", &result));
    TEST_ASSERT(result > 41.9 && result < 42.1);
    
    // 无效输入 / Invalid input
    TEST_ASSERT(!safe_str_to_float("abc", &result));
    TEST_ASSERT(!safe_str_to_float("3.14abc", &result));
    
    // NULL处理 / NULL handling
    TEST_ASSERT(!safe_str_to_float(NULL, &result));
    TEST_ASSERT(!safe_str_to_float("3.14", NULL));
    
    return true;
}

// 测试string_is_empty / Test string_is_empty
bool test_string_is_empty(void) {
    // 空字符串 / Empty string
    TEST_ASSERT(string_is_empty(""));
    TEST_ASSERT(string_is_empty(NULL));
    
    // 非空字符串 / Non-empty string
    TEST_ASSERT(!string_is_empty("hello"));
    TEST_ASSERT(!string_is_empty(" ")); // 空格不算空 / Space is not empty
    
    return true;
}

// 测试safe_strlen / Test safe_strlen
bool test_safe_strlen(void) {
    // 正常长度 / Normal length
    TEST_ASSERT_EQUAL(5, safe_strlen("hello"));
    TEST_ASSERT_EQUAL(0, safe_strlen(""));
    
    // NULL处理 / NULL handling
    TEST_ASSERT_EQUAL(0, safe_strlen(NULL));
    
    return true;
}

// 测试safe_strcat / Test safe_strcat
bool test_safe_strcat(void) {
    // 正常连接 / Normal concatenation
    char* result = safe_strcat("Hello", " World");
    TEST_ASSERT_NOT_NULL(result);
    TEST_ASSERT_STRING_EQUAL("Hello World", result);
    free(result);
    
    // NULL处理 / NULL handling
    result = safe_strcat(NULL, "World");
    TEST_ASSERT_NOT_NULL(result);
    TEST_ASSERT_STRING_EQUAL("World", result);
    free(result);
    
    result = safe_strcat("Hello", NULL);
    TEST_ASSERT_NOT_NULL(result);
    TEST_ASSERT_STRING_EQUAL("Hello", result);
    free(result);
    
    result = safe_strcat(NULL, NULL);
    TEST_ASSERT_NULL(result);
    
    return true;
}

// 主测试函数 / Main test function
int main(void) {
    printf("=== String Utilities Test Suite ===\n\n");
    
    // 运行所有测试 / Run all tests
    RUN_TEST(test_safe_strdup);
    RUN_TEST(test_safe_strcmp);
    RUN_TEST(test_safe_strcpy);
    RUN_TEST(test_safe_sprintf);
    RUN_TEST(test_safe_str_to_int);
    RUN_TEST(test_safe_str_to_float);
    RUN_TEST(test_string_is_empty);
    RUN_TEST(test_safe_strlen);
    RUN_TEST(test_safe_strcat);
    
    // 打印测试结果 / Print test results
    printf("\n=== String Utilities Test Results ===\n");
    printf("Tests run: %d\n", tests_run);
    printf("Tests passed: %d\n", tests_passed);
    printf("Tests failed: %d\n", tests_failed);
    printf("Success rate: %.1f%%\n", 
           tests_run > 0 ? (double)tests_passed / tests_run * 100.0 : 0.0);
    
    if (tests_failed == 0) {
        printf("✅ All string utility tests passed!\n");
        printf("String infrastructure is solid and ready for use.\n");
        return 0;
    } else {
        printf("❌ Some string utility tests failed.\n");
        return 1;
    }
}
