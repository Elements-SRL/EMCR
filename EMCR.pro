QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

include(version.pri)

DEFINES += "VERSION_MAJOR=$$VERSION_MAJOR"\
    "VERSION_MINOR=$$VERSION_MINOR"\
    "VERSION_PATCH=$$VERSION_PATCH"

VERSION_FULL = $${VERSION_MAJOR}.$${VERSION_MINOR}.$${VERSION_PATCH}

SOURCES += \
    src\main.cpp \
    src\mainwindow.cpp \
    src\device\devicedetector.cpp \
    src\customwidget\elementslogowidget.cpp

HEADERS += \
    src\globaldefines.h \
    src\mainwindow.h \
    src\device\devicedetector.h \
    src\customwidget\elementslogowidget.h

INCLUDEPATH += \
    .\src \
    .\src\device \
    .\src\customwidget

DEPENDPATH += \
    .\src \
    .\src\device \
    .\src\customwidget

include(..\e384commLib\frontPanel\includefrontpanel.pri)
include(..\e384commLib\includecommlib.pri)

RESOURCES += \
    resources.qrc
