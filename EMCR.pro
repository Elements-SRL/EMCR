QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG(debug, debug|release) {
    DEFINES += GLB_SHOW_DEBUG_CTRLS
}

CONFIG(release, debug|release) {

}

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
    src/view/bigplot.cpp \
    src/view/bigplotdockwidget.cpp \
    src/view/curve.cpp \
    src/view/chessboard.cpp \
    src/view/myleftrightmousepushbutton.cpp \
    src/view/myspinbox.cpp \
    src/view/stampplot.cpp \
    src/view/channelcontroldockwidget.cpp \
    src/view/boardcontroldockwidget.cpp \
    src/view/devicecontroldockwidget.cpp \
    src/controller/controllermain.cpp \
    src/controller/controllerchannel.cpp \
    src/controller/controllerboard.cpp \
    src/controller/controllerdevice.cpp \
    src/controller/device/devicedetector.cpp \
    src/controller/device/devicedataproducer.cpp \
    src/controller/device/devicedataconsumer.cpp \
    src/controller/device/consumer/plotconsumer.cpp \
    src/model/modeldevice.cpp \
    src/model/modelboard.cpp \
    src/model/modelchannel.cpp

HEADERS += \
    src/globaldefines.h \
    src/view/mainwindow.h \
    src/view/errormanager.h \
    src/view/elementslogowidget.h \
    src/view/bigplot.h \
    src/view/bigplotdockwidget.h \
    src/view/curve.h \
    src/view/chessboard.h \
    src/view/myleftrightmousepushbutton.h \
    src/view/myspinbox.h \
    src/view/stampplot.h \
    src/view/channelcontroldockwidget.h \
    src/view/boardcontroldockwidget.h \
    src/view/devicecontroldockwidget.h \
    src/controller/controllermain.h \
    src/controller/controllerchannel.h \
    src/controller/controllerboard.h \
    src/controller/controllerdevice.h \
    src/controller/device/devicedetector.h \
    src/controller/device/devicedataproducer.h \
    src/controller/device/devicedataconsumer.h \
    src/controller/device/consumer/plotconsumer.h \
    src/model/modeldevice.h \
    src/model/modelboard.h \
    src/model/modelchannel.h

INCLUDEPATH += \
    ./src \
    ./src/view \
    ./src/controller \
    ./src/controller/device \
    ./src/controller/device/consumer \
    ./src/model

DEPENDPATH += \
    ./src \
    ./src/view \
    ./src/controller \
    ./src/controller/device \
    ./src/controller/device/consumer \
    ./src/model

include(../e384commLib/frontPanel/includefrontpanel.pri)
include(../e384commLib/includecommlib.pri)
include(../qwt-6.1.4/includeqwt.pri)

RESOURCES += \
    resources.qrc
