# 内核编译错误记录

## 2026-03-05 - Run 22700329422

**错误**: `E: Unable to locate package lib32ncurses5-dev`

**原因**: Ubuntu 24.04 (noble) 中 `lib32ncurses5-dev` 包已被移除

**修复**: 将 `lib32ncurses5-dev` 改为 `libncurses-dev`

**修改文件**:
- `.github/workflows/build-kernel.yml` - 安装依赖时使用 libncurses-dev 代替 lib32ncurses5-dev

---

## 2026-03-05 03:21 - Run 22700663572

**错误**: `unzip: caution: filename not matched: -D` (exit code 11)

**原因**: `unzip -q ndk.zip -D ~` 命令中 `-D` 被误认为是文件名匹配模式而非选项，`~` 被展开为完整路径

**修复**: 将 `-D ~` 改为 `-d $HOME`，使用 `-d` 选项正确指定解压目标目录

**修改文件**:
- `.github/workflows/build-kernel.yml` - 修复 unzip 命令

---

## 2026-03-05 03:26 - Run 22700752518

**错误**: `compiler '~/toolchain/gcc/bin/aarch64-linux-gnu-gcc' not found`

**原因**: `CROSS_COMPILE=~/toolchain/gcc/...` 中 `~` 在 GitHub Actions 环境变量中没有被正确展开

**修复**: 将 `~` 改为 `$HOME`

**修改文件**:
- `.github/workflows/build-kernel.yml` - CROSS_COMPILE 路径使用 $HOME 代替 ~
