# 内核编译错误记录

... (Keep previous entries)

---

## 2026-03-08 01:45 - Run 22803716292

**错误**: `drivers/kernelsu/selinux/sepolicy.c:510:31: error: storage size of 'key' isn't known` 及大量 SELinux 结构体成员缺失错误。

**原因**: 
1. KernelSU 的 `sepolicy.c` 完整实现是针对较新版本内核（5.x+）设计的，与 4.19 内核的 SELinux 内部结构体定义（如 `filename_trans_key`, `policydb`）不兼容。
2. 之前虽然恢复了 `sepolicy.c` 的实现，但导致了编译失败。

**修复**: 
1. 在 4.19 内核中为 `sepolicy.c` 提供完整的函数 Stub。
2. 涵盖 `rules.c` 和 `handle_sepolicy` 调用所需的所有 `ksu_*` 符号（如 `ksu_allow`, `ksu_typeattribute` 等），确保内核链接成功。
3. 这种方式会禁用 KernelSU 在 4.19 内核上的动态 SELinux 补丁功能，但解决了编译阻塞问题。

**修改文件**:
- `drivers/kernelsu/selinux/sepolicy.c` - 替换为 4.19 兼容的完整 Stub 实现。
