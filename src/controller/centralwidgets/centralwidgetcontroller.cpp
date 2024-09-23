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

void CentralWidgetController::onSamplingRateChanged(Measurement sr) {
	getConsumer()->onSamplingRateChanged(sr);
} 

void CentralWidgetController::onDownsamplingRatioChanged(unsigned int dsr) {
	getConsumer()->onDownsamplingRatioChanged(dsr);
}

void CentralWidgetController::onCurrentRangeChanged(RangedMeasurement rm) {
	getConsumer()->onCurrentRangeChanged(rm);
}

void CentralWidgetController::onVoltageRangeChanged(RangedMeasurement rm) {
	getConsumer()->onVoltageRangeChanged(rm);
}

void CentralWidgetController::onClampingModalityChanged(ClampingModality_t c) {
    getConsumer()->onClampingModalityChanged(c);
}

void CentralWidgetController::onStopConsuming() {
	getConsumer()->onStopConsuming();
}
