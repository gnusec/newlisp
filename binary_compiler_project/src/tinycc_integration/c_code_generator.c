/**
 * @file c_code_generator.c
 * @brief 基础C代码生成器实现 / Basic C Code Generator Implementation
 * @author newLISP Binary Compiler Team
 * @date 2024
 * 
 * MVP版本：专注核心功能，生成简单但正确的C代码
 * MVP version: Focus on core functionality, generate simple but correct C code
 */

#include "c_code_generator.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// C类型映射表 / C Type Mapping Table
static const char* c_type_names[] = {
    "void",         // DATA_TYPE_UNKNOWN
    "long long",    // DATA_TYPE_INTEGER
    "double",       // DATA_TYPE_FLOAT
    "char*",        // DATA_TYPE_STRING
    "int",          // DATA_TYPE_BOOLEAN (用int表示) / Use int for boolean
    "void*",        // DATA_TYPE_LIST
    "void*"         // DATA_TYPE_FUNCTION
};

// 代码缓冲区实现 / Code Buffer Implementation

/**
 * @brief 创建代码缓冲区 / Create code buffer
 */
CodeBuffer* codebuffer_create(size_t initial_capacity) {
    CodeBuffer* buffer = (CodeBuffer*)memory_alloc(sizeof(CodeBuffer), MEM_TYPE_CODE);
    if (!buffer) {
        ERROR_REPORT_ERROR(ERROR_TYPE_MEMORY, -1, "Failed to allocate code buffer");
        return NULL;
    }
    
    buffer->capacity = initial_capacity > 0 ? initial_capacity : 1024;
    buffer->buffer = (char*)memory_alloc(buffer->capacity, MEM_TYPE_CODE);
    if (!buffer->buffer) {
        memory_free(buffer);
        ERROR_REPORT_ERROR(ERROR_TYPE_MEMORY, -1, "Failed to allocate code buffer memory");
        return NULL;
    }
    
    buffer->size = 0;
    buffer->indent_level = 0;
    buffer->buffer[0] = '\0';
    
    return buffer;
}

/**
 * @brief 销毁代码缓冲区 / Destroy code buffer
 */
void codebuffer_destroy(CodeBuffer* buffer) {
    if (buffer) {
        if (buffer->buffer) {
            memory_free(buffer->buffer);
        }
        memory_free(buffer);
    }
}

/**
 * @brief 确保缓冲区容量 / Ensure buffer capacity
 */
static bool codebuffer_ensure_capacity(CodeBuffer* buffer, size_t needed) {
    if (buffer->size + needed >= buffer->capacity) {
        size_t new_capacity = buffer->capacity;
        while (new_capacity <= buffer->size + needed) {
            new_capacity *= 2;
        }
        
        char* new_buffer = (char*)memory_realloc(buffer->buffer, new_capacity, MEM_TYPE_CODE);
        if (!new_buffer) {
            ERROR_REPORT_ERROR(ERROR_TYPE_MEMORY, -1, "Failed to expand code buffer");
            return false;
        }
        
        buffer->buffer = new_buffer;
        buffer->capacity = new_capacity;
    }
    
    return true;
}

/**
 * @brief 追加字符串 / Append string
 */
bool codebuffer_append(CodeBuffer* buffer, const char* str) {
    if (!buffer || !str) {
        return false;
    }
    
    size_t len = strlen(str);
    if (!codebuffer_ensure_capacity(buffer, len + 1)) {
        return false;
    }
    
    strcpy(buffer->buffer + buffer->size, str);
    buffer->size += len;
    
    return true;
}

/**
 * @brief 追加格式化字符串 / Append formatted string
 */
bool codebuffer_appendf(CodeBuffer* buffer, const char* format, ...) {
    if (!buffer || !format) {
        return false;
    }
    
    va_list args;
    va_start(args, format);
    
    // 计算需要的长度 / Calculate needed length
    va_list args_copy;
    va_copy(args_copy, args);
    int needed = vsnprintf(NULL, 0, format, args_copy);
    va_end(args_copy);
    
    if (needed < 0) {
        va_end(args);
        return false;
    }
    
    if (!codebuffer_ensure_capacity(buffer, needed + 1)) {
        va_end(args);
        return false;
    }
    
    vsnprintf(buffer->buffer + buffer->size, needed + 1, format, args);
    buffer->size += needed;
    
    va_end(args);
    return true;
}

