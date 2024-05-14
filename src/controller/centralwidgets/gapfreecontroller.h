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
#include "bigplotcontroller.h"

class GapFreeController : public CentralWidgetController {
    Q_OBJECT

public:
    GapFreeController(ApplicationStatus* appStatus, DeviceDataProducer* producer, Measurement_t defaultPlotDuration, BigPlotWidget* bigPlotWidget, BigPlotController* bigPlotController);
    ~GapFreeController();
    void stop() override;
    void start() override;
    PlotConsumer* getConsumer() override;
private:
    BigPlotModel* model = nullptr;
    GapFreePlotConsumer * consumer = nullptr;
    BigPlot * plot = nullptr;
    std::vector <Curve*> currentCurves;
    std::vector <Curve*> voltageCurves;
    void detachCurves() override;
    void attachCurves() override;

public slots:
    void onRangeUpdated(commlib::RangedMeasurement_t newRange) override;
    void onCurrentColorsChanged(QVector <QColor> colors) override;
    void onCurrentColorChanged(int channelIdx, QColor color) override;
    void onBackgroundColorChanged(QColor color) override;
    void onReplot() override;
    void onExpandTrace(bool flag) override;
    void onSetPlotData(PlotMessage plotMessage) override;

signals:
    void durationChanged(Measurement_t duration);
};

#endif // GAPFREECONTROLLER_H
#pragma once
