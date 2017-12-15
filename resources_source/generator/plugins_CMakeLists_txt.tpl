project(plugins)
CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/../cmake ${PROJECT_SOURCE_DIR}/../dealii/cmake)

# Policy handling
if(POLICY CMP0020)
 cmake_policy(PUSH)
 cmake_policy(SET CMP0020 OLD)
endif()

# For Win64
if(${CMAKE_CL_64})
  set(CMAKE_EXE_LINKER_FLAGS "/machine:X64")
  SET (CMAKE_SHARED_LINKER_FLAGS "/machine:X64")
  SET (CMAKE_MODULE_LINKER_FLAGS "/machine:X64")
endif(${CMAKE_CL_64})

# Debug yes / no.
SET(AGROS_DEBUG NO)
SET(AGROS_PLUGINS_DEBUG NO)

SET(CMAKE_AGROS_DIRECTORY "${CMAKE_HOME_DIRECTORY}/../")

# Allow to override the default values in CMake.vars:
IF(CMAKE_SYSTEM_NAME MATCHES "Linux")
    INCLUDE(../CMake.vars.Linux OPTIONAL)
ELSE()
    INCLUDE(../CMake.vars.Windows OPTIONAL)
ENDIF()
include(../CMake.vars OPTIONAL)

# OpenGL
# FIND_PACKAGE(OpenGLCustom REQUIRED)

# Handle Qt.
# Handle Qt.
SET(CMAKE_AUTOMOC TRUE)

FIND_PACKAGE(Qt5Core REQUIRED)
FIND_PACKAGE(Qt5Svg REQUIRED)
FIND_PACKAGE(Qt5Network REQUIRED)
FIND_PACKAGE(Qt5Xml REQUIRED)
FIND_PACKAGE(Qt5XmlPatterns REQUIRED)

# Build type.
ADD_DEFINITIONS(-DBOOST_ALL_NO_LIB)
IF(AGROS_DEBUG)
SET(DEAL_II_BUILD_TYPE "Debug")
SET(CMAKE_BUILD_TYPE "Debug")
ELSE()
SET(DEAL_II_BUILD_TYPE "Release")
SET(CMAKE_BUILD_TYPE "Release")
ENDIF()

IF(WIN32)
  # Naming of libraries to link to.
  FIND_LIBRARY(PLUGINS_QCUSTOMPLOT_LIBRARY agros_3dparty_qcustomplot PATHS ${CMAKE_AGROS_DIRECTORY}/libs NO_DEFAULT_PATH)
  FIND_LIBRARY(PLUGINS_QUAZIP_LIBRARY agros_3dparty_quazip PATHS ${CMAKE_AGROS_DIRECTORY}/libs NO_DEFAULT_PATH)
  FIND_LIBRARY(PLUGINS_POLY2TRI_LIBRARY agros_3dparty_poly2tri PATHS ${CMAKE_AGROS_DIRECTORY}/libs NO_DEFAULT_PATH)
  FIND_LIBRARY(PLUGINS_DXFLIB_LIBRARY agros_3dparty_dxflib PATHS ${CMAKE_AGROS_DIRECTORY}/libs NO_DEFAULT_PATH)
  FIND_LIBRARY(PLUGINS_CTEMPLATE_LIBRARY agros_3dparty_ctemplate PATHS ${CMAKE_AGROS_DIRECTORY}/libs NO_DEFAULT_PATH)
ENDIF(WIN32)

FIND_LIBRARY(PLUGINS_AGROS_LIBRARY agros_library PATHS ${CMAKE_AGROS_DIRECTORY}/libs NO_DEFAULT_PATH)

# Output paths.
SET(EXECUTABLE_OUTPUT_PATH ${CMAKE_AGROS_DIRECTORY}/libs)
SET(LIBRARY_OUTPUT_PATH ${CMAKE_AGROS_DIRECTORY}/libs)

SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_AGROS_DIRECTORY}/libs)
SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_AGROS_DIRECTORY}/libs)
SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_AGROS_DIRECTORY}/libs)
  
IF(WIN32)
  SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_AGROS_DIRECTORY}/libs)
  SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_AGROS_DIRECTORY}/libs)
  SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${CMAKE_AGROS_DIRECTORY}/libs)
  SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${CMAKE_AGROS_DIRECTORY}/libs)
  SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${CMAKE_AGROS_DIRECTORY}/libs)
  SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${CMAKE_AGROS_DIRECTORY}/libs)
ENDIF(WIN32)

# Set global compiler parameters.
IF(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
  INCLUDE_DIRECTORIES(/usr/include/google)
ENDIF()
IF(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unsupported-friend")
  SET(CMAKE_EXE_LINKER_FLAGS "-Wl,-export-dynamic")
ENDIF()

IF(MSVC)
  SET(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} /NODEFAULTLIB:libcmtd /NODEFAULTLIB:libcmt")
ENDIF(MSVC)

# Python
set(Python_ADDITIONAL_VERSIONS 3.4)
FIND_PACKAGE(PythonLibs 3.4 REQUIRED)
IF(MSVC)
	get_filename_component(PYTHON_INCLUDE_DIRS ${PYTHON_INCLUDE_DIRS} PATH)
ENDIF()
INCLUDE_DIRECTORIES(${PYTHON_INCLUDE_DIRS})

# Include current dir
SET(CMAKE_INCLUDE_CURRENT_DIR ON)

# Turn on Unicode
ADD_DEFINITIONS(-DUNICODE -D_UNICODE)

# Enable debugging symbols for all files all the time:
IF(AGROS_PLUGINS_DEBUG)
  IF(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    # Disable all warnings and turn on only important ones:
    # SET(CMAKE_CXX_FLAGS "-w ${CMAKE_CXX_FLAGS} -fPIC")
    # SET(CMAKE_CXX_FLAGS "-Wuninitialized -Wvla -Wsign-compare ${CMAKE_CXX_FLAGS}")
    # SET(DEBUG_FLAGS "-g")
    SET(CMAKE_BUILD_TYPE Debug)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${DEBUG_FLAGS}")
  ENDIF()
ElSE(AGROS_PLUGINS_DEBUG)
  IF(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    SET(CMAKE_BUILD_TYPE Release)
    SET(RELEASE_FLAGS "-DNDEBUG -g -O3 -Ofast")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${RELEASE_FLAGS}")
  ENDIF()
ENDIF(AGROS_PLUGINS_DEBUG)

# This overrides CXX flags for MSVC
IF(MSVC)
  # On MSVC, we will always use Debug, otherwise the plugins do not work.
  SET(CMAKE_BUILD_TYPE Debug)
  SET (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SAFESEH:NO")
  SET (CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /SAFESEH:NO")
  SET (CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} /SAFESEH:NO")
  # The rest had to be moved to modules / couplings because of a strange linking problems caused by inline functions expansion (/Ob2).
ENDIF(MSVC)

# Include OUR header files location
include(${CMAKE_AGROS_DIRECTORY}/IncludeSubdirs.cmake)

# deal.II
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/../dealii/include/")
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/../dealii/build/include/")
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/../dealii/bundled/boost-1.62.0/include/")
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/../dealii/bundled/umfpack/UMFPACK/Include/")
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/../dealii/bundled/umfpack/AMD/Include/")
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/../dealii/bundled/tbb41_20130401oss/include/")
#INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/../dealii/bundled/muparser_v2_2_3/include/")

FIND_PACKAGE(deal.II HINTS "../dealii/build" REQUIRED)

# modules
{{#SOURCE}}
ADD_SUBDIRECTORY({{ID}}){{/SOURCE}}

# Policy handling
if(POLICY CMP0020)
 cmake_policy(POP)
endif()