/**
 * @brief 追加缩进 / Append indent
 */
bool codebuffer_append_indent(CodeBuffer* buffer) {
    if (!buffer) {
        return false;
    }
    
    for (int i = 0; i < buffer->indent_level; i++) {
        if (!codebuffer_append(buffer, "    ")) { // 4个空格缩进 / 4 spaces indent
            return false;
        }
    }
    
    return true;
}

/**
 * @brief 增加缩进级别 / Increase indent level
 */
void codebuffer_indent(CodeBuffer* buffer) {
    if (buffer) {
        buffer->indent_level++;
    }
}

/**
 * @brief 减少缩进级别 / Decrease indent level
 */
void codebuffer_dedent(CodeBuffer* buffer) {
    if (buffer && buffer->indent_level > 0) {
        buffer->indent_level--;
    }
}

/**
 * @brief 追加新行 / Append newline
 */
bool codebuffer_append_line(CodeBuffer* buffer, const char* line) {
    if (!buffer) {
        return false;
    }
    
    if (!codebuffer_append_indent(buffer)) {
        return false;
    }
    
    if (line && !codebuffer_append(buffer, line)) {
        return false;
    }
    
    return codebuffer_append(buffer, "\n");
}

// 符号表实现 / Symbol Table Implementation

/**
 * @brief 创建符号表 / Create symbol table
 */
SymbolTable* symboltable_create(SymbolTable* parent) {
    SymbolTable* table = (SymbolTable*)memory_alloc(sizeof(SymbolTable), MEM_TYPE_AST);
    if (!table) {
        ERROR_REPORT_ERROR(ERROR_TYPE_MEMORY, -1, "Failed to allocate symbol table");
        return NULL;
    }
    
    table->entries = NULL;
    table->count = 0;
    table->parent = parent;
    
    return table;
}

/**
 * @brief 销毁符号表 / Destroy symbol table
 */
void symboltable_destroy(SymbolTable* table) {
    if (!table) {
        return;
    }
    
    SymbolEntry* current = table->entries;
    while (current) {
        SymbolEntry* next = current->next;
        
        if (current->name) {
            memory_free(current->name);
        }
        if (current->c_name) {
            memory_free(current->c_name);
        }
        memory_free(current);
        
        current = next;
    }
    
    memory_free(table);
}

/**
 * @brief 添加符号 / Add symbol
 */
bool symboltable_add(SymbolTable* table, const char* name, DataType type, const char* c_name) {
    if (!table || !name || !c_name) {
        return false;
    }
    
    SymbolEntry* entry = (SymbolEntry*)memory_alloc(sizeof(SymbolEntry), MEM_TYPE_AST);
    if (!entry) {
        ERROR_REPORT_ERROR(ERROR_TYPE_MEMORY, -1, "Failed to allocate symbol entry");
        return false;
    }
    
    entry->name = memory_strdup(name);
    entry->type = type;
    entry->c_name = memory_strdup(c_name);
    entry->is_function = false;
    entry->is_parameter = false;
    entry->next = table->entries;
    
    if (!entry->name || !entry->c_name) {
        if (entry->name) memory_free(entry->name);
        if (entry->c_name) memory_free(entry->c_name);
        memory_free(entry);
        return false;
    }
    
    table->entries = entry;
    table->count++;
    
    return true;
}

/**
 * @brief 查找符号 / Find symbol
 */
SymbolEntry* symboltable_find(SymbolTable* table, const char* name) {
    if (!table || !name) {
        return NULL;
    }
    
    // 在当前表中查找 / Search in current table
    SymbolEntry* current = table->entries;
    while (current) {
        if (strcmp(current->name, name) == 0) {
            return current;
        }
        current = current->next;
    }
    
    // 在父表中查找 / Search in parent table
    if (table->parent) {
        return symboltable_find(table->parent, name);
    }
    
    return NULL;
}

// 代码生成器实现 / Code Generator Implementation

