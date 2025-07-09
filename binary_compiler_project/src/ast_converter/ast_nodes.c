/**
 * @file ast_nodes.c
 * @brief 基础AST节点实现 / Basic AST Node Implementation
 * @author newLISP Binary Compiler Team
 * @date 2024
 */

#include "ast_nodes.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>

// 节点类型字符串映射 / Node type string mapping
static const char* node_type_strings[] = {
    "INVALID",        // AST_NODE_INVALID
    "INTEGER",        // AST_NODE_INTEGER
    "FLOAT",          // AST_NODE_FLOAT
    "STRING",         // AST_NODE_STRING
    "SYMBOL",         // AST_NODE_SYMBOL
    "BOOLEAN",        // AST_NODE_BOOLEAN
    "LIST",           // AST_NODE_LIST
    "FUNCTION_CALL",  // AST_NODE_FUNCTION_CALL
    "BINARY_OP",      // AST_NODE_BINARY_OP
    "UNARY_OP",       // AST_NODE_UNARY_OP
    "IF",             // AST_NODE_IF
    "WHILE",          // AST_NODE_WHILE
    "BLOCK",          // AST_NODE_BLOCK
    "FUNCTION_DEF",   // AST_NODE_FUNCTION_DEF
    "VARIABLE_DEF"    // AST_NODE_VARIABLE_DEF
};

// 数据类型字符串映射 / Data type string mapping
static const char* data_type_strings[] = {
    "UNKNOWN",        // DATA_TYPE_UNKNOWN
    "INTEGER",        // DATA_TYPE_INTEGER
    "FLOAT",          // DATA_TYPE_FLOAT
    "STRING",         // DATA_TYPE_STRING
    "BOOLEAN",        // DATA_TYPE_BOOLEAN
    "LIST",           // DATA_TYPE_LIST
    "FUNCTION"        // DATA_TYPE_FUNCTION
};

/**
 * @brief 创建AST节点 / Create AST node
 */
ASTNode* ast_create_node(ASTNodeType type) {
    if (type <= AST_NODE_INVALID || type >= AST_NODE_COUNT) {
        ERROR_REPORT_ERROR(ERROR_TYPE_TYPE, -1, "Invalid AST node type: %d", type);
        return NULL;
    }
    
    ASTNode* node = (ASTNode*)memory_alloc(sizeof(ASTNode), MEM_TYPE_AST);
    if (!node) {
        ERROR_REPORT_ERROR(ERROR_TYPE_MEMORY, -1, "Failed to allocate AST node");
        return NULL;
    }
    
    // 初始化节点 / Initialize node
    memset(node, 0, sizeof(ASTNode));
    node->type = type;
    node->data_type = DATA_TYPE_UNKNOWN;
    node->line = -1;
    node->column = -1;
    
    return node;
}

/**
 * @brief 销毁AST节点 / Destroy AST node
 */
