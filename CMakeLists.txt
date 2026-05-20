cmake_minimum_required(VERSION 3.21)

project(EMCR
    VERSION 0.28.1
    LANGUAGES CXX
)

include(CMakePackageConfigHelpers)

#
# Eseguibile
#

add_executable(EMCR
    src/controller/bigplotviewcontroller.cpp
    src/controller/debugcontroller.cpp
    src/controller/device/consumer/analysisconsumer.cpp
    src/controller/device/consumer/ccresistanceestimationconsumer.cpp
    src/controller/device/consumer/membraneestimationconsumer.cpp
    src/controller/device/consumer/pipettecapacitanceestimationconsumer.cpp
    src/controller/device/consumer/resistanceestimationconsumer.cpp
    src/controller/device/consumer/squarecurrentbasedanalysisconsumer.cpp
    src/controller/device/consumer/squarevoltagebasedanalysisconsumer.cpp
    src/controller/durationbasedbigplotviewcontroller.cpp
    src/controller/plotdetailcontroller.cpp
    src/controller/temperaturecontroller.cpp
    src/main.cpp
    src/controller/device/consumer/ivgraphconsumer.cpp
    src/controller/device/consumer/livestatisticsconsumer.cpp
    src/controller/device/consumer/eventdetectionconsumer.cpp
    src/controller/device/consumer/spectrumconsumer.cpp
    src/controller/device/deviceconnector.cpp
    src/controller/controllerwithconsumer.cpp
    src/controller/measurementoverviewcontroller.cpp
    src/controller/multiplechannelcontroller.cpp
    src/controller/plotpreferencescontroller.cpp
    src/controller/singlechannelcontroller.cpp
    src/controller/upgradefwcontroller.cpp
    src/controller/bigplotcontroller.cpp
    src/controller/boardcontroller.cpp
    src/controller/chessboardcontroller.cpp
    src/controller/compensationcontroller.cpp
    src/controller/devicecontroller.cpp
    src/controller/offsetcorrectioncontroller.cpp
    src/controller/maincontroller.cpp
    src/controller/protocol/cursorsmanager.cpp
    src/controller/protocol/protocolitemctrlmanager.cpp
    src/controller/protocol/protocolmanager.cpp
    src/controller/protocol/protocolutils.cpp
    src/controller/statearraycontroller.cpp
    src/controller/device/devicedetector.cpp
    src/controller/device/devicedataproducer.cpp
    src/controller/device/devicedataconsumer.cpp
    src/controller/device/consumer/plotconsumer.cpp
    src/controller/device/consumer/datawriterconsumer.cpp
    src/controller/device/consumer/abfdatawriterconsumer.cpp
    src/controller/device/consumer/autodecloggerconsumer.cpp
    src/controller/centralwidgets/centralwidgetcontroller.cpp
    src/controller/centralwidgets/ivgraphcontroller.cpp
    src/controller/centralwidgets/gapfreecontroller.cpp
    src/controller/centralwidgets/episodiccontroller.cpp
    src/controller/centralwidgets/eventdetectioncontroller.cpp
    src/controller/centralwidgets/spectrumcontroller.cpp
    src/controller/autodeclogger/autodecloggercontroller.cpp
    src/model/channelstate.cpp
    src/model/devicemodel.cpp
    src/model/application_status.cpp
    src/model/bigplotmodel.cpp
    src/model/ivaccumulator.cpp
    src/model/ivchannel.cpp
    src/model/measurementoverviewmodel.cpp
    src/model/multiplechannelmodel.cpp
    src/model/plotdetailmodel.cpp
    src/model/plotpreferencesmodel.cpp
    src/model/protocolmodel.cpp
    src/model/state.cpp
    src/model/statearray.cpp
    src/model/binner.cpp
    src/model/logbigplotmodel.cpp
    src/model/filters/firstorderiirfilter.cpp
    src/model/events/eventdetector.cpp
    src/view/activationbutton.cpp
    src/view/collapsiblesection.cpp
    src/view/chessboarddockwidget.cpp
    src/view/colorselectionbutton.cpp
    src/view/compensationcontroldockwidget.cpp
    src/view/addtagdialog.cpp
    src/view/conversionscaledraw.cpp
    src/view/copyabletable.cpp
    src/view/debugdockwidget.cpp
    src/view/doubleclickmachine.cpp
    src/view/lcddisplay.cpp
    src/view/leftrightmousepushbutton.cpp
    src/view/mainwindow.cpp
    src/view/errormanager.cpp
    src/view/elementslogowidget.cpp
    src/view/channeloverviewwidget.cpp
    src/view/baseplot.cpp
    src/view/curve.cpp
    src/view/measurementsoverviewdockwidget.cpp
    src/view/multiplechannelcontroldockwidget.cpp
    src/view/nowheelspinbox.cpp
    src/view/plotdetail.cpp
    src/view/plotpreferencesdialog.cpp
    src/view/centralwidgets/bigplot.cpp
    src/view/centralwidgets/bigplotwidget.cpp
    src/view/centralwidgets/gapfreewidget.cpp
    src/view/centralwidgets/episodicwidget.cpp
    src/view/centralwidgets/spectrumwidget.cpp
    src/view/centralwidgets/ivgraphwidget.cpp
    src/view/centralwidgets/eventdetectionwidget.cpp
    src/view/protocol/impexpprotocoldialog.cpp
    src/view/protocol/protocolcursor.cpp
    src/view/protocol/protocoldockwidget.cpp
    src/view/protocol/protocoldragitem.cpp
    src/view/protocol/protocoldropitem.cpp
    src/view/protocol/protocoldropitemparam.cpp
    src/view/protocol/protocoleditor.cpp
    src/view/protocol/protocolitem.cpp
    src/view/protocol/protocolitemdraglist.cpp
    src/view/protocol/protocolitemdroplist.cpp
    src/view/protocol/protocollist.cpp
    src/view/protocol/protocolplot.cpp
    src/view/protocol/protocolpreview.cpp
    src/view/protocol/protocolpropertydialog.cpp
    src/view/protocol/protocolsection.cpp
    src/view/protocol/protocolwidget.cpp
    src/view/autodeclogger/autodecloggerwidget.cpp
    src/view/recordsettingsdialog.cpp
    src/view/rect4.cpp
    src/view/singlechannelcontroldockwidget.cpp
    src/view/stampplot.cpp
    src/view/boardcontroldockwidget.cpp
    src/view/devicecontroldockwidget.cpp
    src/view/statearray/statearraydockwidget.cpp
    src/view/advancedmenu/resethwhelpdialog.cpp
    src/view/questionmarkmenu/aboutdialog.cpp
    src/view/questionmarkmenu/supportdialog.cpp
    src/view/questionmarkmenu/deviceinfodialog.cpp
    src/view/questionmarkmenu/releasenotesdialog.cpp
    src/view/questionmarkmenu/messagedialog.cpp
    src/view/temperaturedockwidget.cpp
    src/view/upgradefwview.cpp
)