/**
 * @brief 创建代码生成器 / Create code generator
 */
CodeGenerator* codegen_create(const CodeGenConfig* config) {
    CodeGenerator* codegen = (CodeGenerator*)memory_alloc(sizeof(CodeGenerator), MEM_TYPE_CODE);
    if (!codegen) {
        ERROR_REPORT_ERROR(ERROR_TYPE_MEMORY, -1, "Failed to allocate code generator");
        return NULL;
    }
    
    codegen->buffer = codebuffer_create(4096);
    if (!codegen->buffer) {
        memory_free(codegen);
        return NULL;
    }
    
    codegen->symbols = symboltable_create(NULL);
    if (!codegen->symbols) {
        codebuffer_destroy(codegen->buffer);
        memory_free(codegen);
        return NULL;
    }
    
    if (config) {
        codegen->config = *config;
    } else {
        codegen->config = codegen_create_default_config();
    }
    
    codegen->temp_var_counter = 0;
    codegen->label_counter = 0;
    codegen->has_error = false;
    
    return codegen;
}

/**
 * @brief 销毁代码生成器 / Destroy code generator
 */
void codegen_destroy(CodeGenerator* codegen) {
    if (codegen) {
        if (codegen->buffer) {
            codebuffer_destroy(codegen->buffer);
        }
        if (codegen->symbols) {
            symboltable_destroy(codegen->symbols);
        }
        memory_free(codegen);
    }
}

/**
 * @brief 获取C类型名 / Get C type name
 */
const char* codegen_get_c_type(DataType type) {
    if (type >= 0 && type < DATA_TYPE_COUNT) {
        return c_type_names[type];
    }
    return "void";
}

/**
 * @brief 生成唯一C变量名 / Generate unique C variable name
 */
char* codegen_generate_c_name(CodeGenerator* codegen, const char* base_name) {
    if (!codegen || !base_name) {
        return NULL;
    }

    // 简单的名称清理：替换特殊字符 / Simple name cleanup: replace special characters
    size_t len = strlen(base_name);
    char* clean_name = (char*)memory_alloc(len + 20, MEM_TYPE_TEMP); // 额外空间用于后缀
    if (!clean_name) {
        return NULL;
    }

    strcpy(clean_name, "var_");
    size_t pos = 4;

    for (size_t i = 0; i < len && pos < len + 15; i++) {
        char c = base_name[i];
        if (is_alpha(c) || is_digit(c) || c == '_') {
            clean_name[pos++] = c;
        } else {
            clean_name[pos++] = '_';
        }
    }

    clean_name[pos] = '\0';

    return clean_name;
}

/**
 * @brief 生成临时变量名 / Generate temporary variable name
 */
char* codegen_generate_temp_var(CodeGenerator* codegen) {
    if (!codegen) {
        return NULL;
    }

    char* temp_name = (char*)memory_alloc(32, MEM_TYPE_TEMP);
    if (!temp_name) {
        return NULL;
    }

    snprintf(temp_name, 32, "temp_%d", codegen->temp_var_counter++);
    return temp_name;
}

/**
 * @brief 生成标签名 / Generate label name
 */
char* codegen_generate_label(CodeGenerator* codegen, const char* prefix) {
    if (!codegen || !prefix) {
        return NULL;
    }

    char* label_name = (char*)memory_alloc(64, MEM_TYPE_TEMP);
    if (!label_name) {
        return NULL;
    }

    snprintf(label_name, 64, "%s_%d", prefix, codegen->label_counter++);
    return label_name;
}

/**
 * @brief 转义字符串 / Escape string
 */
char* codegen_escape_string(const char* str) {
    if (!str) {
        return NULL;
    }

    size_t len = strlen(str);
    size_t escaped_len = len * 2 + 3; // 最坏情况：每个字符都需要转义，加上引号
    char* escaped = (char*)memory_alloc(escaped_len, MEM_TYPE_STRING);
    if (!escaped) {
        return NULL;
    }

    size_t pos = 0;
    escaped[pos++] = '"';

    for (size_t i = 0; i < len; i++) {
        switch (str[i]) {
            case '"':  escaped[pos++] = '\\'; escaped[pos++] = '"'; break;
            case '\\': escaped[pos++] = '\\'; escaped[pos++] = '\\'; break;
            case '\n': escaped[pos++] = '\\'; escaped[pos++] = 'n'; break;
            case '\t': escaped[pos++] = '\\'; escaped[pos++] = 't'; break;
            case '\r': escaped[pos++] = '\\'; escaped[pos++] = 'r'; break;
            default:   escaped[pos++] = str[i]; break;
        }
    }

    escaped[pos++] = '"';
    escaped[pos] = '\0';

    return escaped;
}

