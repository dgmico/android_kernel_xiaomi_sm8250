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

---

## 2026-03-05 03:30 - Run 22700919139

**错误**: `compiler '/home/runner/toolchain/gcc/bin/aarch64-linux-gnu-gcc' not found`

**原因**: LineageOS GCC 工具链实际前缀是 `aarch64-buildroot-linux-gnu-`，不是 `aarch64-linux-gnu-`

**修复**: 将 CROSS_COMPILE 改为 `aarch64-buildroot-linux-gnu-`

**修改文件**:
- `.github/workflows/build-kernel.yml` - CROSS_COMPILE 使用正确的前缀 aarch64-buildroot-linux-gnu-

---

## 2026-03-05 03:37 - Run 22701036451

**错误**: `./include/linux/filter.h:551:2: error: unknown type name 'compat_uptr_t'`

**原因**: `arch/arm64/include/asm/compat.h` 中 `#include <asm-generic/compat.h>` 被放在 `#ifdef CONFIG_COMPAT` 条件块内部，导致某些编译路径下 `compat_uptr_t` 未定义。x86 架构将此 include 放在条件外部。

**修复**: 将 `#include <asm-generic/compat.h>` 移到 `#ifdef CONFIG_COMPAT` 之前，使基本的 compat 类型可以被所有代码访问。

**修改文件**:
- `arch/arm64/include/asm/compat.h` - 将 asm-generic/compat.h 的包含移到 CONFIG_COMPAT 条件之前

---

## 2026-03-05 03:41 - Run 22701144922

**错误**: `kernel/rcu/tasks.h:437:28: error: dereferencing pointer to incomplete type 'struct rcu_tasks'`

**原因**: `CONFIG_TASKS_RCU` 没有在配置文件中显式启用。虽然 Kconfig 定义 `def_bool PREEMPT` 应该自动启用它，但配置合并过程中可能出现问题。此外，`CONFIG_BPF_SYSCALL` 会 select `CONFIG_TASKS_TRACE_RCU`，导致代码使用 `rcu_tasks_trace` 等需要完整结构体定义的功能。

**修复**: 在 sm8250-common.config 中显式启用 `CONFIG_TASKS_RCU=y` 和 `CONFIG_TASKS_RCU_TRACE=y`

**修改文件**:
- `arch/arm64/configs/vendor/xiaomi/sm8250-common.config` - 添加 CONFIG_TASKS_RCU=y 和 CONFIG_TASKS_RCU_TRACE=y

---

## 2026-03-05 06:45 - Run 22705751856

**错误**: `kernel/rcu/tasks.h:437:28: error: dereferencing pointer to incomplete type 'struct rcu_tasks'` (重复错误)

**原因**: 虽然已在 sm8250-common.config 中添加 CONFIG_TASKS_RCU=y，但配置合并可能没有正确工作。`make vendor/xxx.config` 需要先有基础 .config 文件才能正确合并片段。

**修复**: 在工作流中先运行 `make defconfig` 创建基础配置，再运行 `make vendor/xxx.config` 合并片段。

**修改文件**:
- `.github/workflows/build-kernel.yml` - 添加 make defconfig 步骤

---

## 2026-03-05 06:48 - Run 22705910623

**错误**: `./include/linux/mmu_notifier.h:346:33: error: parameter 2 ('event') has incomplete type`

**原因**: `enum mmu_notifier_event` 在 `mmu_notifier_range_init` 函数中被使用，但没有定义该枚举类型。

**修复**: 在 mmu_notifier.h 中添加 `enum mmu_notifier_event` 的定义，包含 MMU_NOTIFY_UNMAP, MMU_NOTIFY_CLEAR 等枚举值。

**修改文件**:
- `include/linux/mmu_notifier.h` - 添加 enum mmu_notifier_event 定义

---

## 2026-03-05 07:00 - Run 22706008608

**错误**:
- `virt/kvm/kvm_main.c:413: error: too few arguments to function 'kvm_unmap_hva_range'`
- `virt/kvm/kvm_main.c:1517,1532: error: too many arguments to function 'follow_pte_pmd'`
- `arch/arm64/kernel/cpu-reset.h:19: error: conflicting types for 'cpu_soft_restart'`

