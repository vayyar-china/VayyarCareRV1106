cmake_minimum_required(VERSION 3.13)

project(registry)

include(${PROJECT_ROOT}/src/src_common.cmake)

add_definitions(-Wall -Werror -std=c++17)

include_directories(
    "${PROJECT_SOURCE_DIR}/include"
)

set(DIR_SRCS
    # source/main.cpp
    source/test-publisher.cpp
    # source/test-subscriber.cpp
)

# set(TARGETNAME "mqttadapter")
set(TARGETNAME "testpublisher")
# set(TARGETNAME "testsubscriber")
add_executable(${TARGETNAME} ${DIR_SRCS})

set(LIB_PREFIX)
target_link_directories(${TARGETNAME} PRIVATE ${PREBUILT_LIB_DIR})
target_link_libraries(
    ${TARGETNAME}
    mosquitto
    ssl
    crypto
    libmessagebusmosq.so
)

add_custom_command (
    TARGET ${TARGETNAME}
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} ARGS -E make_directory ${SETUP_ROOT}/bin
    COMMAND rsync ARGS -a ${TARGETNAME} ${SETUP_ROOT}/bin
    COMMENT "copy mqttadapter to setup folder"
)
