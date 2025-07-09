/**
 * @file test_framework.c
 * @brief 轻量级测试框架实现 / Lightweight test framework implementation
 *
 * 提供简单易用的单元测试功能，支持断言、性能测试和报告生成
 * Provides simple unit testing functionality with assertions, performance testing and reporting
 */

#include "test_framework.h"
#include "error_handler.h"
#include "memory_manager.h"
#include "string_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <stdint.h>
#include <stdarg.h>

/**
 * @brief 全局测试框架状态 / Global test framework state
 */
static struct {
    TestConfig config;            // 测试配置 / Test configuration
    TestStats stats;              // 测试统计 / Test statistics
    bool initialized;             // 初始化状态 / Initialization status
    struct timeval start_time;    // 开始时间 / Start time
} g_test_framework = {0};

/**
 * @brief 获取当前时间戳 / Get current timestamp
 */
double test_get_timestamp(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (double)tv.tv_sec + (double)tv.tv_usec / 1000000.0;
}

/**
 * @brief 创建默认测试配置 / Create default test configuration
 */
TestConfig test_create_default_config(void) {
    TestConfig config;
    memset(&config, 0, sizeof(TestConfig));
    config.verbose = true;
    config.stop_on_failure = false;
    config.run_performance = true;
    config.generate_report = false;
    config.report_file = NULL;
    config.default_timeout = 10.0;
    config.output_stream = stdout;
    return config;
}

/**
 * @brief 初始化测试框架 / Initialize test framework
 */
bool test_framework_init(const TestConfig* config) {
    if (g_test_framework.initialized) {
        return true;
    }

    if (config) {
        g_test_framework.config = *config;
    } else {
        g_test_framework.config = test_create_default_config();
    }

    memset(&g_test_framework.stats, 0, sizeof(TestStats));
    g_test_framework.initialized = true;

    gettimeofday(&g_test_framework.start_time, NULL);

    if (g_test_framework.config.verbose) {
        printf("Test framework initialized\n");
    }

    return true;
}

/**
 * @brief 清理测试框架 / Cleanup test framework
 */
void test_framework_cleanup(void) {
    if (!g_test_framework.initialized) {
        return;
    }

    g_test_framework.initialized = false;

    if (g_test_framework.config.verbose) {
        printf("Test framework cleaned up\n");
    }
}

/**
 * @brief 创建测试套件 / Create test suite
 */
TestSuite* test_create_suite(const char* name, const char* description) {
    if (!name) return NULL;

    TestSuite* suite = (TestSuite*)memory_alloc(sizeof(TestSuite), MEM_TYPE_TEST);
    if (!suite) {
        return NULL;
    }

    suite->name = safe_strdup(name);
    suite->description = description ? safe_strdup(description) : NULL;
    suite->test_cases = NULL;
    suite->test_count = 0;
    suite->suite_setup = NULL;
    suite->suite_teardown = NULL;
    suite->next = NULL;

    return suite;
}

/**
 * @brief 添加测试用例 / Add test case
 */
bool test_add_case(TestSuite* suite, const char* name, const char* description,
                   TestType type, TestResult (*test_func)(void)) {
    if (!suite || !name) {
        return false;
    }

    TestCase* test_case = (TestCase*)memory_alloc(sizeof(TestCase), MEM_TYPE_TEST);
    if (!test_case) {
        return false;
    }

    test_case->name = safe_strdup(name);
    test_case->description = description ? safe_strdup(description) : NULL;
    test_case->type = type;
    test_case->test_func = test_func;
    test_case->setup_func = NULL;
    test_case->teardown_func = NULL;
    test_case->timeout = 10.0;
    test_case->enabled = true;
    test_case->next = suite->test_cases;
    suite->test_cases = test_case;
    suite->test_count++;

    return true;
}

/**
 * @brief 运行测试套件 / Run test suite
 */
