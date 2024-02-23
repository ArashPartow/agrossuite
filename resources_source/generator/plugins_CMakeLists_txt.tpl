CMAKE_MINIMUM_REQUIRED(VERSION 3.13.4)
project(plugins)
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/../cmake ${PROJECT_SOURCE_DIR}/../dealii/cmake)

# For Win64
# if(${CMAKE_CL_64})
#   set(CMAKE_EXE_LINKER_FLAGS "/machine:X64")
#   SET (CMAKE_SHARED_LINKER_FLAGS "/machine:X64")
#   SET (CMAKE_MODULE_LINKER_FLAGS "/machine:X64")
# endif(${CMAKE_CL_64})

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

find_package(Qt6 COMPONENTS Widgets REQUIRED)
find_package(Qt6 COMPONENTS Svg REQUIRED)
find_package(Qt6 COMPONENTS SvgWidgets REQUIRED)
find_package(Qt6 COMPONENTS Core REQUIRED)
find_package(Qt6 COMPONENTS Network REQUIRED)
find_package(Qt6 COMPONENTS Core5Compat REQUIRED)

# Build type.
ADD_DEFINITIONS(-DBOOST_ALL_NO_LIB)
SET(DEAL_II_BUILD_TYPE "Release")
SET(CMAKE_BUILD_TYPE "Release")

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
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
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
FIND_PACKAGE(Python COMPONENTS Development REQUIRED)
INCLUDE_DIRECTORIES(${Python_INCLUDE_DIRS})
IF(MSVC)
	get_filename_component(PYTHON_INCLUDE_DIR ${PYTHON_INCLUDE_DIR} PATH)
ENDIF()

# Include current dir
SET(CMAKE_INCLUDE_CURRENT_DIR ON)

# Turn on Unicode
ADD_DEFINITIONS(-DUNICODE -D_UNICODE)

# Enable debugging symbols for all files all the time:
IF(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  SET(CMAKE_BUILD_TYPE RelWithDebInfo)
  # SET(RELEASE_FLAGS "-DNDEBUG -g -O3 -Ofast")
  SET(RELEASE_FLAGS "-DNDEBUG -g -O0 -Ofast")
  SET(CMAKE_CXX_FLAGS "-Wno-deprecated -Wno-deprecated-declarations -Wno-missing-declarations -Wno-return-type ${CMAKE_CXX_FLAGS} ${RELEASE_FLAGS}")
  ADD_DEFINITIONS(-DQT_NO_DEBUG_OUTPUT)
ENDIF()

# This overrides CXX flags for MSVC
#IF(MSVC)
  # On MSVC, we will always use Debug, otherwise the plugins do not work.
  #SET(CMAKE_BUILD_TYPE Debug)
  #SET (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SAFESEH:NO")
  #SET (CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /SAFESEH:NO")
  #SET (CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} /SAFESEH:NO")
  # The rest had to be moved to modules / couplings because of a strange linking problems caused by inline functions expansion (/Ob2).
#ENDIF(MSVC)

# Include OUR header files location
include(${CMAKE_AGROS_DIRECTORY}/IncludeSubdirs.cmake)

# deal.II
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/../dealii/include/")
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/../dealii/build/include/")
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/../dealii/bundled/boost-1.70.0/include/")
# INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/../dealii/bundled/tbb-2018_U2/include/")
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/../dealii/bundled/kokkos-3.7.00/core/src/")
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/../dealii/bundled/kokkos-3.7.00/tpls/desul/include/")


FIND_PACKAGE(deal.II HINTS "${CMAKE_SOURCE_DIR}/../dealii/build" REQUIRED)
# HACK
# message(${DEAL_II_LIBRARIES})
# string(REPLACE "/lib/lib" "/lib" DEAL_II_LIBRARIES ${DEAL_II_LIBRARIES})


# modules
{{#SOURCE}}
ADD_SUBDIRECTORY({{ID}}){{/SOURCE}}
