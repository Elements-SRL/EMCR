QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG(debug, debug|release) {
    DEFINES += GLB_SHOW_DEBUG_CTRLS
    DEFINES += DEBUG
}

CONFIG(release, debug|release) {
    DEFINES += GLB_SHOW_DEBUG_CTRLS
    DEFINES += DEBUG
}

#DEFINES += GLB_ANALYSES_IN_PROTOCOL_EDITOR
#DEFINES += GLB_CURSORS_IN_PROTOCOL_EDITOR

CONFIG += c++17

include(version.pri)

DEFINES += "VERSION_MAJOR=$$VERSION_MAJOR"\
    "VERSION_MINOR=$$VERSION_MINOR"\
    "VERSION_PATCH=$$VERSION_PATCH"

VERSION_FULL = $${VERSION_MAJOR}.$${VERSION_MINOR}.$${VERSION_PATCH}

SOURCES += \
    src/controller/device/consumer/livestatisticsconsumer.cpp \
    src/controller/device/deviceconnector.cpp \
    src/controller/measurementoverviewcontroller.cpp \
    src/controller/multiplechannelcontroller.cpp \
    src/controller/plotpreferencescontroller.cpp \
    src/controller/singlechannelcontroller.cpp \
    src/controller/upgradefwcontroller.cpp \
    src/main.cpp \
    src/controller/bigplotcontroller.cpp \
    src/controller/boardcontroller.cpp \
    src/controller/chessboardcontroller.cpp \
    src/controller/compensationcontroller.cpp \
    src/controller/devicecontroller.cpp \
    src/controller/maincontroller.cpp \
    src/controller/protocol/cursorsmanager.cpp \
    src/controller/protocol/protocolitemctrlmanager.cpp \
    src/controller/protocol/protocolmanager.cpp \
    src/controller/protocol/protocolutils.cpp \
    src/controller/statearraycontroller.cpp \
    src/controller/device/devicedetector.cpp \
    src/controller/device/devicedataproducer.cpp \
    src/controller/device/devicedataconsumer.cpp \
    src/controller/device/consumer/plotconsumer.cpp \
    src/controller/device/consumer/datawriterconsumer.cpp \
    src/controller/device/consumer/abfdatawriterconsumer.cpp \
    src/controller/device/consumer/calibrationconsumer.cpp \
    src/model/analysiscursor.cpp \
    src/model/application_status.cpp \
    src/model/bigplotmodel.cpp \
    src/model/measurementoverviewmodel.cpp \
    src/model/plotpreferencesmodel.cpp \
    src/model/protocolmodel.cpp \
    src/model/state.cpp \
    src/model/statearray.cpp \
    src/model/statisticsresult.cpp \
    src/view/bigplotwidget.cpp \
    src/view/chessboarddockwidget.cpp \
    src/view/colorselectionbutton.cpp \
    src/view/compensationcontroldockwidget.cpp \
    src/view/addtagdialog.cpp \
    src/view/conversionscaledraw.cpp \
    src/view/doubleclickmachine.cpp \
    src/view/lcddisplay.cpp \
    src/view/mainwindow.cpp \
    src/view/errormanager.cpp \
    src/view/elementslogowidget.cpp \
    src/view/channeloverviewwidget.cpp \
    src/view/bigplot.cpp \
    src/view/curve.cpp \
    src/view/measurementsoverviewdockwidget.cpp \
    src/view/multiplechannelcontroldockwidget.cpp \
    src/view/myleftrightmousepushbutton.cpp \
    src/view/myspinbox.cpp \
    src/view/plotpreferencesdialog.cpp \
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
    src/view/protocol/protocolwidget.cpp \
    src/view/recordsettingsdialog.cpp \
    src/view/rect4.cpp \
    src/view/singlechannelcontroldockwidget.cpp \
    src/view/stampplot.cpp \
    src/view/boardcontroldockwidget.cpp \
    src/view/devicecontroldockwidget.cpp \
    src/view/statearray/statearraydockwidget.cpp \
    src/abf/axon.cpp \
    src/view/upgradefwview.cpp

