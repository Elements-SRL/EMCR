QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG(debug, debug|release) {
    DEFINES += GLB_SHOW_DEBUG_CTRLS
    DEFINES += DEBUG
}

CONFIG(release, debug|release) {
#    DEFINES += GLB_SHOW_DEBUG_CTRLS
}

CONFIG += c++17

include(version.pri)

DEFINES += "VERSION_MAJOR=$$VERSION_MAJOR"\
    "VERSION_MINOR=$$VERSION_MINOR"\
    "VERSION_PATCH=$$VERSION_PATCH"

VERSION_FULL = $${VERSION_MAJOR}.$${VERSION_MINOR}.$${VERSION_PATCH}

SOURCES += \
    src/controller/controllerstatearray.cpp \
    src/controller/protocol/cursorsmanager.cpp \
    src/controller/protocol/protocolitemctrlmanager.cpp \
    src/controller/protocol/protocolmanager.cpp \
    src/controller/protocol/protocolutils.cpp \
    src/main.cpp \
    src/model/analysiscursor.cpp \
    src/model/state.cpp \
    src/model/statearray.cpp \
    src/view/addtagdialog.cpp \
    src/view/conversionscaledraw.cpp \
    src/view/doubleclickmachine.cpp \
    src/view/lcddisplay.cpp \
    src/view/mainwindow.cpp \
    src/view/errormanager.cpp \
    src/view/elementslogowidget.cpp \
    src/view/channeloverviewwidget.cpp \
    src/view/bigplot.cpp \
    src/view/bigplotdockwidget.cpp \
    src/view/curve.cpp \
    src/view/chessboard.cpp \
    src/view/myleftrightmousepushbutton.cpp \
    src/view/myspinbox.cpp \
    src/view/protocol/impexpprotocoldialog.cpp \
    src/view/protocol/protocolcursor.cpp \
    src/view/protocol/protocoldockwidget.cpp \
    src/view/protocol/protocoldragitem.cpp \
    src/view/protocol/protocoldropitem.cpp \
    src/view/protocol/protocoldropitemparam.cpp \
    src/view/protocol/protocoleditor.cpp \
    src/view/protocol/protocolitem.cpp \
    src/view/protocol/protocolitemdraglist.cpp \
    src/view/protocol/protocolitemdroplist.cpp \
    src/view/protocol/protocollist.cpp \
    src/view/protocol/protocolplot.cpp \
    src/view/protocol/protocolpreview.cpp \
    src/view/protocol/protocolpropertydialog.cpp \
    src/view/protocol/protocolsection.cpp \
    src/view/protocol/protocolssettingsdialog.cpp \
    src/view/protocol/protocolwidget.cpp \
    src/view/recordsettingsdialog.cpp \
    src/view/rect4.cpp \
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
    src/controller/device/consumer/datawriterconsumer.cpp \
    src/controller/device/consumer/abfdatawriterconsumer.cpp \
    src/controller/device/consumer/livenoiseconsumer.cpp \
    src/controller/device/consumer/calibrationconsumer.cpp \
    src/model/modeldevice.cpp \
    src/model/modelboard.cpp \
    src/model/modelchannel.cpp \
    src/abf/axon.cpp \
    src/view/statearray/statearraywidget.cpp

HEADERS += \
    src/controller/controllerstatearray.h \
    src/controller/protocol/cursorsmanager.h \
    src/controller/protocol/protocolitemctrlmanager.h \
    src/controller/protocol/protocolmanager.h \
    src/controller/protocol/protocolutils.h \
    src/globaldefines.h \
    src/model/analysiscursor.h \
    src/model/state.h \
    src/model/statearray.h \
    src/protocoldefs.h \
    src/view/addtagdialog.h \
    src/view/conversionscaledraw.h \
    src/view/doubleclickmachine.h \
    src/view/lcddisplay.h \
    src/view/mainwindow.h \
    src/view/errormanager.h \
    src/view/elementslogowidget.h \
    src/view/channeloverviewwidget.h \
    src/view/bigplot.h \
    src/view/bigplotdockwidget.h \
    src/view/curve.h \
    src/view/chessboard.h \
    src/view/myleftrightmousepushbutton.h \
    src/view/myspinbox.h \
    src/view/protocol/impexpprotocoldialog.h \
    src/view/protocol/protocolcursor.h \
    src/view/protocol/protocoldockwidget.h \
    src/view/protocol/protocoldragitem.h \
    src/view/protocol/protocoldropitem.h \
    src/view/protocol/protocoldropitemparam.h \
    src/view/protocol/protocoleditor.h \
    src/view/protocol/protocolitem.h \
    src/view/protocol/protocolitemdraglist.h \
    src/view/protocol/protocolitemdroplist.h \
    src/view/protocol/protocollist.h \
    src/view/protocol/protocolplot.h \
    src/view/protocol/protocolpreview.h \
    src/view/protocol/protocolpropertydialog.h \
    src/view/protocol/protocolsection.h \
    src/view/protocol/protocolssettingsdialog.h \
    src/view/protocol/protocolwidget.h \
    src/view/recordsettingsdialog.h \
    src/view/rect4.h \
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
    src/controller/device/consumer/datawriterconsumer.h \
    src/controller/device/consumer/abfdatawriterconsumer.h \
    src/controller/device/consumer/livenoiseconsumer.h \
    src/controller/device/consumer/calibrationconsumer.h \
    src/model/modeldevice.h \
    src/model/modelboard.h \
    src/model/modelchannel.h \
    src/abf/axon.h \
    src/abf/axon_defs.h \
    src/abf/axon_structs.h \
    src/view/statearray/statearraywidget.h

INCLUDEPATH += \
    ./src \
    ./src/view \
    ./src/controller \
    ./src/controller/device \
    ./src/controller/device/consumer \
    ./src/model \
    ./src/abf \
    ./src/view/protocol \
    ./src/controller/protocol

DEPENDPATH += \
    ./src \
    ./src/view \
    ./src/controller \
    ./src/controller/device \
    ./src/controller/device/consumer \
    ./src/model \
    ./src/abf \
    ./src/view/protocol \
    ./src/controller/protocol

RC_ICONS = imgs/Elements.ico
ICON = imgs/Elements.ico.icns

include(../e384commLib/frontPanel/includefrontpanel.pri)
include(../e384commLib/includecommlib.pri)
include(../qwt-6.1.4/includeqwt.pri)
include(C:/protocol-serializer/includeprotocolserializer.pri)

RESOURCES += \
    resources.qrc
