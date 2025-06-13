#include "plotmodel.h"

void axisInfo2Rect(std::pair<QwtPlot::Axis, AxisInfo> ai, Rect4 &r) {
    const auto v = ai.second;
    const auto min = v.fixedMinimum.has_value() ? v.fixedMinimum.value(): v.range.max;
    r[ai.first].setInterval(min, v.range.max);
}

Rect4 buildRect(std::map<QwtPlot::Axis, AxisInfo> axisInfos) {
    Rect4 r;
    for (auto entry: axisInfos) {
        axisInfo2Rect(entry, r);
    }
    return r;
}

PlotModel::PlotModel(std::map<QwtPlot::Axis, AxisInfo> axisInfos) {
    zoom = std::make_unique<Zoom>(buildRect(axisInfos));
    connect(zoom.get(), &Zoom::sigZoomChanged, this, &PlotModel::sigReplot);
    // todo in the plot loop over the plot model enabled axis to enable them
}

void PlotModel::setRangedMeasurement(QwtPlot::Axis axis, e384CommLib::RangedMeasurement_t range) {
    axisInfo[axis].range = range;
    zoom = std::make_unique<Zoom>(buildRect(axisInfo));
    // todo in the plot loop over the plot model enabled axis to enable them
}

std::optional<std::string> PlotModel::getUnitLabel(QwtPlot::Axis axis) {
    return isAxisEnabled(axis) ? std::make_optional(axisInfo[axis].range.getFullUnit()) : std::nullopt;
}

std::map<QwtPlot::Axis, std::string> PlotModel::getUnitLabes() {
    std::map<QwtPlot::Axis, std::string> r;
    for (auto &t: axisInfo) {
        r[t.first] = t.second.range.getFullUnit();
    }
    return r;
}

Rect4 PlotModel::getZoom() {
    return zoom->peek();
}

void PlotModel::onZoomInPickerAppended(const QPointF &p, QWidget* canvas) {
    auto yLeft = axisInfo[QwtPlot::Axis::yLeft];
    auto xBottom = axisInfo[QwtPlot::Axis::xBottom];
    auto dyMin = yLeft.range.min;
    auto dyMax = yLeft.range.max;
    auto dxMin = xBottom.range.min;
    auto dxMax = xBottom.range.max;
    if (yLeft.log) {
        dyMin = log10(dyMin);
        dyMax = log10(dyMax);
    }
    if (xBottom.log) {
        dxMin = log10(dxMin);
        dxMax = log10(dxMax);
    }
    auto dy = dyMax - dyMin;
    auto dx = dxMax - dxMin;
    pickerFirstCornerPos = processPoint(p);

    auto cdx = (double)canvas->width();
    auto cdy = (double)canvas->height();
    pickerZoomDiscriminantNorm = (dy*cdx)/(dx*cdy);
}

void PlotModel::onZoomInPickerMoved(const QPointF &p) {
    QPointF deltaP = processPoint(p);
    auto dx = deltaP.x();
    auto dy = deltaP.y();
    double zoomDiscriminantRatio = abs(dx / dy)*pickerZoomDiscriminantNorm;
    if (zoomDiscriminantRatio < 0.1) {
        rubberBand = std::make_optional(QwtPlotPicker::VLineRubberBand);
    } else if (zoomDiscriminantRatio > 10.0) {
        rubberBand = std::make_optional(QwtPlotPicker::HLineRubberBand);
    } else {
        rubberBand = std::make_optional(QwtPlotPicker::RectRubberBand);
    }
    emit sigRubberBandUpdated();
}

QPointF PlotModel::processPoint(const QPointF &p) {
    auto px = p.x();
    auto py = p.y();
    if (axisInfo[QwtPlot::Axis::yLeft].log) {
        py = log10(p.y());
    }
    if (axisInfo[QwtPlot::Axis::xBottom].log) {
        px = log10(p.x());
    }
    return QPointF(px, py);
}

std::optional<QwtPlotPicker::RubberBand> PlotModel::getRubberBand() {
    return rubberBand;
}

