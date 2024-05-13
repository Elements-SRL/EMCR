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
    IvGraphController(ApplicationStatus* appStatus, DeviceDataProducer* producer, BigPlotWidget* bigPlotWidget, BigPlotController* bigPlotController, MainWindow* mainWindow);
    ~IvGraphController();

    void stop() override;
    void start() override;
    PlotConsumer* getConsumer() override;
    IvGraphWidget* getIvGraphWidget();

private:
    BigPlotModel* model = nullptr;
    IvGraphConsumer* consumer = nullptr;
    BigPlot* plot = nullptr;
    std::vector <Curve*> currentCurves;
    IvGraphWidget* ivGraphWidget = nullptr;
    MainWindow* mainWindow = nullptr;
    IvMessage message;
    void saveToCSV(const QString& filePath, const IvMessage& data);

    void detachCurves() override;
    void attachCurves() override;

signals:
    void durationChanged(Measurement_t duration);

public slots:
    void onRangeUpdated(commlib::RangedMeasurement_t newRange) override;
    void onCurrentColorsChanged(QVector <QColor> colors) override;
    void onCurrentColorChanged(int channelIdx, QColor color) override;
    void onBackgroundColorChanged(QColor color) override;
    void onReplot() override;
    void onExpandTrace(bool flag);
    void onSetPlotData(PlotMessage plotMessage) override;

private slots:
    void onExportIvGraph();
    void onCalcMeanSquared();
    void onStartIvGraph();
    void onStopIvGraph();

};

#endif // IVGRAPHCONTROLLER_H
#pragma once
