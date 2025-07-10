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
#include <inttypes.h>

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
        ERROR_REPORT_ERROR(ERROR_TYPE_MEMORY, -1, "Failed to allocate code buffer%s", "");
        return NULL;
    }
    
    buffer->capacity = initial_capacity > 0 ? initial_capacity : 1024;
    buffer->buffer = (char*)memory_alloc(buffer->capacity, MEM_TYPE_CODE);
    if (!buffer->buffer) {
        memory_free(buffer);
        ERROR_REPORT_ERROR(ERROR_TYPE_MEMORY, -1, "Failed to allocate code buffer memory%s", "");
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
            ERROR_REPORT_ERROR(ERROR_TYPE_MEMORY, -1, "Failed to expand code buffer%s", "");
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
        ERROR_REPORT_ERROR(ERROR_TYPE_MEMORY, -1, "Failed to allocate symbol table%s", "");
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
        ERROR_REPORT_ERROR(ERROR_TYPE_MEMORY, -1, "Failed to allocate symbol entry%s", "");
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
        ERROR_REPORT_ERROR(ERROR_TYPE_MEMORY, -1, "Failed to allocate code generator%s", "");
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
 * @brief 检查是否为算术运算符 / Check if arithmetic operator
 */
static bool is_arithmetic_operator(const char* name) {
    return (strcmp(name, "+") == 0 || strcmp(name, "-") == 0 ||
            strcmp(name, "*") == 0 || strcmp(name, "/") == 0 ||
            strcmp(name, "mod") == 0 || strcmp(name, "abs") == 0 ||
            strcmp(name, "max") == 0 || strcmp(name, "min") == 0 ||
            strcmp(name, "=") == 0 || strcmp(name, "!=") == 0 ||
            strcmp(name, "<") == 0 || strcmp(name, "<=") == 0 ||
            strcmp(name, ">") == 0 || strcmp(name, ">=") == 0 ||
            strcmp(name, "and") == 0 || strcmp(name, "or") == 0 ||
            strcmp(name, "not") == 0);
}

static bool is_comparison_operator(const char* name) {
    return (strcmp(name, "=") == 0 || strcmp(name, "!=") == 0 ||
            strcmp(name, "<") == 0 || strcmp(name, "<=") == 0 ||
            strcmp(name, ">") == 0 || strcmp(name, ">=") == 0);
}

static bool is_logical_operator(const char* name) {
    return (strcmp(name, "and") == 0 || strcmp(name, "or") == 0 ||
            strcmp(name, "not") == 0);
}

static bool is_boolean_operator(const char* name) {
    return is_comparison_operator(name) || is_logical_operator(name);
}

// 暂时未使用，但保留用于未来扩展 / Currently unused, but kept for future expansion
__attribute__((unused))
static bool is_control_structure(const char* name) {
    return (strcmp(name, "if") == 0 || strcmp(name, "when") == 0 ||
            strcmp(name, "unless") == 0 || strcmp(name, "while") == 0 ||
            strcmp(name, "cond") == 0);
}

/**
 * @brief 获取二元运算符类型 / Get binary operator type
 */
static BinaryOpType get_binary_operator_type(const char* name) {
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
    return BINARY_OP_COUNT; // 无效操作符 / Invalid operator
}

/**
 * @brief 生成C函数名 / Generate C function name
 */
char* codegen_generate_c_function_name(CodeGenerator* codegen, const char* base_name) {
    if (!codegen || !base_name) {
        return NULL;
    }

    // 检查是否为内置函数 / Check if built-in function
    if (strcmp(base_name, "println") == 0) {
        char* result = (char*)memory_alloc(16, MEM_TYPE_TEMP);
        if (result) strcpy(result, "nl_println");
        return result;
    }
    if (strcmp(base_name, "print") == 0) {
        char* result = (char*)memory_alloc(16, MEM_TYPE_TEMP);
        if (result) strcpy(result, "nl_print");
        return result;
    }

    // 对于用户定义的函数，直接使用原名（经过清理）
    size_t len = strlen(base_name);
    char* clean_name = (char*)memory_alloc(len + 1, MEM_TYPE_TEMP);
    if (!clean_name) {
        return NULL;
    }

    size_t pos = 0;
    for (size_t i = 0; i < len; i++) {
        char c = base_name[i];
        if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
            (c >= '0' && c <= '9') || c == '_') {
            clean_name[pos++] = c;
        } else if (c == '-') {
            clean_name[pos++] = '_';  // 将连字符转换为下划线
        }
        // 跳过其他无效字符
    }

    clean_name[pos] = '\0';

    // 如果清理后的名称为空，使用默认名称
    if (pos == 0) {
        strcpy(clean_name, "user_func");
    }

    return clean_name;
}