void ast_destroy_node(ASTNode* node) {
    if (!node) return;
    
    // 根据节点类型释放特定数据 / Free type-specific data
    switch (node->type) {
        case AST_NODE_STRING:
            SAFE_FREE(node->data.literal.string_value);
            break;
            
        case AST_NODE_SYMBOL:
            SAFE_FREE(node->data.symbol.name);
            break;
            
        case AST_NODE_LIST:
            if (node->data.list.elements) {
                for (size_t i = 0; i < node->data.list.count; i++) {
                    ast_destroy_node(node->data.list.elements[i]);
                }
                memory_free(node->data.list.elements);
            }
            break;
            
        case AST_NODE_FUNCTION_CALL:
            ast_destroy_node(node->data.call.function);
            if (node->data.call.arguments) {
                for (size_t i = 0; i < node->data.call.arg_count; i++) {
                    ast_destroy_node(node->data.call.arguments[i]);
                }
                memory_free(node->data.call.arguments);
            }
            break;
            
        case AST_NODE_BINARY_OP:
            ast_destroy_node(node->data.binary_op.left);
            ast_destroy_node(node->data.binary_op.right);
            break;
            
        case AST_NODE_UNARY_OP:
            ast_destroy_node(node->data.unary_op.operand);
            break;
            
        case AST_NODE_IF:
            ast_destroy_node(node->data.if_stmt.condition);
            ast_destroy_node(node->data.if_stmt.then_branch);
            ast_destroy_node(node->data.if_stmt.else_branch);
            break;
            
        case AST_NODE_WHILE:
            ast_destroy_node(node->data.while_stmt.condition);
            ast_destroy_node(node->data.while_stmt.body);
            break;
            
        case AST_NODE_BLOCK:
            if (node->data.block.statements) {
                for (size_t i = 0; i < node->data.block.count; i++) {
                    ast_destroy_node(node->data.block.statements[i]);
                }
                memory_free(node->data.block.statements);
            }
            break;
            
        case AST_NODE_FUNCTION_DEF:
            SAFE_FREE(node->data.function_def.name);
            if (node->data.function_def.parameters) {
                for (size_t i = 0; i < node->data.function_def.param_count; i++) {
                    SAFE_FREE(node->data.function_def.parameters[i]);
                }
                memory_free(node->data.function_def.parameters);
            }
            ast_destroy_node(node->data.function_def.body);
            break;
            
        case AST_NODE_VARIABLE_DEF:
            SAFE_FREE(node->data.variable_def.name);
            ast_destroy_node(node->data.variable_def.value);
            break;
            
        default:
            // 其他类型无需特殊处理 / Other types need no special handling
            break;
    }
    
    // 释放通用数据 / Free common data
    SAFE_FREE(node->source_text);
    
    // 释放节点本身 / Free the node itself
    memory_free(node);
}

/**
 * @brief 创建整数字面量节点 / Create integer literal node
 */
ASTNode* ast_create_integer(int64_t value) {
    ASTNode* node = ast_create_node(AST_NODE_INTEGER);
    if (node) {
        node->data.literal.int_value = value;
        node->data_type = DATA_TYPE_INTEGER;
    }
    return node;
}

/**
 * @brief 创建浮点字面量节点 / Create float literal node
 */
ASTNode* ast_create_float(double value) {
    ASTNode* node = ast_create_node(AST_NODE_FLOAT);
    if (node) {
        node->data.literal.float_value = value;
        node->data_type = DATA_TYPE_FLOAT;
    }
    return node;
}

/**
 * @brief 创建字符串字面量节点 / Create string literal node
 */
ASTNode* ast_create_string(const char* value) {
    if (!value) {
        ERROR_REPORT_ERROR(ERROR_TYPE_TYPE, -1, "String value cannot be NULL");
        return NULL;
    }
    
    ASTNode* node = ast_create_node(AST_NODE_STRING);
    if (node) {
        node->data.literal.string_value = memory_strdup(value);
        node->data_type = DATA_TYPE_STRING;
        
        if (!node->data.literal.string_value) {
            ast_destroy_node(node);
            return NULL;
        }
    }
    return node;
}

/**
 * @brief 创建符号节点 / Create symbol node
 */
ASTNode* ast_create_symbol(const char* name) {
    if (!name) {
        ERROR_REPORT_ERROR(ERROR_TYPE_TYPE, -1, "Symbol name cannot be NULL");
        return NULL;
    }
    
    ASTNode* node = ast_create_node(AST_NODE_SYMBOL);
    if (node) {
        node->data.symbol.name = memory_strdup(name);
        node->data_type = DATA_TYPE_UNKNOWN; // 符号类型需要后续推断
        
        if (!node->data.symbol.name) {
            ast_destroy_node(node);
            return NULL;
        }
    }
    return node;
}

/**
 * @brief 创建布尔字面量节点 / Create boolean literal node
 */
ASTNode* ast_create_boolean(bool value) {
    ASTNode* node = ast_create_node(AST_NODE_BOOLEAN);
    if (node) {
        node->data.literal.bool_value = value;
        node->data_type = DATA_TYPE_BOOLEAN;
    }
    return node;
}

/**
 * @brief 创建列表节点 / Create list node
 */
ASTNode* ast_create_list(void) {
    ASTNode* node = ast_create_node(AST_NODE_LIST);
    if (node) {
        node->data.list.elements = NULL;
        node->data.list.count = 0;
        node->data.list.capacity = 0;
        node->data_type = DATA_TYPE_LIST;
    }
    return node;
}

