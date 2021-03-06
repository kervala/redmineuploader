CMAKE_MINIMUM_REQUIRED(VERSION 2.8.7)

# Allows qtmain to be linked auto
IF(POLICY CMP0020)
  CMAKE_POLICY(SET CMP0020 NEW)
ENDIF()

# Let automoc also process generated files
IF(POLICY CMP0071)
  CMAKE_POLICY(SET CMP0071 NEW)
ENDIF()

# Use AppleClang for OS X compiler
IF(POLICY CMP0025)
  CMAKE_POLICY(SET CMP0025 NEW)
ENDIF()

INCLUDE(UseCMakeModules.cmake)

# Define useful variables
SET(VERSION_MAJOR 1)
SET(VERSION_MINOR 1)
SET(VERSION_PATCH "REVISION")
SET(AUTHOR "Kervala")
SET(PRODUCT "RedmineUploader")
SET(DESCRIPTION "Tool to upload files to Redmine")
SET(TARGET "redmineuploader")
SET(YEAR "2015-${CURRENT_YEAR}")

PROJECT(${PRODUCT} C CXX)

# Instruct CMake to run moc automatically when needed.
SET(CMAKE_AUTOMOC ON)

INIT_DEFAULT_OPTIONS()

# Qt doesn't use RTTI or C++ exceptions
SET_OPTION_DEFAULT(WITH_EXCEPTIONS OFF)
SET_OPTION_DEFAULT(WITH_RTTI OFF)
SET_OPTION_DEFAULT(WITH_INSTALL_LIBRARIES OFF)

SETUP_DEFAULT_OPTIONS()

INIT_BUILD_FLAGS()
SETUP_BUILD_FLAGS()

SETUP_PREFIX_PATHS(${TARGET})
SETUP_EXTERNAL()

GEN_CONFIG_H()
GEN_REVISION_H()

USE_QT_MODULES(Network)

FILE(GLOB HEADER src/*.h)
FILE(GLOB SRC src/*.cpp)
FILE(GLOB RES res/*.ico)
FILE(GLOB TS translations/*.ts)

SET_TARGET_CONSOLE_EXECUTABLE(${TARGET} ${SRC} ${RES} ${HEADER} ${TS})
SET_TARGET_LABEL(${TARGET} ${PRODUCT})

IF(APPLE)
  SET(MACOSX_BUNDLE_GUI_IDENTIFIER "net.kervala.${TARGET}")
ENDIF()

IF(WITH_PCH)
  ADD_NATIVE_PRECOMPILED_HEADER(${TARGET} ${CMAKE_CURRENT_SOURCE_DIR}/src/common.h ${CMAKE_CURRENT_SOURCE_DIR}/src/common.cpp)
ENDIF()

INSTALL_RESOURCES(${TARGET} "")
