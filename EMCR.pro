QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

include(version.pri)

DEFINES += "VERSION_MAJOR=$$VERSION_MAJOR"\
    "VERSION_MINOR=$$VERSION_MINOR"\
    "VERSION_PATCH=$$VERSION_PATCH"

VERSION_FULL = $${VERSION_MAJOR}.$${VERSION_MINOR}.$${VERSION_PATCH}

SOURCES += \
    src/main.cpp \
    src/view/mainwindow.cpp \
    src/view/errormanager.cpp \
    src/view/elementslogowidget.cpp \
    src/controller/maincontroller.cpp \
    src/controller/device/devicedetector.cpp \
    src/model/modeldevice.cpp

HEADERS += \
    src/globaldefines.h \
    src/view/mainwindow.h \
    src/view/errormanager.h \
    src/view/elementslogowidget.h \
    src/controller/maincontroller.h \
    src/controller/device/devicedetector.h \
    src/model/modeldevice.h

INCLUDEPATH += \
    ./src \
    ./src/view \
    ./src/controller \
    ./src/controller/device \
    ./src/model

DEPENDPATH += \
    ./src \
    ./src/view \
    ./src/controller \
    ./src/controller/device \
    ./src/model

include(../e384commLib/frontPanel/includefrontpanel.pri)
include(../e384commLib/includecommlib.pri)

RESOURCES += \
    resources.qrc
