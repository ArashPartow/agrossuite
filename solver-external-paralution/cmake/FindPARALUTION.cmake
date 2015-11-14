#
# PARALUTION
# FROM http://www.paralution.com/
#

FIND_PATH(PARALUTION_INCLUDE_DIR paralution.hpp ${PARALUTION_ROOT}/include 3rdparty/paralution/build/inc ${PARALUTION_ROOT}/inc /usr/local/include /usr/include)

IF(WIN64)
  FIND_LIBRARY(PARALUTION_LIBRARY NAMES agros2d_paralution PATHS ${PARALUTION_ROOT} ${PARALUTION_ROOT}/lib/x64 ${PARALUTION_ROOT}/x64/lib ${PARALUTION_ROOT}/x64/${CMAKE_BUILD_TYPE})
ELSE(WIN64)
  IF(WIN32)
    # ${PARALUTION_ROOT}/lib
    FIND_LIBRARY(PARALUTION_LIBRARY NAMES agros2d_paralution PATHS ${PARALUTION_ROOT} ${PARALUTION_ROOT}/lib/${CMAKE_BUILD_TYPE} ${PARALUTION_ROOT}/${CMAKE_BUILD_TYPE})
  ELSE(WIN32)
    # ${PARALUTION_ROOT}/lib
    FIND_LIBRARY(PARALUTION_LIBRARY NAMES libagros2d_paralution agros2d_paralution PATHS ${PARALUTION_ROOT} ${PARALUTION_ROOT}/../../../libs ${PARALUTION_ROOT}/../../../libs/${CMAKE_BUILD_TYPE} /usr/lib /usr/local/lib /usr/lib64 /usr/local/lib64)
  ENDIF(WIN32)
ENDIF(WIN64)

# Report the found libraries, quit with fatal error if any required library has not been found.
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(PARALUTION DEFAULT_MSG PARALUTION_LIBRARY PARALUTION_INCLUDE_DIR)