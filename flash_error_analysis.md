# 刷机包安装失败分析报告 (adb sideload Error 21/Status 1)

## 故障现象
在执行 `adb sideload apollo-20260311-0742.zip` 时，Recovery 提示：
`ERROR: recovery: Error in /sideload/package.zip (status 1)`

## 核心日志提取 (Recovery.log)
- **阶段 1 (挂载)**: `mount: can't find /system_root in /etc/fstab` (非致命，后续通过直接挂载绕过)。
- **阶段 2 (中断)**: 脚本在初始化 AnyKernel 环境时突然中止，没有输出 "Installing..." 之后的逻辑。
- **设备属性校验**:
    - `ro.build.ab_update=false` (A-only 分区方案)。
    - `ro.boot.slot_suffix=` (空后缀)。
    - `ro.lineage.device=apollon` (LineageOS 定义的机型代号)。

## 根源诊断 (Root Cause)
1. **A/B 分区逻辑冲突**: `anykernel.sh` 脚本中硬编码了 `is_slot_device=1;`。在 AnyKernel3 的逻辑中，如果开启了此项但系统属性中没有发现槽位后缀（Slot Suffix），脚本会直接调用 `abort` 报错退出。
2. **机型代号未定义**: 刷机包中定义的机型列表（`device.name1` 到 `device.name12`）包含了 `apollo`，但未显式包含 LineageOS 使用的 `apollon` 代号。如果 `do.devicecheck=1` 开启，这可能触发机型校验失败。

## 修复建议 (Action Plan)
1. **禁用 Slot 强制检查**: 将 `anykernel.sh` 中的 `is_slot_device=1;` 修改为 `0` 或 `auto`。
2. **兼容机型代号**: 在机型列表中添加 `apollon`。
3. **重新打包**: 更新 ZIP 包内的脚本。