/**
 * @brief 生成唯一C变量名 / Generate unique C variable name
 */
char* codegen_generate_c_name(CodeGenerator* codegen, const char* base_name) {
    if (!codegen || !base_name) {
        return NULL;
    }

    // 检查是否为内置函数 / Check if built-in function
    if (strcmp(base_name, "println") == 0) {
        char* result = (char*)memory_alloc(16, MEM_TYPE_TEMP);
        if (result) strcpy(result, "nl_println");
        return result;
    }
    if (strcmp(base_name, "print") == 0) {
        char* result = (char*)memory_alloc(16, MEM_TYPE_TEMP);
        if (result) strcpy(result, "nl_print");
        return result;
    }
    if (strcmp(base_name, "+") == 0 || strcmp(base_name, "-") == 0 ||
        strcmp(base_name, "*") == 0 || strcmp(base_name, "/") == 0) {
        // 算术运算符应该在其他地方处理 / Arithmetic operators should be handled elsewhere
        char* result = (char*)memory_alloc(strlen(base_name) + 1, MEM_TYPE_TEMP);
        if (result) strcpy(result, base_name);
        return result;
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
        if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
            (c >= '0' && c <= '9') || c == '_') {
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
                snprintf(result, 32, "%" PRId64, node->data.literal.int_value);
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

        case AST_NODE_UNARY_OP: {
            char* operand = codegen_generate_expression(codegen, node->data.unary_op.operand);
            if (!operand) return NULL;

            size_t result_len = strlen(operand) + 10;
            char* result = (char*)memory_alloc(result_len, MEM_TYPE_TEMP);
            if (!result) {
                memory_free(operand);
                return NULL;
            }

            const char* op_str = (node->data.unary_op.op == UNARY_OP_NOT) ? "!" : "?";
            snprintf(result, result_len, "%s(%s)", op_str, operand);

            memory_free(operand);
            return result;
        }

        case AST_NODE_FUNCTION_CALL: {
            // 简化处理：生成函数调用 / Simplified: generate function call
            if (node->data.call.function->type == AST_NODE_SYMBOL) {
                const char* func_name = node->data.call.function->data.symbol.name;

                // 检查是否为内置函数 / Check if built-in function
                if (strcmp(func_name, "println") == 0 && node->data.call.arg_count == 1) {
                    // 处理println函数 / Handle println function
                    ASTNode* arg = node->data.call.arguments[0];
                    char* arg_code = codegen_generate_expression(codegen, arg);
                    if (!arg_code) return NULL;

                    char* result = (char*)memory_alloc(256, MEM_TYPE_TEMP);
                    if (!result) {
                        memory_free(arg_code);
                        return NULL;
                    }

                    // 根据参数类型选择合适的输出函数 / Choose appropriate output function based on argument type
                    switch (arg->type) {
                        case AST_NODE_INTEGER:
                            snprintf(result, 256, "nl_print_int(%s), printf(\"\\n\")", arg_code);
                            break;
                        case AST_NODE_FLOAT:
                            snprintf(result, 256, "nl_print_float(%s), printf(\"\\n\")", arg_code);
                            break;
                        case AST_NODE_STRING:
                            snprintf(result, 256, "nl_println(%s)", arg_code);
                            break;
                        case AST_NODE_FUNCTION_CALL:
                            // 检查是否为布尔运算符 / Check if it's a boolean operator
                            if (arg->data.call.function && arg->data.call.function->type == AST_NODE_SYMBOL) {
                                const char* func_name = arg->data.call.function->data.symbol.name;
                                if (is_boolean_operator(func_name)) {
                                    snprintf(result, 256, "nl_print_bool(%s), printf(\"\\n\")", arg_code);
                                } else {
                                    snprintf(result, 256, "nl_print_int(%s), printf(\"\\n\")", arg_code);
                                }
                            } else {
                                snprintf(result, 256, "nl_print_int(%s), printf(\"\\n\")", arg_code);
                            }
                            break;
                        default:
                            snprintf(result, 256, "nl_print_int(%s), printf(\"\\n\")", arg_code);
                            break;
                    }

                    memory_free(arg_code);
                    return result;
                } else if (is_arithmetic_operator(func_name) && node->data.call.arg_count >= 1) {
                    // 处理算术运算 / Handle arithmetic operations

                    // 特殊处理abs函数 / Special handling for abs function
                    if (strcmp(func_name, "abs") == 0 && node->data.call.arg_count == 1) {
                        char* arg_code = codegen_generate_expression(codegen, node->data.call.arguments[0]);
                        if (!arg_code) return NULL;

                        size_t result_len = strlen(arg_code) * 3 + 30;
                        char* result = (char*)memory_alloc(result_len, MEM_TYPE_TEMP);
                        if (!result) {
                            memory_free(arg_code);
                            return NULL;
                        }

                        // 生成条件表达式: ((arg) < 0 ? -(arg) : (arg))
                        snprintf(result, result_len, "((%s) < 0 ? -(%s) : (%s))", arg_code, arg_code, arg_code);
                        memory_free(arg_code);
                        return result;
                    }

                    // 特殊处理not函数 / Special handling for not function
                    if (strcmp(func_name, "not") == 0 && node->data.call.arg_count == 1) {
                        char* arg_code = codegen_generate_expression(codegen, node->data.call.arguments[0]);
                        if (!arg_code) return NULL;

                        size_t result_len = strlen(arg_code) + 10;
                        char* result = (char*)memory_alloc(result_len, MEM_TYPE_TEMP);
                        if (!result) {
                            memory_free(arg_code);
                            return NULL;
                        }

                        snprintf(result, result_len, "!(%s)", arg_code);
                        memory_free(arg_code);
                        return result;
                    }

                    // 特殊处理max函数 / Special handling for max function
                    if (strcmp(func_name, "max") == 0 && node->data.call.arg_count >= 2) {
                        return codegen_generate_max_min(codegen, node->data.call.arguments,
                                                       node->data.call.arg_count, true);
                    }

                    // 特殊处理min函数 / Special handling for min function
                    if (strcmp(func_name, "min") == 0 && node->data.call.arg_count >= 2) {
                        return codegen_generate_max_min(codegen, node->data.call.arguments,
                                                       node->data.call.arg_count, false);
                    }

                    if (node->data.call.arg_count == 1) {
                        // 单参数情况：直接返回参数值 / Single argument: return argument value directly
                        return codegen_generate_expression(codegen, node->data.call.arguments[0]);
                    } else {
                        // 多参数算术运算 / Multi-argument arithmetic operations
                        BinaryOpType op = get_binary_operator_type(func_name);
                        if (op != BINARY_OP_COUNT) {
                            // 生成链式运算 / Generate chained operations
                            char* result = codegen_generate_expression(codegen, node->data.call.arguments[0]);
                            if (!result) return NULL;

                            for (size_t i = 1; i < node->data.call.arg_count; i++) {
                                char* next_arg = codegen_generate_expression(codegen, node->data.call.arguments[i]);
                                if (!next_arg) {
                                    memory_free(result);
                                    return NULL;
                                }

                                size_t new_len = strlen(result) + strlen(next_arg) + 10;
                                char* new_result = (char*)memory_alloc(new_len, MEM_TYPE_TEMP);
                                if (!new_result) {
                                    memory_free(result);
                                    memory_free(next_arg);
                                    return NULL;
                                }

                                const char* op_str = (op == BINARY_OP_ADD) ? " + " :
                                                   (op == BINARY_OP_SUB) ? " - " :
                                                   (op == BINARY_OP_MUL) ? " * " :
                                                   (op == BINARY_OP_DIV) ? " / " :
                                                   (op == BINARY_OP_MOD) ? " % " :
                                                   (op == BINARY_OP_EQ) ? " == " :
                                                   (op == BINARY_OP_NE) ? " != " :
                                                   (op == BINARY_OP_LT) ? " < " :
                                                   (op == BINARY_OP_LE) ? " <= " :
                                                   (op == BINARY_OP_GT) ? " > " :
                                                   (op == BINARY_OP_GE) ? " >= " :
                                                   (op == BINARY_OP_AND) ? " && " :
                                                   (op == BINARY_OP_OR) ? " || " : " ? ";

                                snprintf(new_result, new_len, "(%s%s%s)", result, op_str, next_arg);
                                memory_free(result);
                                memory_free(next_arg);
                                result = new_result;
                            }
                            return result;
                        }
                    }
                }

                // 检查是否为用户自定义函数 / Check if it's a user-defined function
                SymbolEntry* func_entry = symboltable_find(codegen->symbols, func_name);
                if (func_entry && func_entry->type == DATA_TYPE_FUNCTION) {
                    // 用户自定义函数调用 / User-defined function call
                    size_t result_len = strlen(func_entry->c_name) + 10;
                    for (size_t i = 0; i < node->data.call.arg_count; i++) {
                        result_len += 100; // 估算每个参数的长度 / Estimate length per argument
                    }

                    char* result = (char*)memory_alloc(result_len, MEM_TYPE_TEMP);
                    if (!result) return NULL;

                    strcpy(result, func_entry->c_name);
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
                    return result;
                }

                // 普通函数调用 / Regular function call
                char* c_func_name = codegen_generate_c_function_name(codegen, func_name);
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
 * @brief 生成语句代码 / Generate statement code
 * @param codegen 代码生成器 / Code generator
 * @param node AST节点 / AST node
 * @return 是否成功 / Success status
 */
bool codegen_generate_statement(CodeGenerator* codegen, ASTNode* node) {
    if (!codegen || !node) {
        return false;
    }

    switch (node->type) {
        case AST_NODE_IF: {
            // 生成if语句 / Generate if statement
            char* condition = codegen_generate_expression(codegen, node->data.if_stmt.condition);
            if (!condition) return false;

            codebuffer_appendf(codegen->buffer, "if (%s) {\n", condition);
            memory_free(condition);

            codebuffer_indent(codegen->buffer);

            // 生成then分支 / Generate then branch
            if (node->data.if_stmt.then_branch) {
                if (!codegen_generate_statement(codegen, node->data.if_stmt.then_branch)) {
                    return false;
                }
            }

            codebuffer_dedent(codegen->buffer);
            codebuffer_append_line(codegen->buffer, "}");

            // 生成else分支 / Generate else branch
            if (node->data.if_stmt.else_branch) {
                codebuffer_append_line(codegen->buffer, "else {");
                codebuffer_indent(codegen->buffer);

                if (!codegen_generate_statement(codegen, node->data.if_stmt.else_branch)) {
                    return false;
                }

                codebuffer_dedent(codegen->buffer);
                codebuffer_append_line(codegen->buffer, "}");
            }

            return true;
        }

        case AST_NODE_WHILE: {
            // 生成while循环 / Generate while loop
            char* condition = codegen_generate_expression(codegen, node->data.while_stmt.condition);
            if (!condition) return false;

            codebuffer_appendf(codegen->buffer, "while (%s) {\n", condition);
            memory_free(condition);

            codebuffer_indent(codegen->buffer);

            // 生成循环体 / Generate loop body
            if (node->data.while_stmt.body) {
                if (!codegen_generate_statement(codegen, node->data.while_stmt.body)) {
                    return false;
                }
            }

            codebuffer_dedent(codegen->buffer);
            codebuffer_append_line(codegen->buffer, "}");

            return true;
        }

        case AST_NODE_BLOCK: {
            // 生成代码块 / Generate code block
            for (size_t i = 0; i < node->data.block.count; i++) {
                if (!codegen_generate_statement(codegen, node->data.block.statements[i])) {
                    return false;
                }
            }
            return true;
        }

        case AST_NODE_VARIABLE_DEF: {
            // 生成变量定义 / Generate variable definition
            const char* var_name = node->data.variable_def.name;
            ASTNode* value = node->data.variable_def.value;

            // 生成变量值的代码 / Generate code for variable value
            char* value_code = codegen_generate_expression(codegen, value);
            if (!value_code) return false;

            // 生成C变量名 / Generate C variable name
            char* c_var_name = codegen_generate_c_name(codegen, var_name);
            if (!c_var_name) {
                memory_free(value_code);
                return false;
            }

            // 添加到符号表 / Add to symbol table
            symboltable_add(codegen->symbols, var_name, DATA_TYPE_INTEGER, c_var_name);

            // 生成变量声明和赋值 / Generate variable declaration and assignment
            codebuffer_appendf(codegen->buffer, "nl_int %s = %s;\n", c_var_name, value_code);

            memory_free(value_code);
            memory_free(c_var_name);
            return true;
        }

        case AST_NODE_FUNCTION_DEF: {
            // 生成函数定义 / Generate function definition
            const char* func_name = node->data.function_def.name;
            char** parameters = node->data.function_def.parameters;
            size_t param_count = node->data.function_def.param_count;
            ASTNode* body = node->data.function_def.body;

            // 生成C函数名 / Generate C function name
            char* c_func_name = codegen_generate_c_function_name(codegen, func_name);
            if (!c_func_name) return false;

            // 添加函数到符号表 / Add function to symbol table
            symboltable_add(codegen->symbols, func_name, DATA_TYPE_FUNCTION, c_func_name);

            // 生成函数签名 / Generate function signature
            codebuffer_appendf(codegen->buffer, "nl_int %s(", c_func_name);

            // 生成参数列表 / Generate parameter list
            for (size_t i = 0; i < param_count; i++) {
                if (i > 0) {
                    codebuffer_append(codegen->buffer, ", ");
                }
                char* c_param_name = codegen_generate_c_name(codegen, parameters[i]);
                if (!c_param_name) {
                    memory_free(c_func_name);
                    return false;
                }
                codebuffer_appendf(codegen->buffer, "nl_int %s", c_param_name);

                // 添加参数到符号表 / Add parameter to symbol table
                symboltable_add(codegen->symbols, parameters[i], DATA_TYPE_INTEGER, c_param_name);

                memory_free(c_param_name);
            }

            if (param_count == 0) {
                codebuffer_append(codegen->buffer, "void");
            }

            codebuffer_append(codegen->buffer, ") {\n");
            codegen->buffer->indent_level++;

            // 生成函数体 / Generate function body
            char* body_code = codegen_generate_expression(codegen, body);
            if (!body_code) {
                memory_free(c_func_name);
                return false;
            }

            // 生成返回语句 / Generate return statement
            codebuffer_appendf(codegen->buffer, "return %s;\n", body_code);

            codegen->buffer->indent_level--;
            codebuffer_append(codegen->buffer, "}\n\n");

            memory_free(c_func_name);
            memory_free(body_code);
            return true;
        }

        default: {
            // 对于表达式节点，生成表达式代码 / For expression nodes, generate expression code
            char* expr_code = codegen_generate_expression(codegen, node);
            if (!expr_code) return false;

            // 智能处理表达式 / Smart expression handling
            if (node->type == AST_NODE_FUNCTION_CALL) {
                // 检查是否为println等输出函数 / Check if it's an output function like println
                if (node->data.call.function->type == AST_NODE_SYMBOL) {
                    const char* func_name = node->data.call.function->data.symbol.name;
                    if (strcmp(func_name, "println") == 0 || strcmp(func_name, "print") == 0) {
                        // 直接调用输出函数 / Direct call to output function
                        codebuffer_appendf(codegen->buffer, "%s;\n", expr_code);
                    } else if (is_arithmetic_operator(func_name)) {
                        // 算术运算结果不输出，只计算 / Arithmetic operations don't output, just compute
                        codebuffer_appendf(codegen->buffer, "%s; // arithmetic result\n", expr_code);
                    } else {
                        // 其他函数调用 / Other function calls
                        codebuffer_appendf(codegen->buffer, "%s;\n", expr_code);
                    }
                } else {
                    codebuffer_appendf(codegen->buffer, "%s;\n", expr_code);
                }
            } else {
                // 字面量值：根据类型选择打印函数 / Literal values: choose print function based on type
                switch (node->type) {
                    case AST_NODE_INTEGER:
                        codebuffer_appendf(codegen->buffer, "nl_print_int(%s);\n", expr_code);
                        break;
                    case AST_NODE_FLOAT:
                        codebuffer_appendf(codegen->buffer, "nl_print_float(%s);\n", expr_code);
                        break;
                    case AST_NODE_STRING:
                        codebuffer_appendf(codegen->buffer, "nl_println(%s);\n", expr_code);
                        break;
                    default:
                        // 其他表达式不输出 / Other expressions don't output
                        codebuffer_appendf(codegen->buffer, "%s; // expression result\n", expr_code);
                        break;
                }
            }

            memory_free(expr_code);
            return true;
        }
    }
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

    // 首先生成用户自定义函数 / First generate user-defined functions
    if (!codegen_generate_user_functions(codegen, ast)) {
        return NULL;
    }

    // 然后生成main函数 / Then generate main function
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

    codebuffer_append_line(codegen->buffer, "void nl_print_bool(int value) {");
    codebuffer_indent(codegen->buffer);
    codebuffer_append_line(codegen->buffer, "printf(\"%s\", value ? \"true\" : \"false\");");
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
 * @brief 生成用户自定义函数 / Generate user-defined functions
 */
bool codegen_generate_user_functions(CodeGenerator* codegen, ASTNode* program_ast) {
    if (!codegen || !program_ast) {
        return false;
    }

    if (program_ast->type == AST_NODE_LIST) {
        // 遍历所有语句，只处理函数定义 / Traverse all statements, only process function definitions
        for (size_t i = 0; i < program_ast->data.list.count; i++) {
            ASTNode* stmt = program_ast->data.list.elements[i];
            if (stmt->type == AST_NODE_FUNCTION_DEF) {
                if (!codegen_generate_statement(codegen, stmt)) {
                    return false;
                }
            }
        }
    } else if (program_ast->type == AST_NODE_FUNCTION_DEF) {
        // 单个函数定义 / Single function definition
        if (!codegen_generate_statement(codegen, program_ast)) {
            return false;
        }
    }

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

    // 处理程序AST，跳过函数定义 / Process program AST, skip function definitions
    if (program_ast->type == AST_NODE_LIST) {
        // 处理每个语句，跳过函数定义 / Process each statement, skip function definitions
        for (size_t i = 0; i < program_ast->data.list.count; i++) {
            ASTNode* stmt = program_ast->data.list.elements[i];
            if (stmt->type != AST_NODE_FUNCTION_DEF) {
                codegen_generate_statement(codegen, stmt);
            }
        }
    } else if (program_ast->type != AST_NODE_FUNCTION_DEF) {
        // 单个语句，如果不是函数定义 / Single statement, if not function definition
        codegen_generate_statement(codegen, program_ast);
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

/**
 * @brief 生成max/min函数代码 / Generate max/min function code
 * @param codegen 代码生成器 / Code generator
 * @param args 参数数组 / Arguments array
 * @param arg_count 参数数量 / Argument count
 * @param is_max 是否为max函数 / Whether it's max function
 * @return 生成的代码字符串 / Generated code string
 */
char* codegen_generate_max_min(CodeGenerator* codegen, ASTNode** args, size_t arg_count, bool is_max) {
    if (arg_count < 2) return NULL;

    // 对于多参数，我们简化为只支持两个参数，避免复杂的嵌套
    // For multiple arguments, simplify to support only two arguments to avoid complex nesting
    if (arg_count > 2) {
        // 暂时只处理前两个参数 / Temporarily handle only first two arguments
        arg_count = 2;
    }

    // 生成两个参数 / Generate two arguments
    char* arg1 = codegen_generate_expression(codegen, args[0]);
    if (!arg1) return NULL;

    char* arg2 = codegen_generate_expression(codegen, args[1]);
    if (!arg2) {
        memory_free(arg1);
        return NULL;
    }

    // 计算结果长度 / Calculate result length
    size_t arg1_len = strlen(arg1);
    size_t arg2_len = strlen(arg2);
    size_t result_len = arg1_len * 2 + arg2_len * 2 + 30; // 足够的空间

    char* result = (char*)memory_alloc(result_len, MEM_TYPE_TEMP);
    if (!result) {
        memory_free(arg1);
        memory_free(arg2);
        return NULL;
    }

    // 生成条件表达式: (a > b ? a : b) for max, (a < b ? a : b) for min
    const char* op = is_max ? ">" : "<";
    snprintf(result, result_len, "((%s) %s (%s) ? (%s) : (%s))",
            arg1, op, arg2, arg1, arg2);

    memory_free(arg1);
    memory_free(arg2);

    return result;
}
