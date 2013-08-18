# Add QT.
SET(CMAKE_AUTOMOC TRUE)
IF(WITH_QT5)
    FIND_PACKAGE(Qt5Core REQUIRED)
    FIND_PACKAGE(Qt5Gui REQUIRED)
    FIND_PACKAGE(Qt5OpenGL REQUIRED)
    FIND_PACKAGE(Qt5PrintSupport REQUIRED)
    FIND_PACKAGE(Qt5Svg REQUIRED)
    FIND_PACKAGE(Qt5Network REQUIRED)
    FIND_PACKAGE(Qt5Widgets REQUIRED)
    FIND_PACKAGE(Qt5WebKit REQUIRED)
    FIND_PACKAGE(Qt5WebKitWidgets REQUIRED)
    FIND_PACKAGE(Qt5Xml REQUIRED)
    FIND_PACKAGE(Qt5XmlPatterns REQUIRED)
    FIND_PACKAGE(Qt5UiTools REQUIRED)
ELSE(WITH_QT5)
    SET(QT_USE_QTOPENGL TRUE)
    SET(QT_USE_QTUITOOLS TRUE)
    SET(QT_USE_QTNETWORK TRUE)
    SET(QT_USE_QTOPENGL TRUE)
    SET(QT_USE_QTSQL TRUE)
    SET(QT_USE_QTXML TRUE)
    SET(QT_USE_QTSVG TRUE)
    SET(QT_USE_QTTEST TRUE)
    SET(QT_USE_QTDBUS TRUE)
    SET(QT_USE_QTSCRIPT TRUE)
    SET(QT_USE_QTWEBKIT TRUE)
    SET(QT_USE_QTXMLPATTERNS TRUE)
    FIND_PACKAGE(Qt4 REQUIRED)
    # This has to be here, otherwise CMake will not do the magic and will not link to OpenGL
    FIND_PACKAGE(OpenGL REQUIRED)
ENDIF(WITH_QT5)
