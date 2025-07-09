#!/usr/bin/env python3
"""
newLISP编译器对比测试框架
Comparison testing framework for newLISP compiler vs interpreter
"""

import subprocess
import time
import json
import os
from pathlib import Path
import tempfile

class ComparisonFramework:
    def __init__(self):
        self.compiler_path = "../build/nlcc"
        self.interpreter_path = None  # 将尝试查找newlisp解释器
        self.results = {
            'tests': [],
            'summary': {
                'total': 0,
                'compiler_success': 0,
                'interpreter_success': 0,
                'both_success': 0,
                'performance_data': []
            }
        }
        self.find_interpreter()
    
    def find_interpreter(self):
        """查找newLISP解释器"""
        possible_paths = [
            'newlisp',
            '/usr/bin/newlisp',
            '/usr/local/bin/newlisp',
            '../newlisp',
            '../../newlisp'
        ]
        
        for path in possible_paths:
            try:
                result = subprocess.run([path, '-v'], 
                                      capture_output=True, 
                                      text=True, 
                                      timeout=5)
                if result.returncode == 0:
                    self.interpreter_path = path
                    print(f"Found newLISP interpreter: {path}")
                    return
            except (subprocess.TimeoutExpired, FileNotFoundError):
                continue
        
        print("Warning: newLISP interpreter not found. Will only test compiler.")
    
    def compile_with_our_compiler(self, test_file):
        """使用我们的编译器编译测试文件"""
        try:
            output_file = test_file.with_suffix('')
            cmd = [self.compiler_path, str(test_file), '-o', str(output_file)]
            
            start_time = time.time()
            result = subprocess.run(cmd, 
                                  capture_output=True, 
                                  text=True, 
                                  timeout=30)
            compile_time = time.time() - start_time
            
            if result.returncode == 0:
                # 运行编译后的可执行文件
                start_time = time.time()
                run_result = subprocess.run([str(output_file)], 
                                          capture_output=True, 
                                          text=True, 
                                          timeout=10)
                execution_time = time.time() - start_time
                
                # 清理可执行文件
                if output_file.exists():
                    output_file.unlink()
                
                return {
                    'success': run_result.returncode == 0,
                    'output': run_result.stdout,
                    'error': run_result.stderr,
                    'compile_time': compile_time,
                    'execution_time': execution_time,
                    'total_time': compile_time + execution_time
                }
            else:
                return {
                    'success': False,
                    'output': '',
                    'error': result.stderr,
                    'compile_time': compile_time,
                    'execution_time': 0,
                    'total_time': compile_time
                }
        except Exception as e:
            return {
                'success': False,
                'output': '',
                'error': str(e),
                'compile_time': 0,
                'execution_time': 0,
                'total_time': 0
            }
    
    def run_with_interpreter(self, test_file):
        """使用newLISP解释器运行测试文件"""
        if not self.interpreter_path:
            return {
                'success': False,
                'output': '',
                'error': 'Interpreter not available',
                'execution_time': 0
            }
        
        try:
            start_time = time.time()
            result = subprocess.run([self.interpreter_path, str(test_file)], 
                                  capture_output=True, 
                                  text=True, 
                                  timeout=10)
            execution_time = time.time() - start_time
            
            return {
                'success': result.returncode == 0,
                'output': result.stdout,
                'error': result.stderr,
                'execution_time': execution_time
            }
        except Exception as e:
            return {
                'success': False,
                'output': '',
                'error': str(e),
                'execution_time': 0
            }
    
    def compare_outputs(self, compiler_output, interpreter_output):
        """比较编译器和解释器的输出"""
        # 简单的输出比较
        compiler_lines = compiler_output.strip().split('\n')
        interpreter_lines = interpreter_output.strip().split('\n')
        
        # 过滤掉测试标记行
        compiler_content = [line for line in compiler_lines 
                          if not line.startswith('>>>>>')]
        interpreter_content = [line for line in interpreter_lines 
                             if not line.startswith('>>>>>')]
        
        return {
            'identical': compiler_content == interpreter_content,
            'compiler_lines': len(compiler_content),
            'interpreter_lines': len(interpreter_content),
            'difference': abs(len(compiler_content) - len(interpreter_content))
        }
    
    def test_single_file(self, test_file):
        """测试单个文件"""
        print(f"Testing: {test_file}")
        
        # 使用编译器测试
        compiler_result = self.compile_with_our_compiler(test_file)
        
        # 使用解释器测试
        interpreter_result = self.run_with_interpreter(test_file)
        
        # 比较结果
        output_comparison = None
        if compiler_result['success'] and interpreter_result['success']:
            output_comparison = self.compare_outputs(
                compiler_result['output'], 
                interpreter_result['output']
            )
        
        test_result = {
            'test_file': str(test_file),
            'compiler': compiler_result,
            'interpreter': interpreter_result,
            'output_comparison': output_comparison,
            'timestamp': time.time()
        }
        
        self.results['tests'].append(test_result)
        
        # 更新统计
        self.results['summary']['total'] += 1
        if compiler_result['success']:
            self.results['summary']['compiler_success'] += 1
        if interpreter_result['success']:
            self.results['summary']['interpreter_success'] += 1
        if compiler_result['success'] and interpreter_result['success']:
            self.results['summary']['both_success'] += 1
        
        # 记录性能数据
        if compiler_result['success'] and interpreter_result['success']:
            performance_data = {
                'test_file': str(test_file),
                'compiler_total_time': compiler_result['total_time'],
                'interpreter_time': interpreter_result['execution_time'],
                'speedup': interpreter_result['execution_time'] / compiler_result['total_time'] if compiler_result['total_time'] > 0 else 0
            }
            self.results['summary']['performance_data'].append(performance_data)
        
        return test_result
    
    def test_category(self, category):
        """测试特定类别的所有函数"""
        category_dir = Path(f"basic/{category}")
        if not category_dir.exists():
            print(f"Category directory not found: {category_dir}")
            return
        
        print(f"\n=== Testing {category} category ===")
        
        # 测试类别套件
        suite_file = Path(f"basic/{category}_suite.lsp")
        if suite_file.exists():
            self.test_single_file(suite_file)
        
        # 测试单个函数
        for test_file in category_dir.glob("test_*.lsp"):
            self.test_single_file(test_file)
    
    def test_all_supported(self):
        """测试所有支持的函数"""
        with open("function_checklist.json", 'r') as f:
            checklist = json.load(f)
        
        print("=== Testing All Supported Functions ===")
        
        for func_name, func_info in checklist['functions'].items():
            if func_info['supported']:
                category = func_info['category']
                safe_name = func_name.replace('+', 'plus').replace('-', 'minus').replace('*', 'multiply').replace('/', 'divide')
                test_file = Path(f"basic/{category}/test_{safe_name}.lsp")
                
                if test_file.exists():
                    self.test_single_file(test_file)
    
    def generate_report(self, output_file="comparison_report.json"):
        """生成测试报告"""
        # 计算平均性能
        if self.results['summary']['performance_data']:
            total_speedup = sum(p['speedup'] for p in self.results['summary']['performance_data'])
            avg_speedup = total_speedup / len(self.results['summary']['performance_data'])
            self.results['summary']['average_speedup'] = avg_speedup
        
        # 保存详细报告
        with open(output_file, 'w', encoding='utf-8') as f:
            json.dump(self.results, f, indent=2, ensure_ascii=False)
        
        # 打印摘要
        summary = self.results['summary']
        print(f"\n=== Test Summary ===")
        print(f"Total tests: {summary['total']}")
        print(f"Compiler success: {summary['compiler_success']}")
        print(f"Interpreter success: {summary['interpreter_success']}")
        print(f"Both success: {summary['both_success']}")
        
        if 'average_speedup' in summary:
            print(f"Average speedup: {summary['average_speedup']:.2f}x")
        
        print(f"Detailed report saved to: {output_file}")

def main():
    framework = ComparisonFramework()
    
    # 测试所有支持的函数
    framework.test_all_supported()
    
    # 生成报告
    framework.generate_report()

if __name__ == "__main__":
    main()