#
# C standard
#

set_target_properties(EMCR PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)

#
# Header pubblici
#

target_include_directories(EMCR
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/src/controller
        ${CMAKE_CURRENT_SOURCE_DIR}/src/controller/device
        ${CMAKE_CURRENT_SOURCE_DIR}/src/controller/device/consumer
        ${CMAKE_CURRENT_SOURCE_DIR}/src/controller/protocol
        ${CMAKE_CURRENT_SOURCE_DIR}/src/controller/centralwidgets
        ${CMAKE_CURRENT_SOURCE_DIR}/src/controller/autodeclogger
        ${CMAKE_CURRENT_SOURCE_DIR}/src/model
        ${CMAKE_CURRENT_SOURCE_DIR}/src/model/filters
        ${CMAKE_CURRENT_SOURCE_DIR}/src/model/events
        ${CMAKE_CURRENT_SOURCE_DIR}/src/model/autodeclogger
        ${CMAKE_CURRENT_SOURCE_DIR}/src/view
        ${CMAKE_CURRENT_SOURCE_DIR}/src/view/statearray
        ${CMAKE_CURRENT_SOURCE_DIR}/src/view/centralwidgets
        ${CMAKE_CURRENT_SOURCE_DIR}/src/view/protocol
        ${CMAKE_CURRENT_SOURCE_DIR}/src/view/autodeclogger
        ${CMAKE_CURRENT_SOURCE_DIR}/src/view/questionmarkmenu
        ${CMAKE_CURRENT_SOURCE_DIR}/src/view/advancedmenu
)

#
# Define pubbliche
#

target_compile_definitions(prot-ser
    PUBLIC
        VERSION_MAJOR=${PROJECT_VERSION_MAJOR}
        VERSION_MINOR=${PROJECT_VERSION_MINOR}
        VERSION_PATCH=${PROJECT_VERSION_PATCH}
        VERSION_FULL=VERSION_MAJOR.VERSION_MINOR.VERSION_PATCH
)

#
# Dipendenza transitiva
#

cmake_path(SET E384COMLLIB_PATH_CMAKE "$ENV{E384COMLLIB_PATH}")
list(APPEND CMAKE_PREFIX_PATH "${E384COMLLIB_PATH_CMAKE}")

find_package(e384commlib REQUIRED CONFIG)

target_link_libraries(prot-ser
    PRIVATE
        e384commlib::e384commlib
)
