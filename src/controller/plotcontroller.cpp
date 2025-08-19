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
    connect(pm.get(), &PlotModel::sigReplot, this, &PlotController::sigPlotUpdated);

    connect(this, &PlotController::sigAutoZoom, bp, &BasePlot2::onAutoZoom, Qt::QueuedConnection);
    connect(this, &PlotController::sigAutoZoom, bp, &BasePlot2::onAutoZoom, Qt::QueuedConnection);
    connect(this, &PlotController::sigLabelsOverride, bp, &BasePlot2::onLabelsOverride, Qt::QueuedConnection);
}

QwtPlot * PlotController::getPlot() {
    return bp;
}

void PlotController::setRangedMeasurement(QwtPlot::Axis axis, e384CommLib::RangedMeasurement_t rm) {
    pm->setRangedMeasurement(axis, rm);
}

void PlotController::onAutoZoom() {
    emit sigAutoZoom(pm->getActiveAxes());
}

std::shared_ptr<PlotModel> PlotController::getModel() {
    return pm;
}

void PlotController::onLabelsOverride(std::map<QwtPlot::Axis, std::string> newLabels) {
    emit sigLabelsOverride(newLabels);
}