/**
 * @brief 生成表达式代码 / Generate expression code
 */
char* codegen_generate_expression(CodeGenerator* codegen, ASTNode* node) {
    if (!codegen || !node) {
        return NULL;
    }

    switch (node->type) {
        case AST_NODE_INTEGER: {
            char* result = (char*)memory_alloc(32, MEM_TYPE_TEMP);
            if (result) {
                snprintf(result, 32, "%lld", node->data.literal.int_value);
            }
            return result;
        }

        case AST_NODE_FLOAT: {
            char* result = (char*)memory_alloc(32, MEM_TYPE_TEMP);
            if (result) {
                snprintf(result, 32, "%.6f", node->data.literal.float_value);
            }
            return result;
        }

        case AST_NODE_STRING: {
            return codegen_escape_string(node->data.literal.string_value);
        }

        case AST_NODE_BOOLEAN: {
            char* result = (char*)memory_alloc(8, MEM_TYPE_TEMP);
            if (result) {
                strcpy(result, node->data.literal.bool_value ? "1" : "0");
            }
            return result;
        }

        case AST_NODE_SYMBOL: {
            // 查找符号 / Find symbol
            SymbolEntry* entry = symboltable_find(codegen->symbols, node->data.symbol.name);
            if (entry) {
                return memory_strdup(entry->c_name);
            } else {
                // 未定义的符号，生成警告并使用原名 / Undefined symbol, generate warning and use original name
                ERROR_REPORT_WARNING(ERROR_TYPE_SYMBOL, -1,
                                    "Undefined symbol: %s", node->data.symbol.name);
                return codegen_generate_c_name(codegen, node->data.symbol.name);
            }
        }

        case AST_NODE_BINARY_OP: {
            char* left = codegen_generate_expression(codegen, node->data.binary_op.left);
            char* right = codegen_generate_expression(codegen, node->data.binary_op.right);

            if (!left || !right) {
                if (left) memory_free(left);
                if (right) memory_free(right);
                return NULL;
            }

            const char* op_str = "";
            switch (node->data.binary_op.op) {
                case BINARY_OP_ADD: op_str = "+"; break;
                case BINARY_OP_SUB: op_str = "-"; break;
                case BINARY_OP_MUL: op_str = "*"; break;
                case BINARY_OP_DIV: op_str = "/"; break;
                case BINARY_OP_MOD: op_str = "%"; break;
                case BINARY_OP_EQ:  op_str = "=="; break;
                case BINARY_OP_NE:  op_str = "!="; break;
                case BINARY_OP_LT:  op_str = "<"; break;
                case BINARY_OP_LE:  op_str = "<="; break;
                case BINARY_OP_GT:  op_str = ">"; break;
                case BINARY_OP_GE:  op_str = ">="; break;
                case BINARY_OP_AND: op_str = "&&"; break;
                case BINARY_OP_OR:  op_str = "||"; break;
                default: op_str = "?"; break;
            }

            size_t result_len = strlen(left) + strlen(right) + strlen(op_str) + 10;
            char* result = (char*)memory_alloc(result_len, MEM_TYPE_TEMP);
            if (result) {
                snprintf(result, result_len, "(%s %s %s)", left, op_str, right);
            }

            memory_free(left);
            memory_free(right);
            return result;
        }

        case AST_NODE_FUNCTION_CALL: {
            // 简化处理：生成函数调用 / Simplified: generate function call
            if (node->data.call.function->type == AST_NODE_SYMBOL) {
                const char* func_name = node->data.call.function->data.symbol.name;

                // 检查是否为内置函数 / Check if built-in function
                if (is_arithmetic_operator(func_name) && node->data.call.arg_count == 2) {
                    // 转换为二元运算 / Convert to binary operation
                    BinaryOpType op = get_binary_operator_type(func_name);
                    if (op != BINARY_OP_COUNT) {
                        ASTNode temp_node = {0};
                        temp_node.type = AST_NODE_BINARY_OP;
                        temp_node.data.binary_op.op = op;
                        temp_node.data.binary_op.left = node->data.call.arguments[0];
                        temp_node.data.binary_op.right = node->data.call.arguments[1];
                        return codegen_generate_expression(codegen, &temp_node);
                    }
                }

                // 普通函数调用 / Regular function call
                char* c_func_name = codegen_generate_c_name(codegen, func_name);
                if (!c_func_name) {
                    return NULL;
                }

                // 计算结果长度 / Calculate result length
                size_t result_len = strlen(c_func_name) + 10;
                for (size_t i = 0; i < node->data.call.arg_count; i++) {
                    result_len += 100; // 估算每个参数的长度 / Estimate length per argument
                }

                char* result = (char*)memory_alloc(result_len, MEM_TYPE_TEMP);
                if (!result) {
                    memory_free(c_func_name);
                    return NULL;
                }

                strcpy(result, c_func_name);
                strcat(result, "(");

                for (size_t i = 0; i < node->data.call.arg_count; i++) {
                    if (i > 0) {
                        strcat(result, ", ");
                    }

                    char* arg_code = codegen_generate_expression(codegen, node->data.call.arguments[i]);
                    if (arg_code) {
                        strcat(result, arg_code);
                        memory_free(arg_code);
                    }
                }

                strcat(result, ")");
                memory_free(c_func_name);
                return result;
            }
            break;
        }

        default:
            codegen_error(codegen, "Unsupported expression type for code generation");
            return NULL;
    }

    return NULL;
}

