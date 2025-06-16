#include "bigplotcontroller.h"
#include "gapfreecontroller.h"
#include "episodiccontroller.h"
#include "ivgraphcontroller.h"
#include "eventdetectioncontroller.h"
#include "spectrumcontroller.h"

BigPlotController::BigPlotController(ApplicationStatus * appStatus, DeviceDataProducer * producer, RangedMeasurement_t defaultPlotDuration, MainWindow * mainWindow, DeviceController* dc) :
    appStatus(appStatus),
    mainWindow(mainWindow) {

    currentChannelsNum = appStatus->getCurrentChannelsNum();
    voltageChannelsNum = appStatus->getVoltageChannelsNum();
//    we only have one widget with multiple tabs
//    todo maybe we could create a widget for each tab
    bpw = new BigPlotWidget(currentChannelsNum, mainWindow);
    mainWindow->setBigPlotWidget(bpw);
    connect(bpw, &BigPlotWidget::tabBarClicked, this, &BigPlotController::manageStatus);
    
    int translatorInitializer = 0;
    bps = BigPlot::GapFree;
    gapFreeIndex = translatorInitializer;
    translator[translatorInitializer++] = BigPlot::GapFree;
    bpw->addGapFreeTab();

    controllers[BigPlot::GapFree] = new GapFreeController(appStatus, producer, defaultPlotDuration, bpw, mainWindow, dc);
    if (appStatus->isEpisodic()) {
        episodicIndex = translatorInitializer;
        translator[translatorInitializer++] = BigPlot::Episodic;
        bpw->addEpisodicTab();
        controllers[BigPlot::Episodic] = new EpisodicController(appStatus, producer, defaultPlotDuration.getMax(), bpw, mainWindow, dc);
    }
    translator[translatorInitializer++] = BigPlot::Iv;
    bpw->addIvTab();
    controllers[BigPlot::Iv] = new IvGraphController(appStatus, producer, bpw, mainWindow);

    translator[translatorInitializer++] = BigPlot::Spectrum;
    bpw->addSpectrumTab();
    controllers[BigPlot::Spectrum] = new SpectrumController(appStatus, producer, {100.0, UnitPfxKilo, "Hz"}, bpw, mainWindow);
    if (currentChannelsNum == 1) {
        translator[translatorInitializer++] = BigPlot::Event;
        bpw->addEventDetectionTab();
        controllers[BigPlot::Event] = new EventDetectionController(appStatus, producer, bpw);
    }
    controllers[bps]->start();
}

void BigPlotController::manageStatus(int idx) {
    if (!(bpw->isTabEnabled(idx))) {
        return;
    }
    controllers[bps]->stop();
    bps = translator[idx];
    controllers[bps]->start();
}

BigPlotController::~BigPlotController() {
    for (const auto& [k, v] : controllers) {
        delete v;
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
    for (const auto& [k, v] : controllers) {
        v->onRangeUpdated(newRange);
    }
}

void BigPlotController::onProtocolStarted(unsigned int protId, ProtocolWidget * protocol) {
    if (bps != BigPlot::GapFree && bps != BigPlot::Episodic) {
        return;
    }
    if (bps == BigPlot::GapFree && protocol->getType() == ProtocolTypeEpisodic) {
        bpw->setCurrentIndex(episodicIndex);
        this->manageStatus(episodicIndex);
    }
    if (bps == BigPlot::Episodic && protocol->getType() == ProtocolTypeGapfree) {
        bpw->setCurrentIndex(gapFreeIndex);
        this->manageStatus(gapFreeIndex);
    }
}

void BigPlotController::onCurrentColorsChanged(QVector <QColor> colors) {
    for (const auto& [k, v] : controllers) {
        v->onCurrentColorsChanged(colors);
    }
}

void BigPlotController::onCurrentColorChanged(int channelIdx, QColor color) {
    for (const auto& [k, v] : controllers) {
        v->onCurrentColorChanged(channelIdx, color);
    }
}

void BigPlotController::onBackgroundColorChanged(QColor color) {
    for (const auto& [k, v] : controllers) {
        v->onBackgroundColorChanged(color);
    }
}

void BigPlotController::onExpandTrace(bool flag) {
    for (const auto& [k, v] : controllers) {
        v->onExpandTrace(flag);
    }
    if (flag) {
        controllers[bps]->start();
    }
}

std::vector <PlotConsumer*> BigPlotController::getConsumers() {
    std::vector <PlotConsumer*> consumers;
    for (const auto& [k, v] : controllers) {
        consumers.push_back(v->getConsumer());
    }
    return consumers;
}

std::vector <CentralWidgetController*> BigPlotController::getControllers() {
    std::vector <CentralWidgetController*> toRet;
    for (const auto& [k, v] : controllers) {
        toRet.push_back(v);
    }
    return toRet;
}
