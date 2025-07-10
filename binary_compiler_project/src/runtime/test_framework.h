/**
 * @file test_framework.h
 * @brief 轻量级测试框架 / Lightweight Testing Framework
 * @author newLISP Binary Compiler Team
 * @date 2024
 * 
 * 提供单元测试、集成测试和性能测试功能
 * Provides unit testing, integration testing and performance testing functionality
 */

#ifndef TEST_FRAMEWORK_H
#define TEST_FRAMEWORK_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdarg.h>
#include <time.h>
#include <math.h>

#ifdef __cplusplus
extern "C" {
#endif

// 测试结果枚举 / Test Result Enumeration
typedef enum {
    TEST_RESULT_PASS = 0,     // 测试通过 / Test passed
    TEST_RESULT_FAIL,         // 测试失败 / Test failed
    TEST_RESULT_SKIP,         // 测试跳过 / Test skipped
    TEST_RESULT_ERROR         // 测试错误 / Test error
} TestResult;

// 测试类型枚举 / Test Type Enumeration
typedef enum {
    TEST_TYPE_UNIT = 0,       // 单元测试 / Unit test
    TEST_TYPE_INTEGRATION,    // 集成测试 / Integration test
    TEST_TYPE_PERFORMANCE,    // 性能测试 / Performance test
    TEST_TYPE_REGRESSION      // 回归测试 / Regression test
} TestType;

// 测试用例结构 / Test Case Structure
typedef struct TestCase {
    char* name;               // 测试名称 / Test name
    char* description;        // 测试描述 / Test description
    TestType type;            // 测试类型 / Test type
    TestResult (*test_func)(void); // 测试函数 / Test function
    void (*setup_func)(void); // 设置函数 / Setup function
    void (*teardown_func)(void); // 清理函数 / Teardown function
    double timeout;           // 超时时间（秒） / Timeout in seconds
    bool enabled;             // 是否启用 / Whether enabled
    struct TestCase* next;    // 链表下一个 / Next in list
} TestCase;

// 测试套件结构 / Test Suite Structure
typedef struct TestSuite {
    char* name;               // 套件名称 / Suite name
    char* description;        // 套件描述 / Suite description
    TestCase* test_cases;     // 测试用例链表 / Test case list
    int test_count;           // 测试数量 / Test count
    void (*suite_setup)(void); // 套件设置 / Suite setup
    void (*suite_teardown)(void); // 套件清理 / Suite teardown
    struct TestSuite* next;   // 链表下一个 / Next in list
} TestSuite;

// 测试统计信息 / Test Statistics
typedef struct {
    int total_tests;          // 总测试数 / Total tests
    int passed_tests;         // 通过测试数 / Passed tests
    int failed_tests;         // 失败测试数 / Failed tests
    int skipped_tests;        // 跳过测试数 / Skipped tests
    int error_tests;          // 错误测试数 / Error tests
    double total_time;        // 总时间 / Total time
    double min_time;          // 最短时间 / Minimum time
    double max_time;          // 最长时间 / Maximum time
    double avg_time;          // 平均时间 / Average time
} TestStats;

// 测试运行器配置 / Test Runner Configuration
typedef struct {
    bool verbose;             // 详细输出 / Verbose output
    bool stop_on_failure;     // 失败时停止 / Stop on failure
    bool run_performance;     // 运行性能测试 / Run performance tests
    bool generate_report;     // 生成报告 / Generate report
    char* report_file;        // 报告文件 / Report file
    double default_timeout;   // 默认超时 / Default timeout
    FILE* output_stream;      // 输出流 / Output stream
} TestConfig;

// 测试运行器结构 / Test Runner Structure
typedef struct {
    TestSuite* test_suites;   // 测试套件链表 / Test suite list
    int suite_count;          // 套件数量 / Suite count
    TestStats stats;          // 统计信息 / Statistics
    TestConfig config;        // 配置信息 / Configuration
    bool initialized;         // 是否已初始化 / Whether initialized
} TestRunner;

// 全局测试运行器 / Global Test Runner
extern TestRunner* g_test_runner;

// 核心API函数 / Core API Functions

/**
 * @brief 初始化测试框架 / Initialize test framework
 * @param config 配置参数 / Configuration parameters
 * @return 成功返回true / Returns true on success
 */
bool test_framework_init(const TestConfig* config);

/**
 * @brief 清理测试框架 / Cleanup test framework
 */
void test_framework_cleanup(void);

/**
 * @brief 创建测试套件 / Create test suite
 * @param name 套件名称 / Suite name
 * @param description 套件描述 / Suite description
 * @return 测试套件指针 / Test suite pointer
 */
TestSuite* test_create_suite(const char* name, const char* description);

/**
 * @brief 添加测试用例 / Add test case
 * @param suite 测试套件 / Test suite
 * @param name 测试名称 / Test name
 * @param description 测试描述 / Test description
 * @param type 测试类型 / Test type
 * @param test_func 测试函数 / Test function
 * @return 成功返回true / Returns true on success
 */
bool test_add_case(TestSuite* suite, const char* name, const char* description,
                  TestType type, TestResult (*test_func)(void));

/**
 * @brief 设置测试用例的设置和清理函数 / Set test case setup and teardown functions
 * @param suite 测试套件 / Test suite
 * @param test_name 测试名称 / Test name
 * @param setup_func 设置函数 / Setup function
 * @param teardown_func 清理函数 / Teardown function
 */
void test_set_case_hooks(TestSuite* suite, const char* test_name,
                        void (*setup_func)(void), void (*teardown_func)(void));

/**
 * @brief 设置套件的设置和清理函数 / Set suite setup and teardown functions
 * @param suite 测试套件 / Test suite
 * @param setup_func 设置函数 / Setup function
 * @param teardown_func 清理函数 / Teardown function
 */
void test_set_suite_hooks(TestSuite* suite, void (*setup_func)(void), void (*teardown_func)(void));

/**
 * @brief 运行测试套件 / Run test suite
 * @param suite 测试套件 / Test suite
 * @return 成功返回true / Returns true on success
 */
bool test_run_suite(TestSuite* suite);

/**
 * @brief 运行所有测试 / Run all tests
 * @return 成功返回true / Returns true on success
 */
bool test_run_all(void);

/**
 * @brief 获取测试统计信息 / Get test statistics
 * @return 统计信息指针 / Statistics pointer
 */
const TestStats* test_get_stats(void);

/**
 * @brief 释放测试套件 / Free test suite
 * @param suite 测试套件 / Test suite
 */
void test_free_suite(TestSuite* suite);

/**
 * @brief 打印测试报告 / Print test report
 * @param output 输出流 / Output stream
 */
void test_print_report(FILE* output);

/**
 * @brief 生成XML测试报告 / Generate XML test report
 * @param filename 文件名 / Filename
 * @return 成功返回true / Returns true on success
 */
bool test_generate_xml_report(const char* filename);

// 断言宏定义 / Assertion Macros

#define TEST_ASSERT(condition) \
    do { \
        if (!(condition)) { \
            fprintf(stderr, "ASSERTION FAILED: %s at %s:%d in %s\n", \
                   #condition, __FILE__, __LINE__, __FUNCTION__); \
            return TEST_RESULT_FAIL; \
        } \
    } while(0)

#define TEST_ASSERT_TRUE(condition) TEST_ASSERT(condition)

#define TEST_ASSERT_FALSE(condition) TEST_ASSERT(!(condition))

#define TEST_ASSERT_NULL(ptr) TEST_ASSERT((ptr) == NULL)

#define TEST_ASSERT_NOT_NULL(ptr) TEST_ASSERT((ptr) != NULL)

#define TEST_ASSERT_EQUAL(expected, actual) \
    TEST_ASSERT((expected) == (actual))

#define TEST_ASSERT_NOT_EQUAL(expected, actual) \
    TEST_ASSERT((expected) != (actual))

#define TEST_ASSERT_STRING_EQUAL(expected, actual) \
    TEST_ASSERT(strcmp((expected), (actual)) == 0)

#define TEST_ASSERT_STRING_NOT_EQUAL(expected, actual) \
    TEST_ASSERT(strcmp((expected), (actual)) != 0)

#define TEST_ASSERT_MEMORY_EQUAL(expected, actual, size) \
    TEST_ASSERT(memcmp((expected), (actual), (size)) == 0)

#define TEST_ASSERT_FLOAT_EQUAL(expected, actual, tolerance) \
    TEST_ASSERT(fabs((expected) - (actual)) <= (tolerance))

#define TEST_ASSERT_RANGE(value, min, max) \
    TEST_ASSERT((value) >= (min) && (value) <= (max))

// 性能测试宏 / Performance Test Macros

#define TEST_PERFORMANCE_START() \
    double _perf_start = test_get_timestamp()

#define TEST_PERFORMANCE_END(max_time_ms) \
    do { \
        double _perf_end = test_get_timestamp(); \
        double _elapsed = (_perf_end - _perf_start) * 1000.0; \
        TEST_ASSERT(_elapsed <= (max_time_ms)); \
    } while(0)

#define TEST_BENCHMARK(name, iterations, code_block) \
    do { \
        double _bench_start = test_get_timestamp(); \
        for (int _i = 0; _i < (iterations); _i++) { \
            code_block; \
        } \
        double _bench_end = test_get_timestamp(); \
        double _total_time = (_bench_end - _bench_start) * 1000.0; \
        double _avg_time = _total_time / (iterations); \
        printf("BENCHMARK %s: %d iterations, %.3f ms total, %.6f ms avg\n", \
               (name), (iterations), _total_time, _avg_time); \
    } while(0)

// 测试用例注册宏 / Test Case Registration Macros

#define TEST_CASE(suite_name, test_name, test_type) \
    TestResult test_##suite_name##_##test_name(void)

#define TEST_SUITE_BEGIN(suite_name) \
    TestSuite* create_##suite_name##_suite(void) { \
        TestSuite* suite = test_create_suite(#suite_name, "Test suite for " #suite_name);

#define TEST_SUITE_ADD(test_name, test_type) \
        test_add_case(suite, #test_name, "Test case " #test_name, test_type, test_##test_name);

#define TEST_SUITE_END() \
        return suite; \
    }

// 便利函数 / Convenience Functions

/**
 * @brief 获取当前时间戳 / Get current timestamp
 * @return 时间戳 / Timestamp
 */
double test_get_timestamp(void);

/**
 * @brief 创建默认测试配置 / Create default test configuration
 * @return 默认配置 / Default configuration
 */
TestConfig test_create_default_config(void);

/**
 * @brief 跳过测试 / Skip test
 * @param reason 跳过原因 / Skip reason
 * @return TEST_RESULT_SKIP
 */
TestResult test_skip(const char* reason);

/**
 * @brief 测试失败 / Test failure
 * @param format 格式化字符串 / Format string
 * @param ... 可变参数 / Variable arguments
 * @return TEST_RESULT_FAIL
 */
TestResult test_fail(const char* format, ...);

/**
 * @brief 测试通过 / Test pass
 * @return TEST_RESULT_PASS
 */
TestResult test_pass(void);

/**
 * @brief 设置测试超时 / Set test timeout
 * @param suite 测试套件 / Test suite
 * @param test_name 测试名称 / Test name
 * @param timeout 超时时间（秒） / Timeout in seconds
 */
void test_set_timeout(TestSuite* suite, const char* test_name, double timeout);

/**
 * @brief 启用/禁用测试 / Enable/disable test
 * @param suite 测试套件 / Test suite
 * @param test_name 测试名称 / Test name
 * @param enabled 是否启用 / Whether enabled
 */
void test_set_enabled(TestSuite* suite, const char* test_name, bool enabled);



#ifdef __cplusplus
}
#endif

#endif // TEST_FRAMEWORK_H
