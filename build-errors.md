# Build Errors History

## 2026-03-04 17:25 - Run 22662498461

**错误**: `compiler 'aarch64-linux-android33-gcc' not found`

**原因**: NDK 只提供 clang，没有独立的 gcc 二进制文件

**修复**: 将 aarch64-linux-android33-gcc 符号链接指向 clang，添加 gcc-aarch64-linux-gnu 作为备用

---

## 2026-03-04 16:35 - Run 22661432148

**错误**: `unknown type name 'compat_uptr_t'`, `invalid use of undefined type 'struct rcu_tasks'`

**原因**: Ubuntu 自带 Clang/GCC 与 Android 4.19 内核不兼容，CONFIG_COMPAT=n 无效

**修复**: 使用 Android NDK r26b 提供的 Clang 工具链进行编译

---

## 2026-03-04 16:32 - Run 22661353805

**错误**: `compiler 'aarch64-linux-gnu-gcc' not found`

**原因**: 删除了交叉编译工具链安装，但 Kconfig oldconfig 需要它

**修复**: 恢复安装 gcc-aarch64-linux-gnu g++-aarch64-linux-gnu，禁用 CONFIG_COMPAT

---
