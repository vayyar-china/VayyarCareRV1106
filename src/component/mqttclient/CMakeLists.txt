cmake_minimum_required(VERSION 3.13)

project(mqttclient)

include(${PROJECT_ROOT}/src/src_common.cmake)


add_definitions(-Wall -Werror -std=c++17)

include_directories(
    "${PROJECT_SOURCE_DIR}/include"
)

set(DIR_SRCS
    source/MqttClient.cpp
    source/MqttClientManager.cpp
)

set(TARGETNAME "mqttclient")
add_library(${TARGETNAME} SHARED ${DIR_SRCS})
target_link_directories(${TARGETNAME} PRIVATE ${PREBUILT_LIB_DIR})
target_link_libraries(${TARGETNAME} 
    PRIVATE 
    ssl 
    crypto
    paho-mqtt3cs 
)

add_custom_command (
    TARGET ${TARGETNAME}
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} ARGS -E make_directory ${SETUP_ROOT}/lib
    COMMAND rsync ARGS -a libmqttclient.so ${SETUP_ROOT}/lib
    COMMAND ${CMAKE_COMMAND} ARGS -E make_directory ${SETUP_ROOT}/include
    COMMAND rsync ARGS -a ${PROJECT_SOURCE_DIR}/include/ ${SETUP_ROOT}/include
    COMMENT "copy generated mqttclient component output to setup folder"
)