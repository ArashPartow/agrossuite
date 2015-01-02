project(plugins)
CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/../cmake)

# Policy handling
if(POLICY CMP0020)
 cmake_policy(PUSH)
 cmake_policy(SET CMP0020 OLD)
endif()

# For Win64
if(${CMAKE_CL_64})
  set(WIN64 YES)
  set(CMAKE_EXE_LINKER_FLAGS "/machine:X64")
  SET (CMAKE_SHARED_LINKER_FLAGS "/machine:X64")
  SET (CMAKE_MODULE_LINKER_FLAGS "/machine:X64")
else(${CMAKE_CL_64})
  set(WIN64 NO)
endif(${CMAKE_CL_64})

# Debug yes / no.
SET(AGROS_DEBUG NO)
SET(AGROS_PLUGINS_DEBUG NO)

ADD_DEFINITIONS(-DQT_PLUGIN)
ADD_DEFINITIONS(-DQT_SHARED)
ADD_DEFINITIONS(-DQT_DLL)

SET(CMAKE_AGROS_DIRECTORY "${CMAKE_HOME_DIRECTORY}/../")

# Allow to override the default values in CMake.vars:
include(../CMake.vars OPTIONAL)

# OpenGL
FIND_PACKAGE(OpenGLCustom REQUIRED)

# Handle Qt.
include(../QtHandling.cmake)

IF(WIN32)
  # Naming of libraries to link to.
  FIND_LIBRARY(PLUGINS_PARALUTION_LIBRARY agros2d_3dparty_paralution PATHS ${CMAKE_AGROS_DIRECTORY}/libs NO_DEFAULT_PATH)
  FIND_LIBRARY(PLUGINS_QCUSTOMPLOT_LIBRARY agros2d_3dparty_qcustomplot PATHS ${CMAKE_AGROS_DIRECTORY}/libs NO_DEFAULT_PATH)
  FIND_LIBRARY(PLUGINS_QUAZIP_LIBRARY agros2d_3dparty_quazip PATHS ${CMAKE_AGROS_DIRECTORY}/libs NO_DEFAULT_PATH)
  FIND_LIBRARY(PLUGINS_POLY2TRI_LIBRARY agros2d_3dparty_poly2tri PATHS ${CMAKE_AGROS_DIRECTORY}/libs NO_DEFAULT_PATH)
  FIND_LIBRARY(PLUGINS_DXFLIB_LIBRARY agros2d_3dparty_dxflib PATHS ${CMAKE_AGROS_DIRECTORY}/libs NO_DEFAULT_PATH)
  FIND_LIBRARY(PLUGINS_CTEMPLATE_LIBRARY agros2d_3dparty_ctemplate PATHS ${CMAKE_AGROS_DIRECTORY}/libs NO_DEFAULT_PATH)
ENDIF(WIN32)

find_package(XSD 4.0 REQUIRED)
INCLUDE_DIRECTORIES(${XSD_INCLUDE_DIR})
find_package(XERCES REQUIRED)
INCLUDE_DIRECTORIES(${XERCES_INCLUDE_DIR})

FIND_LIBRARY(PLUGINS_AGROS_LIBRARY agros2d_library PATHS ${CMAKE_AGROS_DIRECTORY}/libs NO_DEFAULT_PATH)
FIND_LIBRARY(PLUGINS_PYTHONLAB_LIBRARY agros2d_pythonlab_library PATHS ${CMAKE_AGROS_DIRECTORY}/libs NO_DEFAULT_PATH)
FIND_LIBRARY(PLUGINS_AGROS_UTIL agros2d_util PATHS ${CMAKE_AGROS_DIRECTORY}/libs NO_DEFAULT_PATH)

ADD_DEFINITIONS(-DBOOST_ALL_NO_LIB)
IF(AGROS_DEBUG)
SET(DEAL_II_BUILD_TYPE "Debug")
SET(CMAKE_BUILD_TYPE "Debug")
ELSE()
SET(DEAL_II_BUILD_TYPE "Release")
SET(CMAKE_BUILD_TYPE "Release")
ENDIF()

INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/../dealii/include/")
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/../dealii/bundled/boost-1.56.0/include/")
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/../dealii/bundled/tbb41_20130401oss/include/")
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/../dealii/bundled/umfpack/UMFPACK/Include/")
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/../dealii/bundled/umfpack/AMD/Include/")
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/../dealii/bundled/muparser_v2_2_3/include/")

FIND_PACKAGE(deal.II REQUIRED HINTS ../dealii)
DEAL_II_INITIALIZE_CACHED_VARIABLES()

# DealII linking name & location.
IF(MSVC)
IF(AGROS_DEBUG)
SET(DEAL_II_LIBRARIES ${CMAKE_HOME_DIRECTORY}/../libs/deal_II.g.lib)
ELSE()
SET(DEAL_II_LIBRARIES ${CMAKE_HOME_DIRECTORY}/../libs/deal_II.lib)
ENDIF()
ENDIF()

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

IF(MSVC)
  SET(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} /NODEFAULTLIB:libcmtd /NODEFAULTLIB:libcmt")
ENDIF(MSVC)

# Python
FIND_PACKAGE(PythonLibs 3.2 REQUIRED)
INCLUDE_DIRECTORIES(${PYTHON_INCLUDE_DIR})

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

# Look for UMFPACK, AND MUMPS
IF(WITH_UMFPACK)
  FIND_PACKAGE(UMFPACK REQUIRED)
  INCLUDE_DIRECTORIES(${UMFPACK_INCLUDE_DIRS})
ENDIF()

if(WITH_TRILINOS)
    find_package(TRILINOS REQUIRED)
    include_directories(${TRILINOS_INCLUDE_DIR})
  endif(WITH_TRILINOS)

if(WITH_MPI)
  find_package(MPI REQUIRED)
  include_directories(${MPI_INCLUDE_PATH})
endif(WITH_MPI)


IF(WITH_MUMPS)
  FIND_PACKAGE(MUMPS REQUIRED)
  
  IF(MSVC)
    FIND_PACKAGE(WINBLAS REQUIRED)
  ELSE(MSVC)
    IF (NOT LAPACK_FOUND)
      ENABLE_LANGUAGE(Fortran)
      FIND_PACKAGE(LAPACK REQUIRED)
      
      # If no error occured, LAPACK library has been found. Save the path to
      # it to cache, so that it will not be searched for during next 'cmake .'
      SET(LAPACK_LIBRARIES  ${LAPACK_LIBRARIES}
              CACHE STRING  "Path to LAPACK/BLAS libraries.")
      SET(LAPACK_FOUND      YES
              CACHE STRING  "Have LAPACK/BLAS libraries been found?")
    ENDIF (NOT LAPACK_FOUND)
    
    ADD_DEFINITIONS(-DWITH_BLAS)
  ENDIF(MSVC)
ENDIF()

SET(MUMPS_LIBRARIES ${MUMPS_CPLX_LIBRARIES})
LIST(APPEND MUMPS_LIBRARIES ${MUMPS_REAL_LIBRARIES})
INCLUDE_DIRECTORIES(${MUMPS_INCLUDE_DIR})

# modules
{{#SOURCE}}
ADD_SUBDIRECTORY({{ID}}){{/SOURCE}}

# Policy handling
if(POLICY CMP0020)
 cmake_policy(POP)
endif()
