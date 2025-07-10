#!/bin/bash

# 简单性能基准测试 / Simple Performance Benchmark
echo "🚀 Simple Performance Benchmark"
echo "================================"

# 测试编译速度
echo "📊 Compilation Speed Test"
echo "Testing hello.lsp compilation..."

time ./build/nlcc examples/hello.lsp -o test_hello
if [ -f "test_hello" ]; then
    echo "✅ Compilation successful"
    echo "📏 Executable size: $(stat -c%s test_hello 2>/dev/null || stat -f%z test_hello 2>/dev/null) bytes"
    
    echo "🏃 Runtime Test"
    time ./test_hello
    
    rm -f test_hello
else
    echo "❌ Compilation failed"
fi

echo ""
echo "📊 Complex Function Test"
echo "Testing complex_functions.lsp..."

time ./build/nlcc examples/complex_functions.lsp -o test_complex
if [ -f "test_complex" ]; then
    echo "✅ Compilation successful"
    echo "📏 Executable size: $(stat -c%s test_complex 2>/dev/null || stat -f%z test_complex 2>/dev/null) bytes"
    
    echo "🏃 Runtime Test"
    time ./test_complex
    
    rm -f test_complex
else
    echo "❌ Compilation failed"
fi

echo ""
echo "✅ Benchmark completed!"
