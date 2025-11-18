cmake_minimum_required(VERSION 3.13)

project(log_client)

include(${PROJECT_ROOT}/src/src_common.cmake)


add_definitions(-Wall -Werror )

include_directories(
    "${PROJECT_SOURCE_DIR}/include"
)

set(DIR_SRCS    
    source/log_client.c
)

set(TARGETNAME "log_client")
add_library(${TARGETNAME} SHARED ${DIR_SRCS})
target_link_directories(${TARGETNAME} PRIVATE ${PREBUILT_LIB_DIR})
target_link_libraries(${TARGETNAME}  PRIVATE )

add_custom_command (
    TARGET ${TARGETNAME}
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} ARGS -E make_directory ${SETUP_ROOT}/lib
    COMMAND rsync ARGS -a liblog_client.so ${SETUP_ROOT}/lib
    COMMAND ${CMAKE_COMMAND} ARGS -E make_directory ${SETUP_ROOT}/include
    COMMAND rsync ARGS -a ${PROJECT_SOURCE_DIR}/include/ ${SETUP_ROOT}/include
    COMMENT "copy generated log_client component output to setup folder"
)