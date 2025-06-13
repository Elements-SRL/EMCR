#include "plotcontroller.h"

PlotController::PlotController(std::map<QwtPlot::Axis, AxisInfo> ai, QWidget *parent) {
    pm = std::make_shared<PlotModel>(ai);
    bp = new BasePlot2(pm, parent);

    connect(bp, &BasePlot2::sigZoomOut, pm.get(), &PlotModel::onZoomOut);
    connect(bp, &BasePlot2::sigZoomReset, pm.get(), &PlotModel::onZoomReset);
    connect(bp, &BasePlot2::singleAxisZoomRequest, pm.get(), &PlotModel::onSingleAxisZoom);
    connect(bp, &BasePlot2::singleAxisShiftRequest, pm.get(), &PlotModel::onSingleAxisShift);
    connect(bp, &BasePlot2::sigZoomInPickerAppended, pm.get(), &PlotModel::onZoomInPickerAppended);
    connect(bp, &BasePlot2::sigZoomInPickerMoved, pm.get(), &PlotModel::onZoomInPickerMoved);
    connect(bp, &BasePlot2::sigZoomInPickerSelected, pm.get(), &PlotModel::onZoomInPickerSelected);

    connect(pm.get(), &PlotModel::sigReplot, bp, &BasePlot2::onReplot);
    connect(pm.get(), &PlotModel::sigRubberBandUpdated, bp, &BasePlot2::onRubberBandUpdated);
}
