/**
 * @file test_infrastructure.c
 * @brief 基础设施集成测试 / Infrastructure Integration Tests
 * @author newLISP Binary Compiler Team
 * @date 2024
 * 
 * 测试错误处理、内存管理、日志系统等基础设施的集成
 * Tests integration of error handling, memory management, logging system and other infrastructure
 */

#include "../../src/runtime/error_handler.h"
#include "../../src/runtime/memory_manager.h"
#include "../../src/runtime/test_framework.h"
#include "../../src/runtime/config_manager.h"

// 测试错误处理系统 / Test Error Handling System
TestResult test_error_handling_basic(void) {
    // 初始化错误处理系统 / Initialize error handling system
    TEST_ASSERT(error_system_init(10, 100));
    
    // 测试错误报告 / Test error reporting
    ERROR_REPORT_INFO(ERROR_TYPE_SYSTEM, 0, "Test info message");
    ERROR_REPORT_WARNING(ERROR_TYPE_MEMORY, 1, "Test warning message");
    ERROR_REPORT_ERROR(ERROR_TYPE_COMPILE, 2, "Test error message");
    
    // 检查错误历史 / Check error history
    int error_count;
    const ErrorInfo* history = error_get_history(&error_count);
    TEST_ASSERT_NOT_NULL(history);
    TEST_ASSERT_EQUAL(3, error_count);
    
    // 检查最后一个错误 / Check last error
    const ErrorInfo* last_error = error_get_last();
    TEST_ASSERT_NOT_NULL(last_error);
    TEST_ASSERT_EQUAL(ERROR_LEVEL_ERROR, last_error->level);
    TEST_ASSERT_EQUAL(ERROR_TYPE_COMPILE, last_error->type);
    
    // 检查是否有错误 / Check if there are errors
    TEST_ASSERT_TRUE(error_has_errors(ERROR_LEVEL_WARNING));
    TEST_ASSERT_TRUE(error_has_errors(ERROR_LEVEL_ERROR));
    
    // 清理 / Cleanup
    error_system_cleanup();
    
    return TEST_RESULT_PASS;
}

// 测试内存管理系统 / Test Memory Management System
TestResult test_memory_management_basic(void) {
    // 创建内存管理器配置 / Create memory manager configuration
    MemoryConfig config = {
        .enable_tracking = true,
        .enable_debugging = true,
        .enable_leak_detection = true,
        .enable_bounds_checking = false,
        .max_memory_limit = 1024 * 1024, // 1MB limit
        .alignment = 8
    };
    
    // 初始化内存管理系统 / Initialize memory management system
    TEST_ASSERT(memory_manager_init(&config));
    
    // 测试内存分配 / Test memory allocation
    void* ptr1 = memory_alloc(100, MEM_TYPE_GENERAL);
    TEST_ASSERT_NOT_NULL(ptr1);
    
    void* ptr2 = memory_alloc(200, MEM_TYPE_AST);
    TEST_ASSERT_NOT_NULL(ptr2);
    
    void* ptr3 = memory_alloc(300, MEM_TYPE_STRING);
    TEST_ASSERT_NOT_NULL(ptr3);
    
    // 检查统计信息 / Check statistics
    const MemoryStats* stats = memory_get_stats();
    TEST_ASSERT_NOT_NULL(stats);
    TEST_ASSERT_EQUAL(3, stats->allocation_count);
    TEST_ASSERT_EQUAL(600, stats->total_allocated);
    TEST_ASSERT_EQUAL(600, stats->current_usage);
    TEST_ASSERT_EQUAL(3, stats->active_blocks);
    
    // 测试内存释放 / Test memory deallocation
    memory_free(ptr1);
    memory_free(ptr2);
    
    stats = memory_get_stats();
    TEST_ASSERT_EQUAL(2, stats->deallocation_count);
    TEST_ASSERT_EQUAL(300, stats->current_usage);
    TEST_ASSERT_EQUAL(1, stats->active_blocks);
    
    // 测试内存验证 / Test memory validation
    TEST_ASSERT_TRUE(memory_validate_block(ptr3));
    
    // 释放剩余内存 / Free remaining memory
    memory_free(ptr3);
    
    // 检查内存泄漏 / Check memory leaks
    int leak_count = memory_check_leaks(NULL);
    TEST_ASSERT_EQUAL(0, leak_count);
    
    // 清理 / Cleanup
    memory_manager_cleanup();
    
    return TEST_RESULT_PASS;
}

// 测试内存泄漏检测 / Test Memory Leak Detection
TestResult test_memory_leak_detection(void) {
    MemoryConfig config = {
        .enable_tracking = true,
        .enable_debugging = true,
        .enable_leak_detection = true,
        .enable_bounds_checking = false,
        .max_memory_limit = 0,
        .alignment = 8
    };
    
    TEST_ASSERT(memory_manager_init(&config));
    
    // 故意创建内存泄漏 / Intentionally create memory leak
    void* leaked_ptr = memory_alloc(100, MEM_TYPE_TEMP);
    TEST_ASSERT_NOT_NULL(leaked_ptr);
    
    // 检查泄漏 / Check leaks
    int leak_count = memory_check_leaks(NULL);
    TEST_ASSERT_EQUAL(1, leak_count);
    
    // 修复泄漏 / Fix leak
    memory_free(leaked_ptr);
    
    leak_count = memory_check_leaks(NULL);
    TEST_ASSERT_EQUAL(0, leak_count);
    
    memory_manager_cleanup();
    
    return TEST_RESULT_PASS;
}

