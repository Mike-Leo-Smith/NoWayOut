cmake_minimum_required(VERSION 3.15)
project(NoWayOut)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_ASM_NASM_COMPILER nasm)
set(CMAKE_INSTALL_PREFIX "${CMAKE_SOURCE_DIR}/install")

set(BUILD_SHARED_LIBS OFF)

add_compile_definitions(ASIO_STANDALONE)

include_directories(thirdparty)
include_directories(thirdparty/asio)

add_subdirectory(thirdparty/zstd/build/cmake)
include_directories(thirdparty/zstd/lib)
link_libraries(libzstd_static)

set(RAPIDJSON_BUILD_DOC OFF CACHE STRING "" FORCE)
set(RAPIDJSON_BUILD_EXAMPLES OFF CACHE STRING "" FORCE)
set(RAPIDJSON_BUILD_TESTS OFF CACHE STRING "" FORCE)
set(RAPIDJSON_BUILD_THIRDPARTY_GTEST OFF CACHE STRING "" FORCE)
set(RAPIDJSON_BUILD_CXX11 ON CACHE STRING "" FORCE)
set(RAPIDJSON_HAS_STDSTRING  ON CACHE STRING "" FORCE)
add_subdirectory(thirdparty/rapidjson)
include_directories(thirdparty/rapidjson/include)

add_subdirectory(thirdparty/libjpeg-turbo)
include_directories(thirdparty/libjpeg-turbo)
link_libraries(turbojpeg-static)

find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})
link_libraries(${OpenCV_LIBS})

add_executable(Receiver receiver.cpp)