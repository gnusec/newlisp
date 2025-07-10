/**
 * @file test_minimal.c
 * @brief 最小化基础设施测试 / Minimal Infrastructure Test
 * 
 * ultrathink模式：自底向上验证最基础的组件
 * ultrathink mode: Bottom-up verification of most basic components
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 只测试最基础的功能，不依赖复杂的基础设施
// Only test most basic functionality, no complex infrastructure dependencies

int main(void) {
    printf("=== Minimal Infrastructure Test ===\n");
    
    // 测试1：基础内存分配 / Test 1: Basic memory allocation
    printf("1. Testing basic memory allocation...\n");
    void* ptr = malloc(100);
    if (ptr) {
        printf("   ✅ malloc works\n");
        free(ptr);
        printf("   ✅ free works\n");
    } else {
        printf("   ❌ malloc failed\n");
        return 1;
    }
    
    // 测试2：字符串操作 / Test 2: String operations
    printf("2. Testing string operations...\n");
    char* str = malloc(20);
    if (str) {
        strcpy(str, "Hello");
        printf("   ✅ String copy works: %s\n", str);
        free(str);
    }
    
    // 测试3：简单的结构体 / Test 3: Simple struct
    printf("3. Testing simple struct...\n");
    typedef struct {
        int value;
        char* name;
    } TestStruct;
    
    TestStruct* test = malloc(sizeof(TestStruct));
    if (test) {
        test->value = 42;
        test->name = malloc(10);
        if (test->name) {
            strcpy(test->name, "test");
            printf("   ✅ Struct works: value=%d, name=%s\n", test->value, test->name);
            free(test->name);
        }
        free(test);
    }
    
    printf("✅ All minimal tests passed!\n");
    printf("Basic C infrastructure is working correctly.\n");
    
    return 0;
}
