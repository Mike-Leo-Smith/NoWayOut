cmake_minimum_required(VERSION 3.15)
project(NoWayOut)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_ASM_NASM_COMPILER nasm)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/CMake)

set(BUILD_STATIC_LIBS ON)
set(BUILD_SHARED_LIBS OFF)

add_compile_definitions(ASIO_STANDALONE)

include_directories(thirdparty)
include_directories(thirdparty/asio)

set(ZSTD_BUILD_PROGRAMS OFF CACHE BOOL "" FORCE)
set(ZSTD_BUILD_STATIC ON CACHE BOOL "" FORCE)
set(ZSTD_BUILD_SHARED OFF CACHE BOOL "" FORCE)
add_subdirectory(thirdparty/zstd/build/cmake)
include_directories(thirdparty/zstd/lib)
link_libraries(libzstd_static)

include_directories(thirdparty/rapidjson/include)

find_package(TurboJPEG REQUIRED)
include_directories(${TurboJPEG_INCLUDE_DIRS})
link_libraries(${TurboJPEG_LIBRARIES})

find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})
link_libraries(${OpenCV_LIBS})

add_compile_definitions(GLM_FORCE_AVX2)
add_subdirectory(thirdparty/glm)
link_libraries(glm_static)

add_subdirectory(core)
include_directories(.)
link_libraries(NoWayOutCore)

add_executable(NoWayOut main.cpp)
