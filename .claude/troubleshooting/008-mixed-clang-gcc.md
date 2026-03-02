# 部分步骤仍使用 Clang

## 问题描述
虽然主要编译步骤改为 GCC，但 defconfig 的 olddefconfig 步骤仍使用 Clang

## 错误信息
```
./include/trace/define_trace.h:89:42: fatal error: ./trace.h: No such file or directory
```

## 原因分析
Setup defconfig 步骤中的 olddefconfig 仍使用 clang 作为 CC

## 解决方案
统一使用 GCC，移除 Clang 下载步骤

## 修复文件
- `.github/workflows/build-kernel.yml` - 移除 Clang 步骤，统一使用 gcc
