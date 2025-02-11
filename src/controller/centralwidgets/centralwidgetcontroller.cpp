#include "centralwidgetcontroller.h"

CentralWidgetController::CentralWidgetController(ApplicationStatus* appStatus, DeviceDataProducer* producer, BigPlotWidget* bigPlotWidget) {
	this->appStatus = appStatus;
	this->producer = producer;
	this->bigPlotWidget = bigPlotWidget;

	currentChannelsNum = appStatus->getCurrentChannelsNum();
	voltageChannelsNum = appStatus->getVoltageChannelsNum();
}

bool CentralWidgetController::isAtLeastOneChannelExpanded() {
	auto channels = appStatus->getChannels();
	for (auto c : channels) {
		if (c->isExpanded()) {
			return true;
		}
	}
	return false;
}
