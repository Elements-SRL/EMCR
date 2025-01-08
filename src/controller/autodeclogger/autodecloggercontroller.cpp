#include "autodeclogger/autodecloggercontroller.h"

AutoDecloggerController::AutoDecloggerController(ApplicationStatus* appStatus, MainWindow* mainWindow, DeviceDataProducer* ddt) {
	this->appStatus = appStatus;
	widget = new AutoDecloggerWidget(appStatus->getCurrentRange(), appStatus->getVoltageRange());
	widget->show();
	consumer = new AutodecloggerConsumer(appStatus, ddt);
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

}

void AutoDecloggerController::onStop() {
}

void AutoDecloggerController::onDecloggingStarted() {}
void AutoDecloggerController::onDecloggingCompleted() {}
void AutoDecloggerController::onCurrentRangeChanged(RangedMeasurement cr) {}
void AutoDecloggerController::onVoltageRangeChanged(RangedMeasurement vr) {}