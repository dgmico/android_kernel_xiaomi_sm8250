# Android Kernel Build Skill

## 用途
构建适用于小米设备的 Android 内核（sm8250 平台），生成可刷入的 AnyKernel3 zip 包。

## 触发条件
- 构建 android kernel
- 编译内核
- 构建 AnyKernel3

---

## 问题记录
遇到的问题记录在 `.claude/troubleshooting/` 目录下，按编号命名：
- `001-yaml-syntax-error.md` - YAML 语法错误
- `002-compiler-not-found.md` - 编译器未找到
- `003-defconfig-path-error.md` - defconfig 路径错误
- `004-lockdep-error.md` - lockdep.c 编译错误
- `005-traceh-not-found.md` - trace.h 文件未找到
- `006-werror-warning.md` - 警告被视为错误
- `007-git-clone-slow.md` - git clone 太慢

---

## 构建流程

### 1. 环境配置
```bash
# 安装依赖 (Ubuntu/Debian)
sudo apt-get update
sudo apt-get install -y \
  build-essential bc bison flex \
  libssl-dev libelf-dev python3 \
  crossbuild-essential-arm64 zip \
  gcc-aarch64-linux-gnu
```

### 2. 克隆源码
```bash
git clone https://github.com/LineageOS/android_kernel_xiaomi_sm8250.git
cd android_kernel_xiaomi_sm8250
git checkout lineage-23.2
```

### 3. 配置并编译
```bash
# 配置 (使用 kona_defconfig 作为基础)
make CROSS_COMPILE=aarch64-linux-gnu- ARCH=arm64 vendor/kona_defconfig

# 合并设备配置 (可选)
cat arch/arm64/configs/vendor/xiaomi/apollo.config >> .config
make CROSS_COMPILE=aarch64-linux-gnu- ARCH=arm64 olddefconfig

# 编译 (禁用 Werror 避免警告失败)
make CROSS_COMPILE=aarch64-linux-gnu- ARCH=arm64 \
  KCFLAGS="-Wno-error" -j$(nproc) Image.gz dtbs
```

### 4. 打包 AnyKernel3
```bash
DEVICE=apollo
VERSION=$(make kernelversion)

mkdir -p AnyKernel3/dtbs
cp arch/arm64/boot/Image.gz AnyKernel3/
cp arch/arm64/boot/dts/vendor/qcom/${DEVICE}*.dtb AnyKernel3/dtbs/
cp arch/arm64/boot/dts/vendor/qcom/${DEVICE}*.dtbo AnyKernel3/dtbs/

# 创建 anykernel.sh
cat > AnyKernel3/anykernel.sh << 'EOF'
properties() {
  kernel.string=Kernel
  kernel.device=DEVICE_PLACEHOLDER
  kernel.version=VERSION_PLACEHOLDER
  kernel.arch=arm64
  kernel.platform=android
}
EOF

zip -r kernel-${DEVICE}-${VERSION}.zip AnyKernel3/
```

---

## GitHub Actions 工作流

### 完整 workflow 文件
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
          make CROSS_COMPILE=aarch64-linux-gnu- ARCH=arm64 olddefconfig

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
# 通过 fastboot (需要 boot.img)
fastboot flash boot boot.img

# 通过 TWRP/KernelFlare (使用 zip)
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
