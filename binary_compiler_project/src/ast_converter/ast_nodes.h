/**
 * @file ast_nodes.h
 * @brief 基础AST节点定义 / Basic AST Node Definitions
 * @author newLISP Binary Compiler Team
 * @date 2024
 * 
 * 定义编译器使用的基础AST节点结构，专注于核心功能
 * Defines basic AST node structures used by the compiler, focusing on core functionality
 */

#ifndef AST_NODES_H
#define AST_NODES_H

#include "../runtime/memory_manager.h"
#include "../runtime/error_handler.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// AST节点类型 / AST Node Types (精简版)
typedef enum {
    AST_NODE_INVALID = 0,     // 无效节点 / Invalid node
    
    // 字面量节点 / Literal nodes
    AST_NODE_INTEGER,         // 整数 / Integer
    AST_NODE_FLOAT,           // 浮点数 / Float
    AST_NODE_STRING,          // 字符串 / String
    AST_NODE_SYMBOL,          // 符号 / Symbol
    AST_NODE_BOOLEAN,         // 布尔值 / Boolean
    
    // 表达式节点 / Expression nodes
    AST_NODE_LIST,            // 列表 / List
    AST_NODE_FUNCTION_CALL,   // 函数调用 / Function call
    AST_NODE_BINARY_OP,       // 二元运算 / Binary operation
    AST_NODE_UNARY_OP,        // 一元运算 / Unary operation
    
    // 控制流节点 / Control flow nodes
    AST_NODE_IF,              // 条件语句 / If statement
    AST_NODE_WHILE,           // 循环语句 / While loop
    AST_NODE_BLOCK,           // 代码块 / Code block
    
    // 定义节点 / Definition nodes
    AST_NODE_FUNCTION_DEF,    // 函数定义 / Function definition
    AST_NODE_VARIABLE_DEF,    // 变量定义 / Variable definition
    
    AST_NODE_COUNT            // 节点类型数量 / Node type count
} ASTNodeType;

// 二元运算符类型 / Binary Operator Types
typedef enum {
    BINARY_OP_ADD = 0,        // 加法 / Addition
    BINARY_OP_SUB,            // 减法 / Subtraction
    BINARY_OP_MUL,            // 乘法 / Multiplication
    BINARY_OP_DIV,            // 除法 / Division
    BINARY_OP_MOD,            // 取模 / Modulo
    BINARY_OP_EQ,             // 等于 / Equal
    BINARY_OP_NE,             // 不等于 / Not equal
    BINARY_OP_LT,             // 小于 / Less than
    BINARY_OP_LE,             // 小于等于 / Less than or equal
    BINARY_OP_GT,             // 大于 / Greater than
    BINARY_OP_GE,             // 大于等于 / Greater than or equal
    BINARY_OP_AND,            // 逻辑与 / Logical AND
    BINARY_OP_OR,             // 逻辑或 / Logical OR
    BINARY_OP_COUNT
} BinaryOpType;

// 一元运算符类型 / Unary Operator Types
typedef enum {
    UNARY_OP_NEG = 0,         // 负号 / Negation
    UNARY_OP_NOT,             // 逻辑非 / Logical NOT
    UNARY_OP_COUNT
} UnaryOpType;

// 基础数据类型 / Basic Data Types
typedef enum {
    DATA_TYPE_UNKNOWN = 0,    // 未知类型 / Unknown type
    DATA_TYPE_INTEGER,        // 整数类型 / Integer type
    DATA_TYPE_FLOAT,          // 浮点类型 / Float type
    DATA_TYPE_STRING,         // 字符串类型 / String type
    DATA_TYPE_BOOLEAN,        // 布尔类型 / Boolean type
    DATA_TYPE_LIST,           // 列表类型 / List type
    DATA_TYPE_FUNCTION,       // 函数类型 / Function type
    DATA_TYPE_COUNT
} DataType;

// 前向声明 / Forward declarations
typedef struct ASTNode ASTNode;

// AST节点基础结构 / AST Node Base Structure
struct ASTNode {
    ASTNodeType type;         // 节点类型 / Node type
    DataType data_type;       // 数据类型 / Data type
    int line;                 // 源码行号 / Source line number
    int column;               // 源码列号 / Source column number
    
    // 节点特定数据 / Node-specific data
    union {
        // 字面量数据 / Literal data
        struct {
            int64_t int_value;    // 整数值 / Integer value
            double float_value;   // 浮点值 / Float value
            char* string_value;   // 字符串值 / String value
            bool bool_value;      // 布尔值 / Boolean value
        } literal;
        
