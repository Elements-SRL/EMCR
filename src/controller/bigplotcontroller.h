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

class BigPlotController : public QObject {
    Q_OBJECT

public:
    BigPlotController(ApplicationStatus * appStatus, PlotConsumer * plotConsumer, Measurement_t defaultDuration, MainWindow * mainWindow);
    ~BigPlotController();

    BigPlot * getPlot();
    void clearCurves();
    PlotConsumer * getPlotConsumer();

public slots:
    void onRangeUpdated(commlib::RangedMeasurement_t newRange, QwtPlot::Axis axisIdx);
    void onCurrentColorsChanged(QVector <QColor> colors);
    void onCurrentColorChanged(int channelIdx, QColor color);
    void onBackgroundColorChanged(QColor color);

    void onSetPlotData(PlotMessage plotMessage);
    void onReplot();
    void onExpandTrace(bool);

private:
    ApplicationStatus * appStatus = nullptr;
    MainWindow * mainWindow = nullptr;
    BigPlotWidget * bpw = nullptr;
    BigPlotModel * bpm = nullptr;
    BigPlot * plot;
    PlotConsumer * plotConsumer = nullptr;
    QVector <Curve *> currentCurves;
    QVector <Curve *> voltageCurves;

    int voltageChannelsNum;
    int currentChannelsNum;
    bool isAtLeastOneChannelExpanded();
private slots:
    void handleZoomInRequest(Rect4 r);
    void handleZoomOutRequest();
    void handleZoomResetRequest();
    void handleSingleAxisZoomRequest(QwtPlot::Axis, int, QPointF);
    void handleSingleAxisShiftRequest(QwtPlot::Axis, int);

signals:
    void durationChanged(Measurement_t duration);
};

#endif // BIGPLOTCONTROLLER_H
