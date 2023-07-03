#ifndef BIGPLOTDOCKWIDGET_H
#define BIGPLOTDOCKWIDGET_H

#include <QDockWidget>

#include "messagedispatcher.h"
#include "bigplot.h"
#include "curve.h"

class BigPlotDockWidget : public QDockWidget {
    Q_OBJECT

public:
    BigPlotDockWidget(MessageDispatcher * msgDisp, QWidget * parent = nullptr);

    void clearCurves();

public slots:
    void onRangeUpdated(RangedMeasurement_t newRange, QwtPlot::Axis axisIdx);
    void onDurationUpdated(Measurement_t duration);
    void onSetGapFreePlotData(double * timeValues, QVector <double *> * voltageValues, QVector <double *> * currentValues, int dataSize, int channelsToPlotNumber);
    void onReplot();

private:
    BigPlot * plot = nullptr;
    QVector <Curve *> currentCurves;
    QVector <Curve *> voltageCurves;

    int voltageChannelsNum;
    int currentChannelsNum;
};

#endif // BIGPLOTDOCKWIDGET_H
