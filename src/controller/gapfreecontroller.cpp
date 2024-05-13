#include "gapfreecontroller.h"

GapFreeController::GapFreeController(ApplicationStatus* appStatus, DeviceDataProducer* producer, Measurement_t defaultPlotDuration, BigPlotWidget* bigPlotWidget, BigPlotController* bigPlotController):
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

    connect(gapFreePlot, &BigPlot::zoomInRequest, bigPlotController, &BigPlotController::handleZoomInRequest);
    connect(gapFreePlot, &BigPlot::zoomOutRequest, bigPlotController, &BigPlotController::handleZoomOutRequest);
    connect(gapFreePlot, &BigPlot::zoomResetRequest, bigPlotController, &BigPlotController::handleZoomResetRequest);
    connect(gapFreePlot, &BigPlot::singleAxisZoomRequest, bigPlotController, &BigPlotController::handleSingleAxisZoomRequest);
    connect(gapFreePlot, &BigPlot::singleAxisShiftRequest, bigPlotController, &BigPlotController::handleSingleAxisShiftRequest);

    connect(bigPlotController, &BigPlotController::durationChanged, gapFreePlotConsumer, &PlotConsumer::onDurationChanged);
    connect(gapFreePlotConsumer, &PlotConsumer::setPlotData, bigPlotController, &BigPlotController::onSetPlotData);
    connect(gapFreePlotConsumer, &PlotConsumer::plotDataUpdated, bigPlotController, &BigPlotController::onReplot);
    gapFreePlotConsumer->forceAxisUpdate();
    gapFreePlotConsumer->setMaxSamplesPerPlot(4096);
    gapFreePlotConsumer->onSelectChannels(false);
    gapFreePlotConsumer->onStopConsuming();
}

GapFreeController::~GapFreeController() {
    if (gapFreeModel != nullptr) {
        delete gapFreeModel;
        gapFreeModel = nullptr;
    }
    if (gapFreePlotConsumer != nullptr) {
        delete gapFreePlotConsumer;
        gapFreePlotConsumer = nullptr;
    }
    if (gapFreePlot != nullptr) {
        delete gapFreePlot;
        gapFreePlot = nullptr;
    }
    currentCurves.clear();
    voltageCurves.clear();
}