/**
 * @brief 向列表添加元素 / Add element to list
 */
bool ast_list_add_element(ASTNode* list, ASTNode* element) {
    if (!list || list->type != AST_NODE_LIST || !element) {
        ERROR_REPORT_ERROR(ERROR_TYPE_TYPE, -1, "Invalid list or element");
        return false;
    }
    
    // 扩展数组容量 / Expand array capacity
    if (list->data.list.count >= list->data.list.capacity) {
        size_t new_capacity = list->data.list.capacity == 0 ? 4 : list->data.list.capacity * 2;
        ASTNode** new_elements = (ASTNode**)memory_realloc(
            list->data.list.elements, 
            new_capacity * sizeof(ASTNode*), 
            MEM_TYPE_AST
        );
        
        if (!new_elements) {
            ERROR_REPORT_ERROR(ERROR_TYPE_MEMORY, -1, "Failed to expand list capacity");
            return false;
        }
        
        list->data.list.elements = new_elements;
        list->data.list.capacity = new_capacity;
    }
    
    // 添加元素 / Add element
    list->data.list.elements[list->data.list.count++] = element;
    return true;
}

/**
 * @brief 创建二元运算节点 / Create binary operation node
 */
ASTNode* ast_create_binary_op(BinaryOpType op, ASTNode* left, ASTNode* right) {
    if (!left || !right) {
        ERROR_REPORT_ERROR(ERROR_TYPE_TYPE, -1, "Binary operation operands cannot be NULL");
        return NULL;
    }
    
    ASTNode* node = ast_create_node(AST_NODE_BINARY_OP);
    if (node) {
        node->data.binary_op.op = op;
        node->data.binary_op.left = left;
        node->data.binary_op.right = right;
        // 数据类型需要根据操作数推断
        node->data_type = DATA_TYPE_UNKNOWN;
    }
    return node;
}

/**
 * @brief 创建一元运算节点 / Create unary operation node
 */
ASTNode* ast_create_unary_op(UnaryOpType op, ASTNode* operand) {
    if (!operand) {
        ERROR_REPORT_ERROR(ERROR_TYPE_TYPE, -1, "Unary operation operand cannot be NULL");
        return NULL;
    }

    ASTNode* node = ast_create_node(AST_NODE_UNARY_OP);
    if (node) {
        node->data.unary_op.op = op;
        node->data.unary_op.operand = operand;
        node->data_type = DATA_TYPE_UNKNOWN; // 类型需要根据操作数推断
    }
    return node;
}

/**
 * @brief 创建函数调用节点 / Create function call node
 */
ASTNode* ast_create_function_call(ASTNode* function, ASTNode** arguments, size_t arg_count) {
    if (!function) {
        ERROR_REPORT_ERROR(ERROR_TYPE_TYPE, -1, "Function cannot be NULL");
        return NULL;
    }

    ASTNode* node = ast_create_node(AST_NODE_FUNCTION_CALL);
    if (node) {
        node->data.call.function = function;
        node->data.call.arg_count = arg_count;

        if (arg_count > 0 && arguments) {
            node->data.call.arguments = (ASTNode**)memory_alloc(
                arg_count * sizeof(ASTNode*), MEM_TYPE_AST);

            if (node->data.call.arguments) {
                memcpy(node->data.call.arguments, arguments, arg_count * sizeof(ASTNode*));
            } else {
                ast_destroy_node(node);
                return NULL;
            }
        } else {
            node->data.call.arguments = NULL;
        }

        node->data_type = DATA_TYPE_UNKNOWN; // 返回类型需要推断
    }
    return node;
}

/**
 * @brief 获取节点类型字符串 / Get node type string
 */
const char* ast_node_type_to_string(ASTNodeType type) {
    if (type >= 0 && type < AST_NODE_COUNT) {
        return node_type_strings[type];
    }
    return "UNKNOWN";
}

/**
 * @brief 获取数据类型字符串 / Get data type string
 */
const char* ast_data_type_to_string(DataType type) {
    if (type >= 0 && type < DATA_TYPE_COUNT) {
        return data_type_strings[type];
    }
    return "UNKNOWN";
}

