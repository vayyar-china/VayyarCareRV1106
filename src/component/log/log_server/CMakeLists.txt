cmake_minimum_required(VERSION 3.13)

project(log_server)

include(${PROJECT_ROOT}/src/src_common.cmake)

add_definitions(-Wall -Werror -std=c++17)

include_directories(
    "${PROJECT_SOURCE_DIR}/include"
)

set(DIR_SRCS    
    source/log_server.cpp
)

set(TARGETNAME "log_server")
add_executable(${TARGETNAME} ${DIR_SRCS})
target_link_directories(${TARGETNAME} PRIVATE ${PREBUILT_LIB_DIR})
target_link_libraries(${TARGETNAME}  ssl crypto jsoncpp mosquitto paho-mqtt3cs libauthorization.so libmqttclient.so)

add_custom_command (
    TARGET ${TARGETNAME}
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} ARGS -E make_directory ${SETUP_ROOT}/bin
    COMMAND rsync ARGS -a ${TARGETNAME} ${SETUP_ROOT}/bin
    COMMENT "copy generated log_server component output to setup folder"
)