#!/bin/bash

# 系统化修复剩余编译警告的脚本
# Systematic script to fix remaining compilation warnings

echo "🔧 Fixing remaining compilation warnings systematically..."

# 修复memory_manager.c中的可变参数宏警告
echo "Fixing memory_manager.c..."

# 查找所有没有%格式符的ERROR_REPORT调用并修复
sed -i 's/ERROR_REPORT_WARNING(ERROR_TYPE_MEMORY, -1, "Memory manager not initialized, using standard free");/ERROR_REPORT_WARNING(ERROR_TYPE_MEMORY, -1, "Memory manager not initialized, using standard free%s", "");/g' src/runtime/memory_manager.c

# 修复其他常见模式
sed -i 's/"Failed to allocate memory for memory manager");/"Failed to allocate memory for memory manager%s", "");/g' src/runtime/memory_manager.c
sed -i 's/"Memory manager initialized successfully");/"Memory manager initialized successfully%s", "");/g' src/runtime/memory_manager.c

# 修复AST相关文件
echo "Fixing AST files..."

# 修复ast_nodes.c中的警告
sed -i 's/"Failed to allocate AST node");/"Failed to allocate AST node%s", "");/g' src/ast_converter/ast_nodes.c
sed -i 's/"String value cannot be NULL");/"String value cannot be NULL%s", "");/g' src/ast_converter/ast_nodes.c
sed -i 's/"Symbol name cannot be NULL");/"Symbol name cannot be NULL%s", "");/g' src/ast_converter/ast_nodes.c
sed -i 's/"Invalid list or element");/"Invalid list or element%s", "");/g' src/ast_converter/ast_nodes.c
sed -i 's/"Failed to expand list capacity");/"Failed to expand list capacity%s", "");/g' src/ast_converter/ast_nodes.c
sed -i 's/"Binary operation operands cannot be NULL");/"Binary operation operands cannot be NULL%s", "");/g' src/ast_converter/ast_nodes.c
sed -i 's/"Unary operation operand cannot be NULL");/"Unary operation operand cannot be NULL%s", "");/g' src/ast_converter/ast_nodes.c
sed -i 's/"Function cannot be NULL");/"Function cannot be NULL%s", "");/g' src/ast_converter/ast_nodes.c
sed -i 's/"If condition and then branch cannot be NULL");/"If condition and then branch cannot be NULL%s", "");/g' src/ast_converter/ast_nodes.c
sed -i 's/"Invalid block or statement");/"Invalid block or statement%s", "");/g' src/ast_converter/ast_nodes.c
sed -i 's/"Failed to expand block statements");/"Failed to expand block statements%s", "");/g' src/ast_converter/ast_nodes.c
sed -i 's/"Variable name cannot be NULL");/"Variable name cannot be NULL%s", "");/g' src/ast_converter/ast_nodes.c
sed -i 's/"Function name and body cannot be NULL");/"Function name and body cannot be NULL%s", "");/g' src/ast_converter/ast_nodes.c

# 修复simple_parser.c中的警告
echo "Fixing simple_parser.c..."
sed -i 's/"Input cannot be NULL");/"Input cannot be NULL%s", "");/g' src/ast_converter/simple_parser.c
sed -i 's/"Failed to allocate lexer");/"Failed to allocate lexer%s", "");/g' src/ast_converter/simple_parser.c
sed -i 's/"Failed to allocate parser");/"Failed to allocate parser%s", "");/g' src/ast_converter/simple_parser.c
sed -i 's/"Failed to allocate string");/"Failed to allocate string%s", "");/g' src/ast_converter/simple_parser.c
sed -i 's/"Failed to allocate symbol");/"Failed to allocate symbol%s", "");/g' src/ast_converter/simple_parser.c
sed -i 's/"Failed to allocate file buffer");/"Failed to allocate file buffer%s", "");/g' src/ast_converter/simple_parser.c
sed -i 's/"Filename cannot be NULL");/"Filename cannot be NULL%s", "");/g' src/ast_converter/simple_parser.c
sed -i 's/"Empty symbol");/"Empty symbol%s", "");/g' src/ast_converter/simple_parser.c
sed -i 's/"Unterminated string escape");/"Unterminated string escape%s", "");/g' src/ast_converter/simple_parser.c
sed -i 's/"Unterminated string literal");/"Unterminated string literal%s", "");/g' src/ast_converter/simple_parser.c
sed -i 's/"Input string cannot be NULL");/"Input string cannot be NULL%s", "");/g' src/ast_converter/simple_parser.c

# 修复代码生成器中的警告
echo "Fixing code generator..."
sed -i 's/"Failed to allocate code buffer");/"Failed to allocate code buffer%s", "");/g' src/tinycc_integration/c_code_generator.c
sed -i 's/"Failed to allocate code buffer memory");/"Failed to allocate code buffer memory%s", "");/g' src/tinycc_integration/c_code_generator.c
sed -i 's/"Failed to expand code buffer");/"Failed to expand code buffer%s", "");/g' src/tinycc_integration/c_code_generator.c
sed -i 's/"Failed to allocate symbol table");/"Failed to allocate symbol table%s", "");/g' src/tinycc_integration/c_code_generator.c
sed -i 's/"Failed to allocate symbol entry");/"Failed to allocate symbol entry%s", "");/g' src/tinycc_integration/c_code_generator.c
sed -i 's/"Failed to allocate code generator");/"Failed to allocate code generator%s", "");/g' src/tinycc_integration/c_code_generator.c

echo "✅ Variadic macro warnings fixed!"

# 修复未使用参数警告
echo "🔧 Fixing unused parameter warnings..."

# 在memory_manager.c中添加未使用参数标记
sed -i 's/void memory_free_debug(void\* ptr, const char\* file, int line, const char\* function) {/void memory_free_debug(void* ptr, const char* file, int line, const char* function) {\n    (void)function; \/\/ 标记未使用参数/g' src/runtime/memory_manager.c

# 修复remove_guard_bytes函数
sed -i 's/static void\* remove_guard_bytes(void\* ptr, size_t\* size) {/static void* remove_guard_bytes(void* ptr, size_t* size) {\n    (void)size; \/\/ 标记未使用参数/g' src/runtime/memory_manager.c

echo "✅ Unused parameter warnings fixed!"

echo "🎉 All warnings fixed systematically!"
