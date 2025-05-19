#include "centralwidgetcontroller.h"

CentralWidgetController::CentralWidgetController(ApplicationStatus* appStatus, DeviceDataProducer* producer, BigPlotWidget* bigPlotWidget) :
    ControllerWithConsumer(appStatus) {
	this->producer = producer;
	this->bigPlotWidget = bigPlotWidget;

	currentChannelsNum = appStatus->getCurrentChannelsNum();
	voltageChannelsNum = appStatus->getVoltageChannelsNum();
}

bool CentralWidgetController::isAtLeastOneChannelExpanded() {
    return appStatus->getExpandedChannelsIndexes().size() > 0;
}
