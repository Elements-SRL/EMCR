#include "bigplotcontroller.h"
#include "gapfreecontroller.h"
#include "episodiccontroller.h"
#include "ivgraphcontroller.h"
#include "eventdetectioncontroller.h"
#include "spectrumcontroller.h"

BigPlotController::BigPlotController(ApplicationStatus * appStatus, DeviceDataProducer * producer, Measurement_t defaultPlotDuration, MainWindow * mainWindow, DeviceController* dc) :
    appStatus(appStatus),
    mainWindow(mainWindow) {

    currentChannelsNum = appStatus->getCurrentChannelsNum();
    voltageChannelsNum = appStatus->getVoltageChannelsNum();

//    we only have one widget with multiple tabs
//    todo maybe we could create a widget for each tab
    bpw = new BigPlotWidget(currentChannelsNum, mainWindow);
    mainWindow->setBigPlotWidget(bpw);
    connect(bpw, &BigPlotWidget::tabBarClicked, this, &BigPlotController::manageStatus);
    
    bps = BigPlot::GapFree;
    controllers.push_back(new GapFreeController(appStatus, producer, defaultPlotDuration, bpw, mainWindow, dc));
    controllers.push_back(new EpisodicController(appStatus, producer, defaultPlotDuration, bpw, mainWindow, dc));
    controllers.push_back(new IvGraphController(appStatus, producer, bpw, mainWindow));
    controllers.push_back(new SpectrumController(appStatus, producer, {100.0, UnitPfxKilo, "Hz"}, bpw, mainWindow));
    controllers.push_back(new EventDetectionController(appStatus, producer, bpw));
    controllers[bps]->start();
}

void BigPlotController::manageStatus(int idx) {
    if (idx == bps || !(bpw->isTabEnabled(idx))) {
        return;
    }
    controllers[bps]->stop();
    bps = static_cast<BigPlot::BigPlotStatus>(idx < 0 ? static_cast<int>(BigPlot::BigPlotStatus::NumberOfStatuses) : idx);
    controllers[bps]->start();
}

BigPlotController::~BigPlotController() {
    for (auto &c : controllers) {
        delete c;
    }
    controllers.clear();
    if (bpw != nullptr) {
        delete bpw;
        bpw = nullptr;
        mainWindow->setBigPlotWidget(bpw);
    }
}

//todo Bisognerà controllare anche la clampingmodality
void BigPlotController::onRangeUpdated(RangedMeasurement_t newRange) {
    for (auto c : controllers) {
        c->onRangeUpdated(newRange);
    }
}

void BigPlotController::onProtocolStarted(unsigned int protId, ProtocolWidget * protocol) {
    if (bps != BigPlot::GapFree && bps != BigPlot::Episodic) {
        return;
    }
    if (bps == BigPlot::GapFree && protocol->getType() == ProtocolTypeEpisodic) {
        bpw->setCurrentIndex(BigPlot::Episodic);
        this->manageStatus(BigPlot::Episodic);
    }
    if (bps == BigPlot::Episodic && protocol->getType() == ProtocolTypeGapfree) {
        bpw->setCurrentIndex(BigPlot::GapFree);
        this->manageStatus(BigPlot::GapFree);
    }
}

void BigPlotController::onCurrentColorsChanged(QVector <QColor> colors) {
    for (auto c : controllers) {
        c->onCurrentColorsChanged(colors);
    }
}

void BigPlotController::onCurrentColorChanged(int channelIdx, QColor color) {
    for (auto c : controllers) {
        c->onCurrentColorChanged(channelIdx, color);
    }
}

void BigPlotController::onBackgroundColorChanged(QColor color) {
    for (auto c : controllers) {
        c->onBackgroundColorChanged(color);
    }
}

void BigPlotController::onExpandTrace(bool flag) {
    for (auto c : controllers) {
        c->onExpandTrace(flag);
    }
    if (flag) {
        controllers[bps]->start();
    }
}

std::vector <PlotConsumer*> BigPlotController::getConsumers() {
    std::vector <PlotConsumer*> consumers;
    for (auto c : controllers) {
        consumers.push_back(c->getConsumer());
    }
    return consumers;
}

std::vector <CentralWidgetController*> BigPlotController::getControllers() {
    return controllers;
}
