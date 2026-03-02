# Android Kernel Build Skill

## 用途
通过 GitHub Actions 构建适用于小米设备的 Android 内核（sm8250 平台），生成可刷入的 AnyKernel3 zip 包。

## 触发条件
- 构建 android kernel
- 编译内核
- 构建 AnyKernel3

## 构建失败时
如果构建失败，查看 GitHub Actions 运行记录：
https://github.com/dgmico/android_kernel_xiaomi_sm8250/actions

---

## 已知问题汇总

### 1. YAML 语法错误
- **原因**: heredoc 中嵌套的 Python 代码导致 YAML 解析失败
- **解决**: 使用 `printf` 分行写入脚本

### 2. 编译器未找到
- **原因**: Clang 17 自带交叉编译器路径不同，Kconfig 需要 GCC 风格工具链
- **解决**: 安装 gcc-aarch64-linux-gnu，使用 aarch64-linux-gnu- 作为 CROSS_COMPILE

### 3. defconfig 路径错误
- **原因**: kona_defconfig 位于 vendor 子目录
- **解决**: 使用完整路径 vendor/kona_defconfig

### 4. lockdep.c 编译错误
- **原因**: nested 参数已从 lock_release() 移除但调用处未更新
- **解决**: 修改 kernel/locking/lockdep.c 第 4008 行，将 nested 改为 0

### 5. trace.h 文件未找到
- **原因**: Clang 与 trace 头文件不兼容
- **解决**: 统一使用 GCC 替代 Clang

### 6. 警告被视为错误
- **原因**: CONFIG_CC_WERROR 启用
- **解决**: 添加 KCFLAGS="-Wno-error"

### 7. git clone 太慢
- **原因**: fetch-depth: 0 下载完整 git 历史
- **解决**: 使用 fetch-depth: 1 浅克隆

### 8. 部分步骤仍使用 Clang
- **原因**: olddefconfig 步骤仍使用 clang 作为 CC
- **解决**: 统一使用 GCC，移除 Clang 下载步骤

---

## 构建流程

### 1. 克隆源码
```bash
git clone https://github.com/dgmico/android_kernel_xiaomi_sm8250.git
cd android_kernel_xiaomi_sm8250
git checkout root
```

### 2. 修改代码（如有需要）
```bash
# 修改内核代码后提交
git add .
git commit -m "Your changes"
git push origin root
```

### 3. GitHub Actions 自动构建
- 推送代码后自动触发构建
- 或手动触发：https://github.com/dgmico/android_kernel_xiaomi_sm8250/actions → Build Kernel → Run workflow

### 4. 下载产物
构建完成后在 Actions 页面的 Artifacts 中下载 kernel-*.zip

---

## GitHub Actions 工作流配置

### 完整 workflow 文件 (.github/workflows/build-kernel.yml)
```yaml
name: Build Kernel

on:
  push:
    branches: [lineage-23.2, root]
  workflow_dispatch:
    inputs:
      device:
        description: 'Device codename'
        required: false
        default: 'apollo'

jobs:
  build:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v4
        with:
          fetch-depth: 1

      - name: Install dependencies
        run: |
          sudo apt-get update
          sudo apt-get install -y \
            build-essential bc bison flex \
            libssl-dev libelf-dev python3 \
            crossbuild-essential-arm64 zip \
            gcc-aarch64-linux-gnu

      - name: Setup defconfig
        run: |
          DEVICE="${{ github.event.inputs.device || 'apollo' }}"
          make CROSS_COMPILE=aarch64-linux-gnu- CC=gcc \
              ARCH=arm64 vendor/kona_defconfig
          if [ -f arch/arm64/configs/vendor/xiaomi/${DEVICE}.config ]; then
            cat arch/arm64/configs/vendor/xiaomi/${DEVICE}.config >> .config
          fi
          make CROSS_COMPILE=aarch64-linux-gnu- CC=gcc \
              ARCH=arm64 olddefconfig

      - name: Build kernel
        run: |
          make CROSS_COMPILE=aarch64-linux-gnu- \
              ARCH=arm64 KCFLAGS="-Wno-error" \
              -j$(nproc) Image.gz dtbs

      - name: Create AnyKernel3 package
        run: |
          DEVICE="${{ github.event.inputs.device || 'apollo' }}"
          VERSION=$(make kernelversion)
          mkdir -p AnyKernel3/dtbs
          cp arch/arm64/boot/Image.gz AnyKernel3/
          cp arch/arm64/boot/dts/vendor/qcom/${DEVICE}*.dtb AnyKernel3/dtbs/ 2>/dev/null || true
          cp arch/arm64/boot/dts/vendor/qcom/${DEVICE}*.dtbo AnyKernel3/dtbs/ 2>/dev/null || true
          # ... 创建 anykernel.sh ...
          zip -r kernel-${DEVICE}-${VERSION}.zip AnyKernel3/

      - uses: actions/upload-artifact@v4
        with:
          name: kernel-zip
          path: kernel-*.zip
```

---

## 刷入方式

```bash
# 通过 TWRP/KernelFlare
adb push kernel-apollo-*.zip /sdcard/
# 在 recovery 中刷入
```

---

## 支持的设备 (sm8250)

| 设备代号 | 设备名称 |
|---------|---------|
| apollo | 小米 10T/10T Pro |
| lmi | 小米 10 Pro |
| umi | 小米 10 |
| alioth | Redmi K40 Pro |
| cas | 小米 10S |
| cmi | 小米 10 Pro (海外版) |
| dagu | Redmi K50 |
| elish | Redmi K40 |
| enuma | POCO F3 |
| munch | Redmi K40 Pro+ |
| psyche | 小米 10T Pro |
| thyme | POCO F2 Pro |
