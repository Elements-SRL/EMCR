#ifndef IVGRAPHCONTROLLER_H
#define IVGRAPHCONTROLLER_H

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
#include "ivgraphwidget.h"
#include "centralwidgetcontroller.h"
#include "bigplotcontroller.h"

class IvGraphController : public CentralWidgetController {
    Q_OBJECT

public:
    IvGraphController(ApplicationStatus* appStatus, DeviceDataProducer* producer, Measurement_t defaultPlotDuration, BigPlotWidget* bigPlotWidget, BigPlotController* bigPlotController);
    ~IvGraphController();

    //public slots:
    //    void onRangeUpdated(commlib::RangedMeasurement_t newRange);
    //    void onCurrentColorsChanged(QVector <QColor> colors);
    //    void onCurrentColorChanged(int channelIdx, QColor color);
    //    void onBackgroundColorChanged(QColor color);

private:
    BigPlotModel* model = nullptr;
    IvGraphConsumer* consumer = nullptr;
    BigPlot* plot = nullptr;
    std::vector <Curve*> currentCurves;
    IvGraphWidget* ivGraphWidget = nullptr;
    PlotMessage message;

    void saveToCSV(const QString& filePath, const IvMessage& data);

signals:
    void durationChanged(Measurement_t duration);

private slots:
    void onExportIvGraph();
    void onCalcMeanSquared();
    void onStartIvGraph();
    void onStopIvGraph();

};

#endif // IVGRAPHCONTROLLER_H
#pragma once
