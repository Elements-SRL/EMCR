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
    om = OperationMode_t::GapFree;
    gapFreeIndex = translatorInitializer;
    translator[translatorInitializer++] = OperationMode_t::GapFree;
    bpw->addGapFreeTab();

    controllers[OperationMode_t::GapFree] = new GapFreeController(appStatus, producer, defaultPlotDuration, bpw, mainWindow, dc);
    if (appStatus->isEpisodic()) {
        episodicIndex = translatorInitializer;
        translator[translatorInitializer++] = OperationMode_t::Episodic;
        bpw->addEpisodicTab();
        controllers[OperationMode_t::Episodic] = new EpisodicController(appStatus, producer, defaultPlotDuration, bpw, mainWindow, dc);
    }
    translator[translatorInitializer++] = OperationMode_t::Iv;
    bpw->addIvTab();
    controllers[OperationMode_t::Iv] = new IvGraphController(appStatus, producer, bpw, mainWindow);

    translator[translatorInitializer++] = OperationMode_t::Spectrum;
    bpw->addSpectrumTab();
    controllers[OperationMode_t::Spectrum] = new SpectrumController(appStatus, producer, {0.0, 100.0, 10.0, UnitPfxKilo, "Hz"}, bpw, mainWindow);
    if (currentChannelsNum == 1) {
        translator[translatorInitializer++] = OperationMode_t::Event;
        bpw->addEventDetectionTab();
        controllers[OperationMode_t::Event] = new EventDetectionController(appStatus, producer, bpw);
    }
    controllers[om]->start();
}

void BigPlotController::manageStatus(int idx) {
    if (!(bpw->isTabEnabled(idx))) {
        return;
    }
    controllers[om]->stop();
    om = translator[idx];
    controllers[om]->start();
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
    if (om != OperationMode_t::GapFree && om != OperationMode_t::Episodic) {
        return;
    }
    if (om == OperationMode_t::GapFree && protocol->getType() == ProtocolTypeEpisodic) {
        bpw->setCurrentIndex(episodicIndex);
        this->manageStatus(episodicIndex);
    }
    if (om == OperationMode_t::Episodic && protocol->getType() == ProtocolTypeGapfree) {
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
        controllers[om]->start();
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
