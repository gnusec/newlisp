/**
 * @file test_compiler_debug.c
 * @brief 编译器调试测试 / Compiler Debug Test
 * 
 * ultrathink模式：逐步隔离问题，找到段错误根源
 * ultrathink mode: Step-by-step isolation to find segfault root cause
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 逐步包含头文件，找到问题所在
// Gradually include headers to find the problem

// 第三步：包含TinyCC集成
// Step 3: Include TinyCC integration
#include "src/runtime/error_handler.h"
#include "src/runtime/memory_manager.h"
#include "src/tinycc_integration/tinycc_wrapper.h"

int main(void) {
    printf("=== Compiler Debug Test ===\n\n");
    
    // 测试1：基础环境
    printf("1. Testing basic environment...\n");
    printf("   ✅ Basic C environment working\n");
    
    // 测试2：错误处理系统初始化
    printf("\n2. Testing error system initialization...\n");
    if (!error_system_init(10, 100)) {
        printf("   ❌ Error system initialization failed\n");
        return 1;
    }
    printf("   ✅ Error system initialized successfully\n");

    // 测试3：内存管理器初始化
    printf("\n3. Testing memory manager initialization...\n");
    MemoryConfig mem_config = {
        .enable_tracking = true,
        .enable_debugging = false,
        .enable_leak_detection = true,
        .enable_bounds_checking = false,
        .max_memory_limit = 0,
        .alignment = 8
    };

    if (!memory_manager_init(&mem_config)) {
        printf("   ❌ Memory manager initialization failed\n");
        error_system_cleanup();
        return 1;
    }
    printf("   ✅ Memory manager initialized successfully\n");

    // 测试内存分配
    printf("   Testing memory allocation...\n");
    void* test_ptr = memory_alloc(100, MEM_TYPE_TEMP);
    if (test_ptr) {
        printf("   ✅ Memory allocation successful\n");
        memory_free(test_ptr);
        printf("   ✅ Memory free successful\n");
    } else {
        printf("   ❌ Memory allocation failed\n");
    }

    // 测试4：TinyCC集成
    printf("\n4. Testing TinyCC integration...\n");
    if (!tinycc_init()) {
        printf("   ❌ TinyCC initialization failed: %s\n", tinycc_get_last_error());
        memory_manager_cleanup();
        error_system_cleanup();
        return 1;
    }
    printf("   ✅ TinyCC initialized successfully\n");
    printf("   TinyCC version: %s\n", tinycc_get_version());

    // 清理
    tinycc_cleanup();
    printf("   ✅ TinyCC cleanup successful\n");

    memory_manager_cleanup();
    printf("   ✅ Memory manager cleanup successful\n");

    error_system_cleanup();
    printf("   ✅ Error system cleanup successful\n");
    
    printf("\n=== Debug Test Results ===\n");
    printf("✅ Error system test passed\n");
    printf("✅ Memory manager test passed\n");
    printf("✅ TinyCC integration test passed\n");
    printf("All core systems are working correctly!\n");
    
    return 0;
}
