#ifndef GAPFREECONTROLLER_H
#define GAPFREECONTROLLER_H

#include "bigplotwidget.h"
#include "mainwindow.h"
#include "messagedispatcher.h"
#include "bigplotmodel.h"
#include "plotconsumer.h"
#include <QPointF>
#include "application_status.h"
#include "plotmessage.h"
#include "ivgraphconsumer.h"
#include "eventdetectionconsumer.h"
#include "plotconsumer.h"
#include "ivgraphwidget.h"

#include "centralwidgetcontroller.h"

class GapFreeController : public CentralWidgetController {
    Q_OBJECT

public:
    GapFreeController(ApplicationStatus* appStatus, DeviceDataProducer* producer, Measurement_t defaultPlotDuration, BigPlotWidget* bigPlotWidget);
    ~GapFreeController();

public slots:
    void onRangeUpdated(commlib::RangedMeasurement_t newRange);
    void onCurrentColorsChanged(QVector <QColor> colors);
    void onCurrentColorChanged(int channelIdx, QColor color);
    void onBackgroundColorChanged(QColor color);

    void onSetPlotData(PlotMessage plotMessage);
    void onReplot();
    void onExpandTrace(bool);

private:
    BigPlotModel* gapFreeModel = nullptr;
    GapFreePlotConsumer * gapFreePlotConsumer = nullptr;
    BigPlot * gapFreePlot = nullptr;
    std::vector <Curve*> currentCurves;
    std::vector <Curve*> voltageCurves;

private slots:
    void handleZoomInRequest(Rect4 r);
    void handleZoomOutRequest();
    void handleZoomResetRequest();
    void handleSingleAxisZoomRequest(QwtPlot::Axis, int, QPointF);
    void handleSingleAxisShiftRequest(QwtPlot::Axis, int);
    void onExportIvGraph();
    void onCalcMeanSquared();
    void onStartIvGraph();
    void onStopIvGraph();

signals:
    void durationChanged(Measurement_t duration);
};

#endif // GAPFREECONTROLLER_H
#pragma once
