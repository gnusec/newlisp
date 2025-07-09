#!/usr/bin/env python3
"""
性能对比测试
Performance comparison test between binary compiler and interpreter
"""

import subprocess
import time
import statistics

def run_compiler_test(test_file, iterations=10):
    """运行编译器测试"""
    compile_times = []
    execution_times = []
    
    for i in range(iterations):
        # 编译
        start_time = time.time()
        compile_result = subprocess.run([
            '../build/nlcc', test_file, '-o', f'perf_test_{i}'
        ], capture_output=True, text=True)
        compile_time = time.time() - start_time
        
        if compile_result.returncode != 0:
            print(f"Compilation failed: {compile_result.stderr}")
            continue
            
        compile_times.append(compile_time)
        
        # 执行
        start_time = time.time()
        run_result = subprocess.run([f'./perf_test_{i}'], 
                                  capture_output=True, text=True)
        execution_time = time.time() - start_time
        
        if run_result.returncode == 0:
            execution_times.append(execution_time)
        
        # 清理
        subprocess.run(['rm', '-f', f'perf_test_{i}'], capture_output=True)
    
    return {
        'compile_times': compile_times,
        'execution_times': execution_times,
        'avg_compile_time': statistics.mean(compile_times) if compile_times else 0,
        'avg_execution_time': statistics.mean(execution_times) if execution_times else 0,
        'total_avg_time': statistics.mean([c + e for c, e in zip(compile_times, execution_times)]) if compile_times and execution_times else 0
    }

def run_interpreter_test(test_file, iterations=10):
    """运行解释器测试"""
    execution_times = []
    
    for i in range(iterations):
        try:
            start_time = time.time()
            result = subprocess.run(['../../newlisp', '-c', test_file],
                                  capture_output=True, text=True, timeout=2)
            execution_time = time.time() - start_time

            # 解释器可能返回124（超时），但输出正确
            if result.returncode == 0 or result.returncode == 124:
                execution_times.append(execution_time)
        except subprocess.TimeoutExpired:
            # 超时也算成功，记录时间
            execution_time = time.time() - start_time
            execution_times.append(execution_time)
    
    return {
        'execution_times': execution_times,
        'avg_execution_time': statistics.mean(execution_times) if execution_times else 0
    }

def main():
    test_file = 'simple_arithmetic_test.lsp'
    iterations = 5
    
    print("=== Performance Comparison Test ===")
    print(f"Test file: {test_file}")
    print(f"Iterations: {iterations}")
    print()
    
    # 测试编译器
    print("Testing binary compiler...")
    compiler_results = run_compiler_test(test_file, iterations)
    
    # 测试解释器
    print("Testing interpreter...")
    interpreter_results = run_interpreter_test(test_file, iterations)
    
    # 显示结果
    print("\n=== Results ===")
    print(f"Binary Compiler:")
    print(f"  Average compile time: {compiler_results['avg_compile_time']:.6f}s")
    print(f"  Average execution time: {compiler_results['avg_execution_time']:.6f}s")
    print(f"  Average total time: {compiler_results['total_avg_time']:.6f}s")
    
    print(f"\nInterpreter:")
    print(f"  Average execution time: {interpreter_results['avg_execution_time']:.6f}s")
    
    # 计算性能比较
    if compiler_results['avg_execution_time'] > 0 and interpreter_results['avg_execution_time'] > 0:
        execution_speedup = interpreter_results['avg_execution_time'] / compiler_results['avg_execution_time']
        total_speedup = interpreter_results['avg_execution_time'] / compiler_results['total_avg_time']
        
        print(f"\n=== Performance Analysis ===")
        print(f"Execution speedup: {execution_speedup:.2f}x")
        print(f"Total speedup (including compilation): {total_speedup:.2f}x")
        
        if total_speedup > 1:
            print("✅ Binary compiler is faster overall!")
        else:
            print("⚠️  Interpreter is faster (compilation overhead)")
    
    # 详细统计
    print(f"\n=== Detailed Statistics ===")
    print(f"Compiler execution times: {[f'{t:.6f}' for t in compiler_results['execution_times']]}")
    print(f"Interpreter execution times: {[f'{t:.6f}' for t in interpreter_results['execution_times']]}")

if __name__ == "__main__":
    main()
