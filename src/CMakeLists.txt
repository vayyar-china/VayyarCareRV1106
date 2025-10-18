cmake_minimum_required(VERSION 3.13)

set(PROJECT_ROOT ${PROJECT_SOURCE_DIR}/..)

get_filename_component(PROJECT_ROOT_ABSOLUTE "${PROJECT_ROOT}" ABSOLUTE)
set(PREBUILD_ROOT ${PROJECT_ROOT_ABSOLUTE}/prebuild)

set(PREBUILD_INCLUDE_DIR "${PREBUILD_ROOT}/include")
set(PREBUILD_LIB_DIR "${PREBUILD_ROOT}/lib")

message(STATUS "Prebuild include dir is ${PREBUILD_INCLUDE_DIR}")
message(STATUS "Prebuild lib dir is ${PREBUILD_LIB_DIR}")

include_directories(
    "${PREBUILD_INCLUDE_DIR}"
)

link_directories(
    "${PREBUILD_LIB_DIR}"
)
