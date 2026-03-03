# 内核构建问题记录

## 问题汇总

### 001: YAML 语法错误
- **原因**: heredoc 中嵌套的 Python 代码导致 YAML 解析失败
- **解决**: 使用 `printf` 分行写入脚本

### 002: 编译器未找到
- **原因**: Clang 17 自带交叉编译器路径不同，Kconfig 需要 GCC 风格工具链
- **解决**: 安装 gcc-aarch64-linux-gnu，使用 aarch64-linux-gnu- 作为 CROSS_COMPILE

### 003: defconfig 路径错误
- **原因**: kona_defconfig 位于 vendor 子目录
- **解决**: 使用完整路径 vendor/kona_defconfig

### 004: lockdep.c 编译错误
- **原因**: nested 参数已从 lock_release() 移除但调用处未更新
- **解决**: 修改 kernel/locking/lockdep.c 第 4008 行，将 nested 改为 0

### 005: trace.h 文件未找到
- **原因**: kona_defconfig 缺少必要的 trace 头文件配置
- **解决**: 使用 gki_defconfig 作为基础配置（包含所有必需的 trace 配置）

### 006: 警告被视为错误
- **原因**: CONFIG_CC_WERROR 启用
- **解决**: 添加 KCFLAGS="-Wno-error"

### 007: git clone 太慢
- **原因**: fetch-depth: 0 下载完整 git 历史
- **解决**: 使用 fetch-depth: 1 浅克隆

### 008: 部分步骤仍使用 Clang
- **原因**: olddefconfig 步骤仍使用 clang 作为 CC
- **解决**: 统一使用 GCC，移除 Clang 下载步骤
