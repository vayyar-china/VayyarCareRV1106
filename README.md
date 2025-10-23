# VayyarCareRV1106 HOW-TO

## Prerequisites

In RV1106 SDK root path, enter the `tools/linux/toolchain/arm-rockchip830-linux-uclibcgnueabihf` folder and run 
```console
source ./env_install_toolchain.sh /path/to/VayyarCareRV1106/toolchain
```

## Build
1. Enter `build` folder
2. Build project
```console
cmake ../src
make clean
make
```
## Deploy
1. Enter `Setup`folder
2. copy libraries in `lib` to `/usr/lib` on target board
3. copy executables in `bin` to `/oem` on target board
