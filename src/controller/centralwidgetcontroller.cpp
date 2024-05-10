#include "centralwidgetcontroller.h"

CentralWidgetController::CentralWidgetController(ApplicationStatus* appStatus, DeviceDataProducer* producer, BigPlotWidget* bigPlotWidget) {
	this->appStatus = appStatus;
	this->producer = producer;
	this->bigPlotWidget = bigPlotWidget;

	currentChannelsNum = appStatus->getCurrentChannelsNum();
	voltageChannelsNum = appStatus->getVoltageChannelsNum();
}

CentralWidgetController::~CentralWidgetController() {
	if (appStatus != nullptr) {
		delete appStatus;
		appStatus = nullptr;
	}
	if (producer != nullptr) {
		delete producer;
		producer = nullptr;
	}
	if (bigPlotWidget != nullptr) {
		delete bigPlotWidget;
		bigPlotWidget = nullptr;
	}
}
