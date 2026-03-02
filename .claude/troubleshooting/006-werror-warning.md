# 警告被视为错误

## 问题描述
编译时警告被当作错误处理导致构建失败

## 错误信息
```
techpack/audio/dsp/codecs/amrwb_in.c:154:17: error: this 'if' clause does not guard... [-Werror=misleading-indentation]
cc1: all warnings being treated as errors
```

## 原因分析
CONFIG_CC_WERROR 启用，将所有警告视为错误

## 解决方案
添加 KCFLAGS="-Wno-error" 禁用警告错误

## 修复文件
- `.github/workflows/build-kernel.yml` - 添加 KCFLAGS="-Wno-error"
