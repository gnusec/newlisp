# TinyCC架构设计 / TinyCC Architecture Design

## 系统架构概览 / System Architecture Overview

本文档详细描述TinyCC集成方案的技术架构，包括接口设计、数据流转换、内存管理策略和错误处理机制。

This document details the technical architecture of the TinyCC integration approach, including interface design, data flow transformation, memory management strategies, and error handling mechanisms.

## 整体架构图 / Overall Architecture Diagram

```
┌─────────────────────────────────────────────────────────────────┐
│                    newLISP Binary Compiler                     │
├─────────────────────────────────────────────────────────────────┤
│  Input Layer / 输入层                                           │
│  ┌─────────────┐    ┌─────────────┐    ┌─────────────┐        │
│  │ .lsp Files  │    │ REPL Input  │    │ String Code │        │
│  └─────────────┘    └─────────────┘    └─────────────┘        │
├─────────────────────────────────────────────────────────────────┤
│  Parsing Layer / 解析层                                         │
│  ┌─────────────┐    ┌─────────────┐    ┌─────────────┐        │
│  │ Lexer       │ -> │ Parser      │ -> │ AST Builder │        │
│  │ 词法分析器   │    │ 语法分析器   │    │ AST构建器   │        │
│  └─────────────┘    └─────────────┘    └─────────────┘        │
├─────────────────────────────────────────────────────────────────┤
│  Analysis Layer / 分析层                                        │
│  ┌─────────────┐    ┌─────────────┐    ┌─────────────┐        │
│  │ Type        │    │ Symbol      │    │ Optimization│        │
│  │ Inference   │    │ Resolution  │    │ Analysis    │        │
│  │ 类型推断     │    │ 符号解析     │    │ 优化分析     │        │
│  └─────────────┘    └─────────────┘    └─────────────┘        │
├─────────────────────────────────────────────────────────────────┤
│  Code Generation Layer / 代码生成层                             │
│  ┌─────────────┐    ┌─────────────┐    ┌─────────────┐        │
│  │ C Code      │    │ Runtime     │    │ Memory      │        │
│  │ Generator   │    │ Support     │    │ Management  │        │
│  │ C代码生成器  │    │ 运行时支持   │    │ 内存管理     │        │
│  └─────────────┘    └─────────────┘    └─────────────┘        │
├─────────────────────────────────────────────────────────────────┤
│  Compilation Layer / 编译层                                     │
│  ┌─────────────┐    ┌─────────────┐    ┌─────────────┐        │
│  │ TinyCC      │    │ Linker      │    │ Optimizer   │        │
│  │ Integration │    │ 链接器      │    │ 优化器      │        │
│  └─────────────┘    └─────────────┘    └─────────────┘        │
├─────────────────────────────────────────────────────────────────┤
│  Output Layer / 输出层                                          │
│  ┌─────────────┐    ┌─────────────┐    ┌─────────────┐        │
│  │ Executable  │    │ Debug Info  │    │ Performance │        │
│  │ Binary      │    │ 调试信息    │    │ Report      │        │
│  │ 可执行文件   │    │             │    │ 性能报告     │        │
│  └─────────────┘    └─────────────┘    └─────────────┘        │
└─────────────────────────────────────────────────────────────────┘
```

## 核心组件设计 / Core Component Design

### 1. AST转换引擎 / AST Transformation Engine

#### 接口定义 / Interface Definition

```c
// AST转换器主接口
typedef struct {
    CELL* (*transform)(CELL* ast, TransformContext* context);
    char* (*generate_c_code)(CELL* transformed_ast);
    void  (*cleanup)(void* resources);
} ASTTransformer;

// 转换上下文
typedef struct {
    SymbolTable* symbols;        // 符号表
    TypeTable* types;           // 类型表
    OptimizationLevel level;    // 优化级别
    ErrorHandler* error_handler; // 错误处理器
    MemoryManager* memory;      // 内存管理器
} TransformContext;

// 主要转换函数
ASTTransformer* create_ast_transformer(void);
char* transform_newlisp_to_c(CELL* ast, TransformContext* context);
void destroy_ast_transformer(ASTTransformer* transformer);
```

#### 实现架构 / Implementation Architecture

