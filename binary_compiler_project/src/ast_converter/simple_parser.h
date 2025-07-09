/**
 * @file simple_parser.h
 * @brief 简单newLISP解析器 / Simple newLISP Parser
 * @author newLISP Binary Compiler Team
 * @date 2024
 * 
 * 专注于核心功能的轻量级newLISP解析器
 * Lightweight newLISP parser focusing on core functionality
 */

#ifndef SIMPLE_PARSER_H
#define SIMPLE_PARSER_H

#include "ast_nodes.h"
#include "../runtime/error_handler.h"
#include <stdio.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// 词法单元类型 / Token Types
typedef enum {
    TOKEN_EOF = 0,            // 文件结束 / End of file
    TOKEN_LPAREN,             // 左括号 ( / Left parenthesis
    TOKEN_RPAREN,             // 右括号 ) / Right parenthesis
    TOKEN_INTEGER,            // 整数 / Integer
    TOKEN_FLOAT,              // 浮点数 / Float
    TOKEN_STRING,             // 字符串 / String
    TOKEN_SYMBOL,             // 符号 / Symbol
    TOKEN_QUOTE,              // 引号 ' / Quote
    TOKEN_ERROR               // 错误 / Error
} TokenType;

// 词法单元结构 / Token Structure
typedef struct {
    TokenType type;           // 单元类型 / Token type
    char* value;              // 单元值 / Token value
    int line;                 // 行号 / Line number
    int column;               // 列号 / Column number
    size_t length;            // 长度 / Length
} Token;

// 词法分析器结构 / Lexer Structure
typedef struct {
    const char* input;        // 输入文本 / Input text
    size_t position;          // 当前位置 / Current position
    size_t length;            // 输入长度 / Input length
    int line;                 // 当前行号 / Current line
    int column;               // 当前列号 / Current column
    char current_char;        // 当前字符 / Current character
    bool has_error;           // 是否有错误 / Whether has error
} Lexer;

// 语法分析器结构 / Parser Structure
typedef struct {
    Lexer* lexer;             // 词法分析器 / Lexer
    Token current_token;      // 当前词法单元 / Current token
    bool has_error;           // 是否有错误 / Whether has error
    int error_count;          // 错误计数 / Error count
} Parser;

// 核心API函数 / Core API Functions

/**
 * @brief 创建词法分析器 / Create lexer
 * @param input 输入文本 / Input text
 * @return 词法分析器指针 / Lexer pointer
 */
Lexer* lexer_create(const char* input);

/**
 * @brief 销毁词法分析器 / Destroy lexer
 * @param lexer 词法分析器 / Lexer
 */
void lexer_destroy(Lexer* lexer);

/**
 * @brief 获取下一个词法单元 / Get next token
 * @param lexer 词法分析器 / Lexer
 * @return 词法单元 / Token
 */
Token lexer_next_token(Lexer* lexer);

/**
 * @brief 创建语法分析器 / Create parser
 * @param input 输入文本 / Input text
 * @return 语法分析器指针 / Parser pointer
 */
Parser* parser_create(const char* input);

/**
 * @brief 销毁语法分析器 / Destroy parser
 * @param parser 语法分析器 / Parser
 */
void parser_destroy(Parser* parser);

/**
 * @brief 解析表达式 / Parse expression
 * @param parser 语法分析器 / Parser
 * @return AST节点 / AST node
 */
ASTNode* parser_parse_expression(Parser* parser);

/**
 * @brief 解析程序 / Parse program
 * @param parser 语法分析器 / Parser
 * @return AST节点列表 / AST node list
 */
ASTNode* parser_parse_program(Parser* parser);

/**
 * @brief 从字符串解析 / Parse from string
 * @param input 输入字符串 / Input string
 * @return AST节点 / AST node
 */
ASTNode* parse_string(const char* input);

/**
 * @brief 从文件解析 / Parse from file
 * @param filename 文件名 / Filename
 * @return AST节点 / AST node
 */
ASTNode* parse_file(const char* filename);

/**
 * @brief 获取词法单元类型字符串 / Get token type string
 * @param type 词法单元类型 / Token type
 * @return 类型字符串 / Type string
 */
const char* token_type_to_string(TokenType type);

/**
 * @brief 释放词法单元 / Free token
 * @param token 词法单元 / Token
 */
void token_free(Token* token);

// 内置函数识别 / Built-in Function Recognition

/**
 * @brief 检查是否为内置函数 / Check if built-in function
 * @param name 函数名 / Function name
 * @return 是内置函数返回true / Returns true if built-in
 */
bool is_builtin_function(const char* name);

/**
 * @brief 检查是否为算术运算符 / Check if arithmetic operator
 * @param name 运算符名 / Operator name
 * @return 是算术运算符返回true / Returns true if arithmetic operator
 */
bool is_arithmetic_operator(const char* name);

/**
 * @brief 检查是否为比较运算符 / Check if comparison operator
 * @param name 运算符名 / Operator name
 * @return 是比较运算符返回true / Returns true if comparison operator
 */
bool is_comparison_operator(const char* name);

/**
 * @brief 获取二元运算符类型 / Get binary operator type
 * @param name 运算符名 / Operator name
 * @return 运算符类型 / Operator type
 */
