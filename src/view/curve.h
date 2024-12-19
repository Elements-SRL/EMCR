#ifndef CURVE_H
#define CURVE_H

#include <QColor>

#include "qwt_plot_curve.h"

typedef enum {
    CurveTypePlotSolid,
    CurveTypePlotDashed,
    CurveTypeStampPlotSolid,
    CurveTypeAnalysisDashed,
    CurveTypeProtocolPreviewSolid,
    CurveTypeProtocolPreviewDotted,
    CurveTypeProtocolPreviewDashed,
    CurveTypeScatterPlot,
    CurveTypeNone
} CurveType_t;

class Curve : public QwtPlotCurve {
public:
    Curve(CurveType_t curveType = CurveTypeProtocolPreviewSolid, double size = 1.0);
    Curve(const Curve &curve);

    CurveType_t getCurveType() const;
    double getSize() const;
    void setColor(QColor color);
    QColor getColor();

private:
    CurveType_t curveType;
    QColor color;
    double size;
};

#endif // CURVE_H
