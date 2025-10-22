cmake_minimum_required(VERSION 3.13)

project(restful)

include(${PROJECT_ROOT}/src/src_common.cmake)


add_definitions(-Wall -Werror -std=c++17)

include_directories(
    "${PROJECT_SOURCE_DIR}/include"
)

set(DIR_SRCS
    source/Parameter.cpp
    source/Session.cpp
    source/Util.cpp
)

set(TARGETNAME "restful")
add_library(${TARGETNAME} SHARED ${DIR_SRCS})
# set(CURL_LIB ${PREBUILT_LIB_DIR}/libcurl.so.4.8.0)
# target_link_libraries(${TARGETNAME} PRIVATE ${CURL_LIB})
target_link_directories(${TARGETNAME} PRIVATE ${PREBUILT_LIB_DIR})
target_link_libraries(${TARGETNAME} PRIVATE curl ssl crypto)

add_custom_command (
    TARGET ${TARGETNAME}
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} ARGS -E make_directory ${SETUP_ROOT}/lib
    COMMAND rsync ARGS -a librestful.so ${SETUP_ROOT}/lib
    COMMAND ${CMAKE_COMMAND} ARGS -E make_directory ${SETUP_ROOT}/include
    COMMAND rsync ARGS -a ${PROJECT_SOURCE_DIR}/include/ ${SETUP_ROOT}/include
    COMMENT "copy generated restful component output to setup folder"
)