```c
// 转换器实现结构
typedef struct {
    // 访问者模式实现
    struct {
        char* (*visit_expression)(CELL* expr, TransformContext* ctx);
        char* (*visit_symbol)(CELL* symbol, TransformContext* ctx);
        char* (*visit_literal)(CELL* literal, TransformContext* ctx);
        char* (*visit_lambda)(CELL* lambda, TransformContext* ctx);
    } visitors;
    
    // 代码生成器
    struct {
        CodeBuffer* buffer;
        int indent_level;
        bool in_function;
        char* current_function_name;
    } generator;
    
    // 优化器
    struct {
        bool constant_folding;
        bool dead_code_elimination;
        bool function_inlining;
    } optimizer;
} ASTTransformerImpl;
```

### 2. TinyCC集成接口 / TinyCC Integration Interface

#### TinyCC包装器设计 / TinyCC Wrapper Design

```c
// TinyCC编译器包装器
typedef struct {
    TCCState* tcc_state;
    CompilerOptions options;
    ErrorCollector* errors;
    PerformanceMonitor* perf_monitor;
} TinyCompiler;

// 编译器选项
typedef struct {
    OptimizationLevel optimization;
    bool debug_info;
    bool static_linking;
    char* output_format;  // "exe", "obj", "dll"
    char** include_paths;
    char** library_paths;
    char** libraries;
} CompilerOptions;

// 主要接口函数
TinyCompiler* create_tiny_compiler(CompilerOptions* options);
CompileResult* compile_c_code(TinyCompiler* compiler, char* c_source);
bool output_executable(TinyCompiler* compiler, char* filename);
void destroy_tiny_compiler(TinyCompiler* compiler);
```

## 数据流转换流程 / Data Flow Transformation Process

### 转换流水线 / Transformation Pipeline

```
newLISP AST → 类型分析 → 符号解析 → 优化分析 → C代码生成 → TinyCC编译
     ↓           ↓         ↓         ↓          ↓           ↓
   CELL树    TypeInfo   SymbolRef  OptAST    C Source   Binary
```

### 详细转换步骤 / Detailed Transformation Steps

#### 步骤1: AST预处理 / Step 1: AST Preprocessing

```c
// AST预处理器
typedef struct {
    CELL* (*normalize)(CELL* ast);
    CELL* (*validate)(CELL* ast);
    CELL* (*optimize)(CELL* ast);
} ASTPreprocessor;

// 规范化处理
CELL* normalize_ast(CELL* ast) {
    // 1. 展开宏定义
    ast = expand_macros(ast);
    
    // 2. 规范化语法糖
    ast = desugar_syntax(ast);
    
    // 3. 标准化函数调用
    ast = normalize_function_calls(ast);
    
    return ast;
}
```

#### 步骤2: 类型推断 / Step 2: Type Inference

```c
// 类型推断引擎
typedef struct {
    int inferred_type;
    int confidence;
    char* c_type_name;
    bool requires_runtime_check;
} TypeInfo;

// 类型推断实现
TypeInfo infer_expression_type(CELL* expr, TypeContext* ctx) {
    TypeInfo info = {0};
    
    switch(expr->type) {
        case CELL_LONG:
            info.inferred_type = TYPE_INTEGER;
            info.confidence = 100;
            info.c_type_name = "int64_t";
            info.requires_runtime_check = false;
            break;
            
        case CELL_EXPRESSION: {
            CELL* func = (CELL*)expr->contents;
            if(func->type == CELL_SYMBOL) {
                SYMBOL* sym = (SYMBOL*)func->contents;
                info = infer_function_return_type(sym, func->next, ctx);
            }
            break;
        }
    }
    
    return info;
}
```

#### 步骤3: C代码生成 / Step 3: C Code Generation

```c
// C代码生成器
typedef struct {
    char* buffer;
    size_t buffer_size;
    size_t current_pos;
    int indent_level;
    SymbolTable* symbols;
    TypeTable* types;
} CCodeGenerator;

// 表达式转换
char* generate_expression_code(CELL* expr, CCodeGenerator* gen) {
    switch(expr->type) {
        case CELL_LONG:
            return generate_integer_code(expr, gen);
        case CELL_EXPRESSION:
            return generate_function_call_code(expr, gen);
        case CELL_SYMBOL:
            return generate_symbol_reference_code(expr, gen);
        default:
            return generate_default_code(expr, gen);
    }
}

// 函数调用代码生成
char* generate_function_call_code(CELL* expr, CCodeGenerator* gen) {
    CELL* func = (CELL*)expr->contents;
    SYMBOL* func_sym = (SYMBOL*)func->contents;
    CELL* args = func->next;
    
    if(is_builtin_function(func_sym->name)) {
        return generate_builtin_call(func_sym->name, args, gen);
    } else {
        return generate_user_function_call(func_sym->name, args, gen);
    }
}
```

