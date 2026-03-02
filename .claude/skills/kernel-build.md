# Android Kernel Build Skill

## 用途
构建适用于小米设备的 Android 内核（sm8250 平台），生成可刷入的 AnyKernel3 zip 包。

## 触发条件
- 构建 android kernel
- 编译内核
- 构建 AnyKernel3

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

## 常见问题记录

### 问题 1: YAML 语法错误
- **错误**: `Invalid workflow file - You have an error in your yaml syntax`
- **原因**: heredoc 中的嵌套内容导致 YAML 解析失败
- **解决**: 使用 `printf` 分行写入脚本，或将脚本内容改为单行

### 问题 2: 编译器未找到
- **错误**: `compiler '.../aarch64-linux-gnu-gcc' not found`
- **原因**:
  1. Clang 17 自带交叉编译器，但路径命名不同
  2. Kconfig 需要 GCC 风格工具链
- **解决**: 安装 `gcc-aarch64-linux-gnu`，使用 `aarch64-linux-gnu-` 作为 CROSS_COMPILE

### 问题 3: defconfig 路径错误
- **错误**: `Can't find default configuration "arch/arm64/configs/kona_defconfig"`
- **原因**: kona_defconfig 在 vendor 子目录
- **解决**: 使用完整路径 `vendor/kona_defconfig`

### 问题 4: lockdep.c 编译错误
- **错误**: `error: use of undeclared identifier 'nested'`
- **原因**: 内核源码 bug，nested 参数已移除但调用处未更新
- **解决**: 修改 `kernel/locking/lockdep.c` 第 4008 行
  ```c
  // 将
  if (__lock_release(lock, nested, ip))
  // 改为
  if (__lock_release(lock, 0, ip))
  ```

### 问题 5: trace.h 文件未找到
- **错误**: `fatal error: './trace.h' file not found`
- **原因**: Clang 与内联汇编或 trace 头文件不兼容
- **解决**:
  1. 使用 GCC 替代 Clang
  2. 或添加 `KCFLAGS="-Wno-error"` 忽略警告

### 问题 6: 警告被视为错误
- **错误**: `error: this 'if' clause does not guard... [-Werror=misleading-indentation]`
- **原因**: CONFIG_CC_WERROR 启用，警告被当作错误
- **解决**: 添加 `KCFLAGS="-Wno-error"` 禁用

### 问题 7: git clone 太慢
- **原因**: `fetch-depth: 0` 下载完整 git 历史
- **解决**: 使用 `fetch-depth: 1` 浅克隆

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
