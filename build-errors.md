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

**原因**: `CONFIG_TASKS_RCU` 没有在配置文件中显式启用。虽然 Kconfig 定义 `def_bool PREEMPT` 应该自动启用它，但配置合并过程中可能出现问题。此外，`CONFIG_BPF_SYSCALL` 会 select `CONFIG_TASKS_TRACE_RCU`，导致代码使用 `rcu_tasks_trace` 等需要完整 structure/宏定义的功能。

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

**修复**: 在 mmu_notifier.h 中添加 `enum mmu_notifier_event` 的 definition，包含 MMU_NOTIFY_UNMAP, MMU_NOTIFY_CLEAR 等枚举值。

**修改文件**:
- `include/linux/mmu_notifier.h` - 添加 enum mmu_notifier_event definition

---

## 2026-03-05 07:00 - Run 22706008608

**错误**:
- `virt/kvm/kvm_main.c:413: error: too few arguments to function 'kvm_unmap_hva_range'`
- `virt/kvm/kvm_main.c:1517,1532: error: too many arguments to function 'follow_pte_pmd'`
- `arch/arm64/kernel/cpu-reset.h:19: error: conflicting types for 'cpu_soft_restart'`

**原因**:
1. `kvm_unmap_hva_range` 需要 4 个参数，但只传了 3 个，缺少 `blockable` 参数
2. `follow_pte_pmd` API 变化：旧版 7 参数调用需要改为 6 参数
3. `cpu_soft_restart` 在 `proc-fns.h` 和 `cpu-reset.h` 中 conflicting的声明

**修复**:
1. 添加 `true` 作为 `blockable` 参数
2. 修正 `follow_pte_pmd` 调用参数 from 7 个改为 6 个
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
2. `allowlist.c` 缺失 `<linux/sched/task.h>` 头文件，导致 `put_task_struct` 未声明。

**修复**: 
1. 在 `drivers/kernelsu/ksu.h` 中添加 `TWA_RESUME` 的兼容性宏 definition。
2. 在 `drivers/kernelsu/allowlist.c` 中添加缺失 of `<linux/sched/task.h>` 和 `<linux/sched.h>`。

**修改文件**:
- `drivers/kernelsu/ksu.h` - 添加 `TWA_RESUME` 宏 definition。
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
- `drivers/kernelsu/su_mount_ns.c" - 添加内核版本检查
- `kernel/KSU/kernel/su_mount_ns.c" - 添加内核版本检查

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
1. 在 `drivers/kernelsu/seccomp_cache.c` 中为 `SECCOMP_ARCH_NATIVE_NR` 提供回退 definition，默认使用 `NR_syscalls`。

**修改文件**:
- `drivers/kernelsu/seccomp_cache.c` - 添加兼容性宏定义

---

## 2026-03-06 09:45 - Run 22744912627

**错误**: `./security/selinux/include/objsec.h:31:10: fatal error: flask.h: No such file or directory`

**原因**: KernelSU 在编译自带 of SELinux 钩子时，由于 `flask.h` 是动态生成的头文件（通常位于对象 tree `objtree` 中），导致包含路径不全而无法找到。

**修复**: 
1. 在 `drivers/kernelsu/Kbuild` 中增加对对象 tree 中 SELinux 包含路径的支持：`-I$(objtree)/security/selinux` 和 `-I$(objtree)/security/selinux/include`。

**修改文件**:
- `drivers/kernelsu/Kbuild` - 增加包含路径

---

## 2026-03-06 10:35 - Run 22746085566

**错误**: `./security/selinux/include/objsec.h:31:10: fatal error: flask.h: No such file or directory` (再次出现)

**原因**: 虽然增加了包含路径，但在并行编译过程中，`drivers/kernelsu` 可能在 `security/selinux` 尚未生成头文件时就开始编译。由于 KernelSU 作为驱动模块 be 引入，其依赖关系未能有效约束并行编译顺序。

**修复**: 
1. 在 `drivers/kernelsu/Kbuild` 中为 `selinux.o` 添加显式的头文件依赖。
2. 在 `.github/workflows/build-kernel.yml` 中增加显式生成 SELinux 头文件的步骤。

