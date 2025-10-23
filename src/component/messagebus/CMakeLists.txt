cmake_minimum_required(VERSION 3.13)

project(messagebus)

include(${PROJECT_ROOT}/src/src_common.cmake)


add_definitions(-Wall -Werror -std=c++17)

include_directories(
    "${PROJECT_SOURCE_DIR}/include"
)

set(DIR_SRCS
    source/mb_context.cpp
    source/mb_publisher.cpp
    source/mb.cpp
    source/mb_global.cpp
    source/mb_puller.cpp
    source/mb_pusher.cpp
    source/mb_subscriber.cpp
    source/mb_util.cpp
)

set(TARGETNAME "messagebus")
add_library(${TARGETNAME} SHARED ${DIR_SRCS})
target_link_directories(${TARGETNAME} PRIVATE ${PREBUILT_LIB_DIR})
target_link_libraries(${TARGETNAME} 
    PRIVATE 
    zmq
)

add_custom_command (
    TARGET ${TARGETNAME}
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} ARGS -E make_directory ${SETUP_ROOT}/lib
    COMMAND rsync ARGS -a libmessagebus.so ${SETUP_ROOT}/lib
    COMMAND ${CMAKE_COMMAND} ARGS -E make_directory ${SETUP_ROOT}/include
    COMMAND rsync ARGS -a ${PROJECT_SOURCE_DIR}/include/ ${SETUP_ROOT}/include
    COMMENT "copy generated messagebus component output to setup folder"
)