/**
 * @file test_basic_infrastructure.c
 * @brief 基础设施基本功能验证测试 / Basic infrastructure functionality verification tests
 * 
 * 简单验证基础设施组件的基本功能是否正常工作
 * Simple verification that infrastructure components basic functionality works
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>

// 包含基础设施组件 / Include infrastructure components
#include "../../src/runtime/config_manager.h"
#include "../../src/runtime/logger.h"
#include "../../src/runtime/test_framework.h"
#include "../../src/runtime/memory_manager.h"
#include "../../src/runtime/error_handler.h"

/**
 * @brief 测试配置管理器基本功能 / Test configuration manager basic functionality
 */
void test_config_basic(void) {
    printf("Testing Configuration Manager Basic Functionality...\n");
    
    // 初始化配置管理器 / Initialize config manager
    assert(config_manager_init(NULL) == true);
    
    // 测试基本配置操作 / Test basic configuration operations
    assert(config_set_bool("test.enable", true) == true);
    assert(config_get_bool("test.enable", false) == true);
    
    assert(config_set_int("test.count", 42) == true);
    assert(config_get_int("test.count", 0) == 42);
    
    assert(config_set_string("test.name", "newlisp") == true);
    const char* name = config_get_string("test.name", "default");
    assert(strcmp(name, "newlisp") == 0);
    
    // 测试键存在性 / Test key existence
    assert(config_has_key("test.name") == true);
    assert(config_has_key("nonexistent") == false);
    
    // 清理 / Cleanup
    config_manager_cleanup();
    
    printf("✓ Configuration Manager basic functionality works\n");
}

/**
 * @brief 测试日志系统基本功能 / Test logging system basic functionality
 */
void test_logger_basic(void) {
    printf("Testing Logger System Basic Functionality...\n");
    
    // 初始化日志系统 / Initialize logging system
    LogConfig log_config;
    log_config.min_level = LOG_LEVEL_INFO;
    log_config.targets = LOG_TARGET_CONSOLE;
    log_config.format = LOG_FORMAT_SIMPLE;
    log_config.enable_timestamps = false;
    log_config.enable_colors = false;
    log_config.max_file_size = 0;
    log_config.log_file_path = NULL;
    
    assert(logger_init(&log_config) == true);
    
    // 测试日志级别 / Test log levels
    assert(logger_is_level_enabled(LOG_LEVEL_INFO) == true);
    assert(logger_is_level_enabled(LOG_LEVEL_DEBUG) == false);
    
    // 测试日志写入 / Test log writing
    logger_log(LOG_LEVEL_INFO, __FILE__, __LINE__, __FUNCTION__, 
               "test", "This is a test log message");
    
    // 测试级别设置 / Test level setting
    logger_set_level(LOG_LEVEL_DEBUG);
    assert(logger_get_level() == LOG_LEVEL_DEBUG);
    assert(logger_is_level_enabled(LOG_LEVEL_DEBUG) == true);
    
    // 测试功能检查 / Test functionality checks
    assert(logger_is_console_enabled() == true);
    assert(logger_is_file_enabled() == false);
    
    // 清理 / Cleanup
    logger_cleanup();
    
    printf("✓ Logger System basic functionality works\n");
}

/**
 * @brief 测试内存管理器基本功能 / Test memory manager basic functionality
 */
void test_memory_basic(void) {
    printf("Testing Memory Manager Basic Functionality...\n");

    // 创建内存配置 / Create memory configuration
    MemoryConfig config;
    config.max_memory_limit = 1024 * 1024;  // 1MB
    config.enable_tracking = true;
    config.enable_debugging = true;
    config.enable_leak_detection = true;
    config.enable_bounds_checking = true;
    config.alignment = 8;

    // 初始化内存管理器 / Initialize memory manager
    assert(memory_manager_init(&config) == true);
    
    // 测试内存分配 / Test memory allocation
    void* ptr1 = memory_alloc(100, MEM_TYPE_GENERAL);
    assert(ptr1 != NULL);
    
    void* ptr2 = memory_alloc(200, MEM_TYPE_AST);
    assert(ptr2 != NULL);
    
    // 测试内存释放 / Test memory deallocation
    memory_free(ptr1);
    memory_free(ptr2);
    
    // 检查内存泄漏 / Check memory leaks
    int leak_count = memory_check_leaks(NULL);
    assert(leak_count == 0);
    
    // 清理 / Cleanup
    memory_manager_cleanup();
    
    printf("✓ Memory Manager basic functionality works\n");
}

/**
 * @brief 测试错误处理系统基本功能 / Test error handling system basic functionality
 */
