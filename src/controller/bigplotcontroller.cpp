#include "bigplotcontroller.h"
#include <iostream>
#include <cmath>
#include "gapfreecontroller.h"
#include "ivgraphcontroller.h"
#include "eventdetectioncontroller.h"

BigPlotController::BigPlotController(ApplicationStatus * appStatus, DeviceDataProducer * producer, Measurement_t defaultPlotDuration, MainWindow * mainWindow) :
    appStatus(appStatus),
    mainWindow(mainWindow) {

    currentChannelsNum = appStatus->getCurrentChannelsNum();
    voltageChannelsNum = appStatus->getVoltageChannelsNum();

//    we only have one widget with multiple tabs
//    todo maybe we could create a widget for each tab
    bpw = new BigPlotWidget(mainWindow);
    mainWindow->setBigPlotWidget(bpw);
    connect(bpw, &BigPlotWidget::tabBarClicked, this, &BigPlotController::manageStatus);
    
    bps = BigPlotStatus::GapFree;
    controllers.push_back(new GapFreeController(appStatus, producer, defaultPlotDuration, bpw, this));
    controllers.push_back(new IvGraphController(appStatus, producer, bpw, this, mainWindow));
#ifdef DEBUG
    controllers.push_back(new EventDetectionController(appStatus, producer, bpw));
#endif
    controllers[bps]->start();
}

void BigPlotController::manageStatus(int idx) {
    if (idx == bps) {
        return;
    }
    controllers[bps]->stop();
    bps = static_cast<BigPlotStatus>(idx < 0 ? static_cast<int>(BigPlotStatus::NumberOfStatuses) : idx);
    controllers[bps]->start();
}

BigPlotController::~BigPlotController() {
    controllers.clear();
    if (bpw != nullptr) {
        delete bpw;
        bpw = nullptr;
        mainWindow->setBigPlotWidget(bpw);
    }
}

void BigPlotController::handleZoomInRequest(BigPlotModel* model, BigPlot* plot, Rect4 r) {
//    non idale, rischio di incoerenza con le altre chiamate nel model
    model->updateCurrentZoom(r);
    auto zoom = model->getZoom(BigPlotModel::Zoom::Current);
    plot->setRect(zoom);
    if (bps == BigPlotStatus::GapFree) {
        emit durationChanged({ zoom[QwtPlot::xBottom].width(), model->getCurrentRange(QwtPlot::xBottom).prefix, "s" });
    }
}

void BigPlotController::handleSingleAxisZoomRequest(BigPlotModel* model, BigPlot* plot, QwtPlot::Axis axis, int zoomIn, QPointF mousePosition){
//    non idale, rischio di incoerenza con le altre chiamate nel model
    model->updateCurrentZoom(model->zoomOnSingleAxis(axis, zoomIn, mousePosition));
    auto zoom = model->getZoom(BigPlotModel::Zoom::Current);
    plot->setRect(zoom);
    if (axis == QwtPlot::Axis::xBottom && bps == BigPlotStatus::GapFree){
        emit durationChanged({zoom[QwtPlot::xBottom].width(), model->getCurrentRange(QwtPlot::xBottom).prefix, "s"});
    }
}

void BigPlotController::handleSingleAxisShiftRequest(BigPlotModel* model, BigPlot* plot, QwtPlot::Axis axis, int shift){
//    non idale, rischio di incoerenza con le altre chiamate nel model
    model->updateCurrentZoom(model->shiftOnSingleAxis(axis, shift));
    auto zoom = model->getZoom(BigPlotModel::Zoom::Current);
    plot->setRect(zoom);
}

void BigPlotController::handleZoomOutRequest(BigPlotModel* model, BigPlot* plot){
    auto zoom = model->getZoom(BigPlotModel::Zoom::Previous);
    plot->setRect(zoom);
    if (bps == BigPlotStatus::GapFree) {
        emit durationChanged({ zoom[QwtPlot::xBottom].width(), model->getCurrentRange(QwtPlot::xBottom).prefix, "s" });
    }
}

void BigPlotController::handleZoomResetRequest(BigPlotModel* model, BigPlot* plot){
    auto zoom = model->getZoom(BigPlotModel::Zoom::Default);
    plot->setRect(zoom);
    if (bps == BigPlotStatus::GapFree) {
        emit durationChanged({ zoom[QwtPlot::xBottom].width(), model->getCurrentRange(QwtPlot::xBottom).prefix, "s" });
    }
}

//todo Bisogner controllare anche la clampingmodality
void BigPlotController::onRangeUpdated(commlib::RangedMeasurement_t newRange) {
    for (auto c : controllers) {
        c->onRangeUpdated(newRange);
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

std::vector<PlotConsumer*> BigPlotController::getConsumers() {
    std::vector<PlotConsumer*> consumers;
    for (auto c : controllers) {
        consumers.push_back(c->getConsumer());
    }
    return consumers;
}
