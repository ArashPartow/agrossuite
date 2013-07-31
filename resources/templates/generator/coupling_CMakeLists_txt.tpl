PROJECT(agros2d_plugin_{{ID}})
INCLUDE_DIRECTORIES(${CMAKE_AGROS_DIRECTORY}/util)
INCLUDE_DIRECTORIES(${CMAKE_AGROS_DIRECTORY}/pythonlab-library)
INCLUDE_DIRECTORIES(${CMAKE_AGROS_DIRECTORY}/agros2d-library)
INCLUDE_DIRECTORIES(${CMAKE_AGROS_DIRECTORY}/3rdparty)
INCLUDE_DIRECTORIES(${CMAKE_AGROS_DIRECTORY}/3rdparty/bson)
INCLUDE_DIRECTORIES(${CMAKE_AGROS_DIRECTORY}/3rdparty/paralution/src)
INCLUDE_DIRECTORIES(.)

SET(SOURCES {{ID}}_interface.cpp {{ID}}_weakform.cpp)
SET(HEADERS {{ID}}_interface.h {{ID}}_weakform.h)

ADD_DEFINITIONS(-DQT_PLUGIN)
ADD_DEFINITIONS(-DQT_NO_DEBUG)
ADD_DEFINITIONS(-DQT_SHARED)

ADD_LIBRARY(${PROJECT_NAME} SHARED ${SOURCES} ${HEADERS})
TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${QT_LIBRARIES} ${AGROS_UTIL} ${PYTHON_MODLIBS} ${PYTHON_LIB} ${AGROS_LIBRARY} ${PYTHONLAB_LIBRARY})
IF(WITH_QT5)
    QT5_USE_MODULES(${PROJECT_NAME} Core Widgets Network Xml XmlPatterns WebKit WebKitWidgets Svg UiTools)
ENDIF(WITH_QT5)
SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_AGROS_DIRECTORY}/libs)
INSTALL(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)				

