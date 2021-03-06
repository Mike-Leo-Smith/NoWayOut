cmake_minimum_required(VERSION 3.10)
project(NoWayOut)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_ASM_NASM_COMPILER nasm)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/CMake)

set(BUILD_STATIC_LIBS ON)
set(BUILD_SHARED_LIBS OFF)

if (WIN32)
    macro(get_WIN32_WINNT version)
        if (CMAKE_SYSTEM_VERSION)
            set(ver ${CMAKE_SYSTEM_VERSION})
            string(REGEX MATCH "^([0-9]+).([0-9])" ver ${ver})
            string(REGEX MATCH "^([0-9]+)" verMajor ${ver})
            # Check for Windows 10, b/c we'll need to convert to hex 'A'.
            if ("${verMajor}" MATCHES "10")
                set(verMajor "A")
                string(REGEX REPLACE "^([0-9]+)" ${verMajor} ver ${ver})
            endif ("${verMajor}" MATCHES "10")
            # Remove all remaining '.' characters.
            string(REPLACE "." "" ver ${ver})
            # Prepend each digit with a zero.
            string(REGEX REPLACE "([0-9A-Z])" "0\\1" ver ${ver})
            set(${version} "0x${ver}")
        endif(CMAKE_SYSTEM_VERSION)
    endmacro(get_WIN32_WINNT)
    get_WIN32_WINNT(ver)
    add_definitions(-D_WIN32_WINNT=${ver})
endif(WIN32)
add_definitions(-DASIO_STANDALONE)
include_directories(thirdparty)
include_directories(thirdparty/asio)

set(ZSTD_BUILD_PROGRAMS OFF CACHE BOOL "" FORCE)
set(ZSTD_BUILD_STATIC ON CACHE BOOL "" FORCE)
set(ZSTD_BUILD_SHARED OFF CACHE BOOL "" FORCE)
add_subdirectory(thirdparty/zstd/build/cmake)
include_directories(thirdparty/zstd/lib)
link_libraries(libzstd_static)

if (MSVC)
    add_definitions(-D_SILENCE_CXX17_ITERATOR_BASE_CLASS_DEPRECATION_WARNING)
endif ()
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

set(USE_DOUBLE_PRECISION OFF CACHE BOOL "" FORCE)
set(USE_GRAPHICAL_BENCHMARK OFF CACHE BOOL "" FORCE)
set(BULLET2_MULTITHREADING OFF CACHE BOOL "" FORCE)
set(BUILD_CPU_DEMOS OFF CACHE BOOL "" FORCE)
set(USE_GLUT OFF CACHE BOOL "" FORCE)
set(BUILD_BULLET3 OFF CACHE BOOL "" FORCE)
set(BUILD_PYBULLET OFF CACHE BOOL "" FORCE)
set(BUILD_ENET OFF CACHE BOOL "" FORCE)
set(BUILD_CLSOCKET OFF CACHE BOOL "" FORCE)
set(BUILD_EGL OFF CACHE BOOL "" FORCE)
set(BUILD_OPENGL3_DEMOS OFF CACHE BOOL "" FORCE)
set(BUILD_BULLET2_DEMOS OFF CACHE BOOL "" FORCE)
set(BUILD_EXTRAS OFF CACHE BOOL "" FORCE)
set(INSTALL_LIBS OFF CACHE BOOL "" FORCE)
set(BUILD_UNIT_TESTS OFF CACHE BOOL "" FORCE)
set(INSTALL_CMAKE_FILES OFF CACHE BOOL "" FORCE)
if (MSVC)
    set(BULLET2_USE_PPL_MULTITHREADING ON CACHE BOOL "" FORCE)
    set(USE_MSVC_AVX ON CACHE BOOL "" FORCE)
else ()
    set(BULLET2_USE_OPEN_MP_MULTITHREADING ON CACHE BOOL "" FORCE)
    set(BULLET2_USE_TBB_MULTITHREADING ON CACHE BOOL "" FORCE)
endif ()
add_subdirectory(thirdparty/bullet)
include_directories(thirdparty/bullet/src)
link_libraries(LinearMath BulletDynamics BulletCollision)

add_subdirectory(thirdparty/tinyobjloader)
link_libraries(tinyobjloader)

add_subdirectory(thirdparty/stb)
link_libraries(stb)

set(ALSOFT_DLOPEN OFF CACHE BOOL "" FORCE)
set(ALSOFT_WERROR OFF CACHE BOOL "" FORCE)
set(ALSOFT_UTILS OFF CACHE BOOL "" FORCE)
set(ALSOFT_NO_CONFIG_UTIL ON CACHE BOOL "" FORCE)
set(ALSOFT_EXAMPLES OFF CACHE BOOL "" FORCE)
set(ALSOFT_TESTS OFF CACHE BOOL "" FORCE)
set(ALSOFT_CONFIG OFF CACHE BOOL "" FORCE)
set(ALSOFT_HRTF_DEFS OFF CACHE BOOL "" FORCE)
set(ALSOFT_AMBDEC_PRESETS OFF CACHE BOOL "" FORCE)
set(ALSOFT_INSTALL OFF CACHE BOOL "" FORCE)
add_subdirectory(thirdparty/openal-soft)
include_directories(thirdparty/openal-soft/include)
set(OPENAL_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/thirdparty/openal-soft/include)
link_libraries(OpenAL)

include_directories(.)
add_subdirectory(core)
link_libraries(NoWayOutCore)

add_executable(Backend-Executable backend-main.cpp)

add_subdirectory(display)
add_executable(Display-Executable display-main.cpp)
target_link_libraries(Display-Executable NoWayOutDisplay)