// 测试配置管理系统 / Test Configuration Management System
TestResult test_config_management_basic(void) {
    // 初始化配置管理器 / Initialize configuration manager
    TEST_ASSERT(config_manager_init(NULL));
    
    // 测试设置和获取布尔值 / Test setting and getting boolean values
    TEST_ASSERT(config_set_bool("test.bool_value", true));
    TEST_ASSERT_TRUE(config_get_bool("test.bool_value", false));
    
    // 测试设置和获取整数值 / Test setting and getting integer values
    TEST_ASSERT(config_set_int("test.int_value", 42));
    TEST_ASSERT_EQUAL(42, config_get_int("test.int_value", 0));
    
    // 测试设置和获取浮点值 / Test setting and getting float values
    TEST_ASSERT(config_set_float("test.float_value", 3.14159));
    TEST_ASSERT_FLOAT_EQUAL(3.14159, config_get_float("test.float_value", 0.0), 0.00001);
    
    // 测试设置和获取字符串值 / Test setting and getting string values
    TEST_ASSERT(config_set_string("test.string_value", "Hello, World!"));
    const char* str_value = config_get_string("test.string_value", "");
    TEST_ASSERT_STRING_EQUAL("Hello, World!", str_value);
    
    // 测试键存在性检查 / Test key existence check
    TEST_ASSERT_TRUE(config_has_key("test.bool_value"));
    TEST_ASSERT_FALSE(config_has_key("test.nonexistent_key"));
    
    // 测试默认值 / Test default values
    TEST_ASSERT_FALSE(config_get_bool("nonexistent.bool", false));
    TEST_ASSERT_EQUAL(100, config_get_int("nonexistent.int", 100));
    TEST_ASSERT_STRING_EQUAL("default", config_get_string("nonexistent.string", "default"));
    
    // 测试删除键 / Test key removal
    TEST_ASSERT(config_remove_key("test.bool_value"));
    TEST_ASSERT_FALSE(config_has_key("test.bool_value"));
    
    // 清理 / Cleanup
    config_manager_cleanup();
    
    return TEST_RESULT_PASS;
}

// 测试编译器配置 / Test Compiler Configuration
TestResult test_compiler_config(void) {
    TEST_ASSERT(config_manager_init(NULL));
    
    // 创建默认编译器配置 / Create default compiler configuration
    CompilerConfig config = config_create_default_compiler_config();
    
    // 验证默认值 / Verify default values
    TEST_ASSERT_TRUE(config.enable_compilation);
    TEST_ASSERT_NOT_NULL(config.compilation_mode);
    TEST_ASSERT_NOT_NULL(config.optimization_level);
    
    // 保存配置 / Save configuration
    TEST_ASSERT(config_save_compiler_config(&config));
    
    // 修改配置 / Modify configuration
    config.enable_debug_info = true;
    config.thread_count = 4;
    config.max_memory_usage = 512 * 1024 * 1024; // 512MB
    
    // 重新保存 / Save again
    TEST_ASSERT(config_save_compiler_config(&config));
    
    // 创建新配置并加载 / Create new configuration and load
    CompilerConfig loaded_config = {0};
    TEST_ASSERT(config_load_compiler_config(&loaded_config));
    
    // 验证加载的配置 / Verify loaded configuration
    TEST_ASSERT_TRUE(loaded_config.enable_compilation);
    TEST_ASSERT_TRUE(loaded_config.enable_debug_info);
    TEST_ASSERT_EQUAL(4, loaded_config.thread_count);
    TEST_ASSERT_EQUAL(512 * 1024 * 1024, loaded_config.max_memory_usage);
    
    // 清理 / Cleanup
    config_free_compiler_config(&config);
    config_free_compiler_config(&loaded_config);
    config_manager_cleanup();
    
    return TEST_RESULT_PASS;
}

