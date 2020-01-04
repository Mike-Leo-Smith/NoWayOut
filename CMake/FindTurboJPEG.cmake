# FindTurboJPEG.cmake
# Uses environment variable TurboJPEG_ROOT as backup
# - TurboJPEG_FOUND
# - TurboJPEG_INCLUDE_DIRS
# - TurboJPEG_LIBRARIES

find_path(TurboJPEG_INCLUDE_DIRS
          turbojpeg.h
          DOC "Found TurboJPEG include directory"
          PATHS
          "/opt/libjpeg-turbo"
          "${CMAKE_SOURCE_DIR}/thirdparty/libjpeg-turbo"
          PATH_SUFFIXES
          include)

#Library names:
# debian sid,strech: libturbojpeg0
# debian/ubuntu else: libturbojpeg1-dev #provided by libjpeg-turbo8-dev (ubuntu)
find_library(TurboJPEG_LIBRARIES
             NAMES turbojpeg
             DOC "Found TurboJPEG library path"
             PATHS
             "/opt/libjpeg-turbo"
             "${CMAKE_SOURCE_DIR}/thirdparty/libjpeg-turbo"
             PATH_SUFFIXES
             lib
             lib64)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(TurboJPEG FOUND_VAR TurboJPEG_FOUND
                                  REQUIRED_VARS TurboJPEG_LIBRARIES TurboJPEG_INCLUDE_DIRS)
