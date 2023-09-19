#!/bin/bash

#
# file create at 2018.01.09 for build bootimage
#

#
# set build env
#
function setEnv() {
    myDir="."
    toolDir="$myDir/prebuilts/tools"
    sourceDir="$myDir/kernel-4.19"
    outDir="$myDir/out"
    ramdikDir="$myDir/prebuilts/ramdisk"

    export PATH=$toolDir/bin:`pwd`/tools/gcc/bin:$PATH

    # [ -d "$outDir" ] && `rm -rf $outDir boot.img`
    # mkdir $outDir
}

#
# build all kernel source code
#
function build_kernel() {
    TOP="`pwd`" make -f $sourceDir/Android.mk \
            PRODUCT_OUT=$outDir \
            KERNEL_TARGET_ARCH=arm \
            TARGET_KERNEL_USE_CLANG=true \
            TARGET_BOARD_PLATFORM=mt6761 \
            PLATFORM_DTB_NAME=mt6761 \
            MTK_PLATFORM_DIR=mt6761 \
            BOARD_PREBUILT_DTBIMAGE_DIR=$outDir/obj/PACKAGING/dtb \
            KERNEL_DEFCONFIG=m7_defconfig \
            LINUX_KERNEL_VERSION=kernel-4.19 \
            PROJECT_DTB_NAMES=m7 \
            TARGET_OUT_INTERMEDIATES=$outDir/obj \
			-j `cat /proc/cpuinfo | grep processor| wc -l`  kernel

	if [ $? -ne 0 ];then
        echo -e "\nKernel build failed !!!\n" && exit
    fi

    kernel_build_pass=1
}

#
# build dtb
#
function build_dtb() {
    if [ $kernel_build_pass -eq 1 ];then
        mkdtimg cfg_create $outDir/dtb.img $outDir/obj/KERNEL_OBJ/arch/arm/boot/dts/dtbimg.cfg
        if [ $? -ne 0 ];then
            echo -e "\ndtb build failed !!!\n" && exit
        fi
    fi
}

#
# pkg all config file
#
function build_ramdisk() {
    cd $ramdikDir
    `rm -rf d` && `ln -s /sys/kernel/debug d`
    `rm -rf bin` && `ln -s /system/bin bin`
    `rm -rf cache` && `ln -s /data/cache cache`
    `rm -rf system_ext` && `ln -s /system_ext system_ext`
    `rm -rf bugreports` &&
        `ln -s /data/user_de/0/com.android.shell/files/bugreports bugreports`
    `rm -rf etc` && `ln -s /system/etc etc`
    `rm -rf sdcard` && `ln -s /storage/self/primary sdcard`
    cd -
    
    rm -rf $outDir/ramdisk-recovery.img
    mkbootfs -d $outDir $ramdikDir |
         minigzip > $outDir/ramdisk-recovery.img

    if [ $? -eq 0 ];then
        ramdisk_build_pass=1
    fi
}

#
# zip kernel and ramdisk to boot.img
#
function build_bootimage() {
    rm -rf boot.img
    $pwd
    mkbootimg --kernel  $outDir/kernel \
        --ramdisk $outDir/ramdisk-recovery.img \
        --cmdline "bootopt=64S3,32S1,32S1 buildvariant=userdebug" \
        --base 0x40000000 \
        --dtb $outDir/dtb.img \
        --os_version 11 \
        --os_patch_level 2021-01-05 \
        --kernel_offset 0x00008000 \
        --ramdisk_offset 0x11B00000 \
        --tags_offset 0x07880000 \
        --header_version 2 \
        --dtb_offset 0x07880000 \
        --output  boot.img \
    && \
    avbtool add_hash_footer --image \
        boot.img \
	--partition_size   33554432 \
	--partition_name boot \
	--algorithm SHA256_RSA2048 \
	--key $toolDir/boot_prvk.pem \
	--prop com.android.build.boot.fingerprint:Lenovo/LenovoTB-7306X_GO/7306X:11/RP1A.200720.011/:userdebug/test-keys \
	--prop com.android.build.boot.os_version:11 \
	--prop com.android.build.boot.security_patch:2019-06-06 \
	--rollback_index 0 \

    if [ -e "boot.img" ];then
        boot_image_pass=1
    fi
}



#
# show build result
#
function show_result() {
    if [ -e "boot.img" ];then
        echo -e "\nboot.img build pass, file path is: `pwd`/boot.img\n"
    else
        echo -e "\nbuild failed !!!"
    fi
}

function start_build() {
    setEnv
    build_kernel
    build_dtb
    build_ramdisk
    build_bootimage
    show_result
}

#
# real build
#
start_build
