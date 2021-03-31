cmake_minimum_required(VERSION 3.10)

set(CMAKE_BUILD_TYPE Debug)

project(boralago VERSION 0.1)


#set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

find_package(gflags REQUIRED)
find_package(glog 0.5.0 REQUIRED)
find_package(absl REQUIRED)
find_package(Protobuf REQUIRED)

# protobuf configuration
include_directories(${Protobuf_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_BINARY_DIR})  # TODO(aryap): Ugh.
# set(Protobuf_IMPORT_DIRS ...)
# set(Protobuf_DEBUG ...)
protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS proto/inflator_rules.proto)

# skia includes
set(Skia_BUILD_DIR "${CMAKE_SOURCE_DIR}/third_party/skia" CACHE PATH "Skia build directory" FORCE)

find_library(Skia_LIBRARY
  skia
  PATH ${Skia_BUILD_DIR}/out/Shared
  PATH ${Skia_BUILD_DIR}/out/Static
  PATH ${Skia_BUILD_DIR}/out/Release
  PATH ${Skia_BUILD_DIR}/trunk/out/Release)

add_executable(boralago
                src/cell.cc
                src/line.cc
                src/main.cc
                src/point.cc
                src/poly_line.cc
                src/polygon.cc
                src/renderer.cc
                src/stick_cell.cc
                src/stick_inflator.cc
                ${PROTO_SRCS}
                ${PROTO_HDRS})

target_include_directories(boralago PUBLIC
                           "${PROJECT_BINARY_DIR}/src"
                           ${Skia_BUILD_DIR}
                           ${Skia_BUILD_DIR}/include
                           )

find_library(tcmalloc_lib NAMES tcmalloc REQUIRED)
message(STATUS ${tcmalloc_lib})

target_link_libraries(boralago PUBLIC ${tcmalloc_lib}
                                      gflags
                                      glog::glog
                                      absl::strings
                                      ${Skia_LIBRARY}
                                      ${Protobuf_LIBRARIES})

configure_file(src/c_make_header.h.in src/c_make_header.h)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED true)
