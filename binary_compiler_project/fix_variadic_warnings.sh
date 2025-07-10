#!/bin/bash

# 修复可变参数宏警告的脚本
# Script to fix variadic macro warnings

echo "🔧 Fixing variadic macro warnings..."

# 查找所有需要修复的模式
# Find all patterns that need fixing

# 修复没有可变参数的ERROR_REPORT_*调用
# Fix ERROR_REPORT_* calls without variadic arguments

find src/ -name "*.c" -exec sed -i 's/ERROR_REPORT_ERROR(ERROR_TYPE_[A-Z_]*, [^,]*, "[^"]*");/&/g' {} \;
find src/ -name "*.c" -exec sed -i 's/ERROR_REPORT_WARNING(ERROR_TYPE_[A-Z_]*, [^,]*, "[^"]*");/&/g' {} \;
find src/ -name "*.c" -exec sed -i 's/ERROR_REPORT_INFO(ERROR_TYPE_[A-Z_]*, [^,]*, "[^"]*");/&/g' {} \;
find src/ -name "*.c" -exec sed -i 's/ERROR_REPORT_FATAL(ERROR_TYPE_[A-Z_]*, [^,]*, "[^"]*");/&/g' {} \;

# 更精确的替换：在字符串后面添加%s", ""
# More precise replacement: add %s", "" after strings

# 处理ERROR_REPORT_ERROR
sed -i 's/ERROR_REPORT_ERROR(ERROR_TYPE_MEMORY, -1, "Failed to allocate lexer");/ERROR_REPORT_ERROR(ERROR_TYPE_MEMORY, -1, "Failed to allocate lexer%s", "");/g' src/ast_converter/simple_parser.c
sed -i 's/ERROR_REPORT_ERROR(ERROR_TYPE_MEMORY, -1, "Failed to allocate parser");/ERROR_REPORT_ERROR(ERROR_TYPE_MEMORY, -1, "Failed to allocate parser%s", "");/g' src/ast_converter/simple_parser.c
sed -i 's/ERROR_REPORT_ERROR(ERROR_TYPE_MEMORY, -1, "Failed to allocate string");/ERROR_REPORT_ERROR(ERROR_TYPE_MEMORY, -1, "Failed to allocate string%s", "");/g' src/ast_converter/simple_parser.c
sed -i 's/ERROR_REPORT_ERROR(ERROR_TYPE_MEMORY, -1, "Failed to allocate symbol");/ERROR_REPORT_ERROR(ERROR_TYPE_MEMORY, -1, "Failed to allocate symbol%s", "");/g' src/ast_converter/simple_parser.c
sed -i 's/ERROR_REPORT_ERROR(ERROR_TYPE_MEMORY, -1, "Failed to allocate file buffer");/ERROR_REPORT_ERROR(ERROR_TYPE_MEMORY, -1, "Failed to allocate file buffer%s", "");/g' src/ast_converter/simple_parser.c

# 处理其他常见模式
sed -i 's/ERROR_REPORT_ERROR(ERROR_TYPE_PARSE, -1, "Input cannot be NULL");/ERROR_REPORT_ERROR(ERROR_TYPE_PARSE, -1, "Input cannot be NULL%s", "");/g' src/ast_converter/simple_parser.c
sed -i 's/ERROR_REPORT_ERROR(ERROR_TYPE_PARSE, -1, "Filename cannot be NULL");/ERROR_REPORT_ERROR(ERROR_TYPE_PARSE, -1, "Filename cannot be NULL%s", "");/g' src/ast_converter/simple_parser.c
sed -i 's/ERROR_REPORT_ERROR(ERROR_TYPE_PARSE, -1, "Empty symbol");/ERROR_REPORT_ERROR(ERROR_TYPE_PARSE, -1, "Empty symbol%s", "");/g' src/ast_converter/simple_parser.c
sed -i 's/ERROR_REPORT_ERROR(ERROR_TYPE_PARSE, -1, "Unterminated string escape");/ERROR_REPORT_ERROR(ERROR_TYPE_PARSE, -1, "Unterminated string escape%s", "");/g' src/ast_converter/simple_parser.c
sed -i 's/ERROR_REPORT_ERROR(ERROR_TYPE_PARSE, -1, "Unterminated string literal");/ERROR_REPORT_ERROR(ERROR_TYPE_PARSE, -1, "Unterminated string literal%s", "");/g' src/ast_converter/simple_parser.c

