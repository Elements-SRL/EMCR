#include "autodeclogger/autodecloggercontroller.h"

AutoDecloggerController::AutoDecloggerController(ApplicationStatus* appStatus, MainWindow* mainWindow, DeviceDataProducer* ddt) {
	this->appStatus = appStatus;
	widget = new AutoDecloggerWidget(appStatus->getCurrentRange(), appStatus->getVoltageRange());
	mainWindow->setDockWidget(MainWindow::DWAutoDeclogger, widget);
	consumer = new AutodecloggerConsumer(appStatus, ddt);
	connect(consumer, &AutodecloggerConsumer::sigDecloggingStarted, this, &AutoDecloggerController::onDecloggingStarted);
	connect(consumer, &AutodecloggerConsumer::sigDecloggingCompleted, this, &AutoDecloggerController::onDecloggingCompleted);
	connect(widget, &AutoDecloggerWidget::sigActive, this, &AutoDecloggerController::onActive);
	connect(widget, &AutoDecloggerWidget::sigThFieldChanged, this, &AutoDecloggerController::onThFieldChanged);
	connect(widget, &AutoDecloggerWidget::sigVotageFieldChanged, this, &AutoDecloggerController::onVoltageFieldChanged);
	connect(widget, &AutoDecloggerWidget::sigTimeFieldChanged, this, &AutoDecloggerController::onTimeFieldChanged);
}

AutoDecloggerController::~AutoDecloggerController() {

}

void AutoDecloggerController::onActive(bool active) {
	if (!active) {
		consumer->onStopConsuming();
		return;
	}
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

void AutoDecloggerController::onDecloggingStarted(std::vector<unsigned short> channels) {
	widget->onPoreClogged();
}
void AutoDecloggerController::onDecloggingCompleted(std::vector<unsigned short> channels) {
	widget->onPoreFree();
}
void AutoDecloggerController::onCurrentRangeChanged(RangedMeasurement cr) {}
void AutoDecloggerController::onVoltageRangeChanged(RangedMeasurement vr) {}

void AutoDecloggerController::onThFieldChanged(double th) {
	std::map<int, double> m;
	for (int ch = 0; ch < appStatus->getCurrentChannelsNum(); ch++) {
		m.insert(std::pair<char, int>(ch, th));
	}
	consumer->setThresholds(m);
}

void AutoDecloggerController::onVoltageFieldChanged(double v) {
	std::map<int, double> m;
	for (int ch = 0; ch < appStatus->getCurrentChannelsNum(); ch++) {
		m.insert(std::pair<char, int>(ch, v));
	}
	consumer->setVoltages(m);
}

void AutoDecloggerController::onTimeFieldChanged(double t) {
	std::map<int, double> m;
	for (int ch = 0; ch < appStatus->getCurrentChannelsNum(); ch++) {
		m.insert(std::pair<char, int>(ch, t));
	}
	consumer->setTimes(m);
}