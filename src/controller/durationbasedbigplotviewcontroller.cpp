#include "durationbasedbigplotviewcontroller.h"

DurationBasedBigPlotViewController::DurationBasedBigPlotViewController(std::unique_ptr<BigPlotModel> model, std::unique_ptr<BigPlot> plot, QObject *parent)
    : BigPlotViewController{std::move(model), std::move(plot), parent}
{
}

Rect4 DurationBasedBigPlotViewController::handleZoomInRequest(Rect4 r) {
    const auto zoom = BigPlotViewController::handleZoomInRequest(r);
    emit durationChanged({ zoom[QwtPlot::xBottom].width(), model->getCurrentRange(QwtPlot::xBottom).prefix, "s" });
    return zoom;
}

Rect4 DurationBasedBigPlotViewController::handleSingleAxisZoomRequest(QwtPlot::Axis axis, int zoomIn, QPointF mousePosition){
    const auto zoom = BigPlotViewController::handleSingleAxisZoomRequest(axis, zoomIn, mousePosition);
    if (axis == QwtPlot::Axis::xBottom){
        emit durationChanged({zoom[QwtPlot::xBottom].width(), model->getCurrentRange(QwtPlot::xBottom).prefix, "s"});
    }
    return zoom;
}

Rect4 DurationBasedBigPlotViewController::handleSingleAxisZoomRequest(QwtPlot::Axis axis, QwtInterval i) {
    const auto zoom = BigPlotViewController::handleSingleAxisZoomRequest(axis, i);
    if (axis == QwtPlot::Axis::xBottom){
        emit durationChanged({zoom[QwtPlot::xBottom].width(), model->getCurrentRange(QwtPlot::xBottom).prefix, "s"});
    }
    return zoom;
}

Rect4 DurationBasedBigPlotViewController::handleZoomOutRequest(){
    const auto zoom = BigPlotViewController::handleZoomOutRequest();
    emit durationChanged({ zoom[QwtPlot::xBottom].width(), model->getCurrentRange(QwtPlot::xBottom).prefix, "s" });
    return zoom;
}

Rect4 DurationBasedBigPlotViewController::handleZoomResetRequest(){
    const auto zoom = BigPlotViewController::handleZoomResetRequest();
    emit durationChanged({ zoom[QwtPlot::xBottom].width(), model->getCurrentRange(QwtPlot::xBottom).prefix, "s" });
    return zoom;
}
