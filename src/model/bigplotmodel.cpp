#include "bigplotmodel.h"

BigPlotModel::BigPlotModel(){
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
    zoomStack.push_back(r);
}

Rect4 BigPlotModel::popZoomStack(){
    if (!this->isEmptyZoomStack()){
        auto r = zoomStack.back();
        zoomStack.pop_back();
        return r;
    }
    return resetZoomStack();
}

Rect4 BigPlotModel::resetZoomStack(){
    clearZoomStack();
    Rect4 r;
    for (int i = 0; i<QwtPlot::Axis::axisCnt; i++){
        r[i] = QwtInterval(currentRange[i].min, currentRange[i].max);
    }
    return r;
}

//void BigPlotModel::shiftVertAxis(QwtPlot::Axis axis, double shiftValue){

//}

//RangedMeasurement BigPlotModel::getCurrentRange(QwtPlot::Axis){

//}

//void BigPlotModel::setCurrentRange(QwtPlot::Axis axis, RangedMeasurement currentRange){

//}

//bool BigPlotModel::isRangeInitialized(QwtPlot::Axis){

//}
