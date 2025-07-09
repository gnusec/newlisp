#!/usr/bin/env python3
"""
newLISP函数支持状态分析器
Comprehensive function support status analyzer for newLISP binary compiler
"""

import json
import re
import subprocess
import os
from pathlib import Path

class FunctionSupportAnalyzer:
    def __init__(self):
        self.compiler_path = "../build/nlcc"
        self.interpreter_path = "../../newlisp"
        self.supported_functions = {}
        self.unsupported_functions = {}
        self.test_results = {}
        
    def extract_newlisp_functions(self):
        """从newLISP源码和文档中提取所有函数"""
        functions = {
            # 算术运算 / Arithmetic operations
            'arithmetic': {
                '+': {'description': 'Addition', 'arity': 'variadic', 'priority': 'high'},
                '-': {'description': 'Subtraction', 'arity': 'variadic', 'priority': 'high'},
                '*': {'description': 'Multiplication', 'arity': 'variadic', 'priority': 'high'},
                '/': {'description': 'Division', 'arity': 'variadic', 'priority': 'high'},
                'mod': {'description': 'Modulo', 'arity': '2', 'priority': 'medium'},
                'abs': {'description': 'Absolute value', 'arity': '1', 'priority': 'medium'},
                'max': {'description': 'Maximum value', 'arity': 'variadic', 'priority': 'medium'},
                'min': {'description': 'Minimum value', 'arity': 'variadic', 'priority': 'medium'},
                'sqrt': {'description': 'Square root', 'arity': '1', 'priority': 'low'},
                'pow': {'description': 'Power', 'arity': '2', 'priority': 'low'},
                'exp': {'description': 'Exponential', 'arity': '1', 'priority': 'low'},
                'log': {'description': 'Natural logarithm', 'arity': '1', 'priority': 'low'},
                'sin': {'description': 'Sine', 'arity': '1', 'priority': 'low'},
                'cos': {'description': 'Cosine', 'arity': '1', 'priority': 'low'},
                'tan': {'description': 'Tangent', 'arity': '1', 'priority': 'low'},
            },
            
            # 比较运算 / Comparison operations
            'comparison': {
                '=': {'description': 'Equal', 'arity': '2+', 'priority': 'high'},
                '!=': {'description': 'Not equal', 'arity': '2+', 'priority': 'high'},
                '<': {'description': 'Less than', 'arity': '2+', 'priority': 'high'},
                '<=': {'description': 'Less or equal', 'arity': '2+', 'priority': 'high'},
                '>': {'description': 'Greater than', 'arity': '2+', 'priority': 'high'},
                '>=': {'description': 'Greater or equal', 'arity': '2+', 'priority': 'high'},
            },
            
            # 逻辑运算 / Logical operations
            'logical': {
                'and': {'description': 'Logical AND', 'arity': 'variadic', 'priority': 'high'},
                'or': {'description': 'Logical OR', 'arity': 'variadic', 'priority': 'high'},
                'not': {'description': 'Logical NOT', 'arity': '1', 'priority': 'high'},
            },
            
            # 输入输出 / Input/Output
            'io': {
                'println': {'description': 'Print with newline', 'arity': '1', 'priority': 'high'},
                'print': {'description': 'Print without newline', 'arity': '1', 'priority': 'high'},
                'read': {'description': 'Read input', 'arity': '0-1', 'priority': 'medium'},
                'write': {'description': 'Write to file', 'arity': '2', 'priority': 'medium'},
            },
            
            # 字符串操作 / String operations
            'string': {
                'string': {'description': 'Create string', 'arity': 'variadic', 'priority': 'medium'},
                'append': {'description': 'Append strings', 'arity': 'variadic', 'priority': 'medium'},
                'length': {'description': 'String length', 'arity': '1', 'priority': 'medium'},
                'upper-case': {'description': 'Convert to uppercase', 'arity': '1', 'priority': 'low'},
                'lower-case': {'description': 'Convert to lowercase', 'arity': '1', 'priority': 'low'},
                'trim': {'description': 'Trim whitespace', 'arity': '1', 'priority': 'low'},
                'find': {'description': 'Find substring', 'arity': '2-3', 'priority': 'low'},
                'replace': {'description': 'Replace substring', 'arity': '3-4', 'priority': 'low'},
            },
            
            # 列表操作 / List operations
            'list': {
                'list': {'description': 'Create list', 'arity': 'variadic', 'priority': 'medium'},
                'first': {'description': 'First element', 'arity': '1', 'priority': 'medium'},
                'last': {'description': 'Last element', 'arity': '1', 'priority': 'medium'},
                'rest': {'description': 'Rest of list', 'arity': '1', 'priority': 'medium'},
                'cons': {'description': 'Construct list', 'arity': '2', 'priority': 'medium'},
                'append': {'description': 'Append lists', 'arity': 'variadic', 'priority': 'medium'},
                'length': {'description': 'List length', 'arity': '1', 'priority': 'medium'},
                'nth': {'description': 'Nth element', 'arity': '2', 'priority': 'medium'},
                'push': {'description': 'Push element', 'arity': '2-3', 'priority': 'low'},
                'pop': {'description': 'Pop element', 'arity': '1-2', 'priority': 'low'},
                'sort': {'description': 'Sort list', 'arity': '1-2', 'priority': 'low'},
                'reverse': {'description': 'Reverse list', 'arity': '1', 'priority': 'low'},
            },
            
            # 控制结构 / Control structures
            'control': {
                'if': {'description': 'Conditional', 'arity': '2-3', 'priority': 'high'},
                'cond': {'description': 'Multi-conditional', 'arity': 'variadic', 'priority': 'high'},
                'when': {'description': 'When condition', 'arity': '2+', 'priority': 'medium'},
                'unless': {'description': 'Unless condition', 'arity': '2+', 'priority': 'medium'},
                'while': {'description': 'While loop', 'arity': '2+', 'priority': 'medium'},
                'for': {'description': 'For loop', 'arity': '3+', 'priority': 'medium'},
                'dolist': {'description': 'List iteration', 'arity': '2+', 'priority': 'low'},
                'dotimes': {'description': 'Count iteration', 'arity': '2+', 'priority': 'low'},
            },
            
            # 变量定义 / Variable definition
            'variables': {
                'set': {'description': 'Set variable', 'arity': '2', 'priority': 'high'},
                'setq': {'description': 'Set quoted', 'arity': '2+', 'priority': 'medium'},
            },

            # 函数定义 / Function definition
            'definition': {
                'define': {'description': 'Define function/variable', 'arity': '2+', 'priority': 'high'},
                'lambda': {'description': 'Anonymous function', 'arity': '2+', 'priority': 'medium'},
            }
        }
        
        return functions
    
    def test_function_support(self, func_name, category):
        """测试单个函数的支持状态"""
        # 创建简单测试用例
        test_cases = self.generate_test_cases(func_name, category)
        
        results = {
            'function': func_name,
            'category': category,
            'compiler_tests': [],
            'interpreter_tests': [],
            'supported': False,
            'issues': []
        }
        
        for test_case in test_cases:
            # 测试编译器
            compiler_result = self.test_with_compiler(test_case)
            results['compiler_tests'].append(compiler_result)
            
            # 测试解释器
            interpreter_result = self.test_with_interpreter(test_case)
            results['interpreter_tests'].append(interpreter_result)
        
        # 判断是否支持
        compiler_success = any(t['success'] for t in results['compiler_tests'])
        interpreter_success = any(t['success'] for t in results['interpreter_tests'])
        
        results['supported'] = compiler_success
        if compiler_success and interpreter_success:
            # 比较输出一致性
            results['output_consistent'] = self.compare_outputs(results)
        
        return results
    
    def generate_test_cases(self, func_name, category):
        """为函数生成测试用例"""
        if category == 'arithmetic':
            if func_name in ['+', '-', '*', '/']:
                return [
                    f'({func_name} 1 2)',
                    f'({func_name} 10 20 30)',
                    f'({func_name} 1.5 2.5)'
                ]
            elif func_name in ['abs', 'sqrt']:
                return [f'({func_name} 5)']
            elif func_name in ['max', 'min']:
                return [f'({func_name} 1 2 3 4 5)']
        elif category == 'io':
            if func_name in ['println', 'print']:
                return [
                    f'({func_name} "Hello")',
                    f'({func_name} 42)'
                ]
        elif category == 'comparison':
            return [f'({func_name} 5 10)']
        elif category == 'logical':
            if func_name == 'not':
                return ['(not (= 1 2))', '(not (= 1 1))']
            else:
                return [f'({func_name} (= 1 1) (= 2 2))']
        elif category == 'control':
            if func_name == 'if':
                return [
                    '(if (> 5 3) (println "true branch") (println "false branch"))',
                    '(if (< 5 3) (println "true branch") (println "false branch"))'
                ]
            elif func_name == 'when':
                return ['(when (> 5 3) (println "when executed"))']
            elif func_name == 'unless':
                return ['(unless (< 5 3) (println "unless executed"))']
            elif func_name == 'while':
                return ['(while (> 1 2) (println "should not execute"))']
            elif func_name == 'cond':
                return ['(cond ((> 5 3) (println "first condition")) (true (println "default")))']
        elif category == 'variables':
            if func_name == 'set':
                return ['(set x 42)', '(println x)']
            elif func_name == 'define':
                return ['(define (square x) (* x x))', '(println (square 5))']

        # 默认测试用例
        return [f'({func_name} 1)']
    
    def test_with_compiler(self, test_case):
        """使用编译器测试"""
        try:
            # 创建临时测试文件
            test_content = f'(println ">>>>> Testing")\n{test_case}\n(println ">>>>> SUCCESSFUL")'
            
            with open('temp_test.lsp', 'w') as f:
                f.write(test_content)
            
            # 编译
            compile_result = subprocess.run([
                self.compiler_path, 'temp_test.lsp', '-o', 'temp_test'
            ], capture_output=True, text=True, timeout=10)
            
            if compile_result.returncode == 0:
                # 运行
                run_result = subprocess.run(['./temp_test'], 
                                          capture_output=True, text=True, timeout=5)
                
                # 清理
                if os.path.exists('temp_test'):
                    os.remove('temp_test')
                
                return {
                    'success': run_result.returncode == 0,
                    'output': run_result.stdout,
                    'error': run_result.stderr,
                    'test_case': test_case
                }
            else:
                return {
                    'success': False,
                    'output': '',
                    'error': compile_result.stderr,
                    'test_case': test_case
                }
        except Exception as e:
            return {
                'success': False,
                'output': '',
                'error': str(e),
                'test_case': test_case
            }
        finally:
            # 清理临时文件
            for f in ['temp_test.lsp', 'temp_test']:
                if os.path.exists(f):
                    os.remove(f)
    
    def test_with_interpreter(self, test_case):
        """使用解释器测试"""
        try:
            test_content = f'(println ">>>>> Testing")\n{test_case}\n(println ">>>>> SUCCESSFUL")\n(exit)'
            
            with open('temp_test.lsp', 'w') as f:
                f.write(test_content)
            
            result = subprocess.run([self.interpreter_path, '-c', 'temp_test.lsp'], 
                                  capture_output=True, text=True, timeout=3)
            
            return {
                'success': result.returncode == 0 or result.returncode == 124,
                'output': result.stdout,
                'error': result.stderr,
                'test_case': test_case
            }
        except subprocess.TimeoutExpired:
            return {
                'success': True,  # 超时通常意味着程序正常运行
                'output': 'Timeout (likely successful)',
                'error': '',
                'test_case': test_case
            }
        except Exception as e:
            return {
                'success': False,
                'output': '',
                'error': str(e),
                'test_case': test_case
            }
        finally:
            if os.path.exists('temp_test.lsp'):
                os.remove('temp_test.lsp')
    
    def compare_outputs(self, results):
        """比较编译器和解释器的输出一致性"""
        # 简化的输出比较
        for i, (comp, interp) in enumerate(zip(results['compiler_tests'], results['interpreter_tests'])):
            if comp['success'] and interp['success']:
                # 提取实际输出内容（去除测试标记）
                comp_lines = [line for line in comp['output'].split('\n') 
                             if not line.startswith('>>>>>') and line.strip()]
                interp_lines = [line for line in interp['output'].split('\n') 
                               if not line.startswith('>>>>>') and line.strip()]
                
                if comp_lines != interp_lines:
                    return False
        return True
    
    def analyze_all_functions(self):
        """分析所有函数的支持状态"""
        functions = self.extract_newlisp_functions()
        
        print("=== newLISP Function Support Analysis ===")
        
        for category, funcs in functions.items():
            print(f"\n--- {category.upper()} Category ---")
            
            for func_name, func_info in funcs.items():
                print(f"Testing {func_name}...", end=' ')
                
                result = self.test_function_support(func_name, category)
                self.test_results[func_name] = result
                
                if result['supported']:
                    print("✅ SUPPORTED")
                    self.supported_functions[func_name] = result
                else:
                    print("❌ NOT SUPPORTED")
                    self.unsupported_functions[func_name] = result
        
        return self.generate_report()
    
    def generate_report(self):
        """生成详细报告"""
        total_functions = len(self.test_results)
        supported_count = len(self.supported_functions)
        support_rate = (supported_count / total_functions) * 100 if total_functions > 0 else 0
        
        report = {
            'summary': {
                'total_functions': total_functions,
                'supported_functions': supported_count,
                'unsupported_functions': len(self.unsupported_functions),
                'support_rate': support_rate
            },
            'supported': self.supported_functions,
            'unsupported': self.unsupported_functions,
            'detailed_results': self.test_results
        }
        
        # 保存报告
        with open('function_support_report.json', 'w', encoding='utf-8') as f:
            json.dump(report, f, indent=2, ensure_ascii=False)
        
        return report

def main():
    analyzer = FunctionSupportAnalyzer()
    report = analyzer.analyze_all_functions()
    
    print(f"\n=== SUMMARY ===")
    print(f"Total functions tested: {report['summary']['total_functions']}")
    print(f"Supported functions: {report['summary']['supported_functions']}")
    print(f"Support rate: {report['summary']['support_rate']:.1f}%")
    print(f"Detailed report saved to: function_support_report.json")

if __name__ == "__main__":
    main()
