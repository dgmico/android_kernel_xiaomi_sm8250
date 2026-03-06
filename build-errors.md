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
3. `cpu_soft_restart` 在 `proc-fns.h` 和 `cpu-reset.h` 中 highlighting冲突的声明

**修复**:
1. 添加 `true` 作为 `blockable` 参数
2. 修正 `follow_pte_pmd` 调用参数 from 7 个改为 6 个
3. 删除 `proc-fns.h` 中的旧版 `cpu_soft_restart` 声明

**修改文件**:
- `virt/kvm/kvm_main.c` - 修复 kvm_unmap_hva_range 和 follow_pte_pmd 调用
- `arch/arm64/include/asm/proc-fns.h` - 删除冲突 of cpu_soft_restart 声明

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

**原因**: `kernel/rcu/tasks.h` 中 `call_rcu_tasks_trace` 等函数定义没有被 `#ifdef CONFIG_TASKS_TRACE_RCU` 条件编译保护，导致在某些编译路径下 these 函数不可见

**修复**: 在 tasks.h 中为 `call_rcu_tasks_trace` 和 `rcu_read_unlock_trace_special` 函数添加 `#ifdef CONFIG_TASKS_TRACE_RCU` 条件编译

**修改文件**:
- `kernel/rcu/tasks.h` - 添加条件编译保护

---

## 2026-03-05 07:20 - Run 22706746252

**错误**: `kernel/bpf/trampoline.c: error: implicit declaration of function 'call_rcu_tasks_trace'`

**原因**: `trampoline.c` 中调用 `call_rcu_tasks_trace`、`rcu_read_lock_trace`、`rcu_read_unlock_trace` 的代码没有被 `CONFIG_TASKS_TRACE_RCU` 条件编译保护

**修复**: 在 trampoline.c 中为 these 调用添加 `IS_ENABLED(CONFIG_TASKS_TRACE_RCU)` 条件判断

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

---

## 2026-03-05 16:42 - Run 22709177957

**错误**: `fatal error: ./trace.h: No such file or directory` in `drivers/clk/qcom/clk-debug.c`

**原因**: Tracepoint 头文件 `drivers/clk/qcom/trace.h` 虽然设置了 `TRACE_INCLUDE_PATH .`，但编译系统在包含 `include/trace/define_trace.h` 时无法在当前搜索路径中找到该文件。

**修复**: 在 `drivers/clk/qcom/Makefile` 中为 `clk-debug.o` 添加 `-I$(src)`。同时修正了 `trace.h` 中的宏定义保护不匹配的问题。

**修改文件**:
- `drivers/clk/qcom/Makefile` - 添加 `CFLAGS_clk-debug.o := -I$(src)`
- `drivers/clk/qcom/trace.h` - 修正 `_TRACE_CLOCK_QCOM` 为 `_TRACE_CLOCK_QCOM_H`


---

## 2026-03-05 16:52 - Run 22709608857

**错误**: `drivers/clk/qcom/common.h:49:16: error: field ‘hw’ has incomplete type`

**原因**: `struct clk_dummy` 在 `common.h` 中直接嵌入了 `struct clk_hw` 结构体，但该头文件仅对 `struct clk_hw` 进行了前向声明，未包含其完整定义（通常在 `<linux/clk-provider.h>` 中）。

**修复**: 在 `drivers/clk/qcom/common.h` 中增加 `#include <linux/clk-provider.h>` 并移除冗余的前向声明。

**修改文件**:
- `drivers/clk/qcom/common.h` - 添加头文件包含并优化声明。


---

## 2026-03-05 17:04 - Run 22709974659

**错误**: `drivers/kernelsu/ksu.c:89:1: error: type defaults to ‘int’ in declaration of ‘MODULE_IMPORT_NS’`

**原因**: `MODULE_IMPORT_NS` 是在 Linux 5.4 引入的宏，用于模块命名空间导入。当前内核版本为 4.19，不支持该宏。

**修复**: 在 `drivers/kernelsu/ksu.c` 中包含 `<linux/version.h>`，并将 `MODULE_IMPORT_NS` 调用包裹在 `#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0)` 中。

**修改文件**:
- `drivers/kernelsu/ksu.c`


---

## 2026-03-05 17:35 - Run 22710415798

**错误**: `drivers/kernelsu/allowlist.c:448:32: error: ‘TWA_RESUME’ undeclared` and `drivers/kernelsu/allowlist.c:454:5: error: implicit declaration of function ‘put_task_struct’`

