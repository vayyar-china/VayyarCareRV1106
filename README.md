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
make install
```

