#include "plotmodel.h"

void axisInfo2Rect(std::pair<QwtPlot::Axis, AxisInfo> ai, Rect4 &r) {
    const auto v = ai.second;
    const auto min = v.fixedMinimum.has_value() ? v.fixedMinimum.value(): v.range.min;
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
    connect(zoom.get(), &Zoom::sigZoomChanged, this, &PlotModel::sigReplot, Qt::QueuedConnection);
    axisInfo = axisInfos;
    emit sigReplot();
    // todo in the plot loop over the plot model enabled axis to enable them
}

bool PlotModel::isAxisLog(QwtPlot::Axis ax) {
    for (auto &t: axisInfo) {
        if (t.first == ax){
            return t.second.log;
        }
    }
    return false;
}

void PlotModel::setRangedMeasurement(QwtPlot::Axis axis, e384CommLib::RangedMeasurement_t range) {
    axisInfo[axis].setRange(range);
    zoom = std::make_unique<Zoom>(buildRect(axisInfo));
    // by removing this there is no zoom
    connect(zoom.get(), &Zoom::sigZoomChanged, this, &PlotModel::sigReplot, Qt::QueuedConnection);
    emit sigReplot();
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
    if (pickerFirstCornerPos->isNull()) {
        return;
    }
    QPointF deltaP = pickerFirstCornerPos.value() - processPoint(p);
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
    const auto yLAxis = QwtPlot::Axis::yLeft;
    const auto yRAxis = QwtPlot::Axis::yRight;
    const auto xBottom = QwtPlot::Axis::xBottom;
    if ((rect.at(yLAxis).width() == 0.0) && (rect.at(xBottom).width() == 0.0 || !rubberBand.has_value())) {
        return;
    }
    auto oldRect = zoom->peek();
    auto rY = r.y();
    auto rH = r.height();
    auto yL = oldRect[yLAxis].minValue();
    auto hL = oldRect[yLAxis].width();
    auto yR = oldRect[yRAxis].minValue();
    auto hR = oldRect[yRAxis].width();

    if (axisInfo[yLAxis].log) {
        hL = log(yL + hL);
        yL = log(yL);
        hL -= yL;
    }
    auto yRLog = axisInfo[yRAxis].log;
    if (yRLog) {
        rH = log(rY + rH);
        rY = log(rY);
        rH -= rY;
        hR = log(yR + hR);
        yR = log(yR);
        hR -= yR;
    }
    auto rightLeftRatio = hR/hL;
    auto rightOffset = yR+(rY-yL)*rightLeftRatio;
    auto rightHeight = rH*rightLeftRatio;
    auto yR0 = rightOffset;
    auto yR1 = rightOffset + rightHeight;
    yR0 = yRLog ? log(yR0) : yR0;
    yR1 = yRLog ? log(yR1) : yR1;

    auto rb = rubberBand.value();
    switch (rb) {
    case QwtPlotPicker::RectRubberBand:
        if (isAxisEnabled(yRAxis)) {
            rect[QwtPlot::Axis::yRight].setInterval(yR0, yR1);
        }
        if (axisInfo[xBottom].fixedMinimum.has_value()) {
            rect[xBottom].setInterval(oldRect[xBottom].minValue(), rect[xBottom].maxValue());
        }
        break;
    case QwtPlotPicker::HLineRubberBand:
        rect[yLAxis].setInterval(oldRect[yLAxis].minValue(), oldRect[yLAxis].maxValue());
        rect[yRAxis].setInterval(oldRect[yRAxis].minValue(), oldRect[yRAxis].maxValue());
        if (axisInfo[xBottom].fixedMinimum.has_value()) {
            rect[xBottom].setInterval(oldRect[xBottom].minValue(), rect[xBottom].maxValue());
        }
        break;
    case QwtPlotPicker::VLineRubberBand:
        rect[xBottom].setInterval(oldRect[xBottom].minValue(), oldRect[xBottom].maxValue());
        if (isAxisEnabled(yRAxis)) {
            rect[QwtPlot::Axis::yRight].setInterval(yR0, yR1);
        }
        if (axisInfo[xBottom].fixedMinimum.has_value()) {
            rect[xBottom].setInterval(oldRect[xBottom].minValue(), rect[xBottom].maxValue());
        }
        break;
    }
    emit sigRubberBandUpdated();
    zoom->push(rect);
    rubberBand = std::nullopt;
}

bool PlotModel::isAxisEnabled(QwtPlot::Axis a) {
    return axisInfo.find(a) != axisInfo.end();
}

void PlotModel::onSingleAxisZoom(QwtPlot::Axis ax, int zoomInFactor, QPointF mousePosition) {
    auto currentZoom = zoom->peek();
    auto fm = axisInfo[ax].fixedMinimum;
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
        // if minimum is fixed use fixed minimum
        newMin = fm.value_or(min - newMin);
        newMax = x + ((max - x) / divisor);
    }

    currentZoom[ax].setInterval(newMin, newMax);
    this->zoom->push(currentZoom);
}

void PlotModel::onSingleAxisShift(QwtPlot::Axis ax, int shiftFactor) {
    auto currentZoom = zoom->peek();
    auto fm = axisInfo[ax].fixedMinimum;
    const auto interval = currentZoom[ax];
    const auto min = fm.value_or(interval.minValue());
    const auto max = interval.maxValue();
    const auto shift = (double) shiftFactor/10000*(max-min);
    auto newMin = fm.value_or(min - shift);
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

std::vector <QwtPlot::Axis> PlotModel::getActiveAxes() {
    std::vector <QwtPlot::Axis> axes;
    for (auto a: axisInfo) {
        axes.push_back(a.first);
    }
    return axes;
}

e384CommLib::RangedMeasurement_t PlotModel::getAxisRangedMeasurement(QwtPlot::Axis axis) {
    const auto r = zoom->peek();
    const auto min = r[axis].minValue();
    const auto max = r[axis].maxValue();
    auto rm = axisInfo[axis].range;
    rm.min = min;
    rm.max = max;
    return rm;
}
