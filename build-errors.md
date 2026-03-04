# Build Errors History

## 2026-03-04 16:32 - Run 22661353805

**错误**: `compiler 'aarch64-linux-gnu-gcc' not found`

**原因**: 删除了交叉编译工具链安装，但 Kconfig oldconfig 需要它

**修复**: 恢复安装 gcc-aarch64-linux-gnu g++-aarch64-linux-gnu，禁用 CONFIG_COMPAT

---

## 2026-03-04 16:29 - Run 22661283680

**错误**: wget 下载工具链失败

**原因**: GitHub 下载超时或网络问题

**修复**: 回退方案 - 禁用 CONFIG_COMPAT 以避免 compat_uptr_t 错误

---

## 2026-03-04 16:23 - Run 22661077320

**错误**: `unknown type name 'compat_uptr_t'`, `invalid use of undefined type 'struct rcu_tasks'`

**原因**: 系统 Clang 版本太新 (16+)，与 Android 4.19 内核不兼容

**修复**: 使用 clang-14 替代默认 clang，设置 `CC=clang-14` (无效)

---

## 2026-03-04 16:20 - Run 22660988670

**错误**: `unknown type name 'compat_uptr_t'`

**原因**: Clang 编译 ARM64 内核时缺少 compat 类型定义

**修复**: 添加 `LLVM_IAS=0` 禁用内联汇编，使用传统汇编模式 (无效)

---

## 2026-03-04 16:17 - Run 22660882476

**错误**: `compiler 'aarch64-linux-gnu-gcc' not found`

**原因**: 设置 CC=clang 后，Kconfig oldconfig 阶段仍需要交叉编译工具链验证配置

**修复**: 同时安装 clang 和交叉编译工具链 `gcc-aarch64-linux-gnu g++-aarch64-linux-gnu`

---

## 2026-03-04 16:14 - Run 22660624928

**错误**: 
- `unknown type name 'compat_uptr_t'`
- `invalid use of undefined type 'struct rcu_tasks'`

**原因**: 使用 GCC 编译，但该内核需要使用 Clang

**修复**: 将编译器从 GCC 改为 Clang，设置 `CC=clang`

---
