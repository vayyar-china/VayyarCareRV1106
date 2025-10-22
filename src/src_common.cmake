cmake_minimum_required(VERSION 3.13)


get_filename_component(RV1106_SDK_ROOT_DIR "${PROJECT_ROOT}" ABSOLUTE)
set(RV1106_SDK_TOOLCHAIN_DIR ${RV1106_SDK_ROOT_DIR}/toolchain/arm-rockchip830-linux-uclibcgnueabihf)
set(RV1106_SDK_COMPILER_DIR ${RV1106_SDK_TOOLCHAIN_DIR}/bin)

set(PROJECT_COMPILER_PREFIX ${RV1106_SDK_COMPILER_DIR}/arm-rockchip830-linux-uclibcgnueabihf-)
set(CMAKE_C_COMPILER ${RV1106_SDK_COMPILER_DIR}/arm-rockchip830-linux-uclibcgnueabihf-gcc)
SET(CMAKE_CXX_COMPILER ${RV1106_SDK_COMPILER_DIR}/arm-rockchip830-linux-uclibcgnueabihf-g++)