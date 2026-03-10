# Build Errors Log

## [2026-03-10 07:00] 错误诊断
- **错误原文**: `gzip: stdin: not in gzip format`
- **原因分析**: Clang 下载链接失效或返回非压缩格式；Telegram 脚本语法不兼容 POSIX sh。
- **修复对策**: 使用更稳定的工具链下载方式；将工作流 shell 统一指定为 bash，修复变量截断。

## [2026-03-10 07:03] 错误诊断
- **错误原文**: `错误: 下载的文件过小，链接可能已失效。`
- **原因分析**: Google Git 的 +archive 接口不支持直接 curl 下载大体积存档，返回了 HTML 页面。
- **修复对策**: 改用 git clone --depth 1 获取 Clang 工具链；确保工具链路径正确。
