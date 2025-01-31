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
    for (auto &consumer : getConsumers()) {
        consumer->onSamplingRateChanged(sr);
    }
} 

void CentralWidgetController::onDownsamplingRatioChanged(unsigned int dsr) {
    for (auto &consumer : getConsumers()) {
        consumer->onDownsamplingRatioChanged(dsr);
    }
}

void CentralWidgetController::onCurrentRangeChanged(RangedMeasurement rm) {
    for (auto &consumer : getConsumers()) {
        consumer->onCurrentRangeChanged(rm);
    }
}

void CentralWidgetController::onVoltageRangeChanged(RangedMeasurement rm) {
    for (auto &consumer : getConsumers()) {
        consumer->onVoltageRangeChanged(rm);
    }
}

void CentralWidgetController::onStopConsuming() {
    for (auto &consumer : getConsumers()) {
        consumer->onStopConsuming();
    }
}
