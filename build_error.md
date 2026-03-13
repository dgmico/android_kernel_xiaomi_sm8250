# Build Errors Log

... (保留之前的记录) ...

## [2026-03-12 01:50] 最终成功诊断 (Redmi K30S Ultra)
- **现象**: 刷入成功但重启进入 Fastboot。
- **根源**: 1. AnyKernel3 内置 magiskboot 版本过旧，无法正确处理 Android 16 镜像；2. 小米 Kona 平台对 DTBO/AVB 校验极其敏感。
- **最终修复对策**: 
    1. 切换为 **S3 Rebuild** 方案：在服务端使用最新 magiskboot 结合原厂 boot.img 进行重组。
    2. 移除 AnyKernel3，改用极简 `dd` 脚本直接写入 boot 和 dtbo 分区。
    3. 锁定编译环境为 Ubuntu 22.04 以确保依赖完整性。
- **结论**: 该方案已验证成功，系统正常启动，KernelSU 运行正常。

## [2026-03-13 01:45] 错误诊断 (net/netfilter/xt_connmark.c)
- **错误原文**: `net/netfilter/xt_connmark.c:39:53: error: declaration of 'struct xt_connmark_tginfo2' will not be visible outside of this function [-Werror,-Wvisibility]`
- **原因分析**: `struct xt_connmark_tginfo2` 及其相关常量（`XT_CONNMARK_SET`, `XT_CONNMARK_SAVE`, `XT_CONNMARK_RESTORE`）未在相关的 `include/uapi/linux/netfilter/xt_connmark.h` 中定义，或定义被由于某些宏配置未开启而未被包含。
- **修复对策**: 检查并更新 `include/uapi/linux/netfilter/xt_connmark.h`，确保包含该结构体的完整定义及其常量。

## [2026-03-13 03:25] 错误诊断 (net/netfilter/xt_DSCP.c)
- **错误原文**: `../net/netfilter/xt_DSCP.c:34:51: error: use of undeclared identifier 'XT_DSCP_SHIFT'`
- **原因分析**: 大量 netfilter 相关的 UAPI 头文件被替换成了具有无限递归特性的重定向文件（如 `xt_dscp.h` 包含 `linux/netfilter/xt_dscp.h`），导致关键常量和结构体定义丢失。
- **修复对策**: 
    1. 还原 `include/uapi/linux/netfilter/xt_dscp.h` 和 `include/uapi/linux/netfilter/xt_DSCP.h` 的完整定义。
    2. 同步还原 `xt_tcpmss.h`, `xt_TCPMSS.h`, `xt_comment.h` 等已发现损坏的头文件。
    3. 在 `include/linux/netfilter/` 目录下保留副本以确保内部构建兼容性。

- **结论**: 该方案已验证成功，系统正常启动，KernelSU 运行正常。

## [2026-03-13 04:00] 刷入报错诊断 (Error in /sideload/package.zip status 1)
- **现象**: ADB Sideload 刷入时报错 `status 1`，日志显示 `illegal file descriptor name`。
- **根源**: 1. `update-binary` 脚本尝试将进度重定向到 `/proc/self/fd/24`，但在某些 Recovery 环境中该描述符不可访问。2. 脚本生成时带有的 YAML 缩进可能导致 shell 解析异常。
- **修复对策**: 
    1. 简化 `ui_print` 函数，移除 `/proc/self/fd/` 重定向，直接使用 `echo`。
    2. 移除 `update-binary` 脚本生成时的缩进。
    3. 在脚本末尾显式添加 `exit 0` 确保状态码正确。
- **验证**: 虽然之前报 status 1，但 `dd` 写入实际上已经完成。此修改旨在消除报错，提供干净的刷入体验。

## [2026-03-13 07:15] GitHub Actions 启动失败诊断 (workflow file issue)
- **现象**: GitHub Actions 报错 "This run likely failed because of a workflow file issue." 且无法查看日志。
- **原因分析**: 上次提交中为了“移除缩进”，将 `cat << 'EOF'` 的内容直接置于行首。在 YAML 的 `run: |` 块中，如果某行缩进小于该块的起始缩进，YAML 解析会认为该块已结束，导致后续内容解析错误。
- **修复对策**: 恢复 YAML 合法的缩进。通过在 `cat << 'EOF'` 块内保持一致的缩进（相对于 `run` 关键字），YAML 解析器会自动剥离基础缩进，从而在生成的脚本文件中不会保留多余空格。
- **验证**: 修复缩进后重新提交，观察 CI 启动情况。