**原因**:
1. `kvm_unmap_hva_range` 需要 4 个参数，但只传了 3 个，缺少 `blockable` 参数
2. `follow_pte_pmd` API 变化：旧版 7 参数调用需要改为 6 参数
3. `cpu_soft_restart` 在 `proc-fns.h` 和 `cpu-reset.h` 中有冲突的声明

**修复**:
1. 添加 `true` 作为 `blockable` 参数
2. 修正 `follow_pte_pmd` 调用参数从 7 个改为 6 个
3. 删除 `proc-fns.h` 中的旧版 `cpu_soft_restart` 声明

**修改文件**:
- `virt/kvm/kvm_main.c` - 修复 kvm_unmap_hva_range 和 follow_pte_pmd 调用
- `arch/arm64/include/asm/proc-fns.h` - 删除冲突的 cpu_soft_restart 声明

---

## 2026-03-05 07:10 - Run 22706184322

**错误**: `kernel/bpf/trampoline.c: error: implicit declaration of function 'call_rcu_tasks_trace'`

**原因**: 配置文件使用了错误的配置名 `CONFIG_TASKS_RCU_TRACE`，正确的应该是 `CONFIG_TASKS_TRACE_RCU`

**修复**: 将 `CONFIG_TASKS_RCU_TRACE=y` 改为 `CONFIG_TASKS_TRACE_RCU=y`

**修改文件**:
- `arch/arm64/configs/vendor/xiaomi/sm8250-common.config` - 修正配置名

---

## 2026-03-05 07:15 - Run 22706395117

**错误**: `kernel/bpf/trampoline.c: error: implicit declaration of function 'call_rcu_tasks_trace'`

**原因**: `kernel/rcu/tasks.h` 中 `call_rcu_tasks_trace` 等函数定义没有被 `#ifdef CONFIG_TASKS_TRACE_RCU` 条件编译保护，导致在某些编译路径下这些函数不可见

**修复**: 在 tasks.h 中为 `call_rcu_tasks_trace` 和 `rcu_read_unlock_trace_special` 函数添加 `#ifdef CONFIG_TASKS_TRACE_RCU` 条件编译

**修改文件**:
- `kernel/rcu/tasks.h` - 添加条件编译保护

---

## 2026-03-05 07:20 - Run 22706746252

**错误**: `kernel/bpf/trampoline.c: error: implicit declaration of function 'call_rcu_tasks_trace'`

**原因**: `trampoline.c` 中调用 `call_rcu_tasks_trace`、`rcu_read_lock_trace`、`rcu_read_unlock_trace` 的代码没有被 `CONFIG_TASKS_TRACE_RCU` 条件编译保护

**修复**: 在 trampoline.c 中为这些调用添加 `IS_ENABLED(CONFIG_TASKS_TRACE_RCU)` 条件判断

**修改文件**:
- `kernel/bpf/trampoline.c` - 添加条件编译

---

## 2026-03-05 - KernelSU 集成

**修改**: 集成 KernelSU 到内核源码

**修改文件**:
- `kernel/KSU/` - 添加 KernelSU 内核代码
- `drivers/kernelsu` - 创建符号链接
- `drivers/Makefile` - 添加 kernelsu 引用
- `drivers/Kconfig` - 添加 kernelsu Kconfig source
- `arch/arm64/configs/vendor/xiaomi/sm8250-common.config` - 添加 CONFIG_KPROBES, CONFIG_HAVE_KPROBES, CONFIG_KPROBE_EVENTS, CONFIG_KSU

---

## 2026-03-05 - Run 22707470682

**错误**: `kernel/bpf/trampoline.c: error: implicit declaration of function 'call_rcu_tasks_trace'`

**原因**: 多次尝试修复 TASKS_TRACE_RCU 配置问题仍未解决，暂时禁用 CONFIG_TASKS_TRACE_RCU

**修复**: 禁用 CONFIG_TASKS_TRACE_RCU，使用条件编译处理

**修改文件**:
- `arch/arm64/configs/vendor/xiaomi/sm8250-common.config` - 禁用 CONFIG_TASKS_TRACE_RCU