/**
 * @brief 打印AST节点 / Print AST node
 */
void ast_print_node(const ASTNode* node, int indent) {
    if (!node) {
        printf("%*sNULL\n", indent * 2, "");
        return;
    }
    
    printf("%*s%s", indent * 2, "", ast_node_type_to_string(node->type));
    
    switch (node->type) {
        case AST_NODE_INTEGER:
            printf(": %ld", node->data.literal.int_value);
            break;
        case AST_NODE_FLOAT:
            printf(": %f", node->data.literal.float_value);
            break;
        case AST_NODE_STRING:
            printf(": \"%s\"", node->data.literal.string_value);
            break;
        case AST_NODE_SYMBOL:
            printf(": %s", node->data.symbol.name);
            break;
        case AST_NODE_BOOLEAN:
            printf(": %s", node->data.literal.bool_value ? "true" : "false");
            break;
        default:
            break;
    }
    
    printf("\n");
    
    // 递归打印子节点 / Recursively print child nodes
    switch (node->type) {
        case AST_NODE_LIST:
            for (size_t i = 0; i < node->data.list.count; i++) {
                ast_print_node(node->data.list.elements[i], indent + 1);
            }
            break;
        case AST_NODE_FUNCTION_CALL:
            printf("%*sFunction:\n", (indent + 1) * 2, "");
            ast_print_node(node->data.call.function, indent + 2);
            printf("%*sArguments:\n", (indent + 1) * 2, "");
            for (size_t i = 0; i < node->data.call.arg_count; i++) {
                ast_print_node(node->data.call.arguments[i], indent + 2);
            }
            break;
        case AST_NODE_BINARY_OP:
            printf("%*sLeft:\n", (indent + 1) * 2, "");
            ast_print_node(node->data.binary_op.left, indent + 2);
            printf("%*sRight:\n", (indent + 1) * 2, "");
            ast_print_node(node->data.binary_op.right, indent + 2);
            break;
        default:
            break;
    }
}

/**
 * @brief 创建if语句节点 / Create if statement node
 */
ASTNode* ast_create_if(ASTNode* condition, ASTNode* then_branch, ASTNode* else_branch) {
    if (!condition || !then_branch) {
        ERROR_REPORT_ERROR(ERROR_TYPE_TYPE, -1, "If condition and then branch cannot be NULL");
        return NULL;
    }

    ASTNode* node = ast_create_node(AST_NODE_IF);
    if (node) {
        node->data.if_stmt.condition = condition;
        node->data.if_stmt.then_branch = then_branch;
        node->data.if_stmt.else_branch = else_branch; // 可以为NULL
        node->data_type = DATA_TYPE_UNKNOWN;
    }
    return node;
}

/**
 * @brief 创建while循环节点 / Create while loop node
 */
ASTNode* ast_create_while(ASTNode* condition, ASTNode* body) {
    if (!condition || !body) {
        ERROR_REPORT_ERROR(ERROR_TYPE_TYPE, -1, "While loop condition and body cannot be NULL");
        return NULL;
    }

    ASTNode* node = ast_create_node(AST_NODE_WHILE);
    if (node) {
        node->data.while_stmt.condition = condition;
        node->data.while_stmt.body = body;
        node->data_type = DATA_TYPE_UNKNOWN;
    }
    return node;
}

/**
 * @brief 创建代码块节点 / Create block node
 */
ASTNode* ast_create_block(void) {
    ASTNode* node = ast_create_node(AST_NODE_BLOCK);
    if (node) {
        node->data.block.statements = NULL;
        node->data.block.count = 0;
        node->data_type = DATA_TYPE_UNKNOWN;
    }
    return node;
}

/**
 * @brief 向代码块添加语句 / Add statement to block
 */
