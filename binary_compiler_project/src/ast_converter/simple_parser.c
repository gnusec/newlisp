/**
 * @file simple_parser.c
 * @brief 简单newLISP解析器实现 / Simple newLISP Parser Implementation
 * @author newLISP Binary Compiler Team
 * @date 2024
 * 
 * MVP版本：专注核心功能，避免过度开发
 * MVP version: Focus on core functionality, avoid over-development
 */

#include "simple_parser.h"
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <math.h>
#include "../runtime/string_utils.h"

// 内置函数映射表 / Built-in function mapping table
static const char* builtin_functions[] = {
    "+", "-", "*", "/", "mod",           // 算术运算 / Arithmetic
    "=", "!=", "<", "<=", ">", ">=",     // 比较运算 / Comparison
    "and", "or", "not",                  // 逻辑运算 / Logical
    "if", "cond", "when", "unless",      // 条件语句 / Conditionals
    "while", "for", "dolist", "dotimes", // 循环语句 / Loops
    "define", "set", "setq",             // 定义语句 / Definitions
    "lambda", "fn",                      // 函数定义 / Function definitions
    "list", "cons", "car", "cdr",        // 列表操作 / List operations
    "println", "print",                  // 输出函数 / Output functions
    NULL
};

// 辅助函数实现 / Helper Function Implementation

/**
 * @brief 检查字符是否为数字 / Check if character is digit
 */
bool is_digit(char c) {
    return c >= '0' && c <= '9';
}

/**
 * @brief 检查字符是否为字母 / Check if character is letter
 */
