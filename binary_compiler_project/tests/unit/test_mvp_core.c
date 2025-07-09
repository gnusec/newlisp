/**
 * @file test_mvp_core.c
 * @brief MVP核心功能测试 / MVP Core Functionality Tests
 * @author newLISP Binary Compiler Team
 * @date 2024
 * 
 * 测试编译器核心功能的最小可行版本
 * Tests the minimum viable version of compiler core functionality
 */

#include "../../src/runtime/error_handler.h"
#include "../../src/runtime/memory_manager.h"
#include "../../src/ast_converter/ast_nodes.h"
#include "../../src/ast_converter/simple_parser.h"
#include "../../src/tinycc_integration/c_code_generator.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <math.h>

// 简化的测试框架 / Simplified test framework
#define TEST_ASSERT(condition) \
    do { \
        if (!(condition)) { \
            printf("ASSERTION FAILED: %s at %s:%d\n", #condition, __FILE__, __LINE__); \
            return false; \
        } \
    } while(0)

#define TEST_ASSERT_NOT_NULL(ptr) TEST_ASSERT((ptr) != NULL)
#define TEST_ASSERT_NULL(ptr) TEST_ASSERT((ptr) == NULL)
#define TEST_ASSERT_EQUAL(expected, actual) TEST_ASSERT((expected) == (actual))
#define TEST_ASSERT_STRING_EQUAL(expected, actual) TEST_ASSERT(strcmp((expected), (actual)) == 0)

// 测试计数器 / Test counters
static int tests_run = 0;
static int tests_passed = 0;
static int tests_failed = 0;

// 运行测试宏 / Run test macro
#define RUN_TEST(test_func) \
    do { \
        printf("Running %s... ", #test_func); \
        tests_run++; \
        if (test_func()) { \
            printf("PASS\n"); \
            tests_passed++; \
        } else { \
            printf("FAIL\n"); \
            tests_failed++; \
        } \
    } while(0)

// 测试AST节点创建 / Test AST node creation
bool test_ast_node_creation(void) {
    // 测试整数节点 / Test integer node
    ASTNode* int_node = ast_create_integer(42);
    TEST_ASSERT_NOT_NULL(int_node);
    TEST_ASSERT_EQUAL(AST_NODE_INTEGER, int_node->type);
    TEST_ASSERT_EQUAL(42, int_node->data.literal.int_value);
    TEST_ASSERT_EQUAL(DATA_TYPE_INTEGER, int_node->data_type);
    
    // 测试字符串节点 / Test string node
    ASTNode* str_node = ast_create_string("hello");
    TEST_ASSERT_NOT_NULL(str_node);
    TEST_ASSERT_EQUAL(AST_NODE_STRING, str_node->type);
    TEST_ASSERT_STRING_EQUAL("hello", str_node->data.literal.string_value);
    TEST_ASSERT_EQUAL(DATA_TYPE_STRING, str_node->data_type);
    
    // 测试符号节点 / Test symbol node
    ASTNode* sym_node = ast_create_symbol("x");
    TEST_ASSERT_NOT_NULL(sym_node);
    TEST_ASSERT_EQUAL(AST_NODE_SYMBOL, sym_node->type);
    TEST_ASSERT_STRING_EQUAL("x", sym_node->data.symbol.name);
    
    // 测试布尔节点 / Test boolean node
    ASTNode* bool_node = ast_create_boolean(true);
    TEST_ASSERT_NOT_NULL(bool_node);
    TEST_ASSERT_EQUAL(AST_NODE_BOOLEAN, bool_node->type);
    TEST_ASSERT_EQUAL(true, bool_node->data.literal.bool_value);
    TEST_ASSERT_EQUAL(DATA_TYPE_BOOLEAN, bool_node->data_type);
    
    // 清理 / Cleanup
    ast_destroy_node(int_node);
    ast_destroy_node(str_node);
    ast_destroy_node(sym_node);
    ast_destroy_node(bool_node);
    
    return true;
}

// 测试列表节点 / Test list node
bool test_ast_list_operations(void) {
    // 创建列表 / Create list
    ASTNode* list = ast_create_list();
    TEST_ASSERT_NOT_NULL(list);
    TEST_ASSERT_EQUAL(AST_NODE_LIST, list->type);
    TEST_ASSERT_EQUAL(0, list->data.list.count);
    
    // 添加元素 / Add elements
    ASTNode* elem1 = ast_create_integer(1);
    ASTNode* elem2 = ast_create_integer(2);
    ASTNode* elem3 = ast_create_integer(3);
    
    TEST_ASSERT(ast_list_add_element(list, elem1));
    TEST_ASSERT(ast_list_add_element(list, elem2));
    TEST_ASSERT(ast_list_add_element(list, elem3));
    
    TEST_ASSERT_EQUAL(3, list->data.list.count);
    TEST_ASSERT_EQUAL(1, list->data.list.elements[0]->data.literal.int_value);
    TEST_ASSERT_EQUAL(2, list->data.list.elements[1]->data.literal.int_value);
    TEST_ASSERT_EQUAL(3, list->data.list.elements[2]->data.literal.int_value);
    
    // 清理 / Cleanup
    ast_destroy_node(list);
    
    return true;
}

// 测试二元运算节点 / Test binary operation node
bool test_ast_binary_operations(void) {
    // 创建操作数 / Create operands
    ASTNode* left = ast_create_integer(10);
    ASTNode* right = ast_create_integer(20);
    
    // 创建加法运算 / Create addition operation
    ASTNode* add_op = ast_create_binary_op(BINARY_OP_ADD, left, right);
    TEST_ASSERT_NOT_NULL(add_op);
    TEST_ASSERT_EQUAL(AST_NODE_BINARY_OP, add_op->type);
    TEST_ASSERT_EQUAL(BINARY_OP_ADD, add_op->data.binary_op.op);
    TEST_ASSERT_EQUAL(left, add_op->data.binary_op.left);
    TEST_ASSERT_EQUAL(right, add_op->data.binary_op.right);
    
    // 验证操作数 / Verify operands
    TEST_ASSERT_EQUAL(10, add_op->data.binary_op.left->data.literal.int_value);
    TEST_ASSERT_EQUAL(20, add_op->data.binary_op.right->data.literal.int_value);
    
    // 清理 / Cleanup
    ast_destroy_node(add_op);
    
    return true;
}

// 测试函数调用节点 / Test function call node
bool test_ast_function_call(void) {
    // 创建函数名 / Create function name
    ASTNode* func_name = ast_create_symbol("add");
    
    // 创建参数 / Create arguments
    ASTNode* arg1 = ast_create_integer(5);
    ASTNode* arg2 = ast_create_integer(10);
    ASTNode* arguments[] = {arg1, arg2};
    
    // 创建函数调用 / Create function call
    ASTNode* call = ast_create_function_call(func_name, arguments, 2);
    TEST_ASSERT_NOT_NULL(call);
    TEST_ASSERT_EQUAL(AST_NODE_FUNCTION_CALL, call->type);
    TEST_ASSERT_EQUAL(func_name, call->data.call.function);
    TEST_ASSERT_EQUAL(2, call->data.call.arg_count);
    
    // 验证参数 / Verify arguments
    TEST_ASSERT_EQUAL(5, call->data.call.arguments[0]->data.literal.int_value);
    TEST_ASSERT_EQUAL(10, call->data.call.arguments[1]->data.literal.int_value);
    
    // 清理 / Cleanup
    ast_destroy_node(call);
    
    return true;
}

// 测试浮点数和一元运算 / Test float and unary operations
bool test_ast_float_and_unary(void) {
    // 测试浮点数节点 / Test float node
    ASTNode* float_node = ast_create_float(3.14159);
    TEST_ASSERT_NOT_NULL(float_node);
    TEST_ASSERT_EQUAL(AST_NODE_FLOAT, float_node->type);
    TEST_ASSERT_EQUAL(DATA_TYPE_FLOAT, float_node->data_type);
    // 浮点数比较需要容差 / Float comparison needs tolerance
    TEST_ASSERT(fabs(float_node->data.literal.float_value - 3.14159) < 0.00001);

    // 测试一元运算 / Test unary operation
    ASTNode* operand = ast_create_integer(42);
    ASTNode* unary_op = ast_create_unary_op(UNARY_OP_NEG, operand);
    TEST_ASSERT_NOT_NULL(unary_op);
    TEST_ASSERT_EQUAL(AST_NODE_UNARY_OP, unary_op->type);
    TEST_ASSERT_EQUAL(UNARY_OP_NEG, unary_op->data.unary_op.op);
    TEST_ASSERT_EQUAL(operand, unary_op->data.unary_op.operand);

    // 清理 / Cleanup
    ast_destroy_node(float_node);
    ast_destroy_node(unary_op);

    return true;
}

// 测试控制流节点 / Test control flow nodes
bool test_ast_control_flow(void) {
    // 测试if语句 / Test if statement
    ASTNode* condition = ast_create_boolean(true);
    ASTNode* then_branch = ast_create_integer(1);
    ASTNode* else_branch = ast_create_integer(0);

    ASTNode* if_stmt = ast_create_if(condition, then_branch, else_branch);
    TEST_ASSERT_NOT_NULL(if_stmt);
    TEST_ASSERT_EQUAL(AST_NODE_IF, if_stmt->type);
    TEST_ASSERT_EQUAL(condition, if_stmt->data.if_stmt.condition);
    TEST_ASSERT_EQUAL(then_branch, if_stmt->data.if_stmt.then_branch);
    TEST_ASSERT_EQUAL(else_branch, if_stmt->data.if_stmt.else_branch);

    // 测试while循环 / Test while loop
    ASTNode* loop_condition = ast_create_boolean(false);
    ASTNode* loop_body = ast_create_integer(42);

    ASTNode* while_stmt = ast_create_while(loop_condition, loop_body);
    TEST_ASSERT_NOT_NULL(while_stmt);
    TEST_ASSERT_EQUAL(AST_NODE_WHILE, while_stmt->type);
    TEST_ASSERT_EQUAL(loop_condition, while_stmt->data.while_stmt.condition);
    TEST_ASSERT_EQUAL(loop_body, while_stmt->data.while_stmt.body);

    // 测试代码块 / Test block
    ASTNode* block = ast_create_block();
    TEST_ASSERT_NOT_NULL(block);
    TEST_ASSERT_EQUAL(AST_NODE_BLOCK, block->type);
    TEST_ASSERT_EQUAL(0, block->data.block.count);

    // 向代码块添加语句 / Add statements to block
    ASTNode* stmt1 = ast_create_integer(1);
    ASTNode* stmt2 = ast_create_integer(2);
    TEST_ASSERT(ast_block_add_statement(block, stmt1));
    TEST_ASSERT(ast_block_add_statement(block, stmt2));
    TEST_ASSERT_EQUAL(2, block->data.block.count);

    // 清理 / Cleanup
    ast_destroy_node(if_stmt);
    ast_destroy_node(while_stmt);
    ast_destroy_node(block);

    return true;
}

// 测试函数和变量定义 / Test function and variable definitions
bool test_ast_definitions(void) {
    // 测试变量定义 / Test variable definition
    ASTNode* var_value = ast_create_integer(42);
    ASTNode* var_def = ast_create_variable_def("x", var_value);
    TEST_ASSERT_NOT_NULL(var_def);
    TEST_ASSERT_EQUAL(AST_NODE_VARIABLE_DEF, var_def->type);
    TEST_ASSERT_STRING_EQUAL("x", var_def->data.variable_def.name);
    TEST_ASSERT_EQUAL(var_value, var_def->data.variable_def.value);

    // 测试函数定义 / Test function definition
    char* params[] = {"a", "b"};
    ASTNode* func_body = ast_create_binary_op(BINARY_OP_ADD,
                                             ast_create_symbol("a"),
                                             ast_create_symbol("b"));

    ASTNode* func_def = ast_create_function_def("add", params, 2, func_body);
    TEST_ASSERT_NOT_NULL(func_def);
    TEST_ASSERT_EQUAL(AST_NODE_FUNCTION_DEF, func_def->type);
    TEST_ASSERT_STRING_EQUAL("add", func_def->data.function_def.name);
    TEST_ASSERT_EQUAL(2, func_def->data.function_def.param_count);
    TEST_ASSERT_STRING_EQUAL("a", func_def->data.function_def.parameters[0]);
    TEST_ASSERT_STRING_EQUAL("b", func_def->data.function_def.parameters[1]);
    TEST_ASSERT_EQUAL(func_body, func_def->data.function_def.body);

    // 清理 / Cleanup
    ast_destroy_node(var_def);
    ast_destroy_node(func_def);

    return true;
}

// 测试错误边界条件 / Test error boundary conditions
bool test_ast_error_conditions(void) {
    // 测试NULL参数 / Test NULL parameters
    ASTNode* null_string = ast_create_string(NULL);
    TEST_ASSERT_NULL(null_string);

    ASTNode* null_symbol = ast_create_symbol(NULL);
    TEST_ASSERT_NULL(null_symbol);

    ASTNode* null_binary = ast_create_binary_op(BINARY_OP_ADD, NULL, NULL);
    TEST_ASSERT_NULL(null_binary);

    ASTNode* null_func_call = ast_create_function_call(NULL, NULL, 0);
    TEST_ASSERT_NULL(null_func_call);

    // 测试无效列表操作 / Test invalid list operations
    ASTNode* not_a_list = ast_create_integer(42);
    TEST_ASSERT(!ast_list_add_element(not_a_list, ast_create_integer(1)));
    TEST_ASSERT(!ast_list_add_element(NULL, ast_create_integer(1)));

    // 清理 / Cleanup
    ast_destroy_node(not_a_list);

    return true;
}

// 测试AST验证 / Test AST validation
bool test_ast_validation(void) {
    // 测试有效节点 / Test valid nodes
    ASTNode* valid_int = ast_create_integer(42);
    TEST_ASSERT(ast_validate_node(valid_int));

    ASTNode* valid_str = ast_create_string("test");
    TEST_ASSERT(ast_validate_node(valid_str));

    ASTNode* valid_symbol = ast_create_symbol("x");
    TEST_ASSERT(ast_validate_node(valid_symbol));

    // 测试无效节点 / Test invalid nodes
    TEST_ASSERT(!ast_validate_node(NULL));

    // 测试二元运算验证 / Test binary operation validation
    ASTNode* left = ast_create_integer(1);
    ASTNode* right = ast_create_integer(2);
    ASTNode* valid_binary = ast_create_binary_op(BINARY_OP_ADD, left, right);
    TEST_ASSERT(ast_validate_node(valid_binary));

    // 清理 / Cleanup
    ast_destroy_node(valid_int);
    ast_destroy_node(valid_str);
    ast_destroy_node(valid_symbol);
    ast_destroy_node(valid_binary);

    return true;
}

// 测试代码缓冲区 / Test code buffer
bool test_code_buffer(void) {
    // 创建代码缓冲区 / Create code buffer
    CodeBuffer* buffer = codebuffer_create(256);
    TEST_ASSERT_NOT_NULL(buffer);
    TEST_ASSERT_EQUAL(0, buffer->size);
    TEST_ASSERT(buffer->capacity >= 256);
    
    // 测试追加字符串 / Test append string
    TEST_ASSERT(codebuffer_append(buffer, "hello"));
    TEST_ASSERT_EQUAL(5, buffer->size);
    
    TEST_ASSERT(codebuffer_append(buffer, " world"));
    TEST_ASSERT_EQUAL(11, buffer->size);
    
    // 测试格式化追加 / Test formatted append
    TEST_ASSERT(codebuffer_appendf(buffer, " %d", 42));
    TEST_ASSERT_EQUAL(14, buffer->size);
    
    // 验证内容 / Verify content
    TEST_ASSERT_STRING_EQUAL("hello world 42", buffer->buffer);
    
    // 测试缩进 / Test indentation
    codebuffer_indent(buffer);
    TEST_ASSERT_EQUAL(1, buffer->indent_level);
    
    codebuffer_dedent(buffer);
    TEST_ASSERT_EQUAL(0, buffer->indent_level);
    
    // 清理 / Cleanup
    codebuffer_destroy(buffer);
    
    return true;
}

// 测试符号表 / Test symbol table
bool test_symbol_table(void) {
    // 创建符号表 / Create symbol table
    SymbolTable* table = symboltable_create(NULL);
    TEST_ASSERT_NOT_NULL(table);
    TEST_ASSERT_EQUAL(0, table->count);
    
    // 添加符号 / Add symbols
    TEST_ASSERT(symboltable_add(table, "x", DATA_TYPE_INTEGER, "var_x"));
    TEST_ASSERT(symboltable_add(table, "y", DATA_TYPE_STRING, "var_y"));
    TEST_ASSERT_EQUAL(2, table->count);
    
    // 查找符号 / Find symbols
    SymbolEntry* entry_x = symboltable_find(table, "x");
    TEST_ASSERT_NOT_NULL(entry_x);
    TEST_ASSERT_STRING_EQUAL("x", entry_x->name);
    TEST_ASSERT_EQUAL(DATA_TYPE_INTEGER, entry_x->type);
    TEST_ASSERT_STRING_EQUAL("var_x", entry_x->c_name);
    
    SymbolEntry* entry_y = symboltable_find(table, "y");
    TEST_ASSERT_NOT_NULL(entry_y);
    TEST_ASSERT_STRING_EQUAL("y", entry_y->name);
    TEST_ASSERT_EQUAL(DATA_TYPE_STRING, entry_y->type);
    
    // 查找不存在的符号 / Find non-existent symbol
    SymbolEntry* entry_z = symboltable_find(table, "z");
    TEST_ASSERT_NULL(entry_z);
    
    // 清理 / Cleanup
    symboltable_destroy(table);
    
    return true;
}

// 测试基础代码生成 / Test basic code generation
bool test_basic_code_generation(void) {
    // 创建代码生成器 / Create code generator
    CodeGenConfig config = codegen_create_default_config();
    CodeGenerator* codegen = codegen_create(&config);
    TEST_ASSERT_NOT_NULL(codegen);
    
    // 测试整数表达式生成 / Test integer expression generation
    ASTNode* int_node = ast_create_integer(42);
    char* int_code = codegen_generate_expression(codegen, int_node);
    TEST_ASSERT_NOT_NULL(int_code);
    // 应该生成类似 "42" 的代码 / Should generate code like "42"
    
    // 测试字符串表达式生成 / Test string expression generation
    ASTNode* str_node = ast_create_string("hello");
    char* str_code = codegen_generate_expression(codegen, str_node);
    TEST_ASSERT_NOT_NULL(str_code);
    // 应该生成类似 "\"hello\"" 的代码 / Should generate code like "\"hello\""
    
    // 清理 / Cleanup
    memory_free(int_code);
    memory_free(str_code);
    ast_destroy_node(int_node);
    ast_destroy_node(str_node);
    codegen_destroy(codegen);
    
    return true;
}

// 测试内存管理集成 / Test memory management integration
bool test_memory_integration(void) {
    // 获取初始内存统计 / Get initial memory stats
    const MemoryStats* initial_stats = memory_get_stats();
    size_t initial_usage = initial_stats ? initial_stats->current_usage : 0;
    
    // 创建和销毁多个AST节点 / Create and destroy multiple AST nodes
    for (int i = 0; i < 100; i++) {
        ASTNode* node = ast_create_integer(i);
        TEST_ASSERT_NOT_NULL(node);
        ast_destroy_node(node);
    }
    
    // 检查内存使用 / Check memory usage
    const MemoryStats* final_stats = memory_get_stats();
    if (final_stats) {
        // 内存使用应该回到初始状态 / Memory usage should return to initial state
        TEST_ASSERT_EQUAL(initial_usage, final_stats->current_usage);
    }
    
    return true;
}

// 测试错误处理集成 / Test error handling integration
bool test_error_integration(void) {
    // 清除之前的错误 / Clear previous errors
    error_clear_history();
    
    // 尝试创建无效的AST节点 / Try to create invalid AST node
    ASTNode* invalid_node = ast_create_string(NULL);
    TEST_ASSERT_NULL(invalid_node);
    
    // 检查是否记录了错误 / Check if error was recorded
    TEST_ASSERT(error_has_errors(ERROR_LEVEL_ERROR));
    
    const ErrorInfo* last_error = error_get_last();
    TEST_ASSERT_NOT_NULL(last_error);
    TEST_ASSERT_EQUAL(ERROR_TYPE_TYPE, last_error->type);
    
    return true;
}

// 主测试函数 / Main test function
int main(void) {
    printf("=== MVP Core Functionality Tests ===\n\n");
    
    // 初始化基础设施 / Initialize infrastructure
    if (!error_system_init(10, 100)) {
        printf("Failed to initialize error system\n");
        return 1;
    }
    
    MemoryConfig mem_config = {
        .enable_tracking = true,
        .enable_debugging = true,
        .enable_leak_detection = true,
        .enable_bounds_checking = false,
        .max_memory_limit = 0,
        .alignment = 8
    };
    
    if (!memory_manager_init(&mem_config)) {
        printf("Failed to initialize memory manager\n");
        return 1;
    }
    
    // 运行测试 / Run tests
    RUN_TEST(test_ast_node_creation);
    RUN_TEST(test_ast_list_operations);
    RUN_TEST(test_ast_binary_operations);
    RUN_TEST(test_ast_function_call);
    RUN_TEST(test_ast_float_and_unary);
    RUN_TEST(test_ast_control_flow);
    RUN_TEST(test_ast_definitions);
    RUN_TEST(test_ast_error_conditions);
    RUN_TEST(test_ast_validation);
    RUN_TEST(test_code_buffer);
    RUN_TEST(test_symbol_table);
    RUN_TEST(test_basic_code_generation);
    RUN_TEST(test_memory_integration);
    RUN_TEST(test_error_integration);
    
    // 打印测试结果 / Print test results
    printf("\n=== Test Results ===\n");
    printf("Tests run: %d\n", tests_run);
    printf("Tests passed: %d\n", tests_passed);
    printf("Tests failed: %d\n", tests_failed);
    printf("Success rate: %.1f%%\n", 
           tests_run > 0 ? (double)tests_passed / tests_run * 100.0 : 0.0);
    
    // 检查内存泄漏 / Check memory leaks
    int leak_count = memory_check_leaks(stdout);
    if (leak_count > 0) {
        printf("WARNING: %d memory leaks detected\n", leak_count);
    }
    
    // 清理 / Cleanup
    memory_manager_cleanup();
    error_system_cleanup();
    
    return tests_failed == 0 ? 0 : 1;
}
