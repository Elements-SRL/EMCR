#include "gapfreecontroller.h"

GapFreeController::GapFreeController(ApplicationStatus* appStatus, DeviceDataProducer* producer, Measurement_t defaultPlotDuration, BigPlotWidget* bigPlotWidget):
    CentralWidgetController(appStatus, producer, bigPlotWidget) {

    gapFreeModel = new BigPlotModel();
    gapFreePlotConsumer = new GapFreePlotConsumer(appStatus, producer);
    gapFreePlot = new BigPlot("", "[s]", "", BigPlotStatus::GapFree, bigPlotWidget);
    gapFreePlot->enableAxis(QwtPlot::yRight);
    //    creating curves for gapfree
    for (int i = 0; i < currentChannelsNum; i++) {
        currentCurves.push_back(new Curve(CurveType_t::CurveTypePlotSolid));
    }
    for (int i = 0; i < voltageChannelsNum; i++) {
        voltageCurves.push_back(new Curve(CurveType_t::CurveTypePlotDashed));
        voltageCurves[i]->setYAxis(QwtPlot::yRight);
    }
    for (auto p : plots) {
        connect(p, &BigPlot::zoomInRequest, this, &BigPlotController::handleZoomInRequest);
        connect(p, &BigPlot::zoomOutRequest, this, &BigPlotController::handleZoomOutRequest);
        connect(p, &BigPlot::zoomResetRequest, this, &BigPlotController::handleZoomResetRequest);
        connect(p, &BigPlot::singleAxisZoomRequest, this, &BigPlotController::handleSingleAxisZoomRequest);
        connect(p, &BigPlot::singleAxisShiftRequest, this, &BigPlotController::handleSingleAxisShiftRequest);
    }

    for (auto c : consumers) {
        connect(this, &BigPlotController::durationChanged, c, &PlotConsumer::onDurationChanged);
        connect(c, &PlotConsumer::setPlotData, this, &BigPlotController::onSetPlotData);
        connect(c, &PlotConsumer::plotDataUpdated, this, &BigPlotController::onReplot);
        c->forceAxisUpdate();
        c->setMaxSamplesPerPlot(4096);
        c->onSelectChannels(false);
        c->onStopConsuming();
    }
}