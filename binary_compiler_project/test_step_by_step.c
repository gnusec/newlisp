/**
 * @file test_step_by_step.c
 * @brief 逐步测试编译器组件 / Step-by-step Compiler Component Testing
 * 
 * ultrathink模式：自底向上验证每个组件
 * ultrathink mode: Bottom-up verification of each component
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 只包含必要的头文件，逐步测试
// Only include necessary headers, test step by step

int main(void) {
    printf("=== Step-by-step Compiler Component Testing ===\n\n");
    
    // 第一步：测试基础C环境 / Step 1: Test basic C environment
    printf("Step 1: Testing basic C environment...\n");
    
    char* test_str = malloc(20);
    if (test_str) {
        strcpy(test_str, "Hello");
        printf("   ✅ Basic memory allocation works: %s\n", test_str);
        free(test_str);
    } else {
        printf("   ❌ Basic memory allocation failed\n");
        return 1;
    }
    
    // 第二步：测试TinyCC可用性 / Step 2: Test TinyCC availability
    printf("\nStep 2: Testing TinyCC availability...\n");
    
    int tcc_result = system("tcc --version >/dev/null 2>&1");
    if (tcc_result == 0) {
        printf("   ✅ TinyCC is available\n");
        
        // 显示TinyCC版本 / Show TinyCC version
        printf("   TinyCC version: ");
        fflush(stdout);
        system("tcc --version 2>&1 | head -1");
    } else {
        printf("   ❌ TinyCC is not available\n");
        return 1;
    }
    
    // 第三步：测试简单的C代码编译 / Step 3: Test simple C code compilation
    printf("\nStep 3: Testing simple C code compilation...\n");
    
    const char* simple_c_code = 
        "#include <stdio.h>\n"
        "int main() {\n"
        "    printf(\"Hello from compiled C!\\n\");\n"
        "    return 42;\n"
        "}\n";
    
    // 写入临时文件 / Write to temporary file
    FILE* temp_file = fopen("/tmp/test_simple.c", "w");
    if (temp_file) {
        fwrite(simple_c_code, 1, strlen(simple_c_code), temp_file);
        fclose(temp_file);
        
        // 编译 / Compile
        int compile_result = system("tcc -o /tmp/test_simple /tmp/test_simple.c 2>/dev/null");
        if (compile_result == 0) {
            printf("   ✅ Simple C code compilation successful\n");
            
            // 运行 / Run
            printf("   Running compiled program: ");
            fflush(stdout);
            int run_result = system("/tmp/test_simple");
            printf("   Exit code: %d\n", WEXITSTATUS(run_result));
            
            // 清理 / Cleanup
            system("rm -f /tmp/test_simple /tmp/test_simple.c");
        } else {
            printf("   ❌ Simple C code compilation failed\n");
            return 1;
        }
    } else {
        printf("   ❌ Failed to create temporary file\n");
        return 1;
    }
    
    printf("\n✅ All step-by-step tests passed!\n");
    printf("Basic compilation environment is ready.\n");
    printf("Ready to test newLISP compiler components.\n");
    
    return 0;
}
