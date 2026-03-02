# git clone 太慢

## 问题描述
GitHub Actions 拉取代码非常慢

## 原因分析
fetch-depth: 0 会下载完整 git 历史，内核仓库很大所以很慢

## 解决方案
使用 fetch-depth: 1 浅克隆，只下载最新代码

## 修复文件
- `.github/workflows/build-kernel.yml` - 将 fetch-depth: 0 改为 fetch-depth: 1
