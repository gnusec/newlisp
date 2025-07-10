#!/bin/bash

# newLISP Binary Compiler Project Cleanup Script
# 项目清理脚本

echo "🧹 Starting project cleanup..."

# 1. Remove core dump files
echo "Removing core dump files..."
find . -name "core.*" -type f -delete
echo "✅ Core dump files removed"

# 2. Remove temporary debug executables (keep .lsp files)
echo "Removing temporary debug executables..."
find . -name "debug_*" -type f ! -name "*.lsp" -delete
echo "✅ Debug executables removed"

# 3. Remove temporary test executables (keep .c files in src/)
echo "Removing temporary test executables..."
find . -name "test_*" -type f ! -name "*.c" ! -name "*.h" ! -path "./src/*" ! -path "./tests/*" -delete
echo "✅ Test executables removed"

# 4. Remove other temporary files
echo "Removing other temporary files..."
rm -f a.out
rm -f simple_nlcc parser_nlcc
rm -f *_test computation_test complex_test hello_test math_test stress_test
rm -f verify_newlisp_compiler
rm -f temp_test.lsp
echo "✅ Other temporary files removed"

# 5. Clean build artifacts (but keep build directory structure)
echo "Cleaning build artifacts..."
if [ -d "build" ]; then
    find build -name "*.o" -delete
    find build -name "*.a" -delete
    find build -name "nlcc" -delete
fi
echo "✅ Build artifacts cleaned"

# 6. Remove QA test executables (keep .lsp files)
echo "Removing QA test executables..."
find . -name "qa-*-test" -type f -delete
echo "✅ QA test executables removed"

# 7. Create organized directory structure
echo "Creating organized directory structure..."
mkdir -p archive/debug_files
mkdir -p archive/test_files
mkdir -p archive/temp_files

# Move remaining debug .lsp files to archive
find . -name "debug_*.lsp" -maxdepth 1 -exec mv {} archive/debug_files/ \;

echo "✅ Project cleanup completed!"

# 8. Show current project structure
echo ""
echo "📁 Current project structure:"
tree -L 2 -I 'archive|function_tests|qa-specific-tests' . || ls -la
