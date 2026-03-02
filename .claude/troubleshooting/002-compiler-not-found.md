# 编译器未找到

## 问题描述
Kconfig 无法找到交叉编译器

## 错误信息
```
compiler '/home/runner/.../clang+llvm-17.0.6-x86_64-linux-gnu-ubuntu-22.04/bin/aarch64-linux-gnu-gcc' not found
```

## 原因分析
1. Clang 17 自带交叉编译器，但路径命名方式与 GCC 不同
2. Linux kernel Kconfig 需要 GCC 风格工具链名称

## 解决方案
安装 gcc-aarch64-linux-gnu，使用 aarch64-linux-gnu- 作为 CROSS_COMPILE

## 修复文件
- `.github/workflows/build-kernel.yml` - 安装 gcc-aarch64-linux-gnu，使用 GCC 风格 CROSS_COMPILE