## 内存管理策略 / Memory Management Strategy

### ORO到C内存管理映射 / ORO to C Memory Management Mapping

```c
// 内存管理包装器
typedef struct {
    void* (*alloc)(size_t size);
    void  (*free)(void* ptr);
    void* (*realloc)(void* ptr, size_t new_size);
    void  (*gc_collect)(void);
} MemoryManager;

// newLISP ORO模拟
typedef struct {
    void* data;
    int ref_count;
    size_t size;
    bool marked;  // GC标记
} ManagedObject;

// 内存分配包装
void* managed_alloc(size_t size) {
    ManagedObject* obj = malloc(sizeof(ManagedObject) + size);
    obj->data = (char*)obj + sizeof(ManagedObject);
    obj->ref_count = 1;
    obj->size = size;
    obj->marked = false;
    
    register_object(obj);  // 注册到GC系统
    return obj->data;
}

// 引用计数管理
void managed_retain(void* ptr) {
    ManagedObject* obj = get_object_header(ptr);
    obj->ref_count++;
}

void managed_release(void* ptr) {
    ManagedObject* obj = get_object_header(ptr);
    if(--obj->ref_count == 0) {
        unregister_object(obj);
        free(obj);
    }
}
```

### 垃圾收集策略 / Garbage Collection Strategy

```c
// 简化的标记-清除GC
typedef struct {
    ManagedObject** objects;
    size_t object_count;
    size_t capacity;
    bool gc_enabled;
} GCManager;

// GC执行
void gc_collect(GCManager* gc) {
    if(!gc->gc_enabled) return;
    
    // 1. 标记阶段 - 从根对象开始标记
    mark_reachable_objects(gc);
    
    // 2. 清除阶段 - 释放未标记对象
    sweep_unmarked_objects(gc);
    
    // 3. 重置标记
    reset_marks(gc);
}

// 自动GC触发
void check_gc_trigger(GCManager* gc) {
    static size_t last_collection_size = 0;
    
    if(gc->object_count > last_collection_size * 2 + 1000) {
        gc_collect(gc);
        last_collection_size = gc->object_count;
    }
}
```

## 错误处理机制 / Error Handling Mechanism

### 错误分类与处理 / Error Classification & Handling

```c
// 错误类型定义
typedef enum {
    ERROR_PARSE,           // 解析错误
    ERROR_TYPE_MISMATCH,   // 类型不匹配
    ERROR_UNDEFINED_SYMBOL, // 未定义符号
    ERROR_COMPILATION,     // 编译错误
    ERROR_RUNTIME,         // 运行时错误
    ERROR_MEMORY          // 内存错误
} ErrorType;

// 错误信息结构
typedef struct {
    ErrorType type;
    char* message;
    char* source_location;
    int line_number;
    int column_number;
    CELL* context_ast;
    char* suggestion;
} ErrorInfo;

// 错误处理器
typedef struct {
    ErrorInfo* errors;
    size_t error_count;
    size_t capacity;
    bool fatal_error_occurred;
} ErrorHandler;
```

### 错误恢复策略 / Error Recovery Strategy

```c
// 错误恢复机制
typedef struct {
    bool (*can_recover)(ErrorInfo* error);
    CELL* (*recover)(ErrorInfo* error, CELL* context);
    void (*report)(ErrorInfo* error);
} ErrorRecovery;

// 编译时错误恢复
bool recover_from_compile_error(ErrorInfo* error, CompilerContext* ctx) {
    switch(error->type) {
        case ERROR_TYPE_MISMATCH:
            // 尝试类型转换
            return attempt_type_conversion(error, ctx);
            
        case ERROR_UNDEFINED_SYMBOL:
            // 提供符号建议
            suggest_similar_symbols(error, ctx);
            return false;  // 无法自动恢复
            
        case ERROR_COMPILATION:
            // 回退到解释执行
            return fallback_to_interpreter(error, ctx);
            
        default:
            return false;
    }
}
```

这个架构设计为TinyCC集成方案提供了完整的技术框架，确保系统的可扩展性、可维护性和高性能。