HEADERS += \
    src/controller/device/consumer/livestatisticsconsumer.h \
    src/controller/device/deviceconnector.h \
    src/controller/measurementoverviewcontroller.h \
    src/controller/multiplechannelcontroller.h \
    src/controller/plotpreferencescontroller.h \
    src/controller/singlechannelcontroller.h \
    src/controller/upgradefwcontroller.h \
    src/globaldefines.h \
    src/controller/bigplotcontroller.h \
    src/controller/boardcontroller.h \
    src/controller/chessboardcontroller.h \
    src/controller/compensationcontroller.h \
    src/controller/devicecontroller.h \
    src/controller/maincontroller.h \
    src/controller/protocol/cursorsmanager.h \
    src/controller/protocol/protocolitemctrlmanager.h \
    src/controller/protocol/protocolmanager.h \
    src/controller/protocol/protocolutils.h \
    src/controller/statearraycontroller.h \
    src/controller/device/devicedetector.h \
    src/controller/device/devicedataproducer.h \
    src/controller/device/devicedataconsumer.h \
    src/controller/device/consumer/plotconsumer.h \
    src/controller/device/consumer/datawriterconsumer.h \
    src/controller/device/consumer/abfdatawriterconsumer.h \
    src/controller/device/consumer/calibrationconsumer.h \
    src/model/analysiscursor.h \
    src/model/application_status.h \
    src/model/bigplotmodel.h \
    src/model/channel_and_name.h \
    src/model/channel_mapping.h \
    src/model/measurementoverviewmodel.h \
    src/model/plotpreferencesmodel.h \
    src/model/protocolmodel.h \
    src/model/state.h \
    src/model/statearray.h \
    src/model/statisticsresult.h \
    src/view/bigplotwidget.h \
    src/view/chessboarddockwidget.h \
    src/view/colorselectionbutton.h \
    src/view/compensationcontroldockwidget.h \
    src/view/addtagdialog.h \
    src/view/conversionscaledraw.h \
    src/view/doubleclickmachine.h \
    src/view/lcddisplay.h \
    src/view/mainwindow.h \
    src/view/errormanager.h \
    src/view/elementslogowidget.h \
    src/view/channeloverviewwidget.h \
    src/view/bigplot.h \
    src/view/curve.h \
    src/view/measurementsoverviewdockwidget.h \
    src/view/multiplechannelcontroldockwidget.h \
    src/view/myleftrightmousepushbutton.h \
    src/view/myspinbox.h \
    src/view/plotpreferencesdialog.h \
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
    src/view/protocol/protocolwidget.h \
    src/view/recordsettingsdialog.h \
    src/view/rect4.h \
    src/view/singlechannelcontroldockwidget.h \
    src/view/stampplot.h \
    src/view/boardcontroldockwidget.h \
    src/view/devicecontroldockwidget.h \
    src/view/statearray/statearraydockwidget.h \
    src/protocoldefs.h \
    src/abf/axon.h \
    src/abf/axon_defs.h \
    src/abf/axon_structs.h \
    src/view/upgradefwview.h

INCLUDEPATH += \
    ./src \
    ./src/controller \
    ./src/controller/device \
    ./src/controller/device/consumer \
    ./src/controller/protocol \
    ./src/model \
    ./src/view \
    ./src/view/statearray \
    ./src/view/protocol \
    ./src/abf

DEPENDPATH += \
    ./src \
    ./src/controller \
    ./src/controller/device \
    ./src/controller/device/consumer \
    ./src/controller/protocol \
    ./src/model \
    ./src/view \
    ./src/view/statearray \
    ./src/view/protocol \
    ./src/abf

RC_ICONS = imgs/Elements.ico
ICON = imgs/Elements.ico.icns

include($$(E384COMMLIB_PATH)includecommlib.pri)
include($$(QWT_PATH)includeqwt.pri)
include($$(PROTOCOL_SERIALIZER_PATH)includeprotocolserializer.pri)

RESOURCES += \
    resources.qrc