# 处理error_handler.c
sed -i 's/ERROR_REPORT_ERROR(ERROR_TYPE_SYSTEM, -1, "Callback function is NULL");/ERROR_REPORT_ERROR(ERROR_TYPE_SYSTEM, -1, "Callback function is NULL%s", "");/g' src/runtime/error_handler.c

# 处理memory_manager.c
sed -i 's/ERROR_REPORT_ERROR(ERROR_TYPE_MEMORY, -1, "Memory manager not initialized");/ERROR_REPORT_ERROR(ERROR_TYPE_MEMORY, -1, "Memory manager not initialized%s", "");/g' src/runtime/memory_manager.c
sed -i 's/ERROR_REPORT_WARNING(ERROR_TYPE_MEMORY, -1, "Attempting to allocate 0 bytes");/ERROR_REPORT_WARNING(ERROR_TYPE_MEMORY, -1, "Attempting to allocate 0 bytes%s", "");/g' src/runtime/memory_manager.c

# 处理AST节点相关
sed -i 's/ERROR_REPORT_ERROR(ERROR_TYPE_MEMORY, -1, "Failed to allocate AST node");/ERROR_REPORT_ERROR(ERROR_TYPE_MEMORY, -1, "Failed to allocate AST node%s", "");/g' src/ast_converter/ast_nodes.c
sed -i 's/ERROR_REPORT_ERROR(ERROR_TYPE_TYPE, -1, "String value cannot be NULL");/ERROR_REPORT_ERROR(ERROR_TYPE_TYPE, -1, "String value cannot be NULL%s", "");/g' src/ast_converter/ast_nodes.c
sed -i 's/ERROR_REPORT_ERROR(ERROR_TYPE_TYPE, -1, "Symbol name cannot be NULL");/ERROR_REPORT_ERROR(ERROR_TYPE_TYPE, -1, "Symbol name cannot be NULL%s", "");/g' src/ast_converter/ast_nodes.c
sed -i 's/ERROR_REPORT_ERROR(ERROR_TYPE_TYPE, -1, "Invalid list or element");/ERROR_REPORT_ERROR(ERROR_TYPE_TYPE, -1, "Invalid list or element%s", "");/g' src/ast_converter/ast_nodes.c
sed -i 's/ERROR_REPORT_ERROR(ERROR_TYPE_MEMORY, -1, "Failed to expand list capacity");/ERROR_REPORT_ERROR(ERROR_TYPE_MEMORY, -1, "Failed to expand list capacity%s", "");/g' src/ast_converter/ast_nodes.c

# 处理代码生成器
sed -i 's/ERROR_REPORT_ERROR(ERROR_TYPE_MEMORY, -1, "Failed to allocate code buffer");/ERROR_REPORT_ERROR(ERROR_TYPE_MEMORY, -1, "Failed to allocate code buffer%s", "");/g' src/tinycc_integration/c_code_generator.c
sed -i 's/ERROR_REPORT_ERROR(ERROR_TYPE_MEMORY, -1, "Failed to allocate code buffer memory");/ERROR_REPORT_ERROR(ERROR_TYPE_MEMORY, -1, "Failed to allocate code buffer memory%s", "");/g' src/tinycc_integration/c_code_generator.c
sed -i 's/ERROR_REPORT_ERROR(ERROR_TYPE_MEMORY, -1, "Failed to expand code buffer");/ERROR_REPORT_ERROR(ERROR_TYPE_MEMORY, -1, "Failed to expand code buffer%s", "");/g' src/tinycc_integration/c_code_generator.c

echo "✅ Variadic macro warnings fixed!"
