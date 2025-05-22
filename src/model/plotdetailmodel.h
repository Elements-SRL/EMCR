#ifndef PLOTDETAILMODEL_H
#define PLOTDETAILMODEL_H

#include <stdint.h>
#include <string>
#include <curve.h>

class PlotDetailModel
{
private:
    uint16_t ch;
    Curve* curve;

public:
    PlotDetailModel(uint16_t ch);
    std::string getLabel();
    uint16_t getChannel();
    Curve* getCurve();
    void setCurve(Curve*);
};

#endif // PLOTDETAILMODEL_H
