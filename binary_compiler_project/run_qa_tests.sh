#!/bin/bash

# newLISP二进制编译器QA测试套件
# newLISP Binary Compiler QA Test Suite

echo "========================================"
echo "newLISP Binary Compiler QA Test Suite"
echo "========================================"

# 测试计数器
TOTAL_TESTS=0
PASSED_TESTS=0
FAILED_TESTS=0

# 测试函数
run_test() {
    local test_name="$1"
    local test_file="$2"
    local output_file="$3"
    
    echo ""
    echo "Testing: $test_name"
    echo "----------------------------------------"
    
    TOTAL_TESTS=$((TOTAL_TESTS + 1))
    
    # 编译测试
    echo "Compiling $test_file..."
    if ./build/nlcc "$test_file" -o "$output_file" > /dev/null 2>&1; then
        echo "✓ Compilation successful"
        
        # 运行测试
        echo "Running $output_file..."
        if ./"$output_file" > test_output.tmp 2>&1; then
            echo "✓ Execution successful"
            
            # 检查是否包含SUCCESSFUL标记
            if grep -q "SUCCESSFUL" test_output.tmp; then
                echo "✓ Test PASSED"
                PASSED_TESTS=$((PASSED_TESTS + 1))
            else
                echo "✗ Test output doesn't contain SUCCESSFUL marker"
                echo "Output:"
                cat test_output.tmp
                FAILED_TESTS=$((FAILED_TESTS + 1))
            fi
        else
            echo "✗ Execution failed"
            echo "Error output:"
            cat test_output.tmp
            FAILED_TESTS=$((FAILED_TESTS + 1))
        fi
    else
        echo "✗ Compilation failed"
        FAILED_TESTS=$((FAILED_TESTS + 1))
    fi
    
    # 清理临时文件
    rm -f test_output.tmp "$output_file"
}

# 确保编译器已构建
if [ ! -f "./build/nlcc" ]; then
    echo "Error: Compiler not found. Please run 'make all' first."
    exit 1
fi

# 运行所有测试
run_test "Basic Arithmetic" "qa-tests/qa-basic-arithmetic.lsp" "test_arithmetic"
run_test "String Operations" "qa-tests/qa-strings.lsp" "test_strings"
run_test "Mixed Expressions" "qa-tests/qa-mixed-expressions.lsp" "test_mixed"
run_test "Simple Math" "qa-tests/qa-simple-math.lsp" "test_math"
run_test "Number Operations" "qa-tests/qa-numbers.lsp" "test_numbers"
run_test "Complex Expressions" "qa-tests/qa-expressions.lsp" "test_expressions"

# 运行现有的示例
run_test "Hello World" "examples/hello.lsp" "test_hello"
run_test "Math Examples" "examples/math.lsp" "test_math_examples"
run_test "Complex Functions" "examples/complex_functions.lsp" "test_complex"
run_test "Stress Test" "examples/stress_test.lsp" "test_stress"
run_test "Computation Heavy" "examples/computation_heavy.lsp" "test_computation"

# 显示测试结果
echo ""
echo "========================================"
echo "Test Results Summary"
echo "========================================"
echo "Total Tests: $TOTAL_TESTS"
echo "Passed: $PASSED_TESTS"
echo "Failed: $FAILED_TESTS"

if [ $FAILED_TESTS -eq 0 ]; then
    echo ""
    echo "🎉 ALL TESTS PASSED! 🎉"
    echo "The newLISP binary compiler is working correctly!"
    exit 0
else
    echo ""
    echo "❌ Some tests failed. Please check the output above."
    exit 1
fi
