#include "autodeclogger/autodecloggercontroller.h"

AutoDecloggerController::AutoDecloggerController(ApplicationStatus* appStatus, MainWindow* mainWindow, DeviceDataProducer* ddt) {
	this->appStatus = appStatus;
	widget = new AutoDecloggerWidget(appStatus->getCurrentRange(), appStatus->getVoltageRange());
	widget->show();
	consumer = new AutodecloggerConsumer(appStatus, ddt);
	connect(consumer, &AutodecloggerConsumer::sigDecloggingStarted, this, &AutoDecloggerController::onDecloggingStarted);
	connect(consumer, &AutodecloggerConsumer::sigDecloggingCompleted, this, &AutoDecloggerController::onDecloggingCompleted);
	connect(widget, &AutoDecloggerWidget::sigActivate, this, &AutoDecloggerController::onStart);
	connect(widget, &AutoDecloggerWidget::sigStop, this, &AutoDecloggerController::onStop);
}

AutoDecloggerController::~AutoDecloggerController() {

}

void AutoDecloggerController::onStart() {
	const auto th = widget->getThreshold();
	const auto v = widget->getVoltage();
	const auto mst = widget->getTime();
	const auto chNum = appStatus->getCurrentChannelsNum();
	std::vector<double> ths;
	std::vector<double> vs;
	std::vector<double> msts;
	for (int i = 0; i < chNum; i++) {
		ths.push_back(th);
		vs.push_back(v);
		msts.push_back(mst);
	}
	model = new AutoDecloggerModel(ths, vs, msts);
	if (consumer->isRunning()) {
		consumer->onStopConsuming();
		consumer->setModel(model);
	} else {
		consumer->setModel(model);
	}
	consumer->onStartConsuming();
}

void AutoDecloggerController::onStop() {
}

void AutoDecloggerController::onDecloggingStarted(std::vector<int> channels) {
	widget->onPoreClogged();
}
void AutoDecloggerController::onDecloggingCompleted(std::vector<int> channels) {
	widget->onPoreFree();
}
void AutoDecloggerController::onCurrentRangeChanged(RangedMeasurement cr) {}
void AutoDecloggerController::onVoltageRangeChanged(RangedMeasurement vr) {}