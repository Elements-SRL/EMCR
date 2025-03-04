#include "bigplotviewcontroller.h"

BigPlotViewController::BigPlotViewController(std::unique_ptr<BigPlotModel> model, BigPlot* plot, QObject *parent)
    : QObject{parent}, model(std::move(model)), plot(plot)
{
}

void BigPlotViewController::setup() {
    connect(this->plot, &BigPlot::zoomInRequest, this, [this](Rect4 r) {
        handleZoomInRequest(r);
    });
    connect(this->plot, &BigPlot::zoomOutRequest, this, [this]() {
        handleZoomOutRequest();
    });
    connect(this->plot, &BigPlot::zoomResetRequest, this, [this]() {
        handleZoomResetRequest();
    });
    connect(this->plot, &BigPlot::singleAxisZoomRequest, this, [this](QwtPlot::Axis axis, int zoomIn, QPointF mousePosition) {
        handleSingleAxisZoomRequest(axis, zoomIn, mousePosition);
    });
    connect(this->plot, &BigPlot::singleAxisShiftRequest, this, [this](QwtPlot::Axis axis, int shift) {
        handleSingleAxisShiftRequest(axis, shift);
    });
}

Rect4 BigPlotViewController::handleZoomInRequest(Rect4 r) {
    //    non idale, rischio di incoerenza con le altre chiamate nel model
    model->updateCurrentZoom(r);
    auto zoom = model->getZoom(BigPlotModel::Zoom::Current);
    plot->setRect(zoom);
    return zoom;
}

Rect4 BigPlotViewController::handleSingleAxisZoomRequest(QwtPlot::Axis axis, int zoomIn, QPointF mousePosition){
    //    non idale, rischio di incoerenza con le altre chiamate nel model
    model->updateCurrentZoom(model->zoomOnSingleAxis(axis, zoomIn, mousePosition));
    auto zoom = model->getZoom(BigPlotModel::Zoom::Current);
    plot->setRect(zoom);
    return zoom;
}

Rect4 BigPlotViewController::handleSingleAxisZoomRequest(QwtPlot::Axis axis, QwtInterval i) {
    //    non idale, rischio di incoerenza con le altre chiamate nel model
    model->updateCurrentZoom(i, axis);
    auto zoom = model->getZoom(BigPlotModel::Zoom::Current);
    plot->setRect(zoom);
    return zoom;
}

Rect4 BigPlotViewController::handleSingleAxisShiftRequest(QwtPlot::Axis axis, int shift){
    model->updateCurrentZoom(model->shiftOnSingleAxis(axis, shift));
    auto zoom = model->getZoom(BigPlotModel::Zoom::Current);
    plot->setRect(zoom);
    return zoom;
}

Rect4 BigPlotViewController::handleZoomOutRequest(){
    auto zoom = model->getZoom(BigPlotModel::Zoom::Previous);
    plot->setRect(zoom);
    return zoom;
}

Rect4 BigPlotViewController::handleZoomResetRequest(){
    auto zoom = model->getZoom(BigPlotModel::Zoom::Default);
    plot->setRect(zoom);
    return zoom;
}

BigPlot* BigPlotViewController::getPlot(){
    return plot;
}

BigPlotModel* BigPlotViewController::getModel(){
    return model.get();
}
