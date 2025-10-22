#!/bin/bash

# 设置交叉编译前缀
echo $PWD

export CROSS_COMPILE=${PWD}/arm-rockchip830-linux-uclibcgnueabihf/bin/arm-rockchip830-linux-uclibcgnueabihf-

# 设置编译器变量
export CC=${CROSS_COMPILE}gcc
export CXX=${CROSS_COMPILE}g++
export AR=${CROSS_COMPILE}ar
export STRIP=${CROSS_COMPILE}strip

# 验证设置
echo "CROSS_COMPILE: $CROSS_COMPILE"
echo "CC: $CC"
echo "CXX: $CXX"