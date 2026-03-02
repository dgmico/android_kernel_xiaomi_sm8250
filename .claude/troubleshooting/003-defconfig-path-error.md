# defconfig 路径错误

## 问题描述
找不到 kona_defconfig 配置文件

## 错误信息
```
*** Can't find default configuration "arch/arm64/configs/kona_defconfig"!
```

## 原因分析
kona_defconfig 实际位于 vendor 子目录：arch/arm64/configs/vendor/kona_defconfig

## 解决方案
使用完整路径 vendor/kona_defconfig

## 修复文件
- `.github/workflows/build-kernel.yml` - 将 kona_defconfig 改为 vendor/kona_defconfig
