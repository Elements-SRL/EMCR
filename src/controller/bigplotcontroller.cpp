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
    controllers.push_back(new EpisodicController(appStatus, producer, defaultPlotDuration, bpw, this, mainWindow, dc));
    controllers.push_back(new IvGraphController(appStatus, producer, bpw, mainWindow));
    controllers.push_back(new SpectrumController(appStatus, producer, {100.0, UnitPfxKilo, "Hz"}, bpw, this, mainWindow));
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

void BigPlotController::handleZoomInRequest(BigPlotModel * model, BigPlot* plot, Rect4 r) {
//    non idale, rischio di incoerenza con le altre chiamate nel model
    model->updateCurrentZoom(r);
    auto zoom = model->getZoom(BigPlotModel::Zoom::Current);
    plot->setRect(zoom);
    if (bps == BigPlot::GapFree || bps == BigPlot::Episodic) {
        emit durationChanged({ zoom[QwtPlot::xBottom].width(), model->getCurrentRange(QwtPlot::xBottom).prefix, "s" });
    }
}

void BigPlotController::handleSingleAxisZoomRequest(BigPlotModel * model, BigPlot * plot, QwtPlot::Axis axis, int zoomIn, QPointF mousePosition){
//    non idale, rischio di incoerenza con le altre chiamate nel model
    model->updateCurrentZoom(model->zoomOnSingleAxis(axis, zoomIn, mousePosition));
    auto zoom = model->getZoom(BigPlotModel::Zoom::Current);
    plot->setRect(zoom);
    // if (axis == QwtPlot::Axis::xBottom && (bps == BigPlot::GapFree || bps == BigPlot::Episodic)){
    //     emit durationChanged({zoom[QwtPlot::xBottom].width(), model->getCurrentRange(QwtPlot::xBottom).prefix, "s"});
    // }
}

void BigPlotController::handleSingleAxisZoomRequest(BigPlotModel * model, BigPlot * plot, QwtPlot::Axis axis, QwtInterval i) {
    //    non idale, rischio di incoerenza con le altre chiamate nel model
    model->updateCurrentZoom(i, axis);
    auto zoom = model->getZoom(BigPlotModel::Zoom::Current);
    plot->setRect(zoom);
    // if (axis == QwtPlot::Axis::xBottom && (bps == BigPlot::GapFree || bps == BigPlot::Episodic)){
    //     emit durationChanged({zoom[QwtPlot::xBottom].width(), model->getCurrentRange(QwtPlot::xBottom).prefix, "s"});
    // }
}

void BigPlotController::handleSingleAxisShiftRequest(BigPlotModel * model, BigPlot * plot, QwtPlot::Axis axis, int shift){
//    non idale, rischio di incoerenza con le altre chiamate nel model
    model->updateCurrentZoom(model->shiftOnSingleAxis(axis, shift));
    auto zoom = model->getZoom(BigPlotModel::Zoom::Current);
    plot->setRect(zoom);
}

void BigPlotController::handleZoomOutRequest(BigPlotModel * model, BigPlot * plot){
    auto zoom = model->getZoom(BigPlotModel::Zoom::Previous);
    plot->setRect(zoom);
    if (bps == BigPlot::GapFree || bps == BigPlot::Episodic) {
        emit durationChanged({ zoom[QwtPlot::xBottom].width(), model->getCurrentRange(QwtPlot::xBottom).prefix, "s" });
    }
}

void BigPlotController::handleZoomResetRequest(BigPlotModel * model, BigPlot * plot){
    auto zoom = model->getZoom(BigPlotModel::Zoom::Default);
    plot->setRect(zoom);
    if (bps == BigPlot::GapFree || bps == BigPlot::Episodic) {
        emit durationChanged({ zoom[QwtPlot::xBottom].width(), model->getCurrentRange(QwtPlot::xBottom).prefix, "s" });
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
