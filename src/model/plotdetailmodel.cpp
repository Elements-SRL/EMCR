#include "plotdetailmodel.h"

PlotDetailModel::PlotDetailModel(uint16_t ch) {
    this->ch = ch;
}

std::string PlotDetailModel::getLabel(){
    return std::to_string((this->ch + 1));
}

uint16_t PlotDetailModel::getChannel(){
    return ch;
}
