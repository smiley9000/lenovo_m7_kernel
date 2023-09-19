#!/bin/bash

export PATH=$PWD/prebuilts/build-tools/path/linux-x86:$PATH
export ARCH=arm64
export CLANG_TRIPLE=aarch64-linux-gnu-
export CROSS_COMPILE_COMPAT=arm-linux-androideabi-
export CROSS_COMPILE=aarch64-linux-androidkernel-
export LD_LIBRARY_PATH=prebuilts/clang/host/linux-x86/clang-r383902b/lib64:$$LD_LIBRARY_PATH

cd kernel-4.19/
rm -rf out/

make ARCH=arm64 CC=$PWD/../prebuilts/clang/host/linux-x86/clang-r383902b/bin/clang O=out tb8768tp1_64_bsp_defconfig

make ARCH=arm64 CROSS_COMPILE=$PWD/../prebuilts/gcc/linux-x86/aarch64/aarch64-linux-android-4.9.1/bin/aarch64-linux-androidkernel- CLANG_TRIPLE=aarch64-linux-gnu- CC=$PWD/../prebuilts/clang/host/linux-x86/clang-r383902b/bin/clang  O=out -j12