// 测试系统集成 / Test System Integration
TestResult test_system_integration(void) {
    // 初始化所有系统 / Initialize all systems
    TEST_ASSERT(error_system_init(10, 100));
    
    MemoryConfig mem_config = {
        .enable_tracking = true,
        .enable_debugging = true,
        .enable_leak_detection = true,
        .enable_bounds_checking = false,
        .max_memory_limit = 0,
        .alignment = 8
    };
    TEST_ASSERT(memory_manager_init(&mem_config));
    
    TEST_ASSERT(config_manager_init(NULL));
    
    // 测试系统间的交互 / Test interaction between systems
    
    // 1. 配置系统影响内存管理 / Configuration system affects memory management
    config_set_bool("memory.enable_tracking", true);
    config_set_int("memory.max_memory_usage", 1024 * 1024);
    
    // 2. 内存分配触发错误报告 / Memory allocation triggers error reporting
    void* large_ptr = memory_alloc(2 * 1024 * 1024, MEM_TYPE_GENERAL); // 超过限制 / Exceeds limit
    // 注意：这可能会失败，这是预期的 / Note: This may fail, which is expected
    
    // 3. 错误系统记录内存问题 / Error system records memory issues
    TEST_ASSERT_TRUE(error_has_errors(ERROR_LEVEL_WARNING));
    
    // 4. 分配正常大小的内存 / Allocate normal-sized memory
    void* normal_ptr = memory_alloc(1024, MEM_TYPE_GENERAL);
    TEST_ASSERT_NOT_NULL(normal_ptr);
    
    // 5. 验证统计信息 / Verify statistics
    const MemoryStats* stats = memory_get_stats();
    TEST_ASSERT_NOT_NULL(stats);
    TEST_ASSERT(stats->allocation_count > 0);
    
    // 清理 / Cleanup
    if (normal_ptr) {
        memory_free(normal_ptr);
    }
    if (large_ptr) {
        memory_free(large_ptr);
    }
    
    config_manager_cleanup();
    memory_manager_cleanup();
    error_system_cleanup();
    
    return TEST_RESULT_PASS;
}

// 性能测试 / Performance Test
TestResult test_infrastructure_performance(void) {
    // 初始化系统 / Initialize systems
    TEST_ASSERT(error_system_init(10, 1000));
    
    MemoryConfig config = {
        .enable_tracking = true,
        .enable_debugging = false, // 关闭调试以提高性能 / Disable debugging for performance
        .enable_leak_detection = false,
        .enable_bounds_checking = false,
        .max_memory_limit = 0,
        .alignment = 8
    };
    TEST_ASSERT(memory_manager_init(&config));
    
    // 性能测试：大量内存分配和释放 / Performance test: massive memory allocation and deallocation
    const int iterations = 10000;
    void** ptrs = malloc(iterations * sizeof(void*));
    TEST_ASSERT_NOT_NULL(ptrs);
    
    TEST_PERFORMANCE_START();
    
    // 分配阶段 / Allocation phase
    for (int i = 0; i < iterations; i++) {
        ptrs[i] = memory_alloc(64, MEM_TYPE_TEMP);
        TEST_ASSERT_NOT_NULL(ptrs[i]);
    }
    
    // 释放阶段 / Deallocation phase
    for (int i = 0; i < iterations; i++) {
        memory_free(ptrs[i]);
    }
    
    TEST_PERFORMANCE_END(5000); // 应该在5秒内完成 / Should complete within 5 seconds
    
    // 验证没有内存泄漏 / Verify no memory leaks
    int leak_count = memory_check_leaks(NULL);
    TEST_ASSERT_EQUAL(0, leak_count);
    
    // 清理 / Cleanup
    free(ptrs);
    memory_manager_cleanup();
    error_system_cleanup();
    
    return TEST_RESULT_PASS;
}

// 创建基础设施测试套件 / Create Infrastructure Test Suite
TestSuite* create_infrastructure_test_suite(void) {
    TestSuite* suite = test_create_suite("Infrastructure", "Basic infrastructure integration tests");
    
    test_add_case(suite, "error_handling_basic", "Basic error handling functionality", 
                 TEST_TYPE_UNIT, test_error_handling_basic);
    
    test_add_case(suite, "memory_management_basic", "Basic memory management functionality", 
                 TEST_TYPE_UNIT, test_memory_management_basic);
    
    test_add_case(suite, "memory_leak_detection", "Memory leak detection functionality", 
                 TEST_TYPE_UNIT, test_memory_leak_detection);
    
    test_add_case(suite, "config_management_basic", "Basic configuration management functionality", 
                 TEST_TYPE_UNIT, test_config_management_basic);
    
    test_add_case(suite, "compiler_config", "Compiler configuration functionality", 
                 TEST_TYPE_UNIT, test_compiler_config);
    
    test_add_case(suite, "system_integration", "System integration test", 
                 TEST_TYPE_INTEGRATION, test_system_integration);
    
    test_add_case(suite, "infrastructure_performance", "Infrastructure performance test", 
                 TEST_TYPE_PERFORMANCE, test_infrastructure_performance);
    
    return suite;
}

// 主测试函数 / Main test function
int main(void) {
    // 初始化测试框架 / Initialize test framework
    TestConfig config = test_create_default_config();
    config.verbose = true;
    config.stop_on_failure = false;
    config.run_performance = true;
    
    if (!test_framework_init(&config)) {
        fprintf(stderr, "Failed to initialize test framework\n");
        return 1;
    }
    
    // 创建并运行测试套件 / Create and run test suite
    TestSuite* suite = create_infrastructure_test_suite();
    bool success = test_run_suite(suite);
    
    // 打印测试报告 / Print test report
    test_print_report(stdout);
    
    // 清理 / Cleanup
    test_framework_cleanup();
    
    return success ? 0 : 1;
}