/**
 * @brief 生成C代码 / Generate C code
 */
char* codegen_generate(CodeGenerator* codegen, ASTNode* ast) {
    if (!codegen || !ast) {
        return NULL;
    }

    // 生成头文件 / Generate headers
    if (!codegen_generate_headers(codegen)) {
        return NULL;
    }

    // 生成运行时支持 / Generate runtime support
    if (!codegen_generate_runtime_support(codegen)) {
        return NULL;
    }

    // 生成main函数 / Generate main function
    if (!codegen_generate_main(codegen, ast)) {
        return NULL;
    }

    // 返回生成的代码 / Return generated code
    return memory_strdup(codegen->buffer->buffer);
}

/**
 * @brief 生成运行时头文件 / Generate runtime headers
 */
bool codegen_generate_headers(CodeGenerator* codegen) {
    if (!codegen) {
        return false;
    }

    // 标准头文件 / Standard headers
    codebuffer_append_line(codegen->buffer, "#include <stdio.h>");
    codebuffer_append_line(codegen->buffer, "#include <stdlib.h>");
    codebuffer_append_line(codegen->buffer, "#include <string.h>");
    codebuffer_append_line(codegen->buffer, "#include <math.h>");
    codebuffer_append_line(codegen->buffer, "");

    // 类型定义 / Type definitions
    codebuffer_append_line(codegen->buffer, "// newLISP runtime types");
    codebuffer_append_line(codegen->buffer, "typedef long long nl_int;");
    codebuffer_append_line(codegen->buffer, "typedef double nl_float;");
    codebuffer_append_line(codegen->buffer, "typedef char* nl_string;");
    codebuffer_append_line(codegen->buffer, "typedef int nl_bool;");
    codebuffer_append_line(codegen->buffer, "");

    return true;
}

/**
 * @brief 生成运行时支持函数 / Generate runtime support functions
 */
