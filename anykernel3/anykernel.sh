# AnyKernel3 Image Install Script
# (c) 2019-2023 by osm0sis @ xda-developers

properties() { '
kernel.string=Kernel by Gemini CLI for Redmi K30S Ultra (apollon)
do.devicecheck=1
do.modules=0
do.systemless=1
do.cleanup=1
do.cleanuponabort=0
device.name1=apollo
device.name2=apollon
supported.versions=
supported.patchlevels=
'; }

# shell variables
block=/dev/block/bootdevice/by-name/boot;
dtbo=/dev/block/bootdevice/by-name/dtbo;
is_slot_device=auto;
ramdisk_compression=auto;
patch_vbmeta_flag=1;
# split_boot=0 (default) is better when we provide dtb/dtbo files

## AnyKernel methods (pre-built tools/busybox)
# import for functions
. tools/ak3-core.sh;

## AnyKernel install
dump_boot;
write_boot;

## DTBO install
# flash dtbo for SM8250 (Kona) devices
flash_dtbo;
