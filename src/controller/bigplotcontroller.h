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
#include "plotconsumer.h"
#include "ivgraphwidget.h"

class BigPlotController : public QObject {
    Q_OBJECT

public:
    BigPlotController(ApplicationStatus * appStatus, DeviceDataProducer * producer, Measurement_t defaultDuration, MainWindow * mainWindow);
    ~BigPlotController();

    BigPlot * getPlot();
    void clearCurves();
    std::vector<PlotConsumer *> getConsumers();

public slots:
    void onRangeUpdated(commlib::RangedMeasurement_t newRange, QwtPlot::Axis axisIdx);
    void onCurrentColorsChanged(QVector <QColor> colors);
    void onCurrentColorChanged(int channelIdx, QColor color);
    void onBackgroundColorChanged(QColor color);

    void onSetPlotData(PlotMessage plotMessage);
    void onReplot();
    void onExpandTrace(bool);

private:
    BigPlotStatus bps;
    ApplicationStatus * appStatus = nullptr;
    MainWindow * mainWindow = nullptr;
    BigPlotWidget * bpw = nullptr;
    std::vector<BigPlot *> plots;
    std::vector<PlotConsumer *> consumers;
    std::vector<BigPlotModel *> models;

    std::vector<std::vector <Curve *>> currentCurves;
    std::vector<std::vector <Curve *>> voltageCurves;
    std::vector<PlotMessage> messages;

    BigPlot * currentPlot = nullptr;
    PlotConsumer * currentConsumer = nullptr;
    BigPlotModel * currentModel = nullptr;
    IvGraphWidget * ivGraphWidget = nullptr;

    int voltageChannelsNum;
    int currentChannelsNum;
    bool isAtLeastOneChannelExpanded();
    void manageStatus(int);
    void detachCurves();
    void attachCurves();

private slots:
    void handleZoomInRequest(Rect4 r);
    void handleZoomOutRequest();
    void handleZoomResetRequest();
    void handleSingleAxisZoomRequest(QwtPlot::Axis, int, QPointF);
    void handleSingleAxisShiftRequest(QwtPlot::Axis, int);
    void onExportIvGraph();

signals:
    void durationChanged(Measurement_t duration);
};

#endif // BIGPLOTCONTROLLER_H
