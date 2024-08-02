#!/bin/sh
#
#
#   我自己用的编译脚本
#   如果你想用，没问题，自己改改就行了
echo "\e[1m***************************"
echo "\e[1mZkernel Build Script"
echo "\e[1mKernel version:\e[5m"
make kernelversion
echo "\e[0m\e[1m***************************"
export PATH=$PATH:/home/killerz/e9/aarch64-linux-android-4.9/bin
export CROSS_COMPILE=aarch64-linux-android-
export ARCH=arm64
export GCC_COLORS=auto
rm -rf ./out
mkdir out
make mrproper
make ARCH=arm64 O=./out merge_kirin980_defconfig
make ARCH=arm64 O=./out -j20
cd tools
./pack_kernerimage_cmd.sh
ls -al kernel.img
