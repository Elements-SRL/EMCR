#ifndef BIGPLOTMODEL_H
#define BIGPLOTMODEL_H

#include <QObject>
#include <QRectF>
#include "bigplot.h"
#include "messagedispatcher.h"
#include "rect4.h"

class BigPlotModel: public QObject {
    Q_OBJECT

private:
    BigPlot::BigPlotStatus bigPlotStatus;
    QVector <Rect4> zoomStack;
    bool rangeInitialized[QwtPlot::axisCnt];
    QString title;
    QString xUnit;
    QString yUnit;
    Rect4 currentZoom;
    bool isEmptyZoomStack();
    void clearZoomStack();
    double yScale = 1.0;
    RangedMeasurement_t currentRange[QwtPlot::axisCnt];
    UnitPfx_t xAxisPrefix = UnitPfxNone;
    void pushZoomStack(Rect4);
    void setCurrentZoom(QwtPlot::Axis, double, double);
    Rect4 popZoomStack();
    Rect4 resetZoomStack();

public:
    enum Zoom{
        Current,
        Previous,
        Default,
    };
    BigPlotModel(BigPlot::BigPlotStatus bigPlotStatus);
//    RangedMeasurement getCurrentRange(QwtPlot::Axis);
    void setCurrentRange(QwtPlot::Axis axis, RangedMeasurement currentRange);
    void setCurrentRangeLog(QwtPlot::Axis axis, RangedMeasurement currentRange);
    bool isRangeInitialized(QwtPlot::Axis);
    RangedMeasurement_t getCurrentRange(QwtPlot::Axis);
//    getCurrentZoom will take an enum
    Rect4 getZoom(Zoom);
    QwtInterval getCurrentZoomInterval(QwtPlot::Axis);
    void updateCurrentZoom(Rect4);
    Rect4 zoomOnSingleAxis(QwtPlot::Axis, int, QPointF);
    Rect4 shiftOnSingleAxis(QwtPlot::Axis, int);
    Rect4 initRect(double minX, double maxX, double miny, double maxY);
};
#endif // BIGPLOTMODEL_H
