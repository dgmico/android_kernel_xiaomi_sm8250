# Build Errors Log

## [2026-03-10 07:00] 错误诊断
- **错误原文**: `gzip: stdin: not in gzip format`
- **原因分析**: Clang 下载链接失效或返回非压缩格式；Telegram 脚本语法不兼容 POSIX sh。
- **修复对策**: 使用更稳定的工具链下载方式；将工作流 shell 统一指定为 bash，修复变量截断。

## [2026-03-10 07:03] 错误诊断
- **错误原文**: `错误: 下载的文件过小，链接可能已失效。`
- **原因分析**: Google Git 的 +archive 接口不支持直接 curl 下载大体积存档，返回了 HTML 页面。
- **修复对策**: 改用 git clone --depth 1 获取 Clang 工具链；确保工具链路径正确。

## [2026-03-10 07:14] 错误诊断
- **错误原文**: `error: Sparse checkout leaves no entry on working directory`
- **原因分析**: 默认 master 分支中可能不包含指定的 clang 目录，或者稀疏检出语法在当前 git 版本下不兼容。
- **修复对策**: 尝试直接克隆包含该 clang 版本的特定分支 (master-kernel-build-2022)。

## [2026-03-10 07:16] 错误诊断
- **错误原文**: `错误: 在分支 master-kernel-build-2022 中未找到 clang-r450784d。`
- **原因分析**: 指定版本不存在，但存在其后续版本 clang-r450784e。
- **修复对策**: 将 Clang 版本号更新为该分支中实际存在的 clang-r450784e。

## [2026-03-10 07:22] 错误诊断
- **错误原文**: `../kernel/gen_kheaders.sh: line 61: cpio: command not found`
- **原因分析**: 容器环境缺少 cpio 工具，导致内核头文件生成失败。
- **修复对策**: 在工作流的安装依赖步骤中添加 cpio 软件包。

## [2026-03-10 07:34] 错误诊断
- **错误原文**: `error: stack frame size (2912) exceeds limit (2048) in 'sha3_256_hmac'`
- **原因分析**: Clang 编译器检测到 sha384_software.c 中的栈帧大小超过了内核设定的 2048 字节警告阈值，且由于 -Werror 被视为错误。
- **修复对策**: 在内核配置文件中调大 CONFIG_FRAME_WARN 阈值至 3072 或更高，以适应 LLVM 的编译特性。

## [2026-03-10 08:13] 错误诊断
- **错误原文**: `ld.lld: error: undefined symbol: ps5169_cfg_usb`
- **原因分析**: `dwc3-msm.c` 调用了 `ps5169_cfg_usb()`，但该函数 definition 在 `ps5169.c` 中，且仅在 `CONFIG_PS5169` 启用时才编译。
- **修复对策**: 在 `ps5169.h` 中增加条件宏判断，若未定义 `CONFIG_PS5169` 则提供空函数实现。同时优化了 GitHub Actions 工作流，增加缓存和改进通知。

## [2026-03-10 09:31] 错误诊断
- **错误原文**: `drivers/Kconfig:238: can't open file "drivers/KernelSU/Kconfig"`
- **原因分析**: `drivers/KernelSU` 被作为 gitlink (160000) 提交，导致 CI 检出时缺少文件。
- **修复对策**: 移除 `drivers/KernelSU` 的 git 索引，删除其内部 `.git` 目录，并以普通目录形式重新添加所有文件。

## [2026-03-10 09:41] 错误诊断
- **错误原文**: `../drivers/input/input.c:453:51: error: too many arguments to function call, expected 3, have 4`
- **原因分析**: `ksu_handle_input_handle_event` 函数原型仅需要 3 个参数（type, code, value），但在 `input.c` 中错误地传入了 `dev` 指针。
- **修复对策**: 移除 `input.c` 调用中的第一个参数 `dev`，使其符合函数原型声明。

## [2026-03-10 09:57] 错误诊断
- **错误原文**: `implicit declaration of function 'ksu_handle_vfs_open'` 及 `ksu_handle_vfs_read` 参数不匹配。
- **原因分析**: 不同版本的 KernelSU API 入口不同。当前集成的版本不包含 `vfs_open` 钩子，且 `vfs_read` / `stat` 钩子要求传递指针的指针。
- **修复对策**: 移除 `fs/open.c` 中的非法调用；修正 `fs/stat.c` 和 `fs/read_write.c` 中的函数名及参数传递方式。

