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
    zoom = Zoom(buildRect(axisInfos));
    // todo in the plot loop over the plot model enabled axis to enable them
}

void PlotModel::setRangedMeasurement(QwtPlot::Axis axis, e384CommLib::RangedMeasurement_t range) {
    axisInfo[axis].range = range;
    zoom = Zoom(buildRect(axisInfo));
    // todo in the plot loop over the plot model enabled axis to enable them
}

std::optional<std::string> PlotModel::getUnitLabel(QwtPlot::Axis axis) {
    auto it = axisInfo.find(axis);
    return it != axisInfo.end() ? std::make_optional(it->second.range.getFullUnit()) : std::nullopt;
}

std::map<QwtPlot::Axis, std::string> PlotModel::getUnitLabes() {
    std::map<QwtPlot::Axis, std::string> r;
    for (auto &t: axisInfo) {
        r[t.first] = t.second.range.getFullUnit();
    }
    return r;
}

Rect4 PlotModel::getZoom() {
    return zoom.peek();
}