**修改文件**:
- `drivers/kernelsu/Kbuild` - 添加显式对象依赖。
- `.github/workflows/build-kernel.yml" - 增加手动头文件生成步骤。

---

## 2026-03-06 10:45 - Run 22746407964

**错误**: `make[2]: *** No rule to make target 'security/selinux/flask.h', needed by 'drivers/kernelsu/selinux/selinux.o'. Stop.`

**原因**: 在 `drivers/kernelsu/Kbuild` 中添加的显式依赖路径 `security/selinux/flask.h` 无法被子目录中的 `make` 解析为有效目标。

**修复**: 
1. 移除 `drivers/kernelsu/Kbuild` 中无法解析的显式依赖。
2. 优化 `.github/workflows/build-kernel.yml` 中的头文件生成步骤，通过手动编译 `genheaders` 工具并直接调用来生成 `flask.h`。

**修改文件**:
- `drivers/kernelsu/Kbuild` - 移除显式依赖。
- `.github/workflows/build-kernel.yml" - 优化手动头文件生成步骤。

---

## 2026-03-06 11:05 - Run 22746874543

**错误**: `drivers/kernelsu/file_wrapper.c` 中 `iopoll`, `remap_file_range` 成员缺失及 `REMAP_FILE_DEDUP` 未定义。

**原因**: 4.19 内核的 `struct file_operations` 不包含 these 较新内核引入的成员 and 宏。

**修复**: 
1. 在 `drivers/kernelsu/file_wrapper.c` 中为 `iopoll` 和 `remap_file_range` 的相关逻辑及赋值添加内核版本检查。
2. 处理 `REMAP_FILE_DEDUP` 宏缺失问题。
3. 适配 `mmap_supported_flags` 的内核版本差异。

**修改文件**:
- `drivers/kernelsu/file_wrapper.c" - 添加兼容性条件编译

---

## 2026-03-06 - Run 22747246443

**错误**: `kernel/rcu/tasks.h:437:28: error: dereferencing pointer to incomplete type 'struct rcu_tasks'` 及多个 `RTGS_*` 符号未定义。

**原因**: `kernel/rcu/tasks.h` 中的通用辅助函数（如 `call_rcu_tasks_generic`、`rcu_tasks_wait_gp`）及其使用的结构体/宏 definition 在条件编译保护上存在逻辑漏洞。`struct rcu_tasks` 被限制 in `CONFIG_TASKS_RCU` 中，而通用函数却暴露在外部。由于该内核配置中 `TASKS_RCU` 依赖 `PREEMPT`，在未开启抢占的情况下，`CONFIG_TASKS_RCU` 为 `n`，导致通用函数编译时找不到结构体定义。

**修复**: 重构 `kernel/rcu/tasks.h` 的条件编译 structure。将 `struct rcu_tasks` 定义、`RTGS_*` 宏以及通用辅助函数统一包裹在 `#if defined(CONFIG_TASKS_RCU) || defined(CONFIG_TASKS_TRACE_RCU)` 中。同时保留 Trampoline 和 Tracing 变体各自特有的实现逻辑在各自的 `#ifdef` 块中。

**修改文件**:
- `kernel/rcu/tasks.h" - 重构条件编译逻辑，确保通用定义在任一相关配置开启时均可用。

---

## 2026-03-06 11:25 - Run 22747602317

**错误**: `kernel/sched/core.c:6970:24: error: 'NOHZ_KICK_MASK' undeclared`

**原因**: `kernel/sched/core.c` 中的 `sched_unisolate_cpu_unlocked` 函数调用了 `NOHZ_KICK_MASK` 和 `nohz_flags`，但这些符号在 `kernel/sched/sched.h` 中是被 `CONFIG_NO_HZ_COMMON` 条件编译保护的。当前内核配置未启用 `CONFIG_NO_HZ_COMMON`。

**修复**: 在 `kernel/sched/core.c` 中为相关调用添加 `#ifdef CONFIG_NO_HZ_COMMON` 保护。

