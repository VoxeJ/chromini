cmake_minimum_required(VERSION 3.10)
project(chromini)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

if("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
    message(FATAL_ERROR "In-source building is ill-advides")
endif()

set(ZLIB_ROOT "${CMAKE_SOURCE_DIR}/zlib")
set(ZLIB_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/zlib")
set(ZLIB_LIBRARY "${ZLIB_ROOT}/libzs.a")

set(PNG_INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/libpng" "${CMAKE_BINARY_DIR}/libpng")

link_directories(${CMAKE_BINARY_DIR}/libpng)
link_directories(${CMAKE_BINARY_DIR}/zlib)

add_subdirectory(zlib)
add_subdirectory(libpng)

add_executable(chromini main.cpp)

target_link_libraries(chromini PRIVATE png zs)

target_include_directories(chromini PRIVATE ${ZLIB_INCLUDE_DIR} ${PNG_INCLUDE_DIRS})

add_dependencies(chromini png_static)
