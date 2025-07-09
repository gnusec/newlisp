/**
 * @file c_code_generator.h
 * @brief 基础C代码生成器 / Basic C Code Generator
 * @author newLISP Binary Compiler Team
 * @date 2024
 * 
 * 将AST转换为C代码的核心模块
 * Core module for converting AST to C code
 */

#ifndef C_CODE_GENERATOR_H
#define C_CODE_GENERATOR_H

#include "../ast_converter/ast_nodes.h"
#include "../runtime/error_handler.h"
#include "../runtime/memory_manager.h"
#include <stdio.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// C代码生成器配置 / C Code Generator Configuration
typedef struct {
    bool enable_debug_info;   // 启用调试信息 / Enable debug info
    bool enable_optimization; // 启用优化 / Enable optimization
    bool use_static_functions; // 使用静态函数 / Use static functions
    int indent_size;          // 缩进大小 / Indent size
    char* include_headers;    // 包含头文件 / Include headers
} CodeGenConfig;

// 代码缓冲区 / Code Buffer
typedef struct {
    char* buffer;             // 缓冲区 / Buffer
    size_t size;              // 当前大小 / Current size
    size_t capacity;          // 容量 / Capacity
    int indent_level;         // 缩进级别 / Indent level
} CodeBuffer;

// 符号表项 / Symbol Table Entry
typedef struct SymbolEntry {
    char* name;               // 符号名 / Symbol name
    DataType type;            // 数据类型 / Data type
    char* c_name;             // C变量名 / C variable name
    bool is_function;         // 是否为函数 / Whether is function
    bool is_parameter;        // 是否为参数 / Whether is parameter
    struct SymbolEntry* next; // 链表下一个 / Next in list
} SymbolEntry;

// 符号表 / Symbol Table
typedef struct {
    SymbolEntry* entries;     // 符号表项 / Symbol entries
    int count;                // 符号数量 / Symbol count
    struct SymbolTable* parent; // 父符号表 / Parent symbol table
} SymbolTable;

// C代码生成器 / C Code Generator
typedef struct {
    CodeBuffer* buffer;       // 代码缓冲区 / Code buffer
    SymbolTable* symbols;     // 符号表 / Symbol table
    CodeGenConfig config;     // 配置 / Configuration
    int temp_var_counter;     // 临时变量计数器 / Temporary variable counter
    int label_counter;        // 标签计数器 / Label counter
    bool has_error;           // 是否有错误 / Whether has error
} CodeGenerator;

// 核心API函数 / Core API Functions

/**
 * @brief 创建代码生成器 / Create code generator
 * @param config 配置 / Configuration
 * @return 代码生成器指针 / Code generator pointer
 */
CodeGenerator* codegen_create(const CodeGenConfig* config);

/**
 * @brief 销毁代码生成器 / Destroy code generator
 * @param codegen 代码生成器 / Code generator
 */
void codegen_destroy(CodeGenerator* codegen);

/**
 * @brief 生成C代码 / Generate C code
 * @param codegen 代码生成器 / Code generator
 * @param ast AST节点 / AST node
 * @return 生成的C代码 / Generated C code
 */
char* codegen_generate(CodeGenerator* codegen, ASTNode* ast);

/**
 * @brief 生成表达式代码 / Generate expression code
 * @param codegen 代码生成器 / Code generator
 * @param node AST节点 / AST node
 * @return 表达式C代码 / Expression C code
 */
char* codegen_generate_expression(CodeGenerator* codegen, ASTNode* node);

/**
 * @brief 生成语句代码 / Generate statement code
 * @param codegen 代码生成器 / Code generator
 * @param node AST节点 / AST node
 * @return 成功返回true / Returns true on success
 */
bool codegen_generate_statement(CodeGenerator* codegen, ASTNode* node);

/**
 * @brief 生成函数代码 / Generate function code
 * @param codegen 代码生成器 / Code generator
 * @param node 函数定义节点 / Function definition node
 * @return 成功返回true / Returns true on success
 */
bool codegen_generate_function(CodeGenerator* codegen, ASTNode* node);

// 代码缓冲区操作 / Code Buffer Operations

