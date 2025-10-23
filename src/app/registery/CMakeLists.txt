cmake_minimum_required(VERSION 3.13)

project(registry)

include(${PROJECT_ROOT}/src/src_common.cmake)

add_definitions(-Wall -Werror -std=c++17)

include_directories(
    "${PROJECT_SOURCE_DIR}/include"
)

set(DIR_SRCS
    source/main.cpp
    source/RestfulConnector.cpp
    source/Device.cpp
)

set(TARGETNAME "registery")
add_executable(${TARGETNAME} ${DIR_SRCS})

set(LIB_PREFIX)
target_link_directories(${TARGETNAME} PRIVATE ${PREBUILT_LIB_DIR})
target_link_libraries(
    ${TARGETNAME}
    curl
    ssl
    crypto
    jsoncpp
    paho-mqtt3cs
    libauthorization.so
    librestful.so
    libmqttclient.so
)

add_custom_command (
    TARGET ${TARGETNAME}
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} ARGS -E make_directory ${SETUP_ROOT}/bin
    COMMAND rsync ARGS -a ${TARGETNAME} ${SETUP_ROOT}/bin
    COMMENT "copy registery to setup folder"
)
