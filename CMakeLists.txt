cmake_minimum_required(VERSION 3.21)

project(EMCR
    VERSION 0.28.2
    LANGUAGES CXX
)

include(CMakePackageConfigHelpers)
include(GNUInstallDirs)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# Executable

add_executable(EMCR
    src/main.cpp
    src/controller/device/consumer/analysisconsumer.cpp
    src/controller/device/consumer/ccresistanceestimationconsumer.cpp
    src/controller/device/consumer/membraneestimationconsumer.cpp
    src/controller/device/consumer/pipettecapacitanceestimationconsumer.cpp
    src/controller/device/consumer/resistanceestimationconsumer.cpp
    src/controller/device/consumer/squarecurrentbasedanalysisconsumer.cpp
    src/controller/device/consumer/squarevoltagebasedanalysisconsumer.cpp
    src/controller/device/consumer/ivgraphconsumer.cpp
    src/controller/device/consumer/livestatisticsconsumer.cpp
    src/controller/device/consumer/eventdetectionconsumer.cpp
    src/controller/device/consumer/spectrumconsumer.cpp
    src/controller/device/deviceconnector.cpp
    src/controller/controllerwithconsumer.cpp
    src/controller/bigplotviewcontroller.cpp
    src/controller/debugcontroller.cpp
    src/controller/durationbasedbigplotviewcontroller.cpp
    src/controller/plotdetailcontroller.cpp
    src/controller/temperaturecontroller.cpp
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
    src/globaldefines.h
    src/model/channelstate.h
    src/model/membraneresult.h
    src/model/plotdetailmodel.h
    src/model/singlemeasresult.h
    src/protocoldefs.h
    src/controller/device/consumer/analysisconsumer.h
    src/controller/device/consumer/ccresistanceestimationconsumer.h
    src/controller/device/consumer/membraneestimationconsumer.h
    src/controller/device/consumer/pipettecapacitanceestimationconsumer.h
    src/controller/device/consumer/resistanceestimationconsumer.h
    src/controller/device/consumer/squarecurrentbasedanalysisconsumer.h
    src/controller/device/consumer/squarevoltagebasedanalysisconsumer.h
    src/controller/device/consumer/ivgraphconsumer.h
    src/controller/device/consumer/livestatisticsconsumer.h
    src/controller/device/consumer/eventdetectionconsumer.h
    src/controller/device/consumer/spectrumconsumer.h
    src/controller/device/deviceconnector.h
    src/controller/controllerwithconsumer.h
    src/controller/bigplotviewcontroller.h
    src/controller/debugcontroller.h
    src/controller/durationbasedbigplotviewcontroller.h
    src/controller/plotdetailcontroller.h
    src/controller/temperaturecontroller.h
    src/controller/measurementoverviewcontroller.h
    src/controller/multiplechannelcontroller.h
    src/controller/plotpreferencescontroller.h
    src/controller/singlechannelcontroller.h
    src/controller/upgradefwcontroller.h
    src/controller/bigplotcontroller.h
    src/controller/boardcontroller.h
    src/controller/chessboardcontroller.h
    src/controller/compensationcontroller.h
    src/controller/devicecontroller.h
    src/controller/offsetcorrectioncontroller.h
    src/controller/maincontroller.h
    src/controller/protocol/cursorsmanager.h
    src/controller/protocol/protocolitemctrlmanager.h
    src/controller/protocol/protocolmanager.h
    src/controller/protocol/protocolutils.h
    src/controller/statearraycontroller.h
    src/controller/device/devicedetector.h
    src/controller/device/devicedataproducer.h
    src/controller/device/devicedataconsumer.h
    src/controller/device/consumer/plotconsumer.h
    src/controller/device/consumer/datawriterconsumer.h
    src/controller/device/consumer/abfdatawriterconsumer.h
    src/controller/device/consumer/autodecloggerconsumer.h
    src/controller/centralwidgets/centralwidgetcontroller.h
    src/controller/centralwidgets/ivgraphcontroller.h
    src/controller/centralwidgets/gapfreecontroller.h
    src/controller/centralwidgets/episodiccontroller.h
    src/controller/centralwidgets/eventdetectioncontroller.h
    src/controller/centralwidgets/spectrumcontroller.h
    src/controller/autodeclogger/autodecloggercontroller.h
    src/model/multiplechannelmodel.h
    src/model/devicemodel.h
    src/model/application_status.h
    src/model/bigplotmodel.h
    src/model/channel_and_name.h
    src/model/channel_mapping.h
    src/model/ivaccumulator.h
    src/model/ivchannel.h
    src/model/measurementoverviewmodel.h
    src/model/plotmessage.h
    src/model/plotpreferencesmodel.h
    src/model/protocolmodel.h
    src/model/state.h
    src/model/statearray.h
    src/model/statisticsresult.h
    src/model/resultwrapper.h
    src/model/binner.h
    src/model/logbigplotmodel.h
    src/model/filters/filter.h
    src/model/filters/firstorderiirfilter.h
    src/model/events/event.h
    src/model/events/eventinfo.h
    src/model/events/eventdetector.h
    src/model/events/baseline.h
    src/model/events/eventpacket.h
    src/model/events/eventsandbaseline.h
    src/model/autodeclogger/autodecloggermodel.h
    src/view/activationbutton.h
    src/view/collapsiblesection.h
    src/view/chessboarddockwidget.h
    src/view/colorselectionbutton.h
    src/view/compensationcontroldockwidget.h
    src/view/addtagdialog.h
    src/view/conversionscaledraw.h
    src/view/copyabletable.h
    src/view/debugdockwidget.h
    src/view/doubleclickmachine.h
    src/view/lcddisplay.h
    src/view/leftrightmousepushbutton.h
    src/view/mainwindow.h
    src/view/errormanager.h
    src/view/elementslogowidget.h
    src/view/baseplot.h
    src/view/curve.h
    src/view/measurementsoverviewdockwidget.h
    src/view/multiplechannelcontroldockwidget.h
    src/view/nowheelspinbox.h
    src/view/plotdetail.h
    src/view/plotpreferencesdialog.h
    src/view/centralwidgets/bigplot.h
    src/view/centralwidgets/bigplotwidget.h
    src/view/centralwidgets/gapfreewidget.h
    src/view/centralwidgets/episodicwidget.h
    src/view/centralwidgets/ivgraphwidget.h
    src/view/centralwidgets/eventdetectionwidget.h
    src/view/centralwidgets/spectrumwidget.h
    src/view/protocol/impexpprotocoldialog.h
    src/view/protocol/protocolcursor.h
    src/view/protocol/protocoldockwidget.h
    src/view/protocol/protocoldragitem.h
    src/view/protocol/protocoldropitem.h
    src/view/protocol/protocoldropitemparam.h
    src/view/protocol/protocoleditor.h
    src/view/protocol/protocolitem.h
    src/view/protocol/protocolitemdraglist.h
    src/view/protocol/protocolitemdroplist.h
    src/view/protocol/protocollist.h
    src/view/protocol/protocolplot.h
    src/view/protocol/protocolpreview.h
    src/view/protocol/protocolpropertydialog.h
    src/view/protocol/protocolsection.h
    src/view/protocol/protocolwidget.h
    src/view/autodeclogger/autodecloggerwidget.h
    src/view/recordsettingsdialog.h
    src/view/rect4.h
    src/view/singlechannelcontroldockwidget.h
    src/view/stampplot.h
    src/view/boardcontroldockwidget.h
    src/view/devicecontroldockwidget.h
    src/view/statearray/statearraydockwidget.h
    src/view/advancedmenu/resethwhelpdialog.h
    src/view/questionmarkmenu/aboutdialog.h
    src/view/questionmarkmenu/supportdialog.h
    src/view/questionmarkmenu/deviceinfodialog.h
    src/view/questionmarkmenu/releasenotesdialog.h
    src/view/questionmarkmenu/messagedialog.h
    src/view/temperaturedockwidget.h
    src/view/upgradefwview.h
    resources.qrc
    app.rc
    imgs/Elements.ico.icns
)