/**
 * @brief 创建代码缓冲区 / Create code buffer
 * @param initial_capacity 初始容量 / Initial capacity
 * @return 代码缓冲区指针 / Code buffer pointer
 */
CodeBuffer* codebuffer_create(size_t initial_capacity);

/**
 * @brief 销毁代码缓冲区 / Destroy code buffer
 * @param buffer 代码缓冲区 / Code buffer
 */
void codebuffer_destroy(CodeBuffer* buffer);

/**
 * @brief 追加字符串 / Append string
 * @param buffer 代码缓冲区 / Code buffer
 * @param str 字符串 / String
 * @return 成功返回true / Returns true on success
 */
bool codebuffer_append(CodeBuffer* buffer, const char* str);

/**
 * @brief 追加格式化字符串 / Append formatted string
 * @param buffer 代码缓冲区 / Code buffer
 * @param format 格式字符串 / Format string
 * @param ... 可变参数 / Variable arguments
 * @return 成功返回true / Returns true on success
 */
bool codebuffer_appendf(CodeBuffer* buffer, const char* format, ...);

/**
 * @brief 追加缩进 / Append indent
 * @param buffer 代码缓冲区 / Code buffer
 * @return 成功返回true / Returns true on success
 */
bool codebuffer_append_indent(CodeBuffer* buffer);

/**
 * @brief 增加缩进级别 / Increase indent level
 * @param buffer 代码缓冲区 / Code buffer
 */
void codebuffer_indent(CodeBuffer* buffer);

/**
 * @brief 减少缩进级别 / Decrease indent level
 * @param buffer 代码缓冲区 / Code buffer
 */
void codebuffer_dedent(CodeBuffer* buffer);

/**
 * @brief 追加新行 / Append newline
 * @param buffer 代码缓冲区 / Code buffer
 * @return 成功返回true / Returns true on success
 */
bool codebuffer_append_line(CodeBuffer* buffer, const char* line);

// 符号表操作 / Symbol Table Operations

/**
 * @brief 创建符号表 / Create symbol table
 * @param parent 父符号表 / Parent symbol table
 * @return 符号表指针 / Symbol table pointer
 */
SymbolTable* symboltable_create(SymbolTable* parent);

/**
 * @brief 销毁符号表 / Destroy symbol table
 * @param table 符号表 / Symbol table
 */
void symboltable_destroy(SymbolTable* table);

/**
 * @brief 添加符号 / Add symbol
 * @param table 符号表 / Symbol table
 * @param name 符号名 / Symbol name
 * @param type 数据类型 / Data type
 * @param c_name C变量名 / C variable name
 * @return 成功返回true / Returns true on success
 */
bool symboltable_add(SymbolTable* table, const char* name, DataType type, const char* c_name);

/**
 * @brief 查找符号 / Find symbol
 * @param table 符号表 / Symbol table
 * @param name 符号名 / Symbol name
 * @return 符号表项指针 / Symbol entry pointer
 */
SymbolEntry* symboltable_find(SymbolTable* table, const char* name);

/**
 * @brief 生成C函数名 / Generate C function name
 * @param codegen 代码生成器 / Code generator
 * @param base_name 基础名称 / Base name
 * @return C函数名 / C function name
 */
char* codegen_generate_c_function_name(CodeGenerator* codegen, const char* base_name);

/**
 * @brief 生成唯一C变量名 / Generate unique C variable name
 * @param codegen 代码生成器 / Code generator
 * @param base_name 基础名称 / Base name
 * @return C变量名 / C variable name
 */
char* codegen_generate_c_name(CodeGenerator* codegen, const char* base_name);

// 类型转换 / Type Conversion

/**
 * @brief 获取C类型名 / Get C type name
 * @param type 数据类型 / Data type
 * @return C类型名 / C type name
 */
const char* codegen_get_c_type(DataType type);

/**
 * @brief 生成类型转换代码 / Generate type conversion code
 * @param codegen 代码生成器 / Code generator
 * @param expr 表达式 / Expression
 * @param target_type 目标类型 / Target type
 * @return 转换后的表达式 / Converted expression
 */
