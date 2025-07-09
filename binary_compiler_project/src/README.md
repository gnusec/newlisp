# 源代码目录说明 / Source Code Directory Guide

## 目录结构 / Directory Structure

```
src/
├── compiler/           # 编译器核心 / Compiler Core
│   ├── ast_transformer.c    # AST转换器
│   ├── ast_transformer.h
│   ├── code_generator.c     # 代码生成器
│   ├── code_generator.h
│   ├── optimizer.c          # 优化器
│   ├── optimizer.h
│   └── compiler_main.c      # 编译器主程序
├── runtime/            # 运行时支持 / Runtime Support
│   ├── memory_manager.c     # 内存管理器
│   ├── memory_manager.h
│   ├── type_system.c        # 类型系统
│   ├── type_system.h
│   ├── error_handler.c      # 错误处理
│   └── error_handler.h
├── ast_converter/      # AST转换器 / AST Converter
│   ├── newlisp_parser.c     # newLISP解析器
│   ├── newlisp_parser.h
│   ├── ast_visitor.c        # AST访问者
│   ├── ast_visitor.h
│   ├── type_inference.c     # 类型推断
│   └── type_inference.h
└── tinycc_integration/ # TinyCC集成 / TinyCC Integration
    ├── tinycc_wrapper.c     # TinyCC包装器
    ├── tinycc_wrapper.h
    ├── c_code_generator.c   # C代码生成器
    ├── c_code_generator.h
    └── compilation_manager.c # 编译管理器
```

## 编译说明 / Build Instructions

### 依赖要求 / Dependencies

```bash
# Ubuntu/Debian
sudo apt-get install build-essential tcc libtcc-dev

# CentOS/RHEL
sudo yum install gcc tcc tcc-devel

# macOS
brew install tcc
```

### 构建步骤 / Build Steps

```bash
# 1. 进入项目目录
cd binary_compiler_project

# 2. 创建构建目录
mkdir build && cd build

# 3. 配置构建
cmake .. -DCMAKE_BUILD_TYPE=Release

# 4. 编译
make -j$(nproc)

# 5. 安装 (可选)
sudo make install
```

### 快速测试 / Quick Test

```bash
# 编译简单示例
./nlcc ../examples/basic/hello.lsp -o hello

# 运行生成的程序
./hello
```

## 开发指南 / Development Guide

### 代码规范 / Coding Standards

```c
// 1. 命名规范
typedef struct {
    int member_variable;    // 下划线命名
    char* string_data;
} StructName;              // 驼峰命名

// 2. 函数命名
int create_ast_node(void);          // 动词_名词格式
void destroy_memory_manager(void);  // 清理函数以destroy开头

// 3. 常量定义
#define MAX_BUFFER_SIZE 4096        // 全大写，下划线分隔
#define DEFAULT_OPTIMIZATION_LEVEL 2

// 4. 错误处理
typedef enum {
    ERROR_SUCCESS = 0,
    ERROR_PARSE_FAILED,
    ERROR_COMPILE_FAILED,
    ERROR_MEMORY_ALLOCATION
} ErrorCode;
```

### 内存管理规范 / Memory Management Rules

```c
// 1. 所有分配必须有对应的释放
void* ptr = malloc(size);
// ... 使用ptr
free(ptr);
ptr = NULL;  // 防止悬空指针

// 2. 使用包装函数进行内存管理
void* safe_malloc(size_t size);
void safe_free(void** ptr);

// 3. 结构体初始化和清理
typedef struct {
    char* data;
    size_t size;
    void (*cleanup)(struct MyStruct* self);
} MyStruct;

MyStruct* create_my_struct(void);
void destroy_my_struct(MyStruct** self);
```

### 错误处理模式 / Error Handling Pattern

```c
// 统一的错误处理模式
typedef struct {
    ErrorCode code;
    char* message;
    char* file;
    int line;
} ErrorInfo;

// 错误报告宏
#define REPORT_ERROR(code, msg) \
    report_error((code), (msg), __FILE__, __LINE__)

// 函数返回值检查
ErrorCode some_function(void) {
    ErrorCode result = do_something();
    if(result != ERROR_SUCCESS) {
        REPORT_ERROR(result, "Failed to do something");
        return result;
    }
    return ERROR_SUCCESS;
}
```

## 测试框架 / Testing Framework

### 单元测试 / Unit Tests

```c
// 测试框架使用示例
#include "test_framework.h"

// 测试用例定义
TEST_CASE(test_ast_creation) {
    CELL* ast = create_test_ast();
    ASSERT_NOT_NULL(ast);
    ASSERT_EQUAL(ast->type, CELL_EXPRESSION);
    destroy_ast(ast);
}

// 测试套件定义
TEST_SUITE(ast_tests) {
    RUN_TEST(test_ast_creation);
    RUN_TEST(test_ast_transformation);
    RUN_TEST(test_ast_optimization);
}

// 主测试函数
int main(void) {
    RUN_SUITE(ast_tests);
    RUN_SUITE(compiler_tests);
    RUN_SUITE(runtime_tests);
    
    return get_test_result();
}
```

