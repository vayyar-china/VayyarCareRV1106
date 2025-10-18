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
set(CURL_LIB ${PREBUILT_LIB_DIR}/libcurl.so.4.8.0)
target_link_libraries(${TARGETNAME} PRIVATE ${CURL_LIB})


