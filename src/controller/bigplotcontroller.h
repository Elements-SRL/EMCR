#ifndef BIGPLOTCONTROLLER_H
#define BIGPLOTCONTROLLER_H

#include "bigplotwidget.h"
#include "mainwindow.h"
#include "messagedispatcher.h"
#include "bigplotmodel.h"

class BigPlotController : public QObject {
    Q_OBJECT

public:
    BigPlotController(MessageDispatcher * msgDisp, MainWindow * mainWindow);
    ~BigPlotController();

    BigPlot * getPlot();
    void clearCurves();

public slots:
    void onRangeUpdated(commlib::RangedMeasurement_t newRange, QwtPlot::Axis axisIdx);
    void onCurrentColorChanged(int channelIdx, QColor color);

    void onSetGapFreePlotData(double * timeValues, QVector <double *> * voltageValues, QVector <double *> * currentValues, int dataSize, int channelsToPlotNumber);
    void onReplot();
    void onSelectedColors(QVector <QColor> colors);

private:
    MessageDispatcher * msgDisp = nullptr;
    MainWindow * mainWindow = nullptr;
    BigPlotWidget * bpw = nullptr;
    BigPlotModel * bpm = nullptr;
    BigPlot * plot;

    QVector <Curve *> currentCurves;
    QVector <Curve *> voltageCurves;

    int voltageChannelsNum;
    int currentChannelsNum;

private slots:
    void handleZoomInRequest(Rect4 r);
    void handleZoomOutRequest();
    void handleZoomResetRequest();
    void handleSingleAxisZoomRequest(QwtPlot::Axis, int);

signals:
    void durationChanged(Measurement_t duration);
};

#endif // BIGPLOTCONTROLLER_H
