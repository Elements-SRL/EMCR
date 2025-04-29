#include "controllerwithconsumer.h"

ControllerWithConsumer::ControllerWithConsumer(ApplicationStatus * appStatus) :
    appStatus(appStatus) {

}

ApplicationStatus * ControllerWithConsumer::getAppStatus() {
    return appStatus;
}

void ControllerWithConsumer::onSamplingRateChanged(Measurement sr) {
    for (auto &consumer : getConsumers()) {
        consumer->onSamplingRateChanged(sr);
    }
}

void ControllerWithConsumer::onDownsamplingRatioChanged(unsigned int dsr) {
    for (auto &consumer : getConsumers()) {
        consumer->onDownsamplingRatioChanged(dsr);
    }
}

void ControllerWithConsumer::onCurrentRangeChanged() {
    for (auto &consumer : getConsumers()) {
        consumer->onCurrentRangeChanged();
    }
}

void ControllerWithConsumer::onVoltageRangeChanged() {
    for (auto &consumer : getConsumers()) {
        consumer->onVoltageRangeChanged();
    }
}

void ControllerWithConsumer::onStopConsuming() {
    for (auto &consumer : getConsumers()) {
        consumer->onStopConsuming();
    }
}
