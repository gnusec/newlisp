# newLISP Binary Compiler Project
# newLISP二进制编译器项目

## 项目概述 / Project Overview

本项目旨在为newLISP语言实现真正的二进制编译能力，将动态解释执行转换为静态机器码执行，显著提升性能并增强部署灵活性。

This project aims to implement true binary compilation capabilities for the newLISP language, converting dynamic interpreted execution to static machine code execution, significantly improving performance and enhancing deployment flexibility.

## 核心目标 / Core Objectives

- **性能提升**: 2-10倍执行速度提升 / 2-10x execution speed improvement
- **独立部署**: 生成无依赖的可执行文件 / Generate dependency-free executables  
- **反分析增强**: 提高代码保护能力 / Enhanced code protection capabilities
- **渐进式实施**: 保持向后兼容性 / Maintain backward compatibility

## 项目结构 / Project Structure

```
binary_compiler_project/
├── docs/                    # 技术文档 / Technical Documentation
│   ├── technical_comparison.md
│   ├── cost_benefit_analysis.md
│   ├── implementation_roadmap.md
│   ├── core_principles.md
│   ├── tinycc_integration_plan.md
│   ├── tinycc_architecture.md
│   ├── minimal_language_changes.md
│   ├── lightning_jit_plan.md
│   ├── llvm_backend_plan.md
│   └── hybrid_approach_plan.md
├── src/                     # 源代码 / Source Code
│   ├── compiler/           # 编译器核心
│   ├── runtime/            # 运行时支持
│   ├── ast_converter/      # AST转换器
│   └── tinycc_integration/ # TinyCC集成
├── examples/               # 示例代码 / Example Code
│   ├── basic/             # 基础示例
│   ├── advanced/          # 高级示例
│   └── benchmarks/        # 性能测试
├── tests/                  # 测试用例 / Test Cases
│   ├── unit/              # 单元测试
│   ├── integration/       # 集成测试
│   └── performance/       # 性能测试
└── tools/                  # 辅助工具 / Auxiliary Tools
    ├── build_scripts/     # 构建脚本
    ├── analyzers/         # 分析工具
    └── generators/        # 代码生成器
```

## 快速开始 / Quick Start

1. **环境准备 / Environment Setup**
   ```bash
   # 安装TinyCC / Install TinyCC
   sudo apt-get install tcc
   
   # 克隆项目 / Clone project
   git clone <repository-url>
   cd binary_compiler_project
   ```

2. **构建编译器 / Build Compiler**
   ```bash
   make build-compiler
   ```

3. **编译示例 / Compile Example**
   ```bash
   ./tools/nlcc examples/basic/hello.lsp -o hello
   ./hello
   ```

## 开发阶段 / Development Phases

- **阶段1 (2周)**: TinyCC基础集成 / TinyCC Basic Integration
- **阶段2 (2周)**: 类型系统映射 / Type System Mapping  
- **阶段3 (2周)**: 优化与测试 / Optimization & Testing

## 贡献指南 / Contributing

请参阅 `docs/` 目录下的详细技术文档。

Please refer to the detailed technical documentation in the `docs/` directory.

## 许可证 / License

本项目遵循与newLISP相同的许可证。

This project follows the same license as newLISP.