        // 符号数据 / Symbol data
        struct {
            char* name;           // 符号名 / Symbol name
        } symbol;
        
        // 列表数据 / List data
        struct {
            ASTNode** elements;   // 元素数组 / Element array
            size_t count;         // 元素数量 / Element count
            size_t capacity;      // 数组容量 / Array capacity
        } list;
        
        // 函数调用数据 / Function call data
        struct {
            ASTNode* function;    // 函数节点 / Function node
            ASTNode** arguments;  // 参数数组 / Argument array
            size_t arg_count;     // 参数数量 / Argument count
        } call;
        
        // 二元运算数据 / Binary operation data
        struct {
            BinaryOpType op;      // 运算符 / Operator
            ASTNode* left;        // 左操作数 / Left operand
            ASTNode* right;       // 右操作数 / Right operand
        } binary_op;
        
        // 一元运算数据 / Unary operation data
        struct {
            UnaryOpType op;       // 运算符 / Operator
            ASTNode* operand;     // 操作数 / Operand
        } unary_op;
        
        // 条件语句数据 / If statement data
        struct {
            ASTNode* condition;   // 条件 / Condition
            ASTNode* then_branch; // then分支 / Then branch
            ASTNode* else_branch; // else分支 / Else branch
        } if_stmt;
        
        // 循环语句数据 / While loop data
        struct {
            ASTNode* condition;   // 条件 / Condition
            ASTNode* body;        // 循环体 / Loop body
        } while_stmt;
        
        // 代码块数据 / Block data
        struct {
            ASTNode** statements; // 语句数组 / Statement array
            size_t count;         // 语句数量 / Statement count
        } block;
        
        // 函数定义数据 / Function definition data
        struct {
            char* name;           // 函数名 / Function name
            char** parameters;    // 参数名数组 / Parameter name array
            size_t param_count;   // 参数数量 / Parameter count
            ASTNode* body;        // 函数体 / Function body
        } function_def;
        
        // 变量定义数据 / Variable definition data
        struct {
            char* name;           // 变量名 / Variable name
            ASTNode* value;       // 初始值 / Initial value
        } variable_def;
    } data;
    
    // 元数据 / Metadata
    char* source_text;        // 源码文本 / Source text
    void* user_data;          // 用户数据 / User data
};

// 核心API函数 / Core API Functions

/**
 * @brief 创建AST节点 / Create AST node
 * @param type 节点类型 / Node type
 * @return AST节点指针 / AST node pointer
 */
ASTNode* ast_create_node(ASTNodeType type);

/**
 * @brief 销毁AST节点 / Destroy AST node
 * @param node AST节点 / AST node
 */
void ast_destroy_node(ASTNode* node);

/**
 * @brief 创建整数字面量节点 / Create integer literal node
 * @param value 整数值 / Integer value
 * @return AST节点指针 / AST node pointer
 */
ASTNode* ast_create_integer(int64_t value);

/**
 * @brief 创建浮点字面量节点 / Create float literal node
 * @param value 浮点值 / Float value
 * @return AST节点指针 / AST node pointer
 */
ASTNode* ast_create_float(double value);

/**
 * @brief 创建字符串字面量节点 / Create string literal node
 * @param value 字符串值 / String value
 * @return AST节点指针 / AST node pointer
 */
ASTNode* ast_create_string(const char* value);

/**
 * @brief 创建符号节点 / Create symbol node
 * @param name 符号名 / Symbol name
 * @return AST节点指针 / AST node pointer
 */
ASTNode* ast_create_symbol(const char* name);

/**
 * @brief 创建布尔字面量节点 / Create boolean literal node
 * @param value 布尔值 / Boolean value
 * @return AST节点指针 / AST node pointer
 */
ASTNode* ast_create_boolean(bool value);

/**
 * @brief 创建列表节点 / Create list node
 * @return AST节点指针 / AST node pointer
 */
ASTNode* ast_create_list(void);

/**
 * @brief 向列表添加元素 / Add element to list
 * @param list 列表节点 / List node
 * @param element 元素节点 / Element node
 * @return 成功返回true / Returns true on success
 */
bool ast_list_add_element(ASTNode* list, ASTNode* element);

/**
 * @brief 创建函数调用节点 / Create function call node
 * @param function 函数节点 / Function node
 * @param arguments 参数数组 / Argument array
 * @param arg_count 参数数量 / Argument count
 * @return AST节点指针 / AST node pointer
 */
ASTNode* ast_create_function_call(ASTNode* function, ASTNode** arguments, size_t arg_count);

