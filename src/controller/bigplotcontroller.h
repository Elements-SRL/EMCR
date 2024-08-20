#ifndef BIGPLOTCONTROLLER_H
#define BIGPLOTCONTROLLER_H

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
#include "centralwidgetcontroller.h"
#include "devicecontroller.h"
#include "abfdatawriterconsumer.h"

class BigPlotController : public QObject {
    Q_OBJECT

public:
    BigPlotController(ApplicationStatus * appStatus, DeviceDataProducer * producer, Measurement_t defaultDuration, MainWindow * mainWindow, AbfDataWriterConsumer* abfDataWriterConsumer, DeviceController* dc);
    ~BigPlotController();

    std::vector<PlotConsumer*> getConsumers();
    std::vector<CentralWidgetController*> getControllers();

public slots:
    void onCurrentColorsChanged(QVector <QColor> colors);
    void onCurrentColorChanged(int channelIdx, QColor color);
    void onBackgroundColorChanged(QColor color);
    void onExpandTrace(bool);
    void onRangeUpdated(RangedMeasurement_t newRange);

    void handleZoomInRequest(BigPlotModel* model, BigPlot* plot, Rect4 r);
    void handleZoomOutRequest(BigPlotModel* model, BigPlot* plot);
    void handleZoomResetRequest(BigPlotModel* model, BigPlot* plot);
    void handleSingleAxisZoomRequest(BigPlotModel* model, BigPlot* plot, QwtPlot::Axis, int, QPointF);
    void handleSingleAxisShiftRequest(BigPlotModel* model, BigPlot* plot, QwtPlot::Axis, int);

private:
    BigPlot::BigPlotStatus bps;
    ApplicationStatus * appStatus = nullptr;
    MainWindow * mainWindow = nullptr;
    BigPlotWidget * bpw = nullptr;

    std::vector<CentralWidgetController* > controllers;

    int voltageChannelsNum;
    int currentChannelsNum;
    void manageStatus(int);

signals:
    void durationChanged(Measurement_t duration);
};

#endif // BIGPLOTCONTROLLER_H
