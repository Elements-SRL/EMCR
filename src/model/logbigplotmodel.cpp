#include "logbigplotmodel.h"
#include <qwt_interval.h>

// The LogBigPlotModel need to be used by logaritmic plots, so:
// Spectrum
LogBigPlotModel::LogBigPlotModel()
    : BigPlotModel()
{}


Rect4 LogBigPlotModel::zoomOnSingleAxis(QwtPlot::Axis ax, int zoomInFactor, QPointF mousePosition) {
    auto currentZoom = getZoom(Current);
    const auto interval = currentZoom[ax];
    //if we are using the spectrum we have a logaritmic scale
    bool logFlag = ax == QwtPlot::yLeft || ax == QwtPlot::xBottom;

    const auto min = logFlag ? log10(interval.minValue()) : interval.minValue();
    const auto max = logFlag ? log10(interval.maxValue()) : interval.maxValue();
    const auto zoom = (double) zoomInFactor / 100;
    const auto divisor = (zoom > 0 ? zoom: -1 / zoom);

    auto newMin = min;
    auto newMax = max;
    if (ax == QwtPlot::Axis::yLeft || ax == QwtPlot::Axis::yRight) {
        // zoom only around the cursor
        const auto y = logFlag ? log10(mousePosition.y()) : mousePosition.y();
        newMin = y - ((y - min) / divisor);
        newMax = y + ((max - y) / divisor);

    } else if (ax == QwtPlot::xBottom) {
        // zoom only around the cursor
        const auto x = logFlag ? log10(mousePosition.x()) : mousePosition.x();
        newMin = x - ((x - min) / divisor);
        newMax = x + ((max - x) / divisor);
    }

    //come back to the linear domain
    if (logFlag) {
        newMin = pow(10, newMin);
        newMax = pow(10, newMax);
    }
    currentZoom[ax].setInterval(newMin, newMax);
    return currentZoom;
}


Rect4 LogBigPlotModel::shiftOnSingleAxis(QwtPlot::Axis ax, int shiftFactor){
    auto currentZoom = getZoom(Current);
    const auto interval = currentZoom[ax];
    bool logFlag = ax == QwtPlot::yLeft;
    const auto min = logFlag ? log10(interval.minValue()) : interval.minValue();
    const auto max = logFlag ? log10(interval.maxValue()) : interval.maxValue();
    const auto shift = (double) shiftFactor/10000*(max-min);
    auto newMin = min - shift;
    auto newMax = max - shift;
    if (logFlag) {
        newMin = pow(10, newMin);
        newMax = pow(10, newMax);
    }
    currentZoom[ax].setInterval(newMin, newMax);
    return currentZoom;
}