/**
 * @brief 创建二元运算节点 / Create binary operation node
 * @param op 运算符 / Operator
 * @param left 左操作数 / Left operand
 * @param right 右操作数 / Right operand
 * @return AST节点指针 / AST node pointer
 */
ASTNode* ast_create_binary_op(BinaryOpType op, ASTNode* left, ASTNode* right);

/**
 * @brief 创建一元运算节点 / Create unary operation node
 * @param op 运算符 / Operator
 * @param operand 操作数 / Operand
 * @return AST节点指针 / AST node pointer
 */
ASTNode* ast_create_unary_op(UnaryOpType op, ASTNode* operand);

/**
 * @brief 创建if语句节点 / Create if statement node
 * @param condition 条件 / Condition
 * @param then_branch then分支 / Then branch
 * @param else_branch else分支 / Else branch
 * @return AST节点指针 / AST node pointer
 */
ASTNode* ast_create_if(ASTNode* condition, ASTNode* then_branch, ASTNode* else_branch);

/**
 * @brief 创建while循环节点 / Create while loop node
 * @param condition 条件 / Condition
 * @param body 循环体 / Loop body
 * @return AST节点指针 / AST node pointer
 */
ASTNode* ast_create_while(ASTNode* condition, ASTNode* body);

/**
 * @brief 创建代码块节点 / Create block node
 * @return AST节点指针 / AST node pointer
 */
ASTNode* ast_create_block(void);

/**
 * @brief 向代码块添加语句 / Add statement to block
 * @param block 代码块节点 / Block node
 * @param statement 语句节点 / Statement node
 * @return 成功返回true / Returns true on success
 */
bool ast_block_add_statement(ASTNode* block, ASTNode* statement);

/**
 * @brief 创建变量定义节点 / Create variable definition node
 * @param name 变量名 / Variable name
 * @param value 初始值 / Initial value
 * @return AST节点指针 / AST node pointer
 */
ASTNode* ast_create_variable_def(const char* name, ASTNode* value);

/**
 * @brief 创建函数定义节点 / Create function definition node
 * @param name 函数名 / Function name
 * @param parameters 参数名数组 / Parameter name array
 * @param param_count 参数数量 / Parameter count
 * @param body 函数体 / Function body
 * @return AST节点指针 / AST node pointer
 */
ASTNode* ast_create_function_def(const char* name, char** parameters, 
                                size_t param_count, ASTNode* body);

/**
 * @brief 获取节点类型字符串 / Get node type string
 * @param type 节点类型 / Node type
 * @return 类型字符串 / Type string
 */
const char* ast_node_type_to_string(ASTNodeType type);

/**
 * @brief 获取数据类型字符串 / Get data type string
 * @param type 数据类型 / Data type
 * @return 类型字符串 / Type string
 */
const char* ast_data_type_to_string(DataType type);

/**
 * @brief 打印AST节点 / Print AST node
 * @param node AST节点 / AST node
 * @param indent 缩进级别 / Indent level
 */
void ast_print_node(const ASTNode* node, int indent);

/**
 * @brief 验证AST节点 / Validate AST node
 * @param node AST节点 / AST node
 * @return 验证成功返回true / Returns true if validation succeeds
 */
bool ast_validate_node(const ASTNode* node);

// 便利宏定义 / Convenience Macros

#define AST_IS_LITERAL(node) \
    ((node) && ((node)->type >= AST_NODE_INTEGER && (node)->type <= AST_NODE_BOOLEAN))

#define AST_IS_EXPRESSION(node) \
    ((node) && ((node)->type >= AST_NODE_LIST && (node)->type <= AST_NODE_UNARY_OP))

#define AST_IS_STATEMENT(node) \
    ((node) && ((node)->type >= AST_NODE_IF && (node)->type <= AST_NODE_VARIABLE_DEF))

#define AST_GET_INTEGER(node) \
    (((node) && (node)->type == AST_NODE_INTEGER) ? (node)->data.literal.int_value : 0)

#define AST_GET_FLOAT(node) \
    (((node) && (node)->type == AST_NODE_FLOAT) ? (node)->data.literal.float_value : 0.0)

#define AST_GET_STRING(node) \
    (((node) && (node)->type == AST_NODE_STRING) ? (node)->data.literal.string_value : NULL)

#define AST_GET_SYMBOL_NAME(node) \
    (((node) && (node)->type == AST_NODE_SYMBOL) ? (node)->data.symbol.name : NULL)

#ifdef __cplusplus
}
#endif

#endif // AST_NODES_H
