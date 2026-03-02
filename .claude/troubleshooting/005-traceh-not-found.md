# trace.h 文件未找到

## 问题描述
编译时找不到 trace.h 文件

## 错误信息
```
./include/trace/define_trace.h:89:10: fatal error: './trace.h' file not found
```

## 原因分析
Clang 与内联汇编或 trace 头文件不兼容

## 解决方案
使用 GCC 替代 Clang 进行编译

## 修复文件
- `.github/workflows/build-kernel.yml` - 移除 Clang，使用系统 GCC
