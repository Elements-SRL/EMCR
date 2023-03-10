#ifndef BIGPLOTDOCKWIDGET_H
#define BIGPLOTDOCKWIDGET_H

#include <QDockWidget>

#include "bigplot.h"

class BigPlotDockWidget : public QDockWidget {
    Q_OBJECT

public:
    BigPlotDockWidget(QWidget * parent = nullptr);

public slots:
    void onRangeUpdated(RangedMeasurement_t newRange);
    void onDurationUpdated(Measurement_t duration);
    void onSetGapFreePlotData(double * timeValues, QVector <double *> * voltageValues, QVector <double *> * currentValues, int dataSize);
    void onReplot();

private:
    BigPlot * plot = nullptr;
};

#endif // BIGPLOTDOCKWIDGET_H
