cmake_minimum_required(VERSION 3.10)

set(CMAKE_BUILD_TYPE Debug)

project(boralago VERSION 0.1)

configure_file(src/c_make_header.h.in src/c_make_header.h)

find_package(gflags REQUIRED)
find_package(glog 0.5.0 REQUIRED)
find_package(absl REQUIRED)

add_executable(boralago
                src/main.cc)

target_include_directories(boralago PUBLIC
                           "${PROJECT_BINARY_DIR}/src"
                           )

find_library(tcmalloc_lib NAMES tcmalloc REQUIRED)
message(STATUS ${tcmalloc_lib})

target_link_libraries(boralago PUBLIC ${tcmalloc_lib}
                                      gflags
                                      glog::glog
                                      absl::strings)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED true)
