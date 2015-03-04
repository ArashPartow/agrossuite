PROJECT(agros2d_plugin_{{ID}})
INCLUDE_DIRECTORIES(${CMAKE_AGROS_DIRECTORY}/util)
INCLUDE_DIRECTORIES(${CMAKE_AGROS_DIRECTORY}/pythonlab-library)
INCLUDE_DIRECTORIES(${CMAKE_AGROS_DIRECTORY}/agros2d-library)
INCLUDE_DIRECTORIES(${CMAKE_AGROS_DIRECTORY}/3rdparty)
INCLUDE_DIRECTORIES(${CMAKE_AGROS_DIRECTORY}/3rdparty/bson)
INCLUDE_DIRECTORIES(${CMAKE_AGROS_DIRECTORY}/3rdparty/paralution/src)
INCLUDE_DIRECTORIES(.)

# This overrides CXX flags for MSVC
IF(MSVC)
  SET(CMAKE_CXX_FLAGS_RELEASE        "${CMAKE_CXX_FLAGS_RELEASE} /Ob0")
ENDIF(MSVC)

SET(SOURCES {{ID}}_interface.cpp {{ID}}_weakform.cpp)
SET(HEADERS {{ID}}_interface.h {{ID}}_weakform.h)

SET(SOURCES ${SOURCES} "${CMAKE_AGROS_DIRECTORY}/resources_source/classes/module_xml.cpp")
ADD_DEFINITIONS(-DQT_PLUGIN)
ADD_DEFINITIONS(-DQT_SHARED)

ADD_LIBRARY(${PROJECT_NAME} SHARED ${SOURCES} ${HEADERS})
SET(LIBRARIES_TO_LINK ${PLUGINS_AGROS_UTIL} ${QT_LIBRARIES} ${OPENGL_LIBRARIES} ${PYTHON_MODLIBS} ${PYTHON_LIB} ${PLUGINS_AGROS_LIBRARY} ${PLUGINS_PYTHONLAB_LIBRARY} ${XSD_LIBRARY} ${XERCES_LIBRARY} ${PTHREAD_LIBRARY} ${MUMPS_LIBRARIES} ${DEAL_II_LIBRARIES} ${BLAS_LIBRARIES})
TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${LIBRARIES_TO_LINK})

IF(AGROS_PLUGINS_DEBUG AND WIN32)
TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${TBB_LIBRARY_DEBUG})
ELSE()
TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${TBB_LIBRARY})
ENDIF()

QT5_USE_MODULES(${PROJECT_NAME} Core Widgets Network Xml XmlPatterns WebKit WebKitWidgets Svg UiTools OpenGL)
SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_AGROS_DIRECTORY}/libs)
INSTALL(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)				