BinaryOpType get_binary_operator_type(const char* name);

// 错误处理 / Error Handling

/**
 * @brief 报告解析错误 / Report parse error
 * @param parser 语法分析器 / Parser
 * @param message 错误消息 / Error message
 */
void parser_error(Parser* parser, const char* message);

/**
 * @brief 检查是否有解析错误 / Check if has parse errors
 * @param parser 语法分析器 / Parser
 * @return 有错误返回true / Returns true if has errors
 */
bool parser_has_errors(const Parser* parser);

/**
 * @brief 获取错误计数 / Get error count
 * @param parser 语法分析器 / Parser
 * @return 错误数量 / Error count
 */
int parser_get_error_count(const Parser* parser);

// 便利函数 / Convenience Functions

/**
 * @brief 跳过空白字符 / Skip whitespace
 * @param lexer 词法分析器 / Lexer
 */
void lexer_skip_whitespace(Lexer* lexer);

/**
 * @brief 读取字符串 / Read string
 * @param lexer 词法分析器 / Lexer
 * @return 字符串值 / String value
 */
char* lexer_read_string(Lexer* lexer);

/**
 * @brief 读取数字 / Read number
 * @param lexer 词法分析器 / Lexer
 * @return 词法单元 / Token
 */
Token lexer_read_number(Lexer* lexer);

/**
 * @brief 读取符号 / Read symbol
 * @param lexer 词法分析器 / Lexer
 * @return 符号值 / Symbol value
 */
char* lexer_read_symbol(Lexer* lexer);

/**
 * @brief 检查字符是否为数字 / Check if character is digit
 * @param c 字符 / Character
 * @return 是数字返回true / Returns true if digit
 */
bool is_digit(char c);

/**
 * @brief 检查字符是否为字母 / Check if character is letter
 * @param c 字符 / Character
 * @return 是字母返回true / Returns true if letter
 */
bool is_alpha(char c);

/**
 * @brief 检查字符是否为符号字符 / Check if character is symbol character
 * @param c 字符 / Character
 * @return 是符号字符返回true / Returns true if symbol character
 */
bool is_symbol_char(char c);

/**
 * @brief 检查字符是否为空白字符 / Check if character is whitespace
 * @param c 字符 / Character
 * @return 是空白字符返回true / Returns true if whitespace
 */
bool is_whitespace(char c);

// 内置函数识别 / Built-in Function Recognition

/**
 * @brief 检查是否为内置函数 / Check if built-in function
 * @param name 函数名 / Function name
 * @return 是内置函数返回true / Returns true if built-in function
 */
bool is_builtin_function(const char* name);

/**
 * @brief 检查是否为算术运算符 / Check if arithmetic operator
 * @param name 运算符名 / Operator name
 * @return 是算术运算符返回true / Returns true if arithmetic operator
 */
bool is_arithmetic_operator(const char* name);

/**
 * @brief 检查是否为比较运算符 / Check if comparison operator
 * @param name 运算符名 / Operator name
 * @return 是比较运算符返回true / Returns true if comparison operator
 */
bool is_comparison_operator(const char* name);

/**
 * @brief 获取二元运算符类型 / Get binary operator type
 * @param name 运算符名 / Operator name
 * @return 二元运算符类型 / Binary operator type
 */
BinaryOpType get_binary_operator_type(const char* name);

// 词法单元管理 / Token Management

/**
 * @brief 释放词法单元 / Free token
 * @param token 词法单元 / Token
 */
void token_free(Token* token);

/**
 * @brief 获取词法单元类型字符串 / Get token type string
 * @param type 词法单元类型 / Token type
 * @return 类型字符串 / Type string
 */
const char* token_type_to_string(TokenType type);

// 解析器错误处理 / Parser Error Handling

/**
 * @brief 报告解析错误 / Report parse error
 * @param parser 解析器 / Parser
 * @param message 错误消息 / Error message
 */
void parser_error(Parser* parser, const char* message);

/**
 * @brief 检查是否有解析错误 / Check if has parse errors
 * @param parser 解析器 / Parser
 * @return 有错误返回true / Returns true if has errors
 */
bool parser_has_errors(const Parser* parser);

/**
 * @brief 获取错误计数 / Get error count
 * @param parser 解析器 / Parser
 * @return 错误计数 / Error count
 */
int parser_get_error_count(const Parser* parser);

// 调试函数 / Debug Functions

/**
 * @brief 打印词法单元 / Print token
 * @param token 词法单元 / Token
 */
void token_print(const Token* token);

/**
 * @brief 打印所有词法单元 / Print all tokens
 * @param input 输入文本 / Input text
 */
void lexer_print_all_tokens(const char* input);

// 测试函数 / Test Functions

/**
 * @brief 测试词法分析器 / Test lexer
 * @param input 输入文本 / Input text
 * @return 测试成功返回true / Returns true if test succeeds
 */
bool test_lexer(const char* input);

/**
 * @brief 测试语法分析器 / Test parser
 * @param input 输入文本 / Input text
 * @return 测试成功返回true / Returns true if test succeeds
 */
bool test_parser(const char* input);

#ifdef __cplusplus
}
#endif

#endif // SIMPLE_PARSER_H
