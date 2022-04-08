# SysYCompiler

### 项目结构

- doc：文档
- include：头文件
- lib：官方提供的外部 Sysy 语言库
- src：源代码
  - frontend：前端部分
  - ir：中间表示
  - opt：优化部分
  - backend：后端部分
  - comm：公共部分
- test：测试
  - \*_test.cc：单元测试
  - \*_tool.cc：针对单一文件进行测试的 driver
  - test_\*.sh：批量测试的脚本
- testcase：测试用例
  - example：用于验证的样例
  - function_test：功能测试用例
    - sy：源文件
    - in：输入
    - out：输出
  - performance_test：性能测试用例

## 项目环境

### 操作系统

Ubuntu 20.04

### 构建

- CMake (>= 3.10)，make
- clang，clang++
- flex，bison

### 其他依赖

- qemu-arm
- clang-format (>= 14.0)，非必要
- clang-tidy (>= 14.0)，非必要