**原因**: 
1. `TWA_RESUME` 是在 Linux 5.9 引入的，而 4.19 内核中 `task_work_add` 使用 `bool` 类型的 `notify` 参数。
2. `allowlist.c` 缺少 `<linux/sched/task.h>` 头文件，导致 `put_task_struct` 未声明。

**修复**: 
1. 在 `drivers/kernelsu/ksu.h` 中添加 `TWA_RESUME` 的兼容性宏 definition。
2. 在 `drivers/kernelsu/allowlist.c` 中添加缺失 of `<linux/sched/task.h>` 和 `<linux/sched.h>`。

**修改文件**:
- `drivers/kernelsu/ksu.h` - 添加 `TWA_RESUME` 宏定义。
- `drivers/kernelsu/allowlist.c` - 添加缺失的头文件。

---

## 2026-03-05 17:45 - Run 22711588458

**错误**: `drivers/kernelsu/app_profile.c:90:33: error: ‘struct seccomp’ has no member named ‘filter_count’`

**原因**: 4.19 内核中的 `struct seccomp` 只有 `mode` 和 `filter` 成员，没有 `filter_count`。此外，`seccomp_filter_release` 也是在 Linux 5.9 引入的。

**修复**: 
1. 将 `filter_count` 的重置代码包裹在 `#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 9, 0)` 中。
2. 对于旧版内核，定义 `seccomp_filter_release` 宏调用 `put_seccomp_filter`。

**修改文件**:
- `drivers/kernelsu/app_profile.c` - 添加内核版本检查和兼容性宏

---

## 2026-03-05 18:15 - Run 22712646834

**错误**: `drivers/kernelsu/pkg_observer.c:40:6: error: ‘const struct fsnotify_ops’ has no member named ‘handle_inode_event’`

**原因**: `handle_inode_event` 钩子是在 Linux 5.1 引入的。4.19 内核仅支持通用的 `handle_event` 钩子。

**修复**: 
1. 增加内核版本判断。
2. 对于 < 5.1.0 的内核，在 `fsnotify_ops` 中使用 `handle_event` 代替 `handle_inode_event`。
3. 提供适配旧版参数的 `ksu_handle_event` 实现。

**修改文件**:
- `drivers/kernelsu/pkg_observer.c` - 添加 fsnotify 兼容性处理

---

## 2026-03-06 00:55 - Run 22743487946

**错误**: `drivers/kernelsu/setuid_hook.c:51:40: error: ‘TWA_RESUME’ undeclared`

**原因**: `TWA_RESUME` 是在 Linux 5.9 引入的，4.19 内核没有该定义。虽然 `ksu.h` 中已经提供了兼容性宏，但 `setuid_hook.c` 和 `ksud.c` 没有包含该头文件。

**修复**: 
1. 在 `drivers/kernelsu/setuid_hook.c` 和 `drivers/kernelsu/ksud.c` 中添加 `#include "ksu.h"` 以支持旧版内核。

**修改文件**:
- `drivers/kernelsu/setuid_hook.c` - 包含 ksu.h
- `drivers/kernelsu/ksud.c` - 包含 ksu.h

---

## 2026-03-06 01:10 - Run 22743761842

**错误**: `drivers/kernelsu/su_mount_ns.c:16:10: fatal error: uapi/linux/mount.h: No such file or directory`

**原因**: `uapi/linux/mount.h` 是在 Linux 5.1 引入s的，4.19 内核并不存在。

**修复**: 
1. 在 `drivers/kernelsu/su_mount_ns.c` 和 `kernel/KSU/kernel/su_mount_ns.c` 中，将 `#include <uapi/linux/mount.h>` 包裹在内核版本检查（`#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 1, 0)`）中。

**修改文件**:
- `drivers/kernelsu/su_mount_ns.c` - 添加内核版本检查
- `kernel/KSU/kernel/su_mount_ns.c` - 添加内核版本检查

---

## 2026-03-06 01:45 - Build Environment Migration

**变更**: 将编译环境从 `ubuntu-latest` (Ubuntu 24.04) 迁移到 `ubuntu:20.04` Docker 容器。

**原因**: 
1. LineageOS 官方推荐使用 Ubuntu 20.04 作为构建环境。
2. Ubuntu 24.04 的工具链过于超前，对旧版 4.19 内核产生的警告过多，且某些旧版依赖包已移除。
3. 使用固定版本的 Docker 容器能确保构建环境的长期一致性。

**修改文件**:
- `.github/workflows/build-kernel.yml` - 添加 `container: ubuntu:20.04` 配置，移除 `sudo` 并优化依赖安装流程。

---

## 2026-03-06 01:55 - Run 22744586759

**错误**: `gcc-aarch64-linux-gnu : Depends: gcc-9-aarch64-linux-gnu (>= 9.3.0-3~) but it is not going to be installed`

**原因**: 在 `ubuntu:20.04` 容器中，直接安装 `gcc-aarch64-linux-gnu` 会因依赖冲突失败。由于 Workflow 已经下载了 LineageOS 提供的 GCC 工具链，系统自带的交叉编译器并非必须。

**修复**: 
1. 从 `apt-get install` 列表中移除 `gcc-aarch64-linux-gnu`。
2. 在安装其他依赖前先安装 `ca-certificates`，以确保网络请求正常。

**修改文件**:
- `.github/workflows/build-kernel.yml` - 调整 `安装编译依赖` 步骤。

---

## 2026-03-06 09:35 - Run 22744667816

**错误**: `drivers/kernelsu/seccomp_cache.c:12:34: error: 'SECCOMP_ARCH_NATIVE_NR' undeclared`

**原因**: `SECCOMP_ARCH_NATIVE_NR` 是较新版本内核引入的宏，在 4.19 内核中不存在。

**修复**: 
1. 在 `drivers/kernelsu/seccomp_cache.c` 中为 `SECCOMP_ARCH_NATIVE_NR` 提供回退定义，默认使用 `NR_syscalls`。

**修改文件**:
- `drivers/kernelsu/seccomp_cache.c` - 添加兼容性宏定义

---

## 2026-03-06 09:45 - Run 22744912627

**错误**: `./security/selinux/include/objsec.h:31:10: fatal error: flask.h: No such file or directory`

**原因**: KernelSU 在编译自带的 SELinux 钩子时，由于 `flask.h` 是动态生成的头文件（通常位于对象树 `objtree` 中），导致包含路径不全而无法找到。

**修复**: 
1. 在 `drivers/kernelsu/Kbuild` 中增加对对象树中 SELinux 包含路径的支持：`-I$(objtree)/security/selinux` 和 `-I$(objtree)/security/selinux/include`。

**修改文件**:
- `drivers/kernelsu/Kbuild` - 增加包含路径

---

## 2026-03-06 10:35 - Run 22746085566

**错误**: `./security/selinux/include/objsec.h:31:10: fatal error: flask.h: No such file or directory` (再次出现)

**原因**: 虽然增加了包含路径，但在并行编译过程中，`drivers/kernelsu` 可能在 `security/selinux` 尚未生成头文件时就开始编译。由于 KernelSU 作为驱动模块被引入，其依赖关系未能有效约束并行编译顺序。

**修复**: 
1. 在 `drivers/kernelsu/Kbuild` 中为 `selinux.o` 添加显式的头文件依赖。
2. 在 `.github/workflows/build-kernel.yml` 中增加显式生成 SELinux 头文件的步骤。

**修改文件**:
- `drivers/kernelsu/Kbuild` - 添加显式对象依赖。
- `.github/workflows/build-kernel.yml` - 增加手动头文件生成步骤。

---

## 2026-03-06 10:45 - Run 22746407964

**错误**: `make[2]: *** No rule to make target 'security/selinux/flask.h', needed by 'drivers/kernelsu/selinux/selinux.o'. Stop.`

**原因**: 在 `drivers/kernelsu/Kbuild` 中添加的显式依赖路径 `security/selinux/flask.h` 无法被子目录中的 `make` 解析为有效目标。

**修复**: 
1. 移除 `drivers/kernelsu/Kbuild` 中无法解析的显式依赖。
2. 优化 `.github/workflows/build-kernel.yml` 中的头文件生成步骤，通过手动编译 `genheaders` 工具并直接调用来生成 `flask.h`。

**修改文件**:
- `drivers/kernelsu/Kbuild` - 移除显式依赖。
- `.github/workflows/build-kernel.yml` - 优化手动头文件生成步骤。

---

## 2026-03-06 11:05 - Run 22746874543

**错误**: `drivers/kernelsu/file_wrapper.c` 中 `iopoll`, `remap_file_range` 成员缺失及 `REMAP_FILE_DEDUP` 未定义。

**原因**: 4.19 内核的 `struct file_operations` 不包含这些较新内核引入的成员和宏。

**修复**: 
1. 在 `drivers/kernelsu/file_wrapper.c` 中为 `iopoll` 和 `remap_file_range` 的相关逻辑及赋值添加内核版本检查。
2. 处理 `REMAP_FILE_DEDUP` 宏缺失问题。
3. 适配 `mmap_supported_flags` 的内核版本差异。

**修改文件**:
- `drivers/kernelsu/file_wrapper.c` - 添加兼容性条件编译