**修改文件**:
- `kernel/sched/core.c" - 为 NOHZ 相关调用添加条件编译。

---

## 2026-03-06 11:45 - Run 22747881960

**错误**: `drivers/input/fingerprint/fpc/fpc1020_tee.c: error: implicit declaration of function 'pinctrl_select_state'` 等

**原因**: `fpc1020_tee.c` 使用了 pinctrl 相关 API，但未包含 `<linux/pinctrl/consumer.h>` 头文件。

**修复**: 在 `drivers/input/fingerprint/fpc/fpc1020_tee.c` 中添加 `#include <linux/pinctrl/consumer.h>`。

**修改文件**:
- `drivers/input/fingerprint/fpc/fpc1020_tee.c" - 添加缺失的头文件。

---

## 2026-03-06 11:55 - Run 22748052021

**错误**: `drivers/power/supply/maxim/onewire_gpio.c: error: implicit declaration of function 'devm_pinctrl_get'` 等

**原因**: `onewire_gpio.c` 使用了 pinctrl 相关 API，但未包含 `<linux/pinctrl/consumer.h>` 头文件。

**修复**: 在 `drivers/power/supply/maxim/onewire_gpio.c` 中添加 `#include <linux/pinctrl/consumer.h>`。

**修改文件**:
- `drivers/power/supply/maxim/onewire_gpio.c" - 添加缺失的头文件。

---

## 2026-03-06 12:05 - Run 22748239280

**错误**: 大量 "undefined reference"，如 `mi_drm_register_client`、`init_net`、`kfree_skb` 等。

**原因**: GitHub Actions 工作流中的内核配置步骤存在严重缺陷。它直接将配置片段 `apollo.config` 拷贝为 `.config`，导致基础的 `kona_defconfig`（包含网络、DRM 等核心功能）未被应用，最终生成的内核功能严重缺失。

**修复**: 修正工作流。先应用基础 `vendor/kona_defconfig`，再通过合并小米通用及特定机型配置片段生成最终配置。

**修改文件**:
- `.github/workflows/build-kernel.yml" - 优化内核配置生成逻辑。

---

## 2026-03-06 12:15 - Run 22748494011

**错误**: `kernel/locking/lockdep.c:4008:27: error: 'nested' undeclared`

**原因**: `lock_release` 函数在调用 `__lock_release` 时尝试传递未定义的 `nested` 变量。根据注释，该参数是历史遗留产物且在当前上下文中并无实际意义。

**修复**: 在 `lock_release` 中将传递给 `__lock_release` 的参数改为 `0`。

**修改文件**:
- `kernel/locking/lockdep.c" - 修正 __lock_release 调用参数。

---

## 2026-03-06 12:25 - Run 22748730977

**错误**: `cpio: command not found`

**原因**: Ubuntu 20.04 容器环境未预装 `cpio` 工具，导致 `kernel/gen_kheaders.sh` 脚本在生成内核头文件归档时失败。

**修复**: 在 GitHub Actions 工作流的依赖安装步骤中添加 `cpio`。

**修改文件**:
- `.github/workflows/build-kernel.yml" - 添加 cpio 到 apt-get 安装列表。

---

## 2026-03-06 12:35 - Run 22748900903

**错误**: 多个编译错误，包括变量未初始化、不兼容选项及头文件缺失。

**原因**: 
1. `wm_adsp.c` 在某些分支下未给 `ret` 赋值。
2. `-Wno-enum-conversion` 在旧版 GCC 中不被支持。
3. `techpack` 驱动的包含路径配置不当。

**修复**: 
1. 补全 `wm_adsp.c` 的变量赋值逻辑。
2. 使用 `cc-option` 包装编译选项。
3. 修正 `cam_sensor_i2c.h` 包含路径并为 `hid-trace.o` 添加包含路径。

