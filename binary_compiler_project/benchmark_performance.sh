#!/bin/bash

# newLISP Binary Compiler Performance Benchmark
# 性能基准测试脚本

echo "🚀 newLISP Binary Compiler Performance Benchmark"
echo "================================================"

# 创建基准测试目录
mkdir -p benchmarks/results

# 获取当前时间戳
TIMESTAMP=$(date +"%Y%m%d_%H%M%S")
RESULT_FILE="benchmarks/results/benchmark_${TIMESTAMP}.txt"

echo "Benchmark started at: $(date)" > $RESULT_FILE
echo "Git commit: $(git rev-parse HEAD)" >> $RESULT_FILE
echo "================================================" >> $RESULT_FILE

# 测试1：编译速度基准
echo "📊 Test 1: Compilation Speed Benchmark"
echo "Test 1: Compilation Speed Benchmark" >> $RESULT_FILE

for test_file in qa-tests/*.lsp examples/*.lsp; do
    if [ -f "$test_file" ]; then
        echo "Testing: $test_file"
        echo "Testing: $test_file" >> $RESULT_FILE
        
        # 测量编译时间
        start_time=$(date +%s.%N)
        ./build/nlcc "$test_file" -o "benchmark_temp" > /dev/null 2>&1
        end_time=$(date +%s.%N)
        
        compile_time=$(echo "$end_time - $start_time" | bc -l)
        echo "  Compilation time: ${compile_time}s" | tee -a $RESULT_FILE
        
        # 清理临时文件
        rm -f benchmark_temp
    fi
done

echo "" >> $RESULT_FILE

# 测试2：生成代码大小基准
echo "📊 Test 2: Generated Code Size Benchmark"
echo "Test 2: Generated Code Size Benchmark" >> $RESULT_FILE

for test_file in examples/*.lsp; do
    if [ -f "$test_file" ]; then
        echo "Testing: $test_file"
        echo "Testing: $test_file" >> $RESULT_FILE
        
        # 编译并测量可执行文件大小
        ./build/nlcc "$test_file" -o "benchmark_temp" > /dev/null 2>&1
        if [ -f "benchmark_temp" ]; then
            size=$(stat -c%s "benchmark_temp" 2>/dev/null || stat -f%z "benchmark_temp" 2>/dev/null)
            echo "  Executable size: ${size} bytes" | tee -a $RESULT_FILE
            rm -f benchmark_temp
        fi
    fi
done

echo "" >> $RESULT_FILE

# 测试3：运行时性能基准
echo "📊 Test 3: Runtime Performance Benchmark"
echo "Test 3: Runtime Performance Benchmark" >> $RESULT_FILE

# 创建性能测试文件
cat > benchmarks/perf_test.lsp << 'EOF'
; 计算密集型测试
(define (fibonacci n)
  (if (< n 2)
      n
      (+ (fibonacci (- n 1)) (fibonacci (- n 2)))))

(define (factorial n)
  (if (<= n 1)
      1
      (* n (factorial (- n 1)))))

(println "Fibonacci(20):" (fibonacci 20))
(println "Factorial(10):" (factorial 10))

; 循环测试
(define sum 0)
(define i 0)
(while (< i 1000)
  (set sum (+ sum i))
  (set i (+ i 1)))
(println "Sum 0-999:" sum)
EOF

echo "Testing: benchmarks/perf_test.lsp"
echo "Testing: benchmarks/perf_test.lsp" >> $RESULT_FILE

# 编译性能测试
./build/nlcc benchmarks/perf_test.lsp -o benchmarks/perf_test > /dev/null 2>&1

if [ -f "benchmarks/perf_test" ]; then
    # 测量运行时间
    start_time=$(date +%s.%N)
    ./benchmarks/perf_test > /dev/null
    end_time=$(date +%s.%N)
    
    runtime=$(echo "$end_time - $start_time" | bc -l)
    echo "  Runtime: ${runtime}s" | tee -a $RESULT_FILE
    
    rm -f benchmarks/perf_test
fi

rm -f benchmarks/perf_test.lsp

echo "" >> $RESULT_FILE

# 测试4：内存使用基准
echo "📊 Test 4: Memory Usage Benchmark"
echo "Test 4: Memory Usage Benchmark" >> $RESULT_FILE

# 使用valgrind测量内存使用（如果可用）
if command -v valgrind > /dev/null 2>&1; then
    echo "Testing memory usage with valgrind..."
    echo "Memory usage test with valgrind:" >> $RESULT_FILE
    
    valgrind --tool=massif --massif-out-file=massif.out ./build/nlcc examples/hello.lsp -o temp_hello > /dev/null 2>&1
    if [ -f "massif.out" ]; then
        peak_mem=$(grep "mem_heap_B" massif.out | sort -n | tail -1 | cut -d= -f2)
        echo "  Peak memory usage: ${peak_mem} bytes" | tee -a $RESULT_FILE
        rm -f massif.out
    fi
    rm -f temp_hello
else
    echo "  Valgrind not available, skipping memory test" | tee -a $RESULT_FILE
fi

echo "" >> $RESULT_FILE

# 生成总结
echo "📊 Benchmark Summary"
echo "Benchmark Summary" >> $RESULT_FILE
echo "==================" >> $RESULT_FILE
echo "Benchmark completed at: $(date)" >> $RESULT_FILE
echo "Results saved to: $RESULT_FILE" >> $RESULT_FILE

# 显示结果文件位置
echo ""
echo "✅ Benchmark completed!"
echo "📄 Results saved to: $RESULT_FILE"
echo ""
echo "📊 Quick summary:"
cat $RESULT_FILE | grep -E "(Compilation time|Executable size|Runtime)" | head -10

echo ""
echo "🎯 Performance baseline established for future optimization!"
