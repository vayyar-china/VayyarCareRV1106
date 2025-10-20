cmake_minimum_required(VERSION 3.13)

project(authorization)

include(${PROJECT_ROOT}/src/src_common.cmake)

add_definitions(-Wall -Werror -std=c++17)

include_directories(
    "${PROJECT_SOURCE_DIR}/include"
)

set(DIR_SRCS
    source/Auth.cpp
)

set(TARGETNAME "authorization")
add_library(${TARGETNAME} SHARED ${DIR_SRCS})
target_link_directories(${TARGETNAME} PRIVATE ${PREBUILT_LIB_DIR})
target_link_libraries(${TARGETNAME} PRIVATE libssl.so.3 libcrypto.so.3)
