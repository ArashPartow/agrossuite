PROJECT(${AGROS_GENERATOR})

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

# XSD
find_package(XSD 4.0 REQUIRED)
INCLUDE_DIRECTORIES(${XSD_INCLUDE_DIR})
# xerces
find_package(XercesC REQUIRED)
INCLUDE_DIRECTORIES(${XercesC_INCLUDE_DIRS})

INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/agros-library)
INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/3rdparty)
INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/3rdparty/ctemplate)
IF(WIN32)
    INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/3rdparty/ctemplate/windows)
ELSE(WIN32)
    INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/3rdparty/ctemplate/linux)
ENDIF(WIN32)

SET(SOURCES main.cpp 
    generator.cpp 
    generator_module.cpp 
    generator_weakforms.cpp
    generator_postprocessor.cpp
    generator_documentation.cpp
    parser.cpp
    parser_instance.cpp
    parser_lexical_analyser.cpp
    parser_module_info.cpp
    ${CMAKE_HOME_DIRECTORY}/resources_source/classes/module_xml.cpp
)

SET(HEADERS generator.h
    generator_module.h
    parser.h
)
    
ADD_EXECUTABLE(${PROJECT_NAME} ${SOURCES} ${SOURCES_XML} ${HEADERS})

IF(WIN32)
    foreach(XSD_FILE ${generator_xsd_files})
      GET_FILENAME_COMPONENT(basefile ${XSD_FILE} NAME_WE)
      ADD_DEPENDENCIES(${PROJECT_NAME} generator-${basefile})
    endforeach()
ENDIF(WIN32)

foreach(XSD_FILE ${xsd_files})
  GET_FILENAME_COMPONENT(basefile ${XSD_FILE} NAME_WE)
  ADD_DEPENDENCIES(${PROJECT_NAME} ${basefile})
endforeach()

TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${DEAL_II_LIBRARIES} ${CTEMPLATE_LIBRARY} ${XSD_LIBRARY} ${XercesC_LIBRARIES} ${AGROS_LIBRARY} ${QUAZIP_LIBRARY} ${STB_TRUETYPE_LIBRARY} ${XSD_LIBRARY} Qt6::Core Qt6::Network)