**修改文件**:
- `techpack/audio/asoc/codecs/cs35l41/wm_adsp.c`
- `techpack/audio/asoc/codecs/Kbuild`
- `techpack/camera-xiaomi-cas/drivers/cam_sensor_module/cam_sensor_io/cam_sensor_i2c.h`
- `drivers/hid/hid-trace.h`
- `drivers/hid/Makefile`

---

## 2026-03-06 13:08 - Run 22749823822

**错误**: `drivers/kernelsu/selinux/sepolicy.c:4:10: fatal error: policydb.h: No such file or directory`

**原因**: `policydb.h` 和 `services.h` 位于 `security/selinux/ss/` 目录下，但 KernelSU 的包含路径仅包含 `security/selinux`。`sepolicy.c` 尝试直接包含 `"policydb.h"` 而非 `"ss/policydb.h"`。同时，该文件引用了 4.19 内核中不存在的 `xattr.h` 和 `utils.h`。

**修复**: 
1. 将 `policydb.h` 和 `services.h` 的引用改为带 `ss/` 前缀的形式。
2. 包含 `sepolicy.h` 以确保接口一致性。
3. 移除不存在的 `xattr.h` 和 `utils.h` 引用。

**修改文件**:
- `drivers/kernelsu/selinux/sepolicy.c`

---

## 2026-03-06 13:30 - Run 22750244147

**错误**: `drivers/kernelsu/selinux/rules.c:19:51: error: 'struct selinux_state' has no member named 'policy'`

**原因**: 4.19 内核中 `struct selinux_state` 结构体不包含 `policy` 成员，而是通过 `ss->policydb` 访问策略数据库。

**修复**: 
1. 将 `rules.c` 中的 `selinux_state.policy` 替换为 `selinux_state.ss`。
2. 显式包含 `security.h`。

**修改文件**:
- `drivers/kernelsu/selinux/rules.c`

---

## 2026-03-06 15:05 - Run 22752345875

**错误**: `techpack/audio/asoc/codecs/Kbuild:226: *** Recursive variable 'KBUILD_CPPFLAGS' references itself (eventually). Stop.`

**原因**: 在某些构建环境中，在子 Kbuild 文件中使用 `+=` 向 `KBUILD_CPPFLAGS` 追加内容可能会导致递归变量错误，特别是当该变量被导出或以某种方式形成循环定义时。

**修复**: 将 `techpack/audio` 目录下所有 21 个 Kbuild 文件中的 `KBUILD_CPPFLAGS += $(CDEFINES)` 替换为 `ccflags-y += $(CDEFINES)`。这是 Kbuild 中添加目录特定 C 编译器标志的标准方式。

**修改文件**:
- `techpack/audio/asoc/Kbuild`
- `techpack/audio/dsp/Kbuild`
- `techpack/audio/ipc/Kbuild`
- `techpack/audio/asoc/codecs/sdm660_cdc/Kbuild`
- `techpack/audio/asoc/codecs/msm_sdw/Kbuild`
- `techpack/audio/asoc/codecs/csra66x0/Kbuild`
- `techpack/audio/asoc/codecs/aqt1000/Kbuild`
- `techpack/audio/asoc/codecs/cs35l41/Kbuild`
- `techpack/audio/asoc/codecs/cs35l41_k81/Kbuild`
- `techpack/audio/asoc/codecs/wcd938x/Kbuild`
- `techpack/audio/asoc/codecs/rouleur/Kbuild`
- `techpack/audio/asoc/codecs/tfa98xx/Kbuild`
- `techpack/audio/asoc/codecs/Kbuild`
- `techpack/audio/asoc/codecs/ep92/Kbuild`
- `techpack/audio/asoc/codecs/bolero/Kbuild`
- `techpack/audio/asoc/codecs/wcd937x/Kbuild`
- `techpack/audio/asoc/codecs/wcd934x/Kbuild`
- `techpack/audio/asoc/codecs/tfa9874/Kbuild`
- `techpack/audio/asoc/codecs/wsa883x/Kbuild`
- `techpack/audio/dsp/codecs/Kbuild`
- `techpack/audio/soc/Kbuild`

---

## 2026-03-06 15:30 - Run 22753136301

