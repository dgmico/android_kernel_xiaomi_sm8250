# 合并上游代码助手 (Xiaomi SM8250)

**目的**: 引导用户在 macOS 上安全地从上游仓库合并代码，防止因文件系统大小写不敏感导致已修复的 Netfilter 漏洞复现。

## 核心风险
macOS 磁盘不区分 `xt_mark.h` 和 `xt_MARK.h`。合并后：
1. **头文件**: 上游会覆盖索引，导致 macOS 磁盘上的文件发生冲突，`incomplete type` 错误可能回归。
2. **源文件**: `xt_dscp.c` (Match) 和 `xt_DSCP.c` (Target) 会在磁盘上互相覆盖，导致逻辑丢失。

## 工作流程

### 1. 执行合并
```bash
# 假设 upstream 是上游远程名，branch 是分支
git fetch upstream
git merge upstream/branch --no-commit
```

### 2. 识别大小写冲突对
运行以下命令找出索引中重复的文件名（忽略大小写）：
```bash
git ls-files -s | cut -f2 | tr '[:upper:]' '[:lower:]' | sort | uniq -d
```

### 3. 应用“哈希保护”策略
**严禁执行 `git add .`！**

对于冲突的文件对，必须手动恢复索引：

#### A. 修复 UAPI 头文件 (需指向包含 Match+Target 全量定义的统一 Blob)
参考 `build-errors.md` 记录的最新哈希，或从上一个成功提交获取：
```bash
# 示例：手动更新索引
git update-index --cacheinfo 100644 <HASH> include/uapi/linux/netfilter/xt_mark.h
git update-index --cacheinfo 100644 <HASH> include/uapi/linux/netfilter/xt_MARK.h
```

#### B. 修复源文件 (Match 和 Target 逻辑分离)
```bash
# 示例：恢复 xt_dscp.c (Match) 的独立代码
git update-index --cacheinfo 100644 <MATCH_HASH> net/netfilter/xt_dscp.c
git update-index --cacheinfo 100644 <TARGET_HASH> net/netfilter/xt_DSCP.c
```

### 4. 验证索引完整性
在提交前执行：
```bash
# 确认大小写对是否在索引中同时存在且 SHA1 正确
git ls-files -s | grep -i xt_mark.h
```

### 5. 完成合并
```bash
git commit -m "Merge upstream: 同步最新代码并重新应用 macOS 大小写冲突补丁"
```

## 维护建议
- 每次成功构建后，将 Netfilter 相关文件的 SHA1 哈希记录在 `build-errors.md` 中作为“锚点”。
- 如果合并后 Action 报错，直接回到 `build-errors.md` 找回哈希并运行 `update-index`。
