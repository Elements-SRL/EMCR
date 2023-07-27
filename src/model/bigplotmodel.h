#ifndef BIGPLOTMODEL_H
#define BIGPLOTMODEL_H

#include <QObject>
#include <QRectF>
#include "messagedispatcher.h"
#include "rect4.h"

class BigPlotModel: public QObject
{
    Q_OBJECT

private:
    QVector <Rect4> zoomStack;
    bool rangeInitialized[QwtPlot::axisCnt];
    QString title;
    QString xUnit;
    QString yUnit;

    bool isEmptyZoomStack();
    void clearZoomStack();

    double yScale;
    RangedMeasurement_t currentRange[QwtPlot::axisCnt];
    Measurement_t sweepDuration = {1.0, UnitPfxNone, "s"};
    UnitPfx_t xAxisPrefix = UnitPfxNone;

public:
    BigPlotModel();
    void pushZoomStack(Rect4 r);
    Rect4 popZoomStack();
    Rect4 resetZoomStack();
//    void shiftVertAxis(QwtPlot::Axis axis, double shiftValue);
//    RangedMeasurement getCurrentRange(QwtPlot::Axis);
//    void setCurrentRange(QwtPlot::Axis axis, RangedMeasurement currentRange);
//    bool isRangeInitialized(QwtPlot::Axis);

};
#endif // BIGPLOTMODEL_H