bool ast_block_add_statement(ASTNode* block, ASTNode* statement) {
    if (!block || block->type != AST_NODE_BLOCK || !statement) {
        ERROR_REPORT_ERROR(ERROR_TYPE_TYPE, -1, "Invalid block or statement");
        return false;
    }

    // 重新分配数组 / Reallocate array
    size_t new_count = block->data.block.count + 1;
    ASTNode** new_statements = (ASTNode**)memory_realloc(
        block->data.block.statements,
        new_count * sizeof(ASTNode*),
        MEM_TYPE_AST
    );

    if (!new_statements) {
        ERROR_REPORT_ERROR(ERROR_TYPE_MEMORY, -1, "Failed to expand block statements");
        return false;
    }

    block->data.block.statements = new_statements;
    block->data.block.statements[block->data.block.count] = statement;
    block->data.block.count = new_count;

    return true;
}

/**
 * @brief 创建变量定义节点 / Create variable definition node
 */
ASTNode* ast_create_variable_def(const char* name, ASTNode* value) {
    if (!name) {
        ERROR_REPORT_ERROR(ERROR_TYPE_TYPE, -1, "Variable name cannot be NULL");
        return NULL;
    }

    ASTNode* node = ast_create_node(AST_NODE_VARIABLE_DEF);
    if (node) {
        node->data.variable_def.name = memory_strdup(name);
        node->data.variable_def.value = value;
        node->data_type = DATA_TYPE_UNKNOWN;

        if (!node->data.variable_def.name) {
            ast_destroy_node(node);
            return NULL;
        }
    }
    return node;
}

/**
 * @brief 创建函数定义节点 / Create function definition node
 */
ASTNode* ast_create_function_def(const char* name, char** parameters,
                                size_t param_count, ASTNode* body) {
    if (!name || !body) {
        ERROR_REPORT_ERROR(ERROR_TYPE_TYPE, -1, "Function name and body cannot be NULL");
        return NULL;
    }

    ASTNode* node = ast_create_node(AST_NODE_FUNCTION_DEF);
    if (node) {
        node->data.function_def.name = memory_strdup(name);
        node->data.function_def.param_count = param_count;
        node->data.function_def.body = body;

        if (!node->data.function_def.name) {
            ast_destroy_node(node);
            return NULL;
        }

        // 复制参数数组 / Copy parameter array
        if (param_count > 0 && parameters) {
            node->data.function_def.parameters = (char**)memory_alloc(
                param_count * sizeof(char*), MEM_TYPE_AST);

            if (!node->data.function_def.parameters) {
                ast_destroy_node(node);
                return NULL;
            }

            for (size_t i = 0; i < param_count; i++) {
                node->data.function_def.parameters[i] = memory_strdup(parameters[i]);
                if (!node->data.function_def.parameters[i]) {
                    // 清理已分配的参数 / Cleanup allocated parameters
                    for (size_t j = 0; j < i; j++) {
                        memory_free(node->data.function_def.parameters[j]);
                    }
                    ast_destroy_node(node);
                    return NULL;
                }
            }
        } else {
            node->data.function_def.parameters = NULL;
        }

        node->data_type = DATA_TYPE_FUNCTION;
    }
    return node;
}

/**
 * @brief 验证AST节点 / Validate AST node
 */
bool ast_validate_node(const ASTNode* node) {
    if (!node) {
        return false;
    }

    // 基础验证 / Basic validation
    if (node->type <= AST_NODE_INVALID || node->type >= AST_NODE_COUNT) {
        return false;
    }

    // 类型特定验证 / Type-specific validation
    switch (node->type) {
        case AST_NODE_STRING:
            return node->data.literal.string_value != NULL;
        case AST_NODE_SYMBOL:
            return node->data.symbol.name != NULL;
        case AST_NODE_BINARY_OP:
            return node->data.binary_op.left != NULL && node->data.binary_op.right != NULL;
        case AST_NODE_UNARY_OP:
            return node->data.unary_op.operand != NULL;
        case AST_NODE_FUNCTION_CALL:
            return node->data.call.function != NULL;
        case AST_NODE_IF:
            return node->data.if_stmt.condition != NULL && node->data.if_stmt.then_branch != NULL;
        case AST_NODE_WHILE:
            return node->data.while_stmt.condition != NULL && node->data.while_stmt.body != NULL;
        case AST_NODE_FUNCTION_DEF:
            return node->data.function_def.name != NULL && node->data.function_def.body != NULL;
        case AST_NODE_VARIABLE_DEF:
            return node->data.variable_def.name != NULL;
        default:
            return true;
    }
}