bool codegen_generate_runtime_support(CodeGenerator* codegen) {
    if (!codegen) {
        return false;
    }

    codebuffer_append_line(codegen->buffer, "// newLISP runtime support functions");

    // 打印函数 / Print functions
    codebuffer_append_line(codegen->buffer, "void nl_println(nl_string str) {");
    codebuffer_indent(codegen->buffer);
    codebuffer_append_line(codegen->buffer, "if (str) printf(\"%s\\n\", str);");
    codebuffer_append_line(codegen->buffer, "else printf(\"nil\\n\");");
    codebuffer_dedent(codegen->buffer);
    codebuffer_append_line(codegen->buffer, "}");
    codebuffer_append_line(codegen->buffer, "");

    codebuffer_append_line(codegen->buffer, "void nl_print_int(nl_int value) {");
    codebuffer_indent(codegen->buffer);
    codebuffer_append_line(codegen->buffer, "printf(\"%lld\", value);");
    codebuffer_dedent(codegen->buffer);
    codebuffer_append_line(codegen->buffer, "}");
    codebuffer_append_line(codegen->buffer, "");

    codebuffer_append_line(codegen->buffer, "void nl_print_float(nl_float value) {");
    codebuffer_indent(codegen->buffer);
    codebuffer_append_line(codegen->buffer, "printf(\"%.6f\", value);");
    codebuffer_dedent(codegen->buffer);
    codebuffer_append_line(codegen->buffer, "}");
    codebuffer_append_line(codegen->buffer, "");

    return true;
}

/**
 * @brief 生成main函数 / Generate main function
 */
bool codegen_generate_main(CodeGenerator* codegen, ASTNode* program_ast) {
    if (!codegen || !program_ast) {
        return false;
    }

    codebuffer_append_line(codegen->buffer, "int main(int argc, char* argv[]) {");
    codebuffer_indent(codegen->buffer);

    // 处理程序AST / Process program AST
    if (program_ast->type == AST_NODE_LIST) {
        // 处理每个表达式 / Process each expression
        for (size_t i = 0; i < program_ast->data.list.count; i++) {
            ASTNode* expr = program_ast->data.list.elements[i];
            char* expr_code = codegen_generate_expression(codegen, expr);

            if (expr_code) {
                // 简单处理：如果是函数调用，直接调用；否则打印结果
                // Simple handling: if function call, call directly; otherwise print result
                if (expr->type == AST_NODE_FUNCTION_CALL) {
                    codebuffer_appendf(codegen->buffer, "    %s;\n", expr_code);
                } else {
                    // 根据类型选择打印函数 / Choose print function based on type
                    switch (expr->type) {
                        case AST_NODE_INTEGER:
                            codebuffer_appendf(codegen->buffer, "    nl_print_int(%s);\n", expr_code);
                            break;
                        case AST_NODE_FLOAT:
                            codebuffer_appendf(codegen->buffer, "    nl_print_float(%s);\n", expr_code);
                            break;
                        case AST_NODE_STRING:
                            codebuffer_appendf(codegen->buffer, "    nl_println(%s);\n", expr_code);
                            break;
                        default:
                            codebuffer_appendf(codegen->buffer, "    nl_print_int(%s);\n", expr_code);
                            break;
                    }
                }

                memory_free(expr_code);
            }
        }
    } else {
        // 单个表达式 / Single expression
        char* expr_code = codegen_generate_expression(codegen, program_ast);
        if (expr_code) {
            codebuffer_appendf(codegen->buffer, "    nl_print_int(%s);\n", expr_code);
            memory_free(expr_code);
        }
    }

    codebuffer_append_line(codegen->buffer, "return 0;");
    codebuffer_dedent(codegen->buffer);
    codebuffer_append_line(codegen->buffer, "}");

    return true;
}

/**
 * @brief 创建默认配置 / Create default configuration
 */
CodeGenConfig codegen_create_default_config(void) {
    CodeGenConfig config = {0};
    config.enable_debug_info = false;
    config.enable_optimization = false;
    config.use_static_functions = false;
    config.indent_size = 4;
    config.include_headers = NULL;
    return config;
}

/**
 * @brief 报告代码生成错误 / Report code generation error
 */
void codegen_error(CodeGenerator* codegen, const char* message) {
    if (codegen) {
        codegen->has_error = true;
    }

    ERROR_REPORT_ERROR(ERROR_TYPE_COMPILE, -1, "Code generation error: %s", message);
}

/**
 * @brief 检查是否有错误 / Check if has errors
 */
bool codegen_has_errors(const CodeGenerator* codegen) {
    return codegen ? codegen->has_error : true;
}
