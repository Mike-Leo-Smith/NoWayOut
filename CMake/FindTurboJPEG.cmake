# FindTurboJPEG.cmake
# Uses environment variable TurboJPEG_ROOT as backup
# - TurboJPEG_FOUND
# - TurboJPEG_INCLUDE_DIRS
# - TurboJPEG_LIBRARIES

FIND_PATH(TurboJPEG_INCLUDE_DIRS
          turbojpeg.h
          DOC "Found TurboJPEG include directory"
          PATHS
          "${DEPENDS_DIR}/libjpeg_turbo"
          "${DEPENDS_DIR}/libjpeg-turbo64"
          "/opt/libjpeg-turbo"
          "C:/libjpeg-turbo64"
          ENV TurboJPEG_ROOT
          PATH_SUFFIXES
          include
          )

#Library names:
# debian sid,strech: libturbojpeg0
# debian/ubuntu else: libturbojpeg1-dev #provided by libjpeg-turbo8-dev (ubuntu)
FIND_LIBRARY(TurboJPEG_LIBRARIES
             NAMES turbojpeg
             DOC "Found TurboJPEG library path"
             PATHS
             "${DEPENDS_DIR}/libjpeg_turbo"
             "${DEPENDS_DIR}/libjpeg-turbo64"
             "/opt/libjpeg-turbo"
             "C:/libjpeg-turbo64"
             ENV TurboJPEG_ROOT
             PATH_SUFFIXES
             lib
             lib64
             )

IF(WIN32)
    FIND_FILE(TurboJPEG_DLL
              turbojpeg.dll
              DOC "Found TurboJPEG DLL path"
              PATHS
              "${DEPENDS_DIR}/libjpeg_turbo"
              "${DEPENDS_DIR}/libjpeg-turbo64"
              "C:/libjpeg-turbo64"
              ENV TurboJPEG_ROOT
              PATH_SUFFIXES
              bin
              )
ENDIF()

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(TurboJPEG FOUND_VAR TurboJPEG_FOUND
                                  REQUIRED_VARS TurboJPEG_LIBRARIES TurboJPEG_INCLUDE_DIRS)
