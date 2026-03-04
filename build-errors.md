# Build Errors History

## 2026-03-04 18:00 - Run 22663988084

**错误**: `incomplete definition of type 'struct rcu_tasks'`, `use of undeclared identifier 'RTGS_PRE_WAIT_GP'`

**原因**: CONFIG_TASKS_RCU 未在配置中显式设置，导致 struct rcu_tasks 完整定义被条件编译跳过，但 call_rcu_tasks_generic 函数仍在编译

**修复**: 在配置步骤中添加 `sed -i 's/# CONFIG_TASKS_RCU is not set/CONFIG_TASKS_RCU=y/' .config` 显式启用 CONFIG_TASKS_RCU

---

## 2026-03-04 17:50 - Run 22663782809

**错误**: `unknown type name 'compat_uptr_t'`, `incomplete definition of type 'struct rcu_tasks'`

**原因**: CONFIG_COMPAT 未正确设置，导致 compat_uptr_t 类型未定义

**修复**: 在配置步骤中添加 `sed -i 's/CONFIG_COMPAT=y/# CONFIG_COMPAT is not set/' .config` 禁用 CONFIG_COMPAT

---

## 2026-03-04 17:45 - Run 22663635661

**错误**: `aarch64-linux-android33-objcopy: not found`, `unknown type name 'compat_uptr_t'`

**原因**: NDK 工具链缺少 objcopy，编译 vdso 时需要

**修复**: 添加符号链接 aarch64-linux-android33-objcopy 指向 /usr/bin/aarch64-linux-gnu-objcopy

---

## 2026-03-04 17:40 - Run 22663250408

**错误**: `aarch64-linux-android33-objdump: not found`, `aarch64-linux-android33-nm: not found`, `aarch64-linux-android33-ar: not found`, `unknown type name 'compat_uptr_t'`

**原因**: NDK 工具链缺少 objdump、nm、ar 等工具，需要使用系统交叉编译工具链

**修复**: 添加符号链接 aarch64-linux-android33-objdump/nm/ar/strip 指向 /usr/bin/aarch64-linux-gnu-*

---

## 2026-03-04 17:30 - Run 22663014030

**错误**: `linker 'aarch64-linux-android33-ld' not found`

**原因**: NDK 没有独立的 ld，需要使用系统交叉编译工具链的 ld

**修复**: 添加符号链接 aarch64-linux-android33-ld 指向 /usr/bin/aarch64-linux-gnu-ld

---

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
