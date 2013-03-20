# pythonlab
QT += opengl xml network webkit svg xmlpatterns widgets printsupport webkitwidgets

OBJECTS_DIR = build
MOC_DIR = build
TEMPLATE = app

INCLUDEPATH += ../util \
       ../pythonlab-library

LIBS += -lagros2d_3rdparty
LIBS += -lagros2d_pythonlab_library

RC_FILE = ../resources_source/resources.rc
RESOURCES = ../resources_source/resources.qrc

SOURCES += main.cpp

linux-clang {
    INCLUDEPATH += ../hermes2d/omp
}

linux-g++|linux-g++-64|linux-g++-32|linux-clang {
    TARGET = ../agros2d_pythonlab

    INCLUDEPATH += /usr/include
    INCLUDEPATH += /usr/include/python2.7
    INCLUDEPATH += $$system(python -c "\"import distutils.sysconfig; print distutils.sysconfig.get_python_inc()\"")

    LIBS += -L../libs

    LIBS += $$system(python -c "\"from distutils import sysconfig; print '-lpython'+sysconfig.get_config_var('VERSION')\"")
    LIBS += $$system(python -c "\"import distutils.sysconfig; print distutils.sysconfig.get_config_var('LOCALMODLIBS')\"")
}

macx-g++ {
    ICON += resources/images/agros2d.icns
    QMAKE_INFO_PLIST  += resources/Info.plist
    #target.path = /Applications
    #INSTALLS += target
}

win32-msvc2010 {
    QMAKE_CXXFLAGS += /MP /openmp /Zc:wchar_t
    QMAKE_LFLAGS += /NODEFAULTLIB:libcmtd /NODEFAULTLIB:libcmt
    QMAKE_CXXFLAGS_RELEASE += -MD
    QMAKE_CXXFLAGS_DEBUG += -MDd

    LIBS += -L..
    LIBS += -L../libs
    TARGET = ../../agros2d_pythonlab
}

include(../agros2d_version.pri)
