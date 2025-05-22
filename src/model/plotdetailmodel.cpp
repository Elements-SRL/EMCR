#include "plotdetailmodel.h"

PlotDetailModel::PlotDetailModel(uint16_t ch) {
    this->ch = ch;
    this->curve = new Curve(CurveType_t::CurveTypePlotSolid);
}

std::string PlotDetailModel::getLabel(){
    return std::to_string((this->ch + 1));
}

uint16_t PlotDetailModel::getChannel(){
    return ch;
}

Curve* PlotDetailModel::getCurve() {
    return curve;
}
void PlotDetailModel::setCurve(Curve* c) {
    this->curve = c;
}