**错误**:
1. `lib/fault-inject.c:114:9: error: expected expression before 'do'`
2. `drivers/media/dvb-core/dmxdev.c:4686:3: error: pr_err format string... format string is defined here ... ~^ ... char *`

**原因**:
1. `WRITE_ONCE` 在此内核中被定义为 `do { ... } while (0)` 块，不能用在 `if` 语句作为表达式。
2. `pr_err` 包含 `%s` 但未提供相应的 `__func__` 参数。

**修复**:
1. 将 `WRITE_ONCE` 从 `if` 中拆分出来单独执行。
2. 在 `pr_err` 中添加 `__func__` 参数。

**修改文件**:
- `lib/fault-inject.c`
- `drivers/media/dvb-core/dmxdev.c`

---

## 2026-03-06 15:45 - Run 22753740632

**错误**:
1. `techpack/audio/dsp/q6adm.c:883:2: error: 'port_idx' is used uninitialized`
2. `net/netfilter/xt_mark.c:33:32: error: dereferencing pointer to incomplete type 'const struct xt_mark_tginfo2'`

**原因**:
1. 在 `port_idx` 被赋值前就已在 `pr_info` 中被调用。
2. 缺少 UAPI 结构体 definition。

**修复**:
1. 调换代码顺序，确保 `pr_info` 在变量赋值后执行。
2. 在 `xt_mark.c` 中手动添加 `xt_mark_tginfo2` 和 `xt_mark_mtinfo1` 结构体声明。

**修改文件**:
- `techpack/audio/dsp/q6adm.c`
- `net/netfilter/xt_mark.c`

---

## 2026-03-06 16:05 - Run 22754402655

**错误**:
1. `techpack/audio/dsp/elliptic/elliptic_sysfs.c: error: 'length' is used uninitialized`
2. `net/netfilter/xt_connmark.c: error: dereferencing pointer to incomplete type 'const struct xt_connmark_tginfo2'` 及大量未定义符号。

**原因**:
1. `opmode_show` 等函数中 `length` 变量未初始化即使用 `+=`。
2. 由于在 macOS 大小写不敏感文件系统上进行合并，导致 UAPI 中 `xt_connmark.h` 和 `xt_CONNMARK.h`（以及 `xt_mark.h` 和 `xt_MARK.h`）内容发生混淆，关键结构体 definition 丢失。

**修复**:
1. 初始化 `length = 0`。
2. 使用 `git hash-object` 和 `git update-index` 手动恢复 UAPI 头文件在 Git 索引中的正确内容，确保大小写不同的文件拥有各自正确的 blob。
3. 移除之前在 `xt_mark.c` 中添加的临时结构体声明。

**修改文件**:
- `techpack/audio/dsp/elliptic/elliptic_sysfs.c`
- `include/uapi/linux/netfilter/xt_connmark.h`
- `include/uapi/linux/netfilter/xt_CONNMARK.h`
- `include/uapi/linux/netfilter/xt_mark.h`
- `include/uapi/linux/netfilter/xt_MARK.h`
- `net/netfilter/xt_mark.c`

---

## 2026-03-06 16:55 - Run 22755787843

**错误**:
1. `techpack/audio/dsp/mius/mius_sysfs.c: error: 'length' is used uninitialized`
2. `net/netfilter/xt_DSCP.c: error: 'XT_DSCP_SHIFT' undeclared` 等

**原因**:
1. `mius_sysfs.c` 中存在与 `elliptic_sysfs.c` 相同的变量未初始化问题。
2. UAPI 中更多大小写敏感的头文件对（`xt_dscp.h`/`xt_DSCP.h`、`xt_rateest.h`/`xt_RATEEST.h`、`xt_tcpmss.h`/`xt_TCPMSS.h`）在 macOS 上合并时发生混淆，导致 definition 丢失。

**修复**:
1. 初始化 `mius_sysfs.c` 中的 `length = 0`。
2. 再次使用 `git hash-object` 和 `git update-index` 手动恢复剩余所有哈希重复的 UAPI 头文件内容。

