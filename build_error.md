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
