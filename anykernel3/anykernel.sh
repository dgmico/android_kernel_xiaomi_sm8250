# AnyKernel3 Image Install Script
# (c) 2019-2023 by osm0sis @ xda-developers

properties() { '
kernel.string=Minimal Kernel for apollon
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
is_slot_device=0;
ramdisk_compression=auto;
patch_vbmeta_flag=1;

## AnyKernel methods (pre-built tools/busybox)
# import for functions
. tools/ak3-core.sh;

## AnyKernel install
dump_boot;
write_boot;
