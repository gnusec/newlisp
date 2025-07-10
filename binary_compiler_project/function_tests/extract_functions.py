#!/usr/bin/env python3
"""
newLISP函数提取器
从文档中提取所有函数的信息，生成测试清单
Extract newLISP functions from documentation and generate test checklist
"""

import re
import json
import os
from pathlib import Path

class NewLispFunctionExtractor:
    def __init__(self, doc_dir="../doc"):
        self.doc_dir = Path(doc_dir)
        self.functions = {}
        self.categories = {}
        
    def extract_from_manual(self):
        """从手册中提取函数信息"""
        manual_file = self.doc_dir / "newlisp_manual.html"
        if not manual_file.exists():
            print(f"Manual file not found: {manual_file}")
            return
            
        with open(manual_file, 'r', encoding='utf-8') as f:
            content = f.read()
            
        # 查找函数定义模式
        # 匹配类似 <span class="function">function-name</span> 的模式
        function_pattern = r'<span class="function">([^<]+)</span>'
        functions = re.findall(function_pattern, content)
        
        # 查找函数描述
        # 匹配函数名后的描述文本
        for func in functions:
            self.functions[func] = {
                'name': func,
                'category': self.categorize_function(func),
                'description': f'Function: {func}',
                'supported': self.is_supported(func),
                'platform_specific': self.is_platform_specific(func)
            }
            
    def categorize_function(self, func_name):
        """根据函数名称分类"""
        arithmetic_ops = ['+', '-', '*', '/', '%', 'mod', 'abs', 'max', 'min', 
                         'sqrt', 'pow', 'exp', 'log', 'sin', 'cos', 'tan']
        string_ops = ['string', 'append', 'join', 'split', 'trim', 'upper-case', 
                     'lower-case', 'replace', 'find', 'regex']
        list_ops = ['list', 'first', 'last', 'rest', 'cons', 'append', 'length',
                   'nth', 'push', 'pop', 'sort', 'reverse']
        io_ops = ['print', 'println', 'read', 'write', 'open', 'close', 'load', 'save']
        control_ops = ['if', 'cond', 'case', 'when', 'unless', 'while', 'for', 
                      'dolist', 'dotimes', 'begin', 'progn']
        
        if func_name in arithmetic_ops:
            return 'arithmetic'
        elif func_name in string_ops:
            return 'string'
        elif func_name in list_ops:
            return 'list'
        elif func_name in io_ops:
            return 'io'
        elif func_name in control_ops:
            return 'control'
        else:
            return 'other'
            
    def is_supported(self, func_name):
        """检查我们的编译器是否支持该函数"""
        # 目前我们只支持基本的算术运算和输出
        supported_functions = [
            '+', '-', '*', '/', 'println', 'print'
        ]
        return func_name in supported_functions
        
    def is_platform_specific(self, func_name):
        """检查是否为平台特定函数"""
        windows_specific = ['win32-', 'dll-', 'registry-']
        macos_specific = ['objc-', 'cocoa-']
        unix_specific = ['fork', 'pipe', 'signal']
        
        for prefix in windows_specific + macos_specific + unix_specific:
            if func_name.startswith(prefix):
                return True
        return False
        
    def extract_basic_functions(self):
        """提取基本函数列表（手动定义）"""
        basic_functions = {
            # 算术运算 / Arithmetic
            '+': {'category': 'arithmetic', 'supported': True, 'description': 'Addition'},
            '-': {'category': 'arithmetic', 'supported': True, 'description': 'Subtraction'},
            '*': {'category': 'arithmetic', 'supported': True, 'description': 'Multiplication'},
            '/': {'category': 'arithmetic', 'supported': True, 'description': 'Division'},
            'mod': {'category': 'arithmetic', 'supported': False, 'description': 'Modulo'},
            'abs': {'category': 'arithmetic', 'supported': False, 'description': 'Absolute value'},
            'max': {'category': 'arithmetic', 'supported': False, 'description': 'Maximum'},
            'min': {'category': 'arithmetic', 'supported': False, 'description': 'Minimum'},
            'sqrt': {'category': 'arithmetic', 'supported': False, 'description': 'Square root'},
            'pow': {'category': 'arithmetic', 'supported': False, 'description': 'Power'},
            
            # 输出函数 / Output
            'println': {'category': 'io', 'supported': True, 'description': 'Print line'},
            'print': {'category': 'io', 'supported': True, 'description': 'Print'},
            
            # 字符串函数 / String
            'string': {'category': 'string', 'supported': False, 'description': 'String creation'},
            'append': {'category': 'string', 'supported': False, 'description': 'String append'},
            'length': {'category': 'string', 'supported': False, 'description': 'String length'},
            
            # 列表函数 / List
            'list': {'category': 'list', 'supported': False, 'description': 'List creation'},
            'first': {'category': 'list', 'supported': False, 'description': 'First element'},
            'last': {'category': 'list', 'supported': False, 'description': 'Last element'},
            'rest': {'category': 'list', 'supported': False, 'description': 'Rest of list'},
            
            # 控制结构 / Control
            'if': {'category': 'control', 'supported': False, 'description': 'Conditional'},
            'cond': {'category': 'control', 'supported': False, 'description': 'Multi-conditional'},
            'while': {'category': 'control', 'supported': False, 'description': 'While loop'},
            'for': {'category': 'control', 'supported': False, 'description': 'For loop'},
        }
        
        for name, info in basic_functions.items():
            self.functions[name] = {
                'name': name,
                'category': info['category'],
                'supported': info['supported'],
                'description': info['description'],
                'platform_specific': False
            }
    
    def generate_test_checklist(self):
        """生成测试清单"""
        checklist = {
            'total_functions': len(self.functions),
            'supported_functions': len([f for f in self.functions.values() if f['supported']]),
            'categories': {},
            'functions': self.functions
        }
        
        # 按类别统计
        for func in self.functions.values():
            category = func['category']
            if category not in checklist['categories']:
                checklist['categories'][category] = {
                    'total': 0,
                    'supported': 0,
                    'functions': []
                }
            checklist['categories'][category]['total'] += 1
            if func['supported']:
                checklist['categories'][category]['supported'] += 1
            checklist['categories'][category]['functions'].append(func['name'])
            
        return checklist
    
    def save_checklist(self, filename="function_checklist.json"):
        """保存测试清单到文件"""
        checklist = self.generate_test_checklist()
        with open(filename, 'w', encoding='utf-8') as f:
            json.dump(checklist, f, indent=2, ensure_ascii=False)
        print(f"Function checklist saved to: {filename}")
        return checklist

def main():
    extractor = NewLispFunctionExtractor()
    
    # 提取基本函数（手动定义的核心函数）
    extractor.extract_basic_functions()
    
    # 尝试从文档提取更多函数
    try:
        extractor.extract_from_manual()
    except Exception as e:
        print(f"Warning: Could not extract from manual: {e}")
    
    # 生成并保存测试清单
    checklist = extractor.save_checklist("function_checklist.json")
    
    # 打印摘要
    print("\n=== Function Extraction Summary ===")
    print(f"Total functions found: {checklist['total_functions']}")
    print(f"Supported functions: {checklist['supported_functions']}")
    print("\nBy category:")
    for category, info in checklist['categories'].items():
        print(f"  {category}: {info['supported']}/{info['total']} supported")

if __name__ == "__main__":
    main()
