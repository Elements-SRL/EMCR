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

class BigPlotController : public QObject {
    Q_OBJECT

public:
    BigPlotController(ApplicationStatus * appStatus, DeviceDataProducer * producer, Measurement_t defaultDuration, MainWindow * mainWindow);
    ~BigPlotController();

    BigPlot * getPlot();
    std::vector<PlotConsumer*> getConsumers();

public slots:
    void onCurrentColorsChanged(QVector <QColor> colors);
    void onCurrentColorChanged(int channelIdx, QColor color);
    void onBackgroundColorChanged(QColor color);
    void onExpandTrace(bool);
    void onRangeUpdated(RangedMeasurement_t newRange);

    void handleZoomInRequest(Rect4 r);
    void handleZoomOutRequest();
    void handleZoomResetRequest();
    void handleSingleAxisZoomRequest(QwtPlot::Axis, int, QPointF);
    void handleSingleAxisShiftRequest(QwtPlot::Axis, int);

private:
    BigPlotStatus bps;
    ApplicationStatus * appStatus = nullptr;
    MainWindow * mainWindow = nullptr;
    BigPlotWidget * bpw = nullptr;

    std::vector<PlotMessage> messages;

    BigPlot * currentPlot = nullptr;
    PlotConsumer * currentConsumer = nullptr;
    BigPlotModel * currentModel = nullptr;

    std::vector<CentralWidgetController* > controllers;

    int voltageChannelsNum;
    int currentChannelsNum;
    void manageStatus(int);

signals:
    void durationChanged(Measurement_t duration);
};

#endif // BIGPLOTCONTROLLER_H
