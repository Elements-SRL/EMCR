#ifndef PLOTMODEL_H
#define PLOTMODEL_H

#include <QObject>
// #include "curve.h"
// #include <memory>
#include <QwtPlot>
#include "e384commlib_global_addendum.h"
#include <map>
#include "zoom.h"

struct AxisInfo {
    // if log is false then scale is linear
    e384CommLib::RangedMeasurement_t range;
    std::optional<double> fixedMinimum = std::nullopt;
    bool log = false;
    bool inverted = false;
};

class PlotModel: public QObject {
    Q_OBJECT

private:
    std::map<QwtPlot::Axis, AxisInfo> axisInfo;
    Zoom zoom;

public:
    PlotModel(std::map<QwtPlot::Axis, AxisInfo>);
    // used for gapfree plot which has fixed minimum at 0;
    // std::optional<bool> getMinimumFixed();
    virtual void setRangedMeasurement(QwtPlot::Axis, e384CommLib::RangedMeasurement_t) = 0;
    virtual std::optional<std::string> getUnitLabel(QwtPlot::Axis) = 0;
    virtual std::map<QwtPlot::Axis, std::string> getUnitLabes() = 0;
    Rect4 getZoom();

public slots:
    virtual void onAxisChanged(QwtPlot::Axis, e384CommLib::RangedMeasurement_t) = 0;
    virtual void onCurveColorChanged(std::map<uint16_t, QColor>) = 0;
    virtual void onBackgroundColorChanged(QColor) = 0;
};

#endif // PLOTMODEL_H
