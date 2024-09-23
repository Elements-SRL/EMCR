#include "bigplotmodel.h"
#include <cmath> 

BigPlotModel::BigPlotModel(BigPlot::BigPlotStatus bigPlotStatus) :
    bigPlotStatus(bigPlotStatus) {
    for (int i = 0; i<QwtPlot::Axis::axisCnt; i++){
        rangeInitialized[i] = false;
    }
}

bool BigPlotModel::isEmptyZoomStack(){
    return zoomStack.empty();
}

void BigPlotModel::clearZoomStack(){
    zoomStack.clear();
}

void BigPlotModel::pushZoomStack(Rect4 r){
    zoomStack.push_back(currentZoom);
}

Rect4 BigPlotModel::popZoomStack(){
    if (!this->isEmptyZoomStack()){
        currentZoom = zoomStack.back();
        zoomStack.pop_back();
        return currentZoom;
    }
    return resetZoomStack();
}

Rect4 BigPlotModel::getZoom(Zoom zoom){
    switch (zoom) {
    case Zoom::Current:
        return currentZoom;
    case Zoom::Previous:
        return popZoomStack();
    case Zoom::Default:
        return resetZoomStack();
    }
}

Rect4 BigPlotModel::resetZoomStack(){
    clearZoomStack();
    for (int i = 0; i<QwtPlot::Axis::axisCnt; i++){
        currentZoom[i] = QwtInterval(currentRange[i].min, currentRange[i].max);
    }
    return currentZoom;
}

void BigPlotModel::setCurrentRange(QwtPlot::Axis axisIdx, RangedMeasurement newRange){
    if (!isRangeInitialized(axisIdx) || newRange.unit != getCurrentRange(axisIdx).unit) {
        currentRange[axisIdx] = newRange;
        rangeInitialized[axisIdx] = true;
        setCurrentZoom(axisIdx, newRange.min, newRange.max);
        yScale = 0.5*getCurrentZoomInterval(axisIdx).width();
        return;
    }
    if (newRange == getCurrentRange(axisIdx)){
        return;
    }
    currentRange[axisIdx].max = 1.0;
    currentRange[axisIdx].convertValues(newRange.prefix);
    double coeff = currentRange[axisIdx].max;
    currentRange[axisIdx].max = newRange.max;
    currentRange[axisIdx].min = newRange.min;
    auto interval = getCurrentZoomInterval(axisIdx);
    setCurrentZoom(axisIdx, coeff*interval.minValue(), coeff*interval.maxValue());
    yScale = 0.5*coeff*interval.width();
}

void BigPlotModel::setCurrentRangeLog(QwtPlot::Axis axisIdx, RangedMeasurement newRange){
    auto newMax = newRange.max;
    auto newMin = newMax / 1.0e15;
    if (!isRangeInitialized(axisIdx) || newRange.unit != getCurrentRange(axisIdx).unit) {
        RangedMeasurement rm = RangedMeasurement{ newMin, newMax, newRange.step, newRange.prefix, newRange.unit };
        currentRange[axisIdx] = rm;
        rangeInitialized[axisIdx] = true;
        setCurrentZoom(axisIdx, newMin, newMax);
        yScale = getCurrentZoomInterval(axisIdx).width();
        return;
    }
    if (newRange == getCurrentRange(axisIdx)){
        return;
    }
    currentRange[axisIdx].max = 1.0;
    currentRange[axisIdx].convertValues(newRange.prefix);
    double coeff = currentRange[axisIdx].max;
    currentRange[axisIdx].max = newMax;
    currentRange[axisIdx].min = newMin;
    auto interval = getCurrentZoomInterval(axisIdx);
    setCurrentZoom(axisIdx, coeff*interval.minValue(), coeff*interval.maxValue());
    yScale = coeff*interval.width();
}

bool BigPlotModel::isRangeInitialized(QwtPlot::Axis axisIdx){
    return rangeInitialized[axisIdx];
}

RangedMeasurement_t BigPlotModel::getCurrentRange(QwtPlot::Axis axisIdx){
    return currentRange[axisIdx];
}

void BigPlotModel::setCurrentZoom(QwtPlot::Axis axisIdx, double min, double max){
    currentZoom[axisIdx].setInterval(min, max);
}

QwtInterval BigPlotModel::getCurrentZoomInterval(QwtPlot::Axis axisIdx){
    return currentZoom[axisIdx];
}

void BigPlotModel::updateCurrentZoom(Rect4 r){
    pushZoomStack(currentZoom);
    currentZoom = r;
}

Rect4 BigPlotModel::zoomOnSingleAxis(QwtPlot::Axis ax, int zoomInFactor, QPointF mousePosition) {
    auto currentZoom = getZoom(Current);
    const auto interval = currentZoom[ax];
    //if we are using the spectrum we have a logaritmic scale
    bool logFlag = (bigPlotStatus == BigPlot::BigPlotStatus::Spectrum) && (ax == QwtPlot::yLeft || ax == QwtPlot::xBottom);
    bool zeroLockFlag = (bigPlotStatus == BigPlot::BigPlotStatus::GapFree) && ax == QwtPlot::xBottom;

    const auto min = logFlag ? log10(interval.minValue()) : interval.minValue();
    const auto max = logFlag ? log10(interval.maxValue()) : interval.maxValue();
    const auto zoom = (double) zoomInFactor / 100;
    const auto divisor = (zoom > 0 ? zoom: -1 / zoom);

    auto newMin = min;
    auto newMax = max;
    if (zeroLockFlag) {
        newMin = min / divisor;
        newMax = max / divisor;

    } else if (ax == QwtPlot::Axis::yLeft || ax == QwtPlot::Axis::yRight) {
//        zoom only around the cursor
        const auto y = logFlag ? log10(mousePosition.y()) : mousePosition.y();
        newMin = y - ((y - min) / divisor);
        newMax = y + ((max - y) / divisor);

    } else if (ax == QwtPlot::xBottom) {
        //        zoom only around the cursor
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

Rect4 BigPlotModel::initRect(double minX, double maxX, double minY, double maxY){
    //if the x and y values did not change there's no need to reset the zoom stack
    if (minX == currentZoom[QwtPlot::Axis::xBottom].minValue() && maxX == currentZoom[QwtPlot::Axis::xBottom].maxValue() && minY == currentZoom[QwtPlot::Axis::yLeft].minValue() && maxY == currentZoom[QwtPlot::Axis::yLeft].maxValue()) {
        return currentZoom;
    }
    clearZoomStack();
    currentZoom[QwtPlot::Axis::xBottom] = QwtInterval(minX, maxX);
    currentZoom[QwtPlot::Axis::yLeft] = QwtInterval(minY, maxY);
    return currentZoom;
}

Rect4 BigPlotModel::shiftOnSingleAxis(QwtPlot::Axis ax, int shiftFactor){
    auto currentZoom = getZoom(Current);
    const auto interval = currentZoom[ax];
    bool logFlag = (bigPlotStatus == BigPlot::BigPlotStatus::Spectrum) && ax == QwtPlot::yLeft;
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
