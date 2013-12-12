# This is ALMOST a normal Hermes CMake.vars file.
# EXCEPT everything has to be set here (including the defaults in hermes/CMakeLists.txt) - for plugins to be able to cope with just this file.

# Very important, telling Hermes that this build is for Agros.
SET(AGROS_BUILD YES)

set(HERMES_STATIC_LIBS YES)
set(H2D_WITH_GLUT NO)
set(H2D_WITH_TEST_EXAMPLES  NO)
set(WITH_BSON YES)
SET(BSON_LIBRARY ${BSON_LIBRARY})
SET(MONGO_STATIC_BUILD YES)
set(BSON_INCLUDE_DIR ${CMAKE_HOME_DIRECTORY}/3rdparty/bson)
set(WITH_UMFPACK YES)
set(WITH_MUMPS YES)
set(WITH_PARALUTION YES)
SET(PARALUTION_LIBRARY ${PARALUTION_LIBRARY})
set(PARALUTION_INCLUDE_DIR ${CMAKE_HOME_DIRECTORY}/3rdparty/paralution/src)
set(WITH_MATIO YES)
SET(MATIO_LIBRARY agros2d_3dparty_matio)
set(MATIO_INCLUDE_DIR ${CMAKE_HOME_DIRECTORY}/3rdparty/matio)
set(WITH_TC_MALLOC NO)

IF(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  set(WITH_OPENMP NO)
ENDIF()

# Some search paths.
if (WIN32)
	if(DEFINED DEPENDENCIES_ROOT)
	  set(XERCES_ROOT ${DEPENDENCIES_ROOT})
	  set(PARALUTION_ROOT ${DEPENDENCIES_ROOT})
	  set(XSD_ROOT ${DEPENDENCIES_ROOT})
	  set(BSON_ROOT ${DEPENDENCIES_ROOT})
	  set(TCMALLOC_ROOT ${DEPENDENCIES_ROOT})
	  set(WINBLAS_ROOT ${DEPENDENCIES_ROOT})
	  set(UMFPACK_ROOT ${DEPENDENCIES_ROOT})
	  set(MUMPS_ROOT ${DEPENDENCIES_ROOT})
	  set(CLAPACK_ROOT ${DEPENDENCIES_ROOT})
    else()
	  message(FATAL_ERROR "\nDEPENDENCIES_ROOT should be defined in the main CMake.vars.\n")
	endif()
ENDIF()

if(AGROS_DEBUG)
  set(HERMES_COMMON_DEBUG     YES)
  set(HERMES_COMMON_RELEASE   NO)
  set(H2D_DEBUG               YES)
  set(H2D_RELEASE             NO)
else()
  set(HERMES_COMMON_DEBUG     NO)
  set(HERMES_COMMON_RELEASE   YES)
  set(H2D_DEBUG               NO)
  set(H2D_RELEASE             YES)
endif()

