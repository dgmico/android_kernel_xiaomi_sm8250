# 内核编译错误记录

... (Keep previous entries)

---

## 2026-03-07 23:45 - Run 22803370353

**错误**: `net/netfilter/xt_connmark.c: error: 'D_SHIFT_RIGHT' undeclared` 等多处 Netfilter 编译错误。

**原因**: 
1. 在 macOS 大小写不敏感文件系统上，UAPI 中多组大小写敏感头文件（`xt_mark.h`/`xt_MARK.h` 等）在 Git 索引中发生冲突，导致关键结构体和宏定义丢失或包含错误。
2. 之前的修复方案尝试分离 Match 和 Target 定义，但在 macOS 上依然会导致循环包含或内容覆盖。
3. 部分 `.c` 源文件（如 `xt_dscp.c`, `xt_rateest.c`, `xt_tcpmss.c`）在 Git 索引中内容重复，导致 Match 模块的代码实际上已经丢失。

**修复**: 
1. **头文件统一化**: 为所有 5 对大小写敏感头文件分配包含 **Match + Target 全量定义** 的统一 Git blob。这彻底解决了 macOS 上的冲突和循环包含问题，同时确保 Linux 编译环境能获取完整定义。
2. **源文件修复**: 使用 `git update-index --cacheinfo` 恢复 Git 索引中丢失的 Match 模块源文件内容（`xt_dscp.c`, `xt_rateest.c`, `xt_tcpmss.c`），确保它们具有独立且正确的匹配逻辑。
3. **移除 Workaround**: 撤销之前在各 `.c` 模块中添加的所有临时结构体定义，恢复代码纯净度。
4. **覆盖模块**: `mark`, `connmark`, `DSCP`, `RATEEST`, `TCPMSS`, `SECMARK`, `CONNSECMARK`。

**修改文件**:
- `include/uapi/linux/netfilter/xt_mark.h` / `xt_MARK.h` (Index)
- `include/uapi/linux/netfilter/xt_connmark.h` / `xt_CONNMARK.h` (Index)
- `include/uapi/linux/netfilter/xt_dscp.h` / `xt_DSCP.h` (Index)
- `include/uapi/linux/netfilter/xt_rateest.h` / `xt_RATEEST.h` (Index)
- `include/uapi/linux/netfilter/xt_tcpmss.h` / `xt_TCPMSS.h` (Index)
- `net/netfilter/xt_mark.c`
- `net/netfilter/xt_connmark.c`
- `net/netfilter/xt_DSCP.c` / `xt_dscp.c` (Index)
- `net/netfilter/xt_RATEEST.c` / `xt_rateest.c` (Index)
- `net/netfilter/xt_TCPMSS.c` / `xt_tcpmss.c` (Index)