bool test_run_suite(TestSuite* suite) {
    if (!suite || !g_test_framework.initialized) {
        return false;
    }

    if (g_test_framework.config.verbose) {
        printf("\n=== Running Test Suite: %s ===\n", suite->name);
    }

    TestCase* current = suite->test_cases;
    bool all_passed = true;

    while (current) {
        if (current->enabled) {
            g_test_framework.stats.total_tests++;

            if (g_test_framework.config.verbose) {
                printf("Running %s... ", current->name);
                fflush(stdout);
            }

            double start_time = test_get_timestamp();
            TestResult result = TEST_RESULT_PASS;

            if (current->test_func) {
                result = current->test_func();
            }

            double end_time = test_get_timestamp();
            double execution_time = end_time - start_time;

            if (result == TEST_RESULT_PASS) {
                g_test_framework.stats.passed_tests++;
                if (g_test_framework.config.verbose) {
                    printf("PASS\n");
                }
            } else {
                g_test_framework.stats.failed_tests++;
                all_passed = false;
                if (g_test_framework.config.verbose) {
                    printf("FAIL\n");
                }
            }

            g_test_framework.stats.total_time += execution_time;
            if (g_test_framework.stats.total_tests == 1) {
                g_test_framework.stats.min_time = g_test_framework.stats.max_time = execution_time;
            } else {
                if (execution_time < g_test_framework.stats.min_time)
                    g_test_framework.stats.min_time = execution_time;
                if (execution_time > g_test_framework.stats.max_time)
                    g_test_framework.stats.max_time = execution_time;
            }
        }
        current = current->next;
    }

    if (g_test_framework.stats.total_tests > 0) {
        g_test_framework.stats.avg_time = g_test_framework.stats.total_time / g_test_framework.stats.total_tests;
    }

    return all_passed;
}

/**
 * @brief 打印测试报告 / Print test report
 */
void test_print_report(FILE* output) {
    if (!output) output = stdout;

    if (!g_test_framework.initialized) {
        fprintf(output, "Test framework not initialized\n");
        return;
    }

    fprintf(output, "\n=== Test Results ===\n");
    fprintf(output, "Tests run: %d\n", g_test_framework.stats.total_tests);
    fprintf(output, "Tests passed: %d\n", g_test_framework.stats.passed_tests);
    fprintf(output, "Tests failed: %d\n", g_test_framework.stats.failed_tests);

    if (g_test_framework.stats.total_tests > 0) {
        double success_rate = (double)g_test_framework.stats.passed_tests /
                             g_test_framework.stats.total_tests * 100.0;
        fprintf(output, "Success rate: %.1f%%\n", success_rate);
        fprintf(output, "Total time: %.3f seconds\n", g_test_framework.stats.total_time);
        fprintf(output, "Average time: %.3f seconds\n", g_test_framework.stats.avg_time);
    }

    fprintf(output, "=========================\n\n");
}

/**
 * @brief 释放测试套件 / Free test suite
 */
void test_free_suite(TestSuite* suite) {
    if (!suite) return;

    // 释放测试用例 / Free test cases
    TestCase* current = suite->test_cases;
    while (current) {
        TestCase* next = current->next;
        if (current->name) {
            memory_free(current->name);
        }
        if (current->description) {
            memory_free(current->description);
        }
        memory_free(current);
        current = next;
    }

    // 释放套件名称 / Free suite name
    if (suite->name) {
        memory_free(suite->name);
    }
    if (suite->description) {
        memory_free(suite->description);
    }

    memory_free(suite);
}

/**
 * @brief 测试通过 / Test pass
 */
TestResult test_pass(void) {
    return TEST_RESULT_PASS;
}

/**
 * @brief 测试失败 / Test fail
 */
TestResult test_fail(const char* format, ...) {
    if (format) {
        va_list args;
        va_start(args, format);
        vprintf(format, args);
        va_end(args);
        printf("\n");
    }
    return TEST_RESULT_FAIL;
}