bool is_alpha(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

/**
 * @brief 检查字符是否为符号字符 / Check if character is symbol character
 */
bool is_symbol_char(char c) {
    return is_alpha(c) || is_digit(c) ||
           c == '_' || c == '-' || c == '+' || c == '*' || c == '/' ||
           c == '<' || c == '>' || c == '=' || c == '!' || c == '?' ||
           c == '&' || c == '.' || c == ':' || c == '%';
}

/**
 * @brief 检查字符是否为空白字符 / Check if character is whitespace
 */
bool is_whitespace(char c) {
    return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

/**
 * @brief 释放词法单元 / Free token
 */
void token_free(Token* token) {
    if (token && token->value) {
        memory_free(token->value);
        token->value = NULL;
    }
}

/**
 * @brief 获取词法单元类型字符串 / Get token type string
 */
const char* token_type_to_string(TokenType type) {
    switch (type) {
        case TOKEN_EOF: return "EOF";
        case TOKEN_LPAREN: return "LPAREN";
        case TOKEN_RPAREN: return "RPAREN";
        case TOKEN_INTEGER: return "INTEGER";
        case TOKEN_FLOAT: return "FLOAT";
        case TOKEN_STRING: return "STRING";
        case TOKEN_SYMBOL: return "SYMBOL";
        case TOKEN_QUOTE: return "QUOTE";
        case TOKEN_ERROR: return "ERROR";
        default: return "UNKNOWN";
    }
}

/**
 * @brief 打印词法单元 / Print token
 */
void token_print(const Token* token) {
    if (!token) {
        printf("NULL TOKEN\n");
        return;
    }

    printf("Token { type: %s", token_type_to_string(token->type));

    if (token->value) {
        printf(", value: \"%s\"", token->value);
    }

    printf(", line: %d, column: %d }\n", token->line, token->column);
}

/**
 * @brief 打印所有词法单元 / Print all tokens
 */
void lexer_print_all_tokens(const char* input) {
    if (!input) {
        return;
    }

    Lexer* lexer = lexer_create(input);
    if (!lexer) {
        return;
    }

    Token token;
    do {
        token = lexer_next_token(lexer);
        token_print(&token);
        token_free(&token);
    } while (token.type != TOKEN_EOF && token.type != TOKEN_ERROR);

    lexer_destroy(lexer);
}

/**
 * @brief 报告解析错误 / Report parse error
 */
void parser_error(Parser* parser, const char* message) {
    if (!parser) {
        return;
    }

    parser->has_error = true;
    parser->error_count++;

    ERROR_REPORT_ERROR(ERROR_TYPE_PARSE, parser->error_count,
                      "Parse error at line %d, column %d: %s",
                      parser->current_token.line,
                      parser->current_token.column,
                      message);
}

/**
 * @brief 检查是否有解析错误 / Check if has parse errors
 */
bool parser_has_errors(const Parser* parser) {
    return parser ? parser->has_error : true;
}

/**
 * @brief 获取错误计数 / Get error count
 */
int parser_get_error_count(const Parser* parser) {
    return parser ? parser->error_count : 0;
}

// 词法分析器实现 / Lexer Implementation

/**
 * @brief 创建词法分析器 / Create lexer
 */
Lexer* lexer_create(const char* input) {
    if (!input) {
        ERROR_REPORT_ERROR(ERROR_TYPE_PARSE, -1, "Input cannot be NULL%s", "");
        return NULL;
    }
    
    Lexer* lexer = (Lexer*)memory_alloc(sizeof(Lexer), MEM_TYPE_AST);
    if (!lexer) {
        ERROR_REPORT_ERROR(ERROR_TYPE_MEMORY, -1, "Failed to allocate lexer%s", "");
        return NULL;
    }
    
    lexer->input = input;
    lexer->position = 0;
    lexer->length = strlen(input);
    lexer->line = 1;
    lexer->column = 1;
    lexer->current_char = lexer->length > 0 ? input[0] : '\0';
    lexer->has_error = false;
    
    return lexer;
}

/**
 * @brief 销毁词法分析器 / Destroy lexer
 */
void lexer_destroy(Lexer* lexer) {
    if (lexer) {
        memory_free(lexer);
    }
}

/**
 * @brief 前进一个字符 / Advance one character
 */
static void lexer_advance(Lexer* lexer) {
    if (lexer->position < lexer->length) {
        if (lexer->current_char == '\n') {
            lexer->line++;
            lexer->column = 1;
        } else {
            lexer->column++;
        }
        
        lexer->position++;
        lexer->current_char = (lexer->position < lexer->length) ? 
                             lexer->input[lexer->position] : '\0';
    }
}

/**
 * @brief 查看下一个字符 / Peek next character
 */
static char lexer_peek(Lexer* lexer) {
    size_t next_pos = lexer->position + 1;
    return (next_pos < lexer->length) ? lexer->input[next_pos] : '\0';
}

/**
 * @brief 跳过空白字符 / Skip whitespace
 */
void lexer_skip_whitespace(Lexer* lexer) {
    while (lexer->current_char != '\0' && is_whitespace(lexer->current_char)) {
        lexer_advance(lexer);
    }
}

/**
 * @brief 跳过注释 / Skip comments
 */
static void lexer_skip_comment(Lexer* lexer) {
    if (lexer->current_char == ';') {
        // 跳过到行尾 / Skip to end of line
        while (lexer->current_char != '\0' && lexer->current_char != '\n') {
            lexer_advance(lexer);
        }
    }
}

/**
 * @brief 读取字符串 / Read string
 */
char* lexer_read_string(Lexer* lexer) {
    if (lexer->current_char != '"') {
        ERROR_REPORT_ERROR(ERROR_TYPE_PARSE, -1, "Expected '\"' at start of string");
        return NULL;
    }
    
    lexer_advance(lexer); // 跳过开始的引号 / Skip opening quote
    
    size_t start_pos = lexer->position;
    size_t length = 0;
    
    // 计算字符串长度 / Calculate string length
    while (lexer->current_char != '\0' && lexer->current_char != '"') {
        if (lexer->current_char == '\\') {
            lexer_advance(lexer); // 跳过转义字符 / Skip escape character
            if (lexer->current_char == '\0') {
                ERROR_REPORT_ERROR(ERROR_TYPE_PARSE, -1, "Unterminated string escape%s", "");
                return NULL;
            }
        }
        length++;
        lexer_advance(lexer);
    }
    
    if (lexer->current_char != '"') {
        ERROR_REPORT_ERROR(ERROR_TYPE_PARSE, -1, "Unterminated string literal%s", "");
        return NULL;
    }
    
    // 分配内存并复制字符串 / Allocate memory and copy string
    char* result = (char*)memory_alloc(length + 1, MEM_TYPE_STRING);
    if (!result) {
        ERROR_REPORT_ERROR(ERROR_TYPE_MEMORY, -1, "Failed to allocate string%s", "");
        return NULL;
    }
    
    // 重新解析字符串处理转义 / Re-parse string handling escapes
    lexer->position = start_pos;
    lexer->current_char = lexer->input[lexer->position];
    
    size_t result_pos = 0;
    while (lexer->current_char != '"' && lexer->current_char != '\0') {
        if (lexer->current_char == '\\') {
            lexer_advance(lexer);
            switch (lexer->current_char) {
                case 'n': result[result_pos++] = '\n'; break;
                case 't': result[result_pos++] = '\t'; break;
                case 'r': result[result_pos++] = '\r'; break;
                case '\\': result[result_pos++] = '\\'; break;
                case '"': result[result_pos++] = '"'; break;
                default: result[result_pos++] = lexer->current_char; break;
            }
        } else {
            result[result_pos++] = lexer->current_char;
        }
        lexer_advance(lexer);
    }
    
    result[result_pos] = '\0';
    lexer_advance(lexer); // 跳过结束的引号 / Skip closing quote
    
    return result;
}

/**
 * @brief 读取数字 / Read number
 */
Token lexer_read_number(Lexer* lexer) {
    Token token = {0};
    size_t start_pos = lexer->position;
    bool is_float = false;
    
    // 处理负号 / Handle negative sign
    if (lexer->current_char == '-') {
        lexer_advance(lexer);
    }
    
    // 读取数字 / Read digits
    while (is_digit(lexer->current_char)) {
        lexer_advance(lexer);
    }
    
    // 检查小数点 / Check for decimal point
    if (lexer->current_char == '.' && is_digit(lexer_peek(lexer))) {
        is_float = true;
        lexer_advance(lexer); // 跳过小数点 / Skip decimal point
        
        while (is_digit(lexer->current_char)) {
            lexer_advance(lexer);
        }
    }
    
    // 提取数字字符串 / Extract number string
    size_t length = lexer->position - start_pos;
    char* number_str = (char*)memory_alloc(length + 1, MEM_TYPE_TEMP);
    if (!number_str) {
        token.type = TOKEN_ERROR;
        return token;
    }
    
    strncpy(number_str, &lexer->input[start_pos], length);
    number_str[length] = '\0';
    
    token.type = is_float ? TOKEN_FLOAT : TOKEN_INTEGER;
    token.value = number_str;
    token.line = lexer->line;
    token.column = lexer->column - (int)length;
    token.length = length;
    
    return token;
}

/**
 * @brief 读取符号 / Read symbol
 */
char* lexer_read_symbol(Lexer* lexer) {
    size_t start_pos = lexer->position;
    
    // 读取符号字符 / Read symbol characters
    while (lexer->current_char != '\0' && is_symbol_char(lexer->current_char)) {
        lexer_advance(lexer);
    }
    
    size_t length = lexer->position - start_pos;
    if (length == 0) {
        ERROR_REPORT_ERROR(ERROR_TYPE_PARSE, -1, "Empty symbol%s", "");
        return NULL;
    }
    
    char* result = (char*)memory_alloc(length + 1, MEM_TYPE_STRING);
    if (!result) {
        ERROR_REPORT_ERROR(ERROR_TYPE_MEMORY, -1, "Failed to allocate symbol%s", "");
        return NULL;
    }
    
    strncpy(result, &lexer->input[start_pos], length);
    result[length] = '\0';
    
    return result;
}

/**
 * @brief 获取下一个词法单元 / Get next token
 */
Token lexer_next_token(Lexer* lexer) {
    Token token = {0};
    
    if (!lexer) {
        token.type = TOKEN_ERROR;
        return token;
    }
    
    // 跳过空白字符和注释 / Skip whitespace and comments
    while (lexer->current_char != '\0') {
        if (is_whitespace(lexer->current_char)) {
            lexer_skip_whitespace(lexer);
        } else if (lexer->current_char == ';') {
            lexer_skip_comment(lexer);
        } else {
            break;
        }
    }
    
    token.line = lexer->line;
    token.column = lexer->column;
    
    // 文件结束 / End of file
    if (lexer->current_char == '\0') {
        token.type = TOKEN_EOF;
        return token;
    }
    
    // 左括号 / Left parenthesis
    if (lexer->current_char == '(') {
        token.type = TOKEN_LPAREN;
        token.value = memory_strdup("(");
        token.length = 1;
        lexer_advance(lexer);
        return token;
    }
    
    // 右括号 / Right parenthesis
    if (lexer->current_char == ')') {
        token.type = TOKEN_RPAREN;
        token.value = memory_strdup(")");
        token.length = 1;
        lexer_advance(lexer);
        return token;
    }
    
    // 引号 / Quote
    if (lexer->current_char == '\'') {
        token.type = TOKEN_QUOTE;
        token.value = memory_strdup("'");
        token.length = 1;
        lexer_advance(lexer);
        return token;
    }
    
    // 字符串 / String
    if (lexer->current_char == '"') {
        char* str_value = lexer_read_string(lexer);
        if (str_value) {
            token.type = TOKEN_STRING;
            token.value = str_value;
            token.length = strlen(str_value);
        } else {
            token.type = TOKEN_ERROR;
        }
        return token;
    }
    
    // 数字 / Number
    if (is_digit(lexer->current_char) || 
        (lexer->current_char == '-' && is_digit(lexer_peek(lexer)))) {
        return lexer_read_number(lexer);
    }
    
    // 符号 / Symbol
    if (is_symbol_char(lexer->current_char)) {
        char* symbol_value = lexer_read_symbol(lexer);
        if (symbol_value) {
            token.type = TOKEN_SYMBOL;
            token.value = symbol_value;
            token.length = strlen(symbol_value);
        } else {
            token.type = TOKEN_ERROR;
        }
        return token;
    }
    
    // 未知字符 / Unknown character
    ERROR_REPORT_ERROR(ERROR_TYPE_PARSE, -1, "Unexpected character: '%c'", lexer->current_char);
    token.type = TOKEN_ERROR;
    return token;
}

// 语法分析器实现 / Parser Implementation

/**
 * @brief 创建语法分析器 / Create parser
 */
Parser* parser_create(const char* input) {
    if (!input) {
        ERROR_REPORT_ERROR(ERROR_TYPE_PARSE, -1, "Input cannot be NULL%s", "");
        return NULL;
    }

    Parser* parser = (Parser*)memory_alloc(sizeof(Parser), MEM_TYPE_AST);
    if (!parser) {
        ERROR_REPORT_ERROR(ERROR_TYPE_MEMORY, -1, "Failed to allocate parser%s", "");
        return NULL;
    }

    parser->lexer = lexer_create(input);
    if (!parser->lexer) {
        memory_free(parser);
        return NULL;
    }

    parser->has_error = false;
    parser->error_count = 0;

    // 获取第一个词法单元 / Get first token
    parser->current_token = lexer_next_token(parser->lexer);

    return parser;
}

/**
 * @brief 销毁语法分析器 / Destroy parser
 */
void parser_destroy(Parser* parser) {
    if (parser) {
        if (parser->lexer) {
            lexer_destroy(parser->lexer);
        }
        token_free(&parser->current_token);
        memory_free(parser);
    }
}

/**
 * @brief 前进到下一个词法单元 / Advance to next token
 */
static void parser_advance(Parser* parser) {
    token_free(&parser->current_token);
    parser->current_token = lexer_next_token(parser->lexer);
}

/**
 * @brief 检查当前词法单元类型 / Check current token type
 */
static bool parser_check(Parser* parser, TokenType type) {
    return parser->current_token.type == type;
}

/**
 * @brief 匹配并消费词法单元 / Match and consume token
 */
static bool parser_match(Parser* parser, TokenType type) {
    if (parser_check(parser, type)) {
        parser_advance(parser);
        return true;
    }
    return false;
}

/**
 * @brief 解析原子表达式 / Parse atomic expression
 */
static ASTNode* parser_parse_atom(Parser* parser) {
    Token* token = &parser->current_token;

    switch (token->type) {
        case TOKEN_INTEGER: {
            long long value = strtoll(token->value, NULL, 10);
            parser_advance(parser);
            return ast_create_integer(value);
        }

        case TOKEN_FLOAT: {
            double value = strtod(token->value, NULL);
            parser_advance(parser);
            return ast_create_float(value);
        }

        case TOKEN_STRING: {
            ASTNode* node = ast_create_string(token->value);
            parser_advance(parser);
            return node;
        }

        case TOKEN_SYMBOL: {
            // 检查特殊符号 / Check special symbols
            if (strcmp(token->value, "true") == 0) {
                parser_advance(parser);
                return ast_create_boolean(true);
            } else if (strcmp(token->value, "false") == 0) {
                parser_advance(parser);
                return ast_create_boolean(false);
            } else if (strcmp(token->value, "nil") == 0) {
                parser_advance(parser);
                return ast_create_symbol("nil");
            } else {
                ASTNode* node = ast_create_symbol(token->value);
                parser_advance(parser);
                return node;
            }
        }

        default:
            parser_error(parser, "Expected atomic expression");
            return NULL;
    }
}

/**
 * @brief 解析列表表达式 / Parse list expression
 */
static ASTNode* parser_parse_list(Parser* parser) {
    if (!parser_match(parser, TOKEN_LPAREN)) {
        parser_error(parser, "Expected '('");
        return NULL;
    }

    // 空列表 / Empty list
    if (parser_check(parser, TOKEN_RPAREN)) {
        parser_advance(parser);
        return ast_create_list();
    }

    // 解析第一个元素 / Parse first element
    ASTNode* first = parser_parse_expression(parser);
    if (!first) {
        return NULL;
    }

    // 检查是否为特殊形式或函数调用 / Check if special form or function call
    if (first->type == AST_NODE_SYMBOL) {
        const char* symbol_name = first->data.symbol.name;

        // 检查特殊形式 / Check special forms
        if (strcmp(symbol_name, "if") == 0) {
            // 解析if语句: (if condition then-expr [else-expr])
            ASTNode* condition = parser_parse_expression(parser);
            if (!condition) {
                ast_destroy_node(first);
                return NULL;
            }

            ASTNode* then_branch = parser_parse_expression(parser);
            if (!then_branch) {
                ast_destroy_node(first);
                ast_destroy_node(condition);
                return NULL;
            }

            ASTNode* else_branch = NULL;
            if (!parser_check(parser, TOKEN_RPAREN)) {
                else_branch = parser_parse_expression(parser);
                if (!else_branch) {
                    ast_destroy_node(first);
                    ast_destroy_node(condition);
                    ast_destroy_node(then_branch);
                    return NULL;
                }
            }

            if (!parser_match(parser, TOKEN_RPAREN)) {
                ast_destroy_node(first);
                ast_destroy_node(condition);
                ast_destroy_node(then_branch);
                if (else_branch) ast_destroy_node(else_branch);
                parser_error(parser, "Expected ')' after if expression");
                return NULL;
            }

            ast_destroy_node(first); // 不再需要symbol节点
            return ast_create_if(condition, then_branch, else_branch);
        }

        if (strcmp(symbol_name, "while") == 0) {
            // 解析while循环: (while condition body)
            ASTNode* condition = parser_parse_expression(parser);
            if (!condition) {
                ast_destroy_node(first);
                return NULL;
            }

            ASTNode* body = parser_parse_expression(parser);
            if (!body) {
                ast_destroy_node(first);
                ast_destroy_node(condition);
                return NULL;
            }

            if (!parser_match(parser, TOKEN_RPAREN)) {
                ast_destroy_node(first);
                ast_destroy_node(condition);
                ast_destroy_node(body);
                parser_error(parser, "Expected ')' after while expression");
                return NULL;
            }

            ast_destroy_node(first); // 不再需要symbol节点
            return ast_create_while(condition, body);
        }

        if (strcmp(symbol_name, "when") == 0) {
            // 解析when语句: (when condition body) - 等价于 (if condition body)
            ASTNode* condition = parser_parse_expression(parser);
            if (!condition) {
                ast_destroy_node(first);
                return NULL;
            }

            ASTNode* body = parser_parse_expression(parser);
            if (!body) {
                ast_destroy_node(first);
                ast_destroy_node(condition);
                return NULL;
            }

            if (!parser_match(parser, TOKEN_RPAREN)) {
                ast_destroy_node(first);
                ast_destroy_node(condition);
                ast_destroy_node(body);
                parser_error(parser, "Expected ')' after when expression");
                return NULL;
            }

            ast_destroy_node(first); // 不再需要symbol节点
            return ast_create_if(condition, body, NULL); // when等价于没有else的if
        }

        if (strcmp(symbol_name, "unless") == 0) {
            // 解析unless语句: (unless condition body) - 等价于 (if (not condition) body)
            ASTNode* condition = parser_parse_expression(parser);
            if (!condition) {
                ast_destroy_node(first);
                return NULL;
            }

            ASTNode* body = parser_parse_expression(parser);
            if (!body) {
                ast_destroy_node(first);
                ast_destroy_node(condition);
                return NULL;
            }

            if (!parser_match(parser, TOKEN_RPAREN)) {
                ast_destroy_node(first);
                ast_destroy_node(condition);
                ast_destroy_node(body);
                parser_error(parser, "Expected ')' after unless expression");
                return NULL;
            }

            // 创建not条件: (not condition)
            ASTNode* not_condition = ast_create_unary_op(UNARY_OP_NOT, condition);
            if (!not_condition) {
                ast_destroy_node(first);
                ast_destroy_node(condition);
                ast_destroy_node(body);
                return NULL;
            }

            ast_destroy_node(first); // 不再需要symbol节点
            return ast_create_if(not_condition, body, NULL); // unless等价于if not
        }

        if (strcmp(symbol_name, "set") == 0) {
            // 解析set语句: (set variable-name value)
            ASTNode* var_name = parser_parse_expression(parser);
            if (!var_name) {
                ast_destroy_node(first);
                return NULL;
            }

            // 变量名必须是符号
            if (var_name->type != AST_NODE_SYMBOL) {
                ast_destroy_node(first);
                ast_destroy_node(var_name);
                parser_error(parser, "Variable name must be a symbol");
                return NULL;
            }

            ASTNode* value = parser_parse_expression(parser);
            if (!value) {
                ast_destroy_node(first);
                ast_destroy_node(var_name);
                return NULL;
            }

            if (!parser_match(parser, TOKEN_RPAREN)) {
                ast_destroy_node(first);
                ast_destroy_node(var_name);
                ast_destroy_node(value);
                parser_error(parser, "Expected ')' after set expression");
                return NULL;
            }

            // 创建变量定义节点
            const char* name = var_name->data.symbol.name;
            ASTNode* var_def = ast_create_variable_def(name, value);

            ast_destroy_node(first); // 不再需要symbol节点
            ast_destroy_node(var_name); // 不再需要变量名节点

            return var_def;
        }

        if (strcmp(symbol_name, "define") == 0) {
            // 解析define语句: (define (function-name param1 param2 ...) body)
            // 或者: (define function-name value) 用于变量定义

            // 检查下一个token是否为左括号（函数定义）
            if (parser_check(parser, TOKEN_LPAREN)) {
                // 函数定义: (define (function-name param1 param2 ...) body)
                parser_advance(parser); // 消费 '('

                // 解析函数名
                if (!parser_check(parser, TOKEN_SYMBOL)) {
                    ast_destroy_node(first);
                    parser_error(parser, "Expected function name");
                    return NULL;
                }

                char* func_name = safe_strdup(parser->current_token.value);
                if (!func_name) {
                    ast_destroy_node(first);
                    parser_error(parser, "Failed to allocate function name");
                    return NULL;
                }
                parser_advance(parser);

                // 解析参数列表
                size_t param_count = 0;
                char** parameters = NULL;

                while (!parser_check(parser, TOKEN_RPAREN) && !parser_check(parser, TOKEN_EOF)) {
                    if (!parser_check(parser, TOKEN_SYMBOL)) {
                        ast_destroy_node(first);
                        memory_free(func_name);
                        if (parameters) {
                            for (size_t i = 0; i < param_count; i++) {
                                memory_free(parameters[i]);
                            }
                            memory_free(parameters);
                        }
                        parser_error(parser, "Function parameters must be symbols");
                        return NULL;
                    }

                    // 扩展参数数组
                    char** new_params = (char**)memory_alloc(sizeof(char*) * (param_count + 1), MEM_TYPE_TEMP);
                    if (!new_params) {
                        ast_destroy_node(first);
                        memory_free(func_name);
                        if (parameters) {
                            for (size_t i = 0; i < param_count; i++) {
                                memory_free(parameters[i]);
                            }
                            memory_free(parameters);
                        }
                        return NULL;
                    }

                    for (size_t i = 0; i < param_count; i++) {
                        new_params[i] = parameters[i];
                    }
                    if (parameters) memory_free(parameters);
                    parameters = new_params;

                    parameters[param_count] = safe_strdup(parser->current_token.value);
                    param_count++;
                    parser_advance(parser);
                }

                if (!parser_match(parser, TOKEN_RPAREN)) {
                    ast_destroy_node(first);
                    memory_free(func_name);
                    if (parameters) {
                        for (size_t i = 0; i < param_count; i++) {
                            memory_free(parameters[i]);
                        }
                        memory_free(parameters);
                    }
                    parser_error(parser, "Expected ')' after parameter list");
                    return NULL;
                }

                // 解析函数体
                ASTNode* body = parser_parse_expression(parser);
                if (!body) {
                    ast_destroy_node(first);
                    memory_free(func_name);
                    if (parameters) {
                        for (size_t i = 0; i < param_count; i++) {
                            memory_free(parameters[i]);
                        }
                        memory_free(parameters);
                    }
                    return NULL;
                }

                if (!parser_match(parser, TOKEN_RPAREN)) {
                    ast_destroy_node(first);
                    ast_destroy_node(body);
                    memory_free(func_name);
                    if (parameters) {
                        for (size_t i = 0; i < param_count; i++) {
                            memory_free(parameters[i]);
                        }
                        memory_free(parameters);
                    }
                    parser_error(parser, "Expected ')' after function definition");
                    return NULL;
                }

                // 创建函数定义节点
                ASTNode* func_def = ast_create_function_def(func_name, parameters, param_count, body);

                ast_destroy_node(first);
                memory_free(func_name);

                return func_def;
            } else {
                // 变量定义: (define variable-name value)
                ASTNode* first_arg = parser_parse_expression(parser);
                if (!first_arg) {
                    ast_destroy_node(first);
                    return NULL;
                }

                if (first_arg->type != AST_NODE_SYMBOL) {
                    ast_destroy_node(first);
                    ast_destroy_node(first_arg);
                    parser_error(parser, "Variable name must be a symbol");
                    return NULL;
                }

                ASTNode* value = parser_parse_expression(parser);
                if (!value) {
                    ast_destroy_node(first);
                    ast_destroy_node(first_arg);
                    return NULL;
                }

                if (!parser_match(parser, TOKEN_RPAREN)) {
                    ast_destroy_node(first);
                    ast_destroy_node(first_arg);
                    ast_destroy_node(value);
                    parser_error(parser, "Expected ')' after variable definition");
                    return NULL;
                }

                // 创建变量定义节点
                const char* var_name = first_arg->data.symbol.name;
                ASTNode* var_def = ast_create_variable_def(var_name, value);

                ast_destroy_node(first);
                ast_destroy_node(first_arg);

                return var_def;
            }
        }

        // 解析函数参数 / Parse function arguments
        ASTNode** arguments = NULL;
        size_t arg_count = 0;
        size_t arg_capacity = 0;

        while (!parser_check(parser, TOKEN_RPAREN) && !parser_check(parser, TOKEN_EOF)) {
            // 扩展参数数组 / Expand argument array
            if (arg_count >= arg_capacity) {
                arg_capacity = arg_capacity == 0 ? 4 : arg_capacity * 2;
                ASTNode** new_args = (ASTNode**)memory_realloc(
                    arguments, arg_capacity * sizeof(ASTNode*), MEM_TYPE_AST);
                if (!new_args) {
                    // 清理已分配的参数 / Cleanup allocated arguments
                    for (size_t i = 0; i < arg_count; i++) {
                        ast_destroy_node(arguments[i]);
                    }
                    memory_free(arguments);
                    ast_destroy_node(first);
                    parser_error(parser, "Memory allocation failed");
                    return NULL;
                }
                arguments = new_args;
            }

            ASTNode* arg = parser_parse_expression(parser);
            if (!arg) {
                // 清理 / Cleanup
                for (size_t i = 0; i < arg_count; i++) {
                    ast_destroy_node(arguments[i]);
                }
                memory_free(arguments);
                ast_destroy_node(first);
                return NULL;
            }

            arguments[arg_count++] = arg;
        }

        if (!parser_match(parser, TOKEN_RPAREN)) {
            // 清理 / Cleanup
            for (size_t i = 0; i < arg_count; i++) {
                ast_destroy_node(arguments[i]);
            }
            memory_free(arguments);
            ast_destroy_node(first);
            parser_error(parser, "Expected ')'");
            return NULL;
        }

        // 创建函数调用节点 / Create function call node
        ASTNode* call = ast_create_function_call(first, arguments, arg_count);
        memory_free(arguments); // 函数调用节点已复制参数 / Function call node has copied arguments
        return call;
    } else {
        // 普通列表 / Regular list
        ASTNode* list = ast_create_list();
        if (!list) {
            ast_destroy_node(first);
            return NULL;
        }

        if (!ast_list_add_element(list, first)) {
            ast_destroy_node(first);
            ast_destroy_node(list);
            return NULL;
        }

        // 解析剩余元素 / Parse remaining elements
        while (!parser_check(parser, TOKEN_RPAREN) && !parser_check(parser, TOKEN_EOF)) {
            ASTNode* element = parser_parse_expression(parser);
            if (!element) {
                ast_destroy_node(list);
                return NULL;
            }

            if (!ast_list_add_element(list, element)) {
                ast_destroy_node(element);
                ast_destroy_node(list);
                return NULL;
            }
        }

        if (!parser_match(parser, TOKEN_RPAREN)) {
            ast_destroy_node(list);
            parser_error(parser, "Expected ')'");
            return NULL;
        }

        return list;
    }
}

/**
 * @brief 解析表达式 / Parse expression
 */
ASTNode* parser_parse_expression(Parser* parser) {
    if (!parser) {
        return NULL;
    }

    switch (parser->current_token.type) {
        case TOKEN_LPAREN:
            return parser_parse_list(parser);

        case TOKEN_QUOTE:
            parser_advance(parser);
            // 简化处理：直接解析被引用的表达式 / Simplified: directly parse quoted expression
            return parser_parse_expression(parser);

        case TOKEN_INTEGER:
        case TOKEN_FLOAT:
        case TOKEN_STRING:
        case TOKEN_SYMBOL:
            return parser_parse_atom(parser);

        case TOKEN_EOF:
            return NULL;

        default:
            parser_error(parser, "Unexpected token in expression");
            return NULL;
    }
}

/**
 * @brief 解析程序 / Parse program
 */
ASTNode* parser_parse_program(Parser* parser) {
    if (!parser) {
        return NULL;
    }

    ASTNode* program = ast_create_list();
    if (!program) {
        return NULL;
    }

    while (!parser_check(parser, TOKEN_EOF)) {
        ASTNode* expr = parser_parse_expression(parser);
        if (!expr) {
            if (parser->has_error) {
                ast_destroy_node(program);
                return NULL;
            }
            break; // 正常结束 / Normal end
        }

        if (!ast_list_add_element(program, expr)) {
            ast_destroy_node(expr);
            ast_destroy_node(program);
            return NULL;
        }
    }

    return program;
}

// 内置函数识别 / Built-in Function Recognition

/**
 * @brief 检查是否为内置函数 / Check if built-in function
 */
bool is_builtin_function(const char* name) {
    if (!name) {
        return false;
    }

    for (int i = 0; builtin_functions[i] != NULL; i++) {
        if (strcmp(name, builtin_functions[i]) == 0) {
            return true;
        }
    }

    return false;
}

/**
 * @brief 检查是否为算术运算符 / Check if arithmetic operator
 */
bool is_arithmetic_operator(const char* name) {
    if (!name) {
        return false;
    }

    return strcmp(name, "+") == 0 || strcmp(name, "-") == 0 ||
           strcmp(name, "*") == 0 || strcmp(name, "/") == 0 ||
           strcmp(name, "mod") == 0;
}

/**
 * @brief 检查是否为比较运算符 / Check if comparison operator
 */
bool is_comparison_operator(const char* name) {
    if (!name) {
        return false;
    }

    return strcmp(name, "=") == 0 || strcmp(name, "!=") == 0 ||
           strcmp(name, "<") == 0 || strcmp(name, "<=") == 0 ||
           strcmp(name, ">") == 0 || strcmp(name, ">=") == 0;
}

/**
 * @brief 获取二元运算符类型 / Get binary operator type
 */
BinaryOpType get_binary_operator_type(const char* name) {
    if (!name) {
        return BINARY_OP_COUNT; // 无效值 / Invalid value
    }

    if (strcmp(name, "+") == 0) return BINARY_OP_ADD;
    if (strcmp(name, "-") == 0) return BINARY_OP_SUB;
    if (strcmp(name, "*") == 0) return BINARY_OP_MUL;
    if (strcmp(name, "/") == 0) return BINARY_OP_DIV;
    if (strcmp(name, "mod") == 0) return BINARY_OP_MOD;
    if (strcmp(name, "=") == 0) return BINARY_OP_EQ;
    if (strcmp(name, "!=") == 0) return BINARY_OP_NE;
    if (strcmp(name, "<") == 0) return BINARY_OP_LT;
    if (strcmp(name, "<=") == 0) return BINARY_OP_LE;
    if (strcmp(name, ">") == 0) return BINARY_OP_GT;
    if (strcmp(name, ">=") == 0) return BINARY_OP_GE;
    if (strcmp(name, "and") == 0) return BINARY_OP_AND;
    if (strcmp(name, "or") == 0) return BINARY_OP_OR;

    return BINARY_OP_COUNT; // 无效值 / Invalid value
}

// 公共接口实现 / Public Interface Implementation

/**
 * @brief 从字符串解析 / Parse from string
 */
ASTNode* parse_string(const char* input) {
    if (!input) {
        ERROR_REPORT_ERROR(ERROR_TYPE_PARSE, -1, "Input string cannot be NULL");
        return NULL;
    }

    Parser* parser = parser_create(input);
    if (!parser) {
        return NULL;
    }

    ASTNode* result = parser_parse_program(parser);

    if (parser_has_errors(parser)) {
        if (result) {
            ast_destroy_node(result);
            result = NULL;
        }
    }

    parser_destroy(parser);
    return result;
}

/**
 * @brief 从文件解析 / Parse from file
 */
ASTNode* parse_file(const char* filename) {
    if (!filename) {
        ERROR_REPORT_ERROR(ERROR_TYPE_PARSE, -1, "Filename cannot be NULL%s", "");
        return NULL;
    }

    FILE* file = fopen(filename, "r");
    if (!file) {
        ERROR_REPORT_ERROR(ERROR_TYPE_IO, errno, "Failed to open file: %s", filename);
        return NULL;
    }

    // 获取文件大小 / Get file size
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    if (file_size < 0) {
        fclose(file);
        ERROR_REPORT_ERROR(ERROR_TYPE_IO, -1, "Failed to get file size: %s", filename);
        return NULL;
    }

    // 分配缓冲区 / Allocate buffer
    char* buffer = (char*)memory_alloc(file_size + 1, MEM_TYPE_TEMP);
    if (!buffer) {
        fclose(file);
        ERROR_REPORT_ERROR(ERROR_TYPE_MEMORY, -1, "Failed to allocate file buffer%s", "");
        return NULL;
    }

    // 读取文件内容 / Read file content
    size_t bytes_read = fread(buffer, 1, file_size, file);
    buffer[bytes_read] = '\0';

    fclose(file);

    // 解析内容 / Parse content
    ASTNode* result = parse_string(buffer);

    // 清理缓冲区 / Cleanup buffer
    memory_free(buffer);

    return result;
}

// 测试函数 / Test Functions

/**
 * @brief 测试词法分析器 / Test lexer
 */
bool test_lexer(const char* input) {
    if (!input) {
        return false;
    }

    printf("Testing lexer with input: \"%s\"\n", input);

    Lexer* lexer = lexer_create(input);
    if (!lexer) {
        printf("Failed to create lexer\n");
        return false;
    }

    Token token;
    bool success = true;

    do {
        token = lexer_next_token(lexer);
        token_print(&token);

        if (token.type == TOKEN_ERROR) {
            success = false;
        }

        token_free(&token);
    } while (token.type != TOKEN_EOF && token.type != TOKEN_ERROR);

    lexer_destroy(lexer);

    printf("Lexer test %s\n\n", success ? "PASSED" : "FAILED");
    return success;
}

/**
 * @brief 测试语法分析器 / Test parser
 */
bool test_parser(const char* input) {
    if (!input) {
        return false;
    }

    printf("Testing parser with input: \"%s\"\n", input);

    ASTNode* ast = parse_string(input);
    if (!ast) {
        printf("Parser test FAILED: No AST generated\n\n");
        return false;
    }

    printf("Generated AST:\n");
    ast_print_node(ast, 0);

    ast_destroy_node(ast);

    printf("Parser test PASSED\n\n");
    return true;
}
