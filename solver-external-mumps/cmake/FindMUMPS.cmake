#
# MUMPS
#
# set WITH_MUMPS to YES to enable MUMPS support
# set MUMPS_ROOT to point to the directory containing your MUMPS library
#

# You can specify your own version of the library instead of the one provided by
# Femhub by specifying the environment variables MY_MUMPS_LIB_DIRS and 
# MY_MUMPS_INC_DIRS.

INCLUDE(FindPackageHandleStandardArgs)

if(WIN64)
  SET(MUMPS_LIB_SEARCH_PATH ${MUMPS_ROOT}/lib/x64 ${MUMPS_ROOT}/lib)
else(WIN64)  
  SET(MUMPS_LIB_SEARCH_PATH ${MUMPS_ROOT}/lib /usr/lib /usr/local/lib /usr/lib64 /usr/local/lib64)
endif(WIN64)

FIND_PATH(MUMPS_INCLUDE_PATH mumps_c_types.h ${MUMPS_ROOT}/include /usr/include /usr/include/mumps_seq /usr/local/include/	/usr/local/include/mumps_seq)

FIND_LIBRARY(MUMPS_MPISEQ_LIBRARY NAMES mpiseq_seq libseq_c PATHS ${MUMPS_LIB_SEARCH_PATH})
FIND_LIBRARY(MUMPS_COMMON_LIBRARY NAMES mumps_common_seq mumps_common_c PATHS ${MUMPS_LIB_SEARCH_PATH})
FIND_LIBRARY(MUMPS_PORD_LIBRARY NAMES pord_seq pord_c PATHS ${MUMPS_LIB_SEARCH_PATH})

SET(MUMPS_INCLUDE_PATH ${MUMPS_INCLUDE_PATH} ${MUMPS_MPISEQ_INCLUDE_PATH})
FIND_LIBRARY(MUMPSD_SEQ_LIBRARY NAMES dmumps_seq dmumps_c PATHS ${MUMPS_LIB_SEARCH_PATH})
LIST(APPEND REQUIRED_REAL_LIBRARIES "MUMPSD_SEQ_LIBRARY")

FIND_LIBRARY(MUMPSZ_SEQ_LIBRARY NAMES zmumps_seq zmumps_c PATHS ${MUMPS_LIB_SEARCH_PATH})
LIST(APPEND REQUIRED_CPLX_LIBRARIES "MUMPSZ_SEQ_LIBRARY")

LIST(APPEND REQUIRED_REAL_LIBRARIES "MUMPS_MPISEQ_LIBRARY")
LIST(APPEND REQUIRED_CPLX_LIBRARIES "MUMPS_MPISEQ_LIBRARY")  

LIST(APPEND REQUIRED_REAL_LIBRARIES "MUMPS_COMMON_LIBRARY" "MUMPS_PORD_LIBRARY")
LIST(APPEND REQUIRED_CPLX_LIBRARIES "MUMPS_COMMON_LIBRARY" "MUMPS_PORD_LIBRARY")

IF(NOT(MSVC))
  FIND_LIBRARY(GFORTRAN_LIBRARY NAMES libgfortran.so.3.0.0 gfortran.so.3.0.0 libgfortran gfortran li
bgfortran3 gfortran3 PATHS "/usr/lib/x86_64-linux-gnu" "/usr/lib/x86-linux-gnu")
  FIND_PACKAGE_HANDLE_STANDARD_ARGS(GFORTRAN DEFAULT_MSG GFORTRAN_LIBRARY)
ENDIF()

# Fortran libraries.
FIND_LIBRARY(MUMPS_MPISEQ_FORTRAN_LIBRARY NAMES libseq_fortran PATHS ${MUMPS_LIB_SEARCH_PATH})
  if(NOT(${MUMPS_MPISEQ_FORTRAN_LIBRARY} STREQUAL  "MUMPS_MPISEQ_FORTRAN_LIBRARY-NOTFOUND"))
    LIST(APPEND REQUIRED_REAL_LIBRARIES "MUMPS_MPISEQ_FORTRAN_LIBRARY")
  endif(NOT(${MUMPS_MPISEQ_FORTRAN_LIBRARY} STREQUAL "MUMPS_MPISEQ_FORTRAN_LIBRARY-NOTFOUND"))  
  
FIND_LIBRARY(MUMPSC_SEQ_FORTRAN_LIBRARY NAMES cmumps_fortran PATHS ${MUMPS_LIB_SEARCH_PATH})
  if(NOT(${MUMPSC_SEQ_FORTRAN_LIBRARY} STREQUAL  "MUMPSC_SEQ_FORTRAN_LIBRARY-NOTFOUND"))
    LIST(APPEND REQUIRED_REAL_LIBRARIES "MUMPSC_SEQ_FORTRAN_LIBRARY")
  else(NOT(${MUMPSC_SEQ_FORTRAN_LIBRARY} STREQUAL  "MUMPSC_SEQ_FORTRAN_LIBRARY-NOTFOUND"))
    MESSAGE(STATUS "MUMPSC_SEQ_FORTRAN_LIBRARY not found - if WinMUMPS is used, this is an error.")
  endif(NOT(${MUMPSC_SEQ_FORTRAN_LIBRARY} STREQUAL "MUMPSC_SEQ_FORTRAN_LIBRARY-NOTFOUND"))
  
