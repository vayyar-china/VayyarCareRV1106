cmake_minimum_required(VERSION 3.13)

set(PROJECT_ROOT ${PROJECT_SOURCE_DIR}/..)

get_filename_component(PROJECT_ROOT_ABSOLUTE "${PROJECT_ROOT}" ABSOLUTE)
set(PREBUILT_ROOT ${PROJECT_ROOT_ABSOLUTE}/prebuilt)

set(PREBUILT_INCLUDE_DIR "${PREBUILT_ROOT}/include")
set(PREBUILT_LIB_DIR "${PREBUILT_ROOT}/lib")

message(STATUS "Prebuild include dir is ${PREBUILT_INCLUDE_DIR}")
message(STATUS "Prebuild lib dir is ${PREBUILT_LIB_DIR}")

include_directories(
    "${PREBUILT_INCLUDE_DIR}"
)

link_directories(
    "${PREBUILT_LIB_DIR}"
)

add_subdirectory("component/restful")