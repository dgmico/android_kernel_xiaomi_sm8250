# YAML 语法错误

## 问题描述
GitHub Actions 工作流文件 YAML 语法错误

## 错误信息
```
Invalid workflow file: .github/workflows/build-kernel.yml#L125 - You have an error in your yaml syntax on line 125
```

## 原因分析
heredoc 中嵌套的 Python 代码导致 YAML 解析失败

## 解决方案
使用 `printf` 分行写入脚本，避免 YAML 解析 heredoc 时的语法问题

## 修复文件
- `.github/workflows/build-kernel.yml` - 将 heredoc 改为 printf 分行写入