FIND_LIBRARY(MUMPSS_SEQ_FORTRAN_LIBRARY NAMES smumps_fortran PATHS ${MUMPS_LIB_SEARCH_PATH})
  if(NOT(${MUMPSS_SEQ_FORTRAN_LIBRARY} STREQUAL  "MUMPSS_SEQ_FORTRAN_LIBRARY-NOTFOUND"))
    LIST(APPEND REQUIRED_REAL_LIBRARIES "MUMPSS_SEQ_FORTRAN_LIBRARY")
  else(NOT(${MUMPSS_SEQ_FORTRAN_LIBRARY} STREQUAL  "MUMPSS_SEQ_FORTRAN_LIBRARY-NOTFOUND"))
    MESSAGE(STATUS "MUMPSS_SEQ_FORTRAN_LIBRARY not found - if WinMUMPS is used, this is an error.")
  endif(NOT(${MUMPSS_SEQ_FORTRAN_LIBRARY} STREQUAL "MUMPSS_SEQ_FORTRAN_LIBRARY-NOTFOUND"))
  
FIND_LIBRARY(MUMPSD_SEQ_FORTRAN_LIBRARY NAMES dmumps_fortran PATHS ${MUMPS_LIB_SEARCH_PATH})
  if(NOT(${MUMPSD_SEQ_FORTRAN_LIBRARY} STREQUAL  "MUMPSD_SEQ_FORTRAN_LIBRARY-NOTFOUND"))
    LIST(APPEND REQUIRED_REAL_LIBRARIES "MUMPSD_SEQ_FORTRAN_LIBRARY")
  else(NOT(${MUMPSD_SEQ_FORTRAN_LIBRARY} STREQUAL  "MUMPSD_SEQ_FORTRAN_LIBRARY-NOTFOUND"))
    MESSAGE(STATUS "MUMPSD_SEQ_FORTRAN_LIBRARY not found - if WinMUMPS is used, this is an error.")
  endif(NOT(${MUMPSD_SEQ_FORTRAN_LIBRARY} STREQUAL "MUMPSD_SEQ_FORTRAN_LIBRARY-NOTFOUND"))
  
FIND_LIBRARY(MUMPSZ_SEQ_FORTRAN_LIBRARY NAMES zmumps_fortran PATHS ${MUMPS_LIB_SEARCH_PATH})
  if(NOT(${MUMPSZ_SEQ_FORTRAN_LIBRARY} STREQUAL  "MUMPSZ_SEQ_FORTRAN_LIBRARY-NOTFOUND"))
    LIST(APPEND REQUIRED_REAL_LIBRARIES "MUMPSZ_SEQ_FORTRAN_LIBRARY")
  else(NOT(${MUMPSZ_SEQ_FORTRAN_LIBRARY} STREQUAL  "MUMPSZ_SEQ_FORTRAN_LIBRARY-NOTFOUND"))
    MESSAGE(STATUS "MUMPSZ_SEQ_FORTRAN_LIBRARY not found - if WinMUMPS is used, this is an error.")
  endif(NOT(${MUMPSZ_SEQ_FORTRAN_LIBRARY} STREQUAL "MUMPSZ_SEQ_FORTRAN_LIBRARY-NOTFOUND"))  

# Test if all the required libraries have been found. If they haven't, end with fatal error...
FIND_PACKAGE_HANDLE_STANDARD_ARGS(  MUMPS DEFAULT_MSG
   ${REQUIRED_REAL_LIBRARIES} ${REQUIRED_CPLX_LIBRARIES} MUMPS_INCLUDE_PATH
)

# ...if they have, append them all to the MUMPS_{REAL/CPLX}_LIBRARIES variable.
  FOREACH(_LIB ${REQUIRED_REAL_LIBRARIES})
    LIST(APPEND MUMPS_REAL_LIBRARIES ${${_LIB}})
  ENDFOREACH(_LIB ${REQUIRED_REAL_LIBRARIES})
  FOREACH(_LIB ${REQUIRED_CPLX_LIBRARIES})
    LIST(APPEND MUMPS_CPLX_LIBRARIES ${${_LIB}})
  ENDFOREACH(_LIB ${REQUIRED_CPLX_LIBRARIES})

# Finally, set MUMPS_INCLUDE_DIR to point to the MUMPS include directory.
SET(MUMPS_INCLUDE_DIR ${MUMPS_INCLUDE_DIR} ${MUMPS_INCLUDE_PATH})