### 集成测试 / Integration Tests

```bash
#!/bin/bash
# integration_test.sh

# 测试基础编译功能
test_basic_compilation() {
    echo "Testing basic compilation..."
    ./nlcc test_input.lsp -o test_output
    if [ $? -eq 0 ]; then
        echo "✓ Compilation successful"
    else
        echo "✗ Compilation failed"
        return 1
    fi
}

# 测试性能提升
test_performance_improvement() {
    echo "Testing performance improvement..."
    
    # 解释执行时间
    interpreted_time=$(time_execution "./newlisp test_script.lsp")
    
    # 编译执行时间
    ./nlcc test_script.lsp -o compiled_test
    compiled_time=$(time_execution "./compiled_test")
    
    # 计算加速比
    speedup=$(echo "scale=2; $interpreted_time / $compiled_time" | bc)
    echo "Speedup: ${speedup}x"
    
    if (( $(echo "$speedup > 1.5" | bc -l) )); then
        echo "✓ Performance improvement achieved"
    else
        echo "✗ Performance improvement insufficient"
        return 1
    fi
}

# 运行所有测试
run_all_tests() {
    test_basic_compilation || exit 1
    test_performance_improvement || exit 1
    echo "All tests passed!"
}

run_all_tests
```

## 调试指南 / Debugging Guide

### 调试工具 / Debugging Tools

```bash
# 1. 使用GDB调试编译器
gdb ./nlcc
(gdb) set args test.lsp -o test_output
(gdb) run
(gdb) bt  # 查看调用栈

# 2. 使用Valgrind检查内存泄漏
valgrind --leak-check=full ./nlcc test.lsp -o test_output

# 3. 使用AddressSanitizer
gcc -fsanitize=address -g compiler.c -o nlcc
./nlcc test.lsp -o test_output
```

### 日志系统 / Logging System

```c
// 日志级别定义
typedef enum {
    LOG_DEBUG,
    LOG_INFO,
    LOG_WARNING,
    LOG_ERROR
} LogLevel;

// 日志宏定义
#define LOG_DEBUG(fmt, ...) \
    log_message(LOG_DEBUG, __FILE__, __LINE__, fmt, ##__VA_ARGS__)

#define LOG_INFO(fmt, ...) \
    log_message(LOG_INFO, __FILE__, __LINE__, fmt, ##__VA_ARGS__)

#define LOG_ERROR(fmt, ...) \
    log_message(LOG_ERROR, __FILE__, __LINE__, fmt, ##__VA_ARGS__)

// 使用示例
void compile_expression(CELL* expr) {
    LOG_DEBUG("Compiling expression of type %d", expr->type);
    
    if(!expr) {
        LOG_ERROR("Null expression passed to compiler");
        return;
    }
    
    LOG_INFO("Successfully compiled expression");
}
```

## 性能分析 / Performance Profiling

### 性能测试工具 / Performance Testing Tools

```c
// 简单的性能计时器
typedef struct {
    struct timespec start_time;
    struct timespec end_time;
} Timer;

void start_timer(Timer* timer) {
    clock_gettime(CLOCK_MONOTONIC, &timer->start_time);
}

double stop_timer(Timer* timer) {
    clock_gettime(CLOCK_MONOTONIC, &timer->end_time);
    
    double start = timer->start_time.tv_sec + timer->start_time.tv_nsec / 1e9;
    double end = timer->end_time.tv_sec + timer->end_time.tv_nsec / 1e9;
    
    return end - start;
}

// 使用示例
void benchmark_compilation(void) {
    Timer timer;
    
    start_timer(&timer);
    compile_program("test.lsp");
    double compile_time = stop_timer(&timer);
    
    printf("Compilation time: %.3f seconds\n", compile_time);
}
```

### 内存使用分析 / Memory Usage Analysis

```c
// 内存使用统计
typedef struct {
    size_t total_allocated;
    size_t current_usage;
    size_t peak_usage;
    int allocation_count;
} MemoryStats;

extern MemoryStats global_memory_stats;

void* tracked_malloc(size_t size) {
    void* ptr = malloc(size);
    if(ptr) {
        global_memory_stats.total_allocated += size;
        global_memory_stats.current_usage += size;
        global_memory_stats.allocation_count++;
        
        if(global_memory_stats.current_usage > global_memory_stats.peak_usage) {
            global_memory_stats.peak_usage = global_memory_stats.current_usage;
        }
    }
    return ptr;
}

void print_memory_stats(void) {
    printf("Memory Statistics:\n");
    printf("  Total allocated: %zu bytes\n", global_memory_stats.total_allocated);
    printf("  Current usage: %zu bytes\n", global_memory_stats.current_usage);
    printf("  Peak usage: %zu bytes\n", global_memory_stats.peak_usage);
    printf("  Allocation count: %d\n", global_memory_stats.allocation_count);
}
```

这个源代码目录为newLISP二进制编译器项目提供了完整的开发框架和指导，确保代码质量和开发效率。
