cmake_minimum_required(VERSION 3.10)
project(NoWayOut)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_ASM_NASM_COMPILER nasm)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/CMake)

set(BUILD_STATIC_LIBS ON)
set(BUILD_SHARED_LIBS OFF)

add_definitions(-DASIO_STANDALONE)

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

set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
set(GLFW_VULKAN_STATIC OFF CACHE BOOL "" FORCE)
add_subdirectory(thirdparty/glfw)
include_directories(thirdparty/glfw/include)
link_libraries(glfw)

add_subdirectory(thirdparty/glad)
include_directories(thirdparty/glad)
link_libraries(glad)

find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})
link_libraries(${OpenCV_LIBS})

add_definitions(-DGLM_FORCE_AVX2)
add_subdirectory(thirdparty/glm)
link_libraries(glm_static)

include_directories(.)
add_subdirectory(core)
link_libraries(NoWayOutCore)

add_executable(Backend-Executable backend-main.cpp)

add_subdirectory(display)
add_executable(Display-Executable display-main.cpp)
target_link_libraries(Display-Executable NoWayOutDisplay)
