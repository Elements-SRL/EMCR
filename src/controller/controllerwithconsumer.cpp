#include "controllerwithconsumer.h"

ControllerWithConsumer::ControllerWithConsumer() {

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

void ControllerWithConsumer::onCurrentRangeChanged(RangedMeasurement rm) {
    for (auto &consumer : getConsumers()) {
        consumer->onCurrentRangeChanged(rm);
    }
}

void ControllerWithConsumer::onVoltageRangeChanged(RangedMeasurement rm) {
    for (auto &consumer : getConsumers()) {
        consumer->onVoltageRangeChanged(rm);
    }
}

void ControllerWithConsumer::onStopConsuming() {
    for (auto &consumer : getConsumers()) {
        consumer->onStopConsuming();
    }
}