char* codegen_generate_type_conversion(CodeGenerator* codegen, const char* expr, DataType target_type);

// 内置函数处理 / Built-in Function Handling

/**
 * @brief 生成内置函数调用 / Generate built-in function call
 * @param codegen 代码生成器 / Code generator
 * @param function_name 函数名 / Function name
 * @param arguments 参数 / Arguments
 * @param arg_count 参数数量 / Argument count
 * @return 函数调用代码 / Function call code
 */
char* codegen_generate_builtin_call(CodeGenerator* codegen, const char* function_name, 
                                   char** arguments, size_t arg_count);

/**
 * @brief 生成算术运算 / Generate arithmetic operation
 * @param codegen 代码生成器 / Code generator
 * @param op 运算符 / Operator
 * @param left 左操作数 / Left operand
 * @param right 右操作数 / Right operand
 * @return 运算表达式 / Operation expression
 */
char* codegen_generate_arithmetic(CodeGenerator* codegen, BinaryOpType op, 
                                 const char* left, const char* right);

// 运行时支持 / Runtime Support

/**
 * @brief 生成运行时头文件 / Generate runtime headers
 * @param codegen 代码生成器 / Code generator
 * @return 成功返回true / Returns true on success
 */
bool codegen_generate_headers(CodeGenerator* codegen);

/**
 * @brief 生成用户自定义函数 / Generate user-defined functions
 * @param codegen 代码生成器 / Code generator
 * @param program_ast 程序AST / Program AST
 * @return 成功返回true / Returns true on success
 */
bool codegen_generate_user_functions(CodeGenerator* codegen, ASTNode* program_ast);

/**
 * @brief 生成main函数 / Generate main function
 * @param codegen 代码生成器 / Code generator
 * @param program_ast 程序AST / Program AST
 * @return 成功返回true / Returns true on success
 */
bool codegen_generate_main(CodeGenerator* codegen, ASTNode* program_ast);

/**
 * @brief 生成运行时支持函数 / Generate runtime support functions
 * @param codegen 代码生成器 / Code generator
 * @return 成功返回true / Returns true on success
 */
bool codegen_generate_runtime_support(CodeGenerator* codegen);

// 便利函数 / Convenience Functions

/**
 * @brief 创建默认配置 / Create default configuration
 * @return 默认配置 / Default configuration
 */
CodeGenConfig codegen_create_default_config(void);

/**
 * @brief 生成临时变量名 / Generate temporary variable name
 * @param codegen 代码生成器 / Code generator
 * @return 临时变量名 / Temporary variable name
 */
char* codegen_generate_temp_var(CodeGenerator* codegen);

/**
 * @brief 生成标签名 / Generate label name
 * @param codegen 代码生成器 / Code generator
 * @param prefix 前缀 / Prefix
 * @return 标签名 / Label name
 */
char* codegen_generate_label(CodeGenerator* codegen, const char* prefix);

/**
 * @brief 转义字符串 / Escape string
 * @param str 原字符串 / Original string
 * @return 转义后的字符串 / Escaped string
 */
char* codegen_escape_string(const char* str);

// 错误处理 / Error Handling

/**
 * @brief 报告代码生成错误 / Report code generation error
 * @param codegen 代码生成器 / Code generator
 * @param message 错误消息 / Error message
 */
void codegen_error(CodeGenerator* codegen, const char* message);

/**
 * @brief 检查是否有错误 / Check if has errors
 * @param codegen 代码生成器 / Code generator
 * @return 有错误返回true / Returns true if has errors
 */
bool codegen_has_errors(const CodeGenerator* codegen);

/**
 * @brief 生成max/min函数代码 / Generate max/min function code
 * @param codegen 代码生成器 / Code generator
 * @param args 参数数组 / Arguments array
 * @param arg_count 参数数量 / Argument count
 * @param is_max 是否为max函数 / Whether it's max function
 * @return 生成的代码字符串 / Generated code string
 */
char* codegen_generate_max_min(CodeGenerator* codegen, ASTNode** args, size_t arg_count, bool is_max);

#ifdef __cplusplus
}
#endif

#endif // C_CODE_GENERATOR_H
