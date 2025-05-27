#include "plotdetailmodel.h"

PlotDetailModel::PlotDetailModel(uint16_t ch, e384CommLib::RangedMeasurement_t cr) {
    this->ch = ch;
    this->curve = new Curve(CurveType_t::CurveTypePlotSolid);
    this->cr = cr;
}

std::string PlotDetailModel::getLabel() {
    return std::to_string((this->ch + 1));
}

std::string PlotDetailModel::getUom() {
    return cr.getFullUnit();
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
void PlotDetailModel::setCurrentRange(e384CommLib::RangedMeasurement_t cr) {
    this->cr = cr;
}

void PlotDetailModel::updateMargins() {
    QRectF br = qwtBoundingRect(* this->getCurve()->data());
    const auto bY = br.bottom();
    const auto tY = br.top();
    const auto padding = abs(bY-tY)/10;
    bottomY = bY + padding;
    topY = tY - padding;
}

double PlotDetailModel::getTopY() {
    return this->topY;
}

double PlotDetailModel::getBottomY() {
    return this->bottomY;
}
