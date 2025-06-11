#ifndef BIGPLOTMODEL_H
#define BIGPLOTMODEL_H

#include <QObject>
#include <QRectF>
#include "e384commlib_global.h"
#include "rect4.h"

class BigPlotModel: public QObject {
    Q_OBJECT

private:
    QVector <Rect4> zoomStack;
    bool rangeInitialized[QwtPlot::axisCnt];
    QString title;
    QString xUnit;
    QString yUnit;
    Rect4 currentZoom;
    bool isEmptyZoomStack();
    void clearZoomStack();
    double yScale = 1.0;
    e384CommLib::RangedMeasurement_t currentRange[QwtPlot::axisCnt];
    e384CommLib::UnitPfx_t xAxisPrefix = e384CommLib::UnitPfxNone;
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
    BigPlotModel();
//    RangedMeasurement getCurrentRange(QwtPlot::Axis);
    void setCurrentRange(QwtPlot::Axis axis, e384CommLib::RangedMeasurement currentRange);
    void setCurrentRangeLog(QwtPlot::Axis axis, e384CommLib::RangedMeasurement currentRange);
    bool isRangeInitialized(QwtPlot::Axis);
    e384CommLib::RangedMeasurement_t getCurrentRange(QwtPlot::Axis);
//    getCurrentZoom will take an enum
    Rect4 getZoom(Zoom);
    QwtInterval getCurrentZoomInterval(QwtPlot::Axis);
    void updateCurrentZoom(Rect4);
    void updateCurrentZoom(QwtInterval i, QwtPlot::Axis ax);
    virtual Rect4 zoomOnSingleAxis(QwtPlot::Axis, int, QPointF);
    virtual Rect4 shiftOnSingleAxis(QwtPlot::Axis, int);
    Rect4 initRect(double minX, double maxX, double miny, double maxY);
};
#endif // BIGPLOTMODEL_H