**修改文件**:
- `techpack/audio/dsp/mius/mius_sysfs.c`
- `include/uapi/linux/netfilter/xt_dscp.h`
- `include/uapi/linux/netfilter/xt_DSCP.h`
- `include/uapi/linux/netfilter/xt_rateest.h`
- `include/uapi/linux/netfilter/xt_RATEEST.h`
- `include/uapi/linux/netfilter/xt_tcpmss.h`
- `include/uapi/linux/netfilter/xt_TCPMSS.h`

---

## 2026-03-06 17:15 - Run 22757326537

**错误**:
1. `./include/trace/define_trace.h:89:42: fatal error: ./pll_trace.h: No such file or directory`
2. `techpack/camera-xiaomi-cas/drivers/cam_sync/cam_sync.c: error: format string mismatches`

**原因**:
1. `techpack/display/pll/` 目录下 Tracepoint 包含路径配置缺失，导致编译器找不到同目录下的 `pll_trace.h`。
2. 相机同步驱动 `cam_sync.c` 中存在多处格式化字符串与变量类型不匹配的问题（如 `atomic_t` 误用 `%d`，`long` 误用 `%d`），在开启 `-Werror` 的环境下导致编译失败。

**修复**:
1. 在 `techpack/display/pll/Makefile` 中添加 `ccflags-y += -I$(src)`。
2. 修复 `techpack` 中所有 `cam_sync.c` 变体的格式化字符串错误：将 `atomic_t` 类型包装为 `atomic_read()`，并修正 `%d` 为 `%ld`（对应 `long`）或 `%u`（对应 `uint32_t`）。

**修改文件**:
- `techpack/display/pll/Makefile`
- `techpack/camera-xiaomi-tablet/drivers/cam_sync/cam_sync.c`
- `techpack/camera/drivers/cam_sync/cam_sync.c`
- `techpack/camera-xiaomi/drivers/cam_sync/cam_sync.c`
- `techpack/camera-bengal/drivers/cam_sync/cam_sync.c`
- `techpack/camera-xiaomi-cas/drivers/cam_sync/cam_sync.c`

---

## 2026-03-06 23:45 - Run 22769942297

**错误**: `vmlinux: multiple definition of ...` (nt36xxx vs nt36672c)

**原因**: 基础配置 `kona_defconfig` 启用了 `CONFIG_TOUCHSCREEN_NT36XXX=y`，而机型配置 `apollo.config` 启用了 `CONFIG_TOUCHSCREEN_NT36xxx_HOSTDL_SPI=y`。由于这两个驱动包含大量重名的全局函数，同时编入内核导致链接阶段符号冲突。

**修复**: 在 `sm8250-common.config` 中显式禁用 `CONFIG_TOUCHSCREEN_NT36XXX=n`，以消除符号重复定义冲突。

**修改文件**:
- `arch/arm64/configs/vendor/xiaomi/sm8250-common.config`

---

## 2026-03-07 - Run 22770823703

**错误**: 大量 "undefined reference" 错误。
- `rcu_trace_lock_map` 未定义 (BPF helpers)
- `__tracepoint_android_vh_...` 未定义 (BPF trampoline/struct_ops)
- `path_mount` / `path_umount` 未定义 (KernelSU)
- `ksu_...` 未定义 (KernelSU SELinux rules)

**原因**: 
1. `rcu_trace_lock_map` 在开启 `CONFIG_DEBUG_LOCK_ALLOC` 时缺少变量定义。
2. Android Vendor Hooks 虽然在源码中，但配置未开启，导致相关符号无法导出。
3. `path_mount` 和 `path_umount` 是 Linux 5.x 的 API，在 4.19 中不存在。
4. `drivers/kernelsu/selinux/sepolicy.c` 包含的是空 stub，导致 `rules.c` 调用时链接失败。

