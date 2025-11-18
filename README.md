# VayyarCareRV1106 HOW-TO

## Prerequisites
1. tar -xzf ./toolchain.tar.gz
2. cd toolchain/arm-rockchip830-linux-uclibcgnueabihf
    source ./env_install_toolchain.sh /opt/toolchain/
3. cd ../../
    source ./env_setup.sh

## Build
1. cd VayyarCareRV1106-main
   mkdir build
   cd build
   rm -rf *
   rm -rf ../setup/*

2. Build project
   cmake ../src
   make clean
   make

## Deploy
1. Enter `Setup`folder
2. copy libraries in `lib` to `/usr/lib` on target board
3. copy executables in `bin` to `/oem` on target board

## Tips
1. mkdir a folder named `ssl` in the same path as `registery`, and copy `/prebuilt/etc/openssl.cnf` to the folder
2. mkdir a folder as the path `/etc/ssl/certs`, and copy `/prebuilt/etc/ca-certificates.crt` to the folder