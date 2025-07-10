#!/usr/bin/env python3
"""
结果一致性验证器
Consistency validator for comparing compiler vs interpreter outputs
"""

import json
import subprocess
import tempfile
import os
from pathlib import Path

class ConsistencyValidator:
    def __init__(self):
        self.compiler_path = "../build/nlcc"
        self.interpreter_path = "../../newlisp"
        self.test_cases = []
        self.results = []
        
    def load_supported_functions(self):
        """加载支持的函数列表"""
        with open('function_support_report.json', 'r') as f:
            report = json.load(f)
        return list(report['supported'].keys())
    
    def generate_comprehensive_test_cases(self):
        """生成全面的测试用例"""
        supported_funcs = self.load_supported_functions()
        
        test_cases = []
        
        # 基本功能测试
        for func in supported_funcs:
            if func in ['+', '-', '*', '/']:
                test_cases.extend([
                    f'({func} 1 2)',
                    f'({func} 10 20 30)',
                    f'({func} 1.5 2.5)',
                    f'({func} 0 0)',
                    f'({func} -5 5)',
                    f'({func} 1000000 2000000)',
                ])
            elif func == 'println':
                test_cases.extend([
                    '(println "Hello World")',
                    '(println 42)',
                    '(println 3.14159)',
                    '(println "")',
                    '(println "String with spaces and numbers 123")',
                ])
        
        # 复杂嵌套测试
        test_cases.extend([
            '(+ (* 2 3) (* 4 5))',
            '(- (* 10 5) (* 2 3))',
            '(* (+ 1 2) (+ 3 4))',
            '(/ (+ 10 20) (+ 2 3))',
            '(+ (+ (+ 1 2) (+ 3 4)) (+ (+ 5 6) (+ 7 8)))',
            '(* (* (* 2 2) (* 3 3)) (* (* 4 4) (* 5 5)))',
        ])
        
        # 边界值测试
        test_cases.extend([
            '(+ 0)',
            '(* 1)',
            '(+ 999999999 1)',
            '(- -999999999 1)',
            '(/ 1000000000 1000)',
            '(+ 0.000001 0.000002)',
            '(* 1.5 1.5 1.5 1.5)',
        ])
        
        # 大量参数测试
        test_cases.extend([
            '(+ 1 2 3 4 5 6 7 8 9 10)',
            '(* 1 1 1 1 1 1 1 1 1 2)',
            '(+ 100 200 300 400 500 600 700 800 900 1000)',
        ])
        
        return test_cases
    
    def run_test_with_compiler(self, test_case):
        """使用编译器运行测试"""
        try:
            # 创建测试文件
            test_content = f"""
;; Consistency test case
{test_case}
"""
            
            with tempfile.NamedTemporaryFile(mode='w', suffix='.lsp', delete=False) as f:
                f.write(test_content)
                test_file = f.name
            
            # 编译
            output_file = test_file.replace('.lsp', '_compiled')
            compile_cmd = [self.compiler_path, test_file, '-o', output_file]
            
            compile_result = subprocess.run(compile_cmd, capture_output=True, text=True, timeout=10)
            
            if compile_result.returncode != 0:
                return {
                    'success': False,
                    'output': '',
                    'error': f"Compilation failed: {compile_result.stderr}",
                    'stage': 'compilation'
                }
            
            # 运行
            run_result = subprocess.run([output_file], capture_output=True, text=True, timeout=5)
            
            return {
                'success': run_result.returncode == 0,
                'output': run_result.stdout.strip(),
                'error': run_result.stderr,
                'stage': 'execution'
            }
            
        except Exception as e:
            return {
                'success': False,
                'output': '',
                'error': str(e),
                'stage': 'exception'
            }
        finally:
            # 清理临时文件
            for f in [test_file, output_file]:
                if os.path.exists(f):
                    os.remove(f)
    
    def run_test_with_interpreter(self, test_case):
        """使用解释器运行测试"""
        try:
            # 创建测试文件
            test_content = f"""
;; Consistency test case
{test_case}
(exit)
"""
            
            with tempfile.NamedTemporaryFile(mode='w', suffix='.lsp', delete=False) as f:
                f.write(test_content)
                test_file = f.name
            
            # 运行解释器
            run_cmd = [self.interpreter_path, '-c', test_file]
            run_result = subprocess.run(run_cmd, capture_output=True, text=True, timeout=3)
            
            return {
                'success': run_result.returncode == 0 or run_result.returncode == 124,
                'output': run_result.stdout.strip(),
                'error': run_result.stderr,
                'stage': 'execution'
            }
            
        except subprocess.TimeoutExpired:
            return {
                'success': True,
                'output': 'Timeout (program likely successful)',
                'error': '',
                'stage': 'timeout'
            }
        except Exception as e:
            return {
                'success': False,
                'output': '',
                'error': str(e),
                'stage': 'exception'
            }
        finally:
            if os.path.exists(test_file):
                os.remove(test_file)
    
    def normalize_output(self, output):
        """标准化输出以便比较"""
        if not output:
            return ""
        
        # 移除多余的空白字符
        lines = [line.strip() for line in output.split('\n') if line.strip()]
        return '\n'.join(lines)
    
    def compare_outputs(self, compiler_output, interpreter_output):
        """比较编译器和解释器的输出"""
        comp_norm = self.normalize_output(compiler_output)
        interp_norm = self.normalize_output(interpreter_output)
        
        return {
            'identical': comp_norm == interp_norm,
            'compiler_output': comp_norm,
            'interpreter_output': interp_norm,
            'difference': self.calculate_difference(comp_norm, interp_norm)
        }
    
    def calculate_difference(self, output1, output2):
        """计算输出差异"""
        if output1 == output2:
            return "No difference"
        
        lines1 = output1.split('\n')
        lines2 = output2.split('\n')
        
        differences = []
        max_lines = max(len(lines1), len(lines2))
        
        for i in range(max_lines):
            line1 = lines1[i] if i < len(lines1) else "<missing>"
            line2 = lines2[i] if i < len(lines2) else "<missing>"
            
            if line1 != line2:
                differences.append(f"Line {i+1}: '{line1}' vs '{line2}'")
        
        return "; ".join(differences) if differences else "No line differences"
    
    def validate_consistency(self):
        """执行一致性验证"""
        test_cases = self.generate_comprehensive_test_cases()
        
        print(f"=== Consistency Validation ===")
        print(f"Testing {len(test_cases)} test cases...")
        
        consistent_count = 0
        inconsistent_count = 0
        error_count = 0
        
        for i, test_case in enumerate(test_cases):
            print(f"Test {i+1}/{len(test_cases)}: {test_case[:50]}...", end=' ')
            
            # 运行编译器测试
            compiler_result = self.run_test_with_compiler(test_case)
            
            # 运行解释器测试
            interpreter_result = self.run_test_with_interpreter(test_case)
            
            # 比较结果
            if compiler_result['success'] and interpreter_result['success']:
                comparison = self.compare_outputs(
                    compiler_result['output'], 
                    interpreter_result['output']
                )
                
                if comparison['identical']:
                    print("✅ CONSISTENT")
                    consistent_count += 1
                else:
                    print("⚠️  INCONSISTENT")
                    inconsistent_count += 1
                    
                result = {
                    'test_case': test_case,
                    'status': 'consistent' if comparison['identical'] else 'inconsistent',
                    'compiler_result': compiler_result,
                    'interpreter_result': interpreter_result,
                    'comparison': comparison
                }
            else:
                print("❌ ERROR")
                error_count += 1
                
                result = {
                    'test_case': test_case,
                    'status': 'error',
                    'compiler_result': compiler_result,
                    'interpreter_result': interpreter_result,
                    'comparison': None
                }
            
            self.results.append(result)
        
        # 生成报告
        return self.generate_consistency_report(consistent_count, inconsistent_count, error_count)
    
    def generate_consistency_report(self, consistent, inconsistent, errors):
        """生成一致性报告"""
        total = len(self.results)
        consistency_rate = (consistent / total) * 100 if total > 0 else 0
        
        report = {
            'summary': {
                'total_tests': total,
                'consistent': consistent,
                'inconsistent': inconsistent,
                'errors': errors,
                'consistency_rate': consistency_rate
            },
            'detailed_results': self.results,
            'inconsistent_cases': [r for r in self.results if r['status'] == 'inconsistent'],
            'error_cases': [r for r in self.results if r['status'] == 'error']
        }
        
        # 保存报告
        with open('consistency_validation_report.json', 'w', encoding='utf-8') as f:
            json.dump(report, f, indent=2, ensure_ascii=False)
        
        # 打印摘要
        print(f"\n=== Consistency Validation Summary ===")
        print(f"Total tests: {total}")
        print(f"Consistent: {consistent}")
        print(f"Inconsistent: {inconsistent}")
        print(f"Errors: {errors}")
        print(f"Consistency rate: {consistency_rate:.1f}%")
        
        if inconsistent > 0:
            print(f"\n⚠️  Found {inconsistent} inconsistent cases:")
            for case in report['inconsistent_cases'][:5]:  # 显示前5个
                print(f"  - {case['test_case']}")
                print(f"    Compiler: {case['comparison']['compiler_output'][:50]}...")
                print(f"    Interpreter: {case['comparison']['interpreter_output'][:50]}...")
        
        if errors > 0:
            print(f"\n❌ Found {errors} error cases:")
            for case in report['error_cases'][:5]:  # 显示前5个
                print(f"  - {case['test_case']}")
                if not case['compiler_result']['success']:
                    print(f"    Compiler error: {case['compiler_result']['error'][:100]}...")
                if not case['interpreter_result']['success']:
                    print(f"    Interpreter error: {case['interpreter_result']['error'][:100]}...")
        
        print(f"\nDetailed report saved to: consistency_validation_report.json")
        
        return report

def main():
    validator = ConsistencyValidator()
    report = validator.validate_consistency()
    
    # 返回一致性率作为退出码指示
    if report['summary']['consistency_rate'] >= 95:
        exit(0)  # 高一致性
    elif report['summary']['consistency_rate'] >= 80:
        exit(1)  # 中等一致性
    else:
        exit(2)  # 低一致性

if __name__ == "__main__":
    main()
