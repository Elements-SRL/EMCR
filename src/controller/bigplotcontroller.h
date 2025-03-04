#ifndef BIGPLOTCONTROLLER_H
#define BIGPLOTCONTROLLER_H

#include "bigplotwidget.h"
#include "mainwindow.h"
#include "messagedispatcher.h"
#include "bigplotmodel.h"
#include "plotconsumer.h"
#include <QPointF>
#include "application_status.h"
#include "plotconsumer.h"
#include "centralwidgetcontroller.h"
#include "devicecontroller.h"

class BigPlotController : public QObject {
    Q_OBJECT

public:
    BigPlotController(ApplicationStatus * appStatus, DeviceDataProducer * producer, Measurement_t defaultDuration, MainWindow * mainWindow, DeviceController* dc);
    ~BigPlotController();

    std::vector<PlotConsumer*> getConsumers();
    std::vector<CentralWidgetController*> getControllers();

public slots:
    void onCurrentColorsChanged(QVector <QColor> colors);
    void onCurrentColorChanged(int channelIdx, QColor color);
    void onBackgroundColorChanged(QColor color);
    void onExpandTrace(bool);
    void onRangeUpdated(RangedMeasurement_t newRange);
    void onProtocolStarted(unsigned int protId, ProtocolWidget * protocol);

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