## [2026-03-10 10:15] 错误诊断
- **错误原文**: `../net/netfilter/xt_mark.c:33:32: error: incomplete definition of type 'struct xt_mark_tginfo2'`
- **原因分析**: Commit `096a9ee7ec423983c1ec8f370de6890e920d2b4c` 错误地替换了多个 netfilter UAPI 头文件，将其内容改为了递归包含自身或指向不存在的路径，导致结构体定义缺失。
- **修复对策**: 将受影响的 netfilter 相关头文件和源码恢复到 KernelSU 集成之前的版本（Commit `b19371a15235c783a6c24583053c47a84e36f574`）。
## [2026-03-11 03:29] 错误诊断
- **错误原文**: `Error: revocery: Failed to find update binary META-INF/com/google/android/update-binary`
- **原因分析**: LineageOS Recovery 等现代 Recovery 环境不再提供 `/sbin/sh`，导致以 `#!/sbin/sh` 作为 Shebang 的脚本执行失败，报错找不到文件。
- **修复对策**: 将 `anykernel3/META-INF/com/google/android/update-binary` 的 Shebang 修改为更通用的 `#!/bin/sh`。
## [2026-03-11 06:40] 错误诊断
- **错误原文**: `Error: revocery: Failed to find update binary META-INF/com/google/android/update-binary` (持续)
- **原因分析**: 1. GitHub Artifacts 会对上传文件进行二次打包，用户可能直接 sideload 了外层包装包；2. 手动 zip 命令可能在某些环境下产生了 Recovery 无法识别的索引结构。
- **修复对策**: 修改工作流，取消手动 zip 步骤，直接将 anykernel3 目录内容作为 artifact 上传。这样从浏览器下载到的 ZIP 即是标准 flashable 结构。
## [2026-03-11 07:03] 错误诊断
- **错误原文**: `刷入成功但重启进入 Fastboot`
- **原因分析**: 内核虽已刷入，但可能因未处理 AVB 校验或内核压缩格式不兼容导致启动失败 (Bootloop to Fastboot)。
- **修复对策**: 1. 在 anykernel.sh 中强制开启 patch_vbmeta_flag；2. 优化 Image 文件的选取逻辑，优先尝试 Image.gz。
## [2026-03-11 07:20] 错误诊断
- **错误原文**: `Error in /sideload/package.zip`
- **原因分析**: 1. ZIP 结构可能不符合 LineageOS Recovery 的严格检查（缺少 metadata）；2. 脚本执行可能因路径问题在初始化阶段崩溃。
- **修复对策**: 1. 在 ZIP 中添加 META-INF/com/android/metadata 文件；2. 将 Shebang 统一为 /bin/sh 并放宽压缩等级。
## [2026-03-11 07:36] 错误诊断
- **错误原文**: `Error in /sideload/package.zip` (持续)
- **原因分析**: LineageOS Recovery 的解析器可能对 ZIP 的元数据和权限有特殊要求，不匹配会导致直接报包错误。
- **修复对策**: 1. 移除自定义 metadata；2. 使用 `zip -r9X` 排除扩展属性；3. 强制对 anykernel3 目录执行 `chmod 755`，并恢复 `/sbin/sh` Shebang。
## [2026-03-11 07:59] 错误诊断
- **错误原文**: `Error in /sideload/package.zip (status 1)`
- **原因分析**: 根据 @flash_error_analysis.md，1. 强制开启 is_slot_device=1 与 A-only 系统逻辑冲突；2. 缺少 LineageOS 特有的机型代号 'apollon' 导致校验失败。
- **修复对策**: 1. 将 is_slot_device 恢复为 auto；2. 在 anykernel.sh 中添加 device.name13=apollon 兼容机型。
## [2026-03-11 08:31] 错误诊断
- **错误原文**: `刷入 Status 1 修复后，重启依然进入 Fastboot`
- **原因分析**: 1. 内核解压失败或格式不兼容；2. AVB 校验在当前 Recovery 下未被完全绕过；3. 缺少关键驱动配置。
- **修复对策**: 1. 在工作流中强制优先选取未压缩的 `Image` 镜像；2. 优化 anykernel.sh 的变量设置。
## [2026-03-11 08:56] 错误诊断
- **错误原文**: `刷入后依然进入 Fastboot (持续)`
- **原因分析**: 小米 Kona (apollo) 设备在引导时需要匹配的 DTB 和 DTBO。此前工作流未收集并打包这些文件，导致引导程序无法加载硬件描述。
- **修复对策**: 1. 在工作流中收集 kona-v2.dtb 和 apollo-sm8250-overlay.dtbo；2. 将它们分别命名为 dtb 和 dtbo.img 放入 AnyKernel3 根目录。
## [2026-03-11 09:04] 错误诊断
- **错误原文**: `刷入成功但重启进入 Fastboot (持续)`
- **原因分析**: 机型确认为 Redmi K30S Ultra (apollon)。此前脚本未显式处理 `dtbo` 分区，且代号匹配逻辑可能跳过了关键硬件补丁。
- **修复对策**: 1. 在 anykernel.sh 中添加 `dtbo=/dev/block/bootdevice/by-name/dtbo`；2. 在安装逻辑中显式调用 `flash_dtbo`；3. 确保所有代号 (apollo/apollon) 都在校验列表中。
