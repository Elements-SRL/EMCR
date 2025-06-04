#ifndef PLOTDETAILMODEL_H
#define PLOTDETAILMODEL_H

#include <stdint.h>
#include <string>
#include <curve.h>
#include "e384commlib_global_addendum.h"

class PlotDetailModel
{
private:
    uint16_t ch;
    e384CommLib::RangedMeasurement_t cr;
    double bottomY;
    double topY;
    Curve* curve;

public:
    PlotDetailModel(uint16_t ch, e384CommLib::RangedMeasurement_t cr);
    std::string getLabel();
    std::string getUom();
    void setCurrentRange(e384CommLib::RangedMeasurement_t cr);
    uint16_t getChannel();
    Curve* getCurve();
    void setCurve(Curve*);
    void updateMargins();
    double getTopY();
    double getBottomY();
    void setCurveColor(QColor);
};

#endif // PLOTDETAILMODEL_H