**修复**: 
1. 在 `kernel/rcu/update.c` 中为 `rcu_trace_lock_map` 添加变量定义。
2. 在 `sm8250-common.config` 中启用 `CONFIG_ANDROID_VENDOR_HOOKS=y`。
3. 在 `su_mount_ns.c` 和 `kernel_umount.c` 中为旧版内核添加 `path_mount`/`path_umount` 到 `do_mount`/`do_umount` 的宏映射。
4. 从 `kernel/KSU` 恢复真实的 `sepolicy.c` 和 `sepolicy.h` 实现。

**修改文件**:
- `kernel/rcu/update.c`
- `arch/arm64/configs/vendor/xiaomi/sm8250-common.config`
- `drivers/kernelsu/su_mount_ns.c`
- `drivers/kernelsu/kernel_umount.c`
- `drivers/kernelsu/selinux/sepolicy.c`
- `drivers/kernelsu/selinux/sepolicy.h`

---

## 2026-03-07 00:45 - Run 22773269842

**错误**: `net/netfilter/xt_connmark.c: error: dereferencing pointer to incomplete type 'const struct xt_connmark_tginfo2'` 及大量未定义符号。

**原因**: 由于在 macOS 大小写不敏感文件系统上进行提交，导致 UAPI 中多组大小写敏感头文件（`xt_connmark.h`/`xt_CONNMARK.h`、`xt_mark.h`/`xt_MARK.h`、`xt_tcpmss.h`/`xt_TCPMSS.h`、`xt_dscp.h`/`xt_DSCP.h`、`xt_rateest.h`/`xt_RATEEST.h`）的内容在 Git 索引中被错误地统一为其中一个版本，导致关键结构体定义丢失。

**修复**: 
1. 移除 `net/netfilter/xt_connmark.c` 和 `net/netfilter/xt_mark.c` 中之前为了规避头文件缺失而添加的结构体定义 workaround。
2. 使用 `git update-index --cacheinfo` 手动恢复 Git 索引中所有 10 个 UAPI 头文件与其正确 blob 的对应关系，确保在 Linux 编译环境下能访问到各自正确的定义。

**修改文件**:
- `net/netfilter/xt_connmark.c`
- `net/netfilter/xt_mark.c`
- `include/uapi/linux/netfilter/xt_connmark.h` (Index only)
- `include/uapi/linux/netfilter/xt_CONNMARK.h` (Index only)
- `include/uapi/linux/netfilter/xt_mark.h` (Index only)
- `include/uapi/linux/netfilter/xt_MARK.h` (Index only)
- `include/uapi/linux/netfilter/xt_tcpmss.h` (Index only)
- `include/uapi/linux/netfilter/xt_TCPMSS.h` (Index only)
- `include/uapi/linux/netfilter/xt_dscp.h` (Index only)
- `include/uapi/linux/netfilter/xt_DSCP.h` (Index only)
- `include/uapi/linux/netfilter/xt_rateest.h` (Index only)
- `include/uapi/linux/netfilter/xt_RATEEST.h` (Index only)

---

## 2026-03-07 - Run N/A (offline analysis)

**错误**: netfilter UAPI 头文件与模块源码定义错位，导致后续编译可能出现结构体重定义/不完整类型相关错误。

**原因**:
1. 在大小写不敏感文件系统上，`xt_*.h` 与 `xt_*.h`（大小写不同）内容再次发生串写，导致小写头文件被错误替换为 target 版本定义。
2. 为绕过该问题临时加入的 `xt_dscp.c`/`xt_rateest.c`/`xt_tcpmss.c` 本地 workaround 与正确 UAPI 同时存在时会产生二次冲突。

**修复**:
1. 恢复 netfilter 模块源码，移除临时 workaround（让定义来源回到正确 UAPI 头文件）。
2. 保持 Git index 中大小写敏感头文件的正确 blob 对应关系，避免错误内容被提交。

**修改文件**:
- `net/netfilter/xt_dscp.c` - 移除 `XT_DSCP_SHIFT` 临时宏
- `net/netfilter/xt_rateest.c` - 移除 `xt_rateest_target_info` 临时结构体
- `net/netfilter/xt_tcpmss.c` - 移除 `XT_TCPMSS_CLAMP_MSS` 临时宏