void test_error_basic(void) {
    printf("Testing Error Handling System Basic Functionality...\n");
    
    // 初始化错误处理系统 / Initialize error handling system
    assert(error_system_init(10, 100) == true);
    
    // 测试错误报告 / Test error reporting
    ERROR_REPORT_INFO(ERROR_TYPE_SYSTEM, 0, "Test info message");
    ERROR_REPORT_WARNING(ERROR_TYPE_MEMORY, 1, "Test warning message");
    
    // 检查是否有错误 / Check if there are errors
    assert(error_has_errors(ERROR_LEVEL_INFO) == true);
    assert(error_has_errors(ERROR_LEVEL_WARNING) == true);
    
    // 清理 / Cleanup
    error_system_cleanup();
    
    printf("✓ Error Handling System basic functionality works\n");
}

/**
 * @brief 测试框架基本功能 / Test framework basic functionality
 */
TestResult test_sample_pass(void) {
    return test_pass();
}

TestResult test_sample_fail(void) {
    return test_fail("Sample failure for testing");
}

void test_framework_basic(void) {
    printf("Testing Test Framework Basic Functionality...\n");
    
    // 创建测试配置 / Create test configuration
    TestConfig config = test_create_default_config();
    config.verbose = false;  // 减少输出 / Reduce output
    
    // 初始化测试框架 / Initialize test framework
    assert(test_framework_init(&config) == true);
    
    // 创建测试套件 / Create test suite
    TestSuite* suite = test_create_suite("Basic Test Suite", "Basic functionality test");
    assert(suite != NULL);
    
    // 添加测试用例 / Add test cases
    assert(test_add_case(suite, "Pass Test", "Test that passes", 
                        TEST_TYPE_UNIT, test_sample_pass) == true);
    assert(test_add_case(suite, "Fail Test", "Test that fails", 
                        TEST_TYPE_UNIT, test_sample_fail) == true);
    
    // 运行测试套件 / Run test suite
    bool result = test_run_suite(suite);
    assert(result == false);  // 应该失败，因为有一个故意失败的测试 / Should fail due to intentional failure
    
    // 释放测试套件 / Free test suite
    test_free_suite(suite);
    
    // 清理测试框架 / Cleanup test framework
    test_framework_cleanup();
    
    printf("✓ Test Framework basic functionality works\n");
}

/**
 * @brief 测试基础设施集成 / Test infrastructure integration
 */
void test_integration_basic(void) {
    printf("Testing Basic Infrastructure Integration...\n");
    
    // 创建内存配置 / Create memory configuration
    MemoryConfig mem_config;
    mem_config.max_memory_limit = 1024 * 1024;  // 1MB
    mem_config.enable_tracking = true;
    mem_config.enable_debugging = true;
    mem_config.enable_leak_detection = true;
    mem_config.enable_bounds_checking = true;
    mem_config.alignment = 8;

    // 初始化所有组件 / Initialize all components
    assert(memory_manager_init(&mem_config) == true);
    assert(error_system_init(10, 100) == true);
    assert(config_manager_init(NULL) == true);
    
    LogConfig log_config;
    log_config.min_level = LOG_LEVEL_INFO;
    log_config.targets = LOG_TARGET_CONSOLE;
    log_config.format = LOG_FORMAT_SIMPLE;
    log_config.enable_timestamps = false;
    log_config.enable_colors = false;
    log_config.max_file_size = 0;
    log_config.log_file_path = NULL;
    assert(logger_init(&log_config) == true);
    
    // 测试组件间协作 / Test component collaboration
    config_set_string("app.name", "newlisp-compiler");
    config_set_int("app.version", 1);
    
    const char* app_name = config_get_string("app.name", "unknown");
    int64_t version = config_get_int("app.version", 0);
    
    logger_log(LOG_LEVEL_INFO, __FILE__, __LINE__, __FUNCTION__, 
               "integration", "Application: %s, Version: %lld", app_name, (long long)version);
    
    // 测试内存分配 / Test memory allocation
    void* test_ptr = memory_alloc(1024, MEM_TYPE_TEMP);
    assert(test_ptr != NULL);
    memory_free(test_ptr);
    
    // 检查内存泄漏 / Check memory leaks
    int leak_count = memory_check_leaks(NULL);
    assert(leak_count == 0);
    
    // 清理所有组件 / Cleanup all components
    logger_cleanup();
    config_manager_cleanup();
    error_system_cleanup();
    memory_manager_cleanup();
    
    printf("✓ Basic Infrastructure Integration works\n");
}

/**
 * @brief 主测试函数 / Main test function
 */
int main(void) {
    printf("=== Basic Infrastructure Verification Tests ===\n\n");
    
    test_config_basic();
    test_logger_basic();
    test_memory_basic();
    test_error_basic();
    test_framework_basic();
    test_integration_basic();
    
    printf("\n=== All Basic Infrastructure Tests PASSED ===\n");
    printf("✓ Configuration Management System - Working\n");
    printf("✓ Logging System - Working\n");
    printf("✓ Memory Management System - Working\n");
    printf("✓ Error Handling System - Working\n");
    printf("✓ Test Framework - Working\n");
    printf("✓ Infrastructure Integration - Working\n");
    
    return 0;
}
