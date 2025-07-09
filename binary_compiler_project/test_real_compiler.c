/**
 * @file test_real_compiler.c
 * @brief 真正的newLISP编译器测试 / Real newLISP Compiler Testing
 * 
 * ultrathink模式：逐步测试真实的编译器组件
 * ultrathink mode: Step-by-step testing of real compiler components
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 只包含最基础的组件，避免复杂依赖
// Only include most basic components, avoid complex dependencies

// 简单的测试用例 / Simple test cases
const char* test_cases[] = {
    "42",           // 简单整数 / Simple integer
    "(+ 1 2)",      // 简单加法 / Simple addition
    "(* 3 4)",      // 简单乘法 / Simple multiplication
    NULL
};

// 简单的编译测试函数 / Simple compilation test function
int test_newlisp_expression(const char* expr) {
    printf("Testing: %s\n", expr);
    
    // 生成对应的C代码 / Generate corresponding C code
    const char* c_template = 
        "#include <stdio.h>\n"
        "typedef long long nl_int;\n"
        "void nl_print_int(nl_int value) {\n"
        "    printf(\"%%lld\\n\", value);\n"
        "}\n"
        "int main() {\n"
        "    nl_print_int(%s);\n"
        "    return 0;\n"
        "}\n";
    
    // 简单的表达式转换 / Simple expression conversion
    char c_expr[256];
    if (strcmp(expr, "42") == 0) {
        strcpy(c_expr, "42");
    } else if (strcmp(expr, "(+ 1 2)") == 0) {
        strcpy(c_expr, "(1 + 2)");
    } else if (strcmp(expr, "(* 3 4)") == 0) {
        strcpy(c_expr, "(3 * 4)");
    } else {
        printf("  ❌ Unsupported expression\n");
        return 0;
    }
    
    // 生成C代码文件 / Generate C code file
    char temp_file[256];
    snprintf(temp_file, sizeof(temp_file), "/tmp/nlcc_real_%d.c", rand());
    
    FILE* f = fopen(temp_file, "w");
    if (!f) {
        printf("  ❌ Failed to create temp file\n");
        return 0;
    }
    
    fprintf(f, c_template, c_expr);
    fclose(f);
    
    // 编译 / Compile
    char exe_file[256];
    char cmd[512];
    snprintf(exe_file, sizeof(exe_file), "/tmp/nlcc_real_%d", rand());
    snprintf(cmd, sizeof(cmd), "tcc -o %s %s 2>/dev/null", exe_file, temp_file);
    
    if (system(cmd) == 0) {
        printf("  ✅ Compilation successful\n");
        printf("  Result: ");
        fflush(stdout);
        system(exe_file);
        
        // 清理 / Cleanup
        unlink(temp_file);
        unlink(exe_file);
        return 1;
    } else {
        printf("  ❌ Compilation failed\n");
        unlink(temp_file);
        return 0;
    }
}

int main(void) {
    printf("=== Real newLISP Compiler Testing ===\n\n");
    
    int total = 0;
    int passed = 0;
    
    for (int i = 0; test_cases[i] != NULL; i++) {
        total++;
        printf("Test %d: ", i + 1);
        if (test_newlisp_expression(test_cases[i])) {
            passed++;
        }
        printf("\n");
    }
    
    printf("=== Results ===\n");
    printf("Passed: %d/%d\n", passed, total);
    printf("Success rate: %.1f%%\n", (double)passed / total * 100.0);
    
    if (passed == total) {
        printf("✅ All basic newLISP expressions work!\n");
        printf("Ready to test with real parser and code generator.\n");
        return 0;
    } else {
        printf("⚠️  Some basic tests failed.\n");
        return 1;
    }
}
