PROJECT(${CTEMPLATE_LIBRARY})

SET(SOURCES base/arena.cc 
    htmlparser/htmlparser.cc 
    htmlparser/htmlparser_cpp.h 
    htmlparser/jsparser.cc 
    htmlparser/statemachine.cc 
    per_expand_data.cc 
    template_annotator.cc 
    template_cache.cc 
    template.cc 
    template_dictionary.cc 
    template_modifiers.cc 
    template_namelist.cc 
    template_pathops.cc 
    template_string.cc
  )

IF(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  INCLUDE_DIRECTORIES(ctemplate)
  INCLUDE_DIRECTORIES(linux)
ENDIF()

IF(MSVC)
  INCLUDE_DIRECTORIES(ctemplate)
  INCLUDE_DIRECTORIES(windows)
  SET(SOURCES windows/port.cc ${SOURCES})
ENDIF(MSVC)

IF (MSVC)
    ADD_LIBRARY(${PROJECT_NAME} STATIC ${SOURCES})
ELSE()
    ADD_LIBRARY(${PROJECT_NAME} SHARED ${SOURCES})
ENDIF()

TARGET_LINK_LIBRARIES(${PROJECT_NAME})