void PlotModel::onZoomInPickerSelected(const QRectF &r) {
    Rect4 rect(r);
    if ((rect.at(QwtPlot::Axis::yLeft).width() == 0.0) && (rect.at(QwtPlot::Axis::xBottom).width() == 0.0 || !rubberBand.has_value())) {
        return;
    }
    auto rY = r.y();
    auto rH = r.height();
    const auto yLeft = axisInfo[QwtPlot::Axis::yLeft];
    const auto yRight = axisInfo[QwtPlot::Axis::yRight];
    auto yL = yLeft.range.min;
    auto hL = yLeft.range.max;
    auto yR = yRight.range.min;
    auto hR = yRight.range.max;

    if (yLeft.log) {
        hL = log(yL + hL);
        yL = log(yL);
        hL -= yL;
        rH = log(rY + rH);
        rY = log(rY);
        rH -= rY;
    }

    const auto rightLogFlag = yRight.log;
    if (rightLogFlag) {
        hR = log(yR + hR);
        yR = log(yR);
        hR -= yR;
    }
    const auto rightLeftRatio = hR/hL;
    const auto rightOffset = yR+(rY-yL)*rightLeftRatio;
    const auto rightHeight = rH*rightLeftRatio;
    const auto rb = rubberBand.value();
    const auto cache = zoom->peek();

    const auto i0 = rightLogFlag ? exp(rightOffset) : rightOffset;
    const auto i1 = rightLogFlag ? exp(rightOffset + rightHeight) : rightOffset + rightHeight;

    switch (rb) {
    case QwtPlotPicker::RectRubberBand:
        if (isAxisEnabled(QwtPlot::Axis::yRight)) {
            rect[QwtPlot::Axis::yRight].setInterval(i0, i1);
        }
        break;

    case QwtPlotPicker::HLineRubberBand:
        rect[QwtPlot::Axis::yLeft].setInterval(cache[QwtPlot::Axis::yLeft].minValue(), cache[QwtPlot::Axis::yLeft].maxValue());
        rect[QwtPlot::Axis::yRight].setInterval(cache[QwtPlot::Axis::yRight].minValue(), cache[QwtPlot::Axis::yRight].maxValue());
        break;

    case QwtPlotPicker::VLineRubberBand:
        rect[QwtPlot::Axis::xBottom].setInterval(cache[QwtPlot::Axis::xBottom].minValue(), cache[QwtPlot::Axis::xBottom].maxValue());
        if (isAxisEnabled(QwtPlot::Axis::yRight)) {
            rect[QwtPlot::Axis::yRight].setInterval(i0, i1);
        }
        break;
    }
    zoom->push(rect);
    rubberBand = std::nullopt;
    emit sigRubberBandUpdated();
}

bool PlotModel::isAxisEnabled(QwtPlot::Axis a) {
    return axisInfo.find(a) != axisInfo.end();
}

void PlotModel::onSingleAxisZoom(QwtPlot::Axis ax, int zoomInFactor, QPointF mousePosition) {
    auto currentZoom = zoom->peek();
    const auto interval = currentZoom[ax];
    // suppose that linear stuff all start at zero
    bool zeroLockFlag = ax == QwtPlot::xBottom;

    const auto min = interval.minValue();
    const auto max = interval.maxValue();
    const auto zoom = (double) zoomInFactor / 100;
    const auto divisor = (zoom > 0 ? zoom: -1 / zoom);

    auto newMin = min;
    auto newMax = max;
    if (zeroLockFlag) {
        newMin = min / divisor;
        newMax = max / divisor;

    } else if (ax == QwtPlot::Axis::yLeft || ax == QwtPlot::Axis::yRight) {
        // zoom only around the cursor
        const auto y = mousePosition.y();
        newMin = y - ((y - min) / divisor);
        newMax = y + ((max - y) / divisor);

    } else if (ax == QwtPlot::xBottom) {
        // zoom only around the cursor
        const auto x = mousePosition.x();
        newMin = x - ((x - min) / divisor);
        newMax = x + ((max - x) / divisor);
    }

    currentZoom[ax].setInterval(newMin, newMax);
    this->zoom->push(currentZoom);
}

void PlotModel::onSingleAxisShift(QwtPlot::Axis ax, int shiftFactor) {
    auto currentZoom = zoom->peek();
    const auto interval = currentZoom[ax];
    const auto min = interval.minValue();
    const auto max = interval.maxValue();
    const auto shift = (double) shiftFactor/10000*(max-min);
    auto newMin = min - shift;
    auto newMax = max - shift;
    currentZoom[ax].setInterval(newMin, newMax);
    zoom->push(currentZoom);
}

void PlotModel::onZoomOut() {
    zoom->pop();
}

void PlotModel::onZoomReset() {
    zoom->reset();
}