# resources

target_sources(EMCR PRIVATE
    resources.qrc
)

set(APP_ICON_RESOURCE_WINDOWS "${CMAKE_CURRENT_SOURCE_DIR}/imgs/Elements.ico")
target_sources(EMCR PRIVATE ${APP_ICON_RESOURCE_WINDOWS})

# C standard

set_target_properties(EMCR PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)

# incldue directories

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

# Defines

target_compile_definitions(EMCR
    PUBLIC
    VERSION_MAJOR=${PROJECT_VERSION_MAJOR}
    VERSION_MINOR=${PROJECT_VERSION_MINOR}
    VERSION_PATCH=${PROJECT_VERSION_PATCH}
    VERSION_FULL=VERSION_MAJOR.VERSION_MINOR.VERSION_PATCH
    _MATH_DEFINES_DEFINED
)

# Libraries

find_package(Qt6 REQUIRED COMPONENTS Core Widgets)

cmake_path(SET QWT_PATH_CMAKE "$ENV{QWT_PATH}")
include(${QWT_PATH_CMAKE}cmake/Findqwt.cmake)

cmake_path(SET POCKETFFT_PATH_CMAKE "$ENV{POCKETFFT_PATH}")
include(${POCKETFFT_PATH_CMAKE}cmake/Findpocketfft.cmake)

cmake_path(SET PROTOCOL_SERIALIZER_PATH_CMAKE "$ENV{PROTOCOL_SERIALIZER_PATH}")
list(APPEND CMAKE_PREFIX_PATH "${PROTOCOL_SERIALIZER_PATH_CMAKE}")
find_package(prot-ser REQUIRED CONFIG)

cmake_path(SET AXONLIB_PATH_CMAKE "$ENV{AXONLIB_PATH}")
list(APPEND CMAKE_PREFIX_PATH "${AXONLIB_PATH_CMAKE}")
find_package(axonlib REQUIRED CONFIG)

cmake_path(SET HDF5_PATH_CMAKE "$ENV{HDF5_PATH}")
include(${HDF5_PATH_CMAKE}cmake/Findhdf5.cmake)

cmake_path(SET E384COMMLIB_PATH_CMAKE "$ENV{E384COMMLIB_PATH}")
list(APPEND CMAKE_PREFIX_PATH "${E384COMMLIB_PATH_CMAKE}")
find_package(e384commlib REQUIRED CONFIG)

target_link_libraries(EMCR
    PRIVATE
    Qt6::Core
    Qt6::Widgets
    pocketfft::pocketfft
    qwt::qwt
    prot-ser::prot-ser
    axonlib::axonlib
    hdf5::hdf5
    hdf5::hdf5_cpp
    e384commlib::e384commlib
)
