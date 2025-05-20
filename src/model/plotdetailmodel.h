#ifndef PLOTDETAILMODEL_H
#define PLOTDETAILMODEL_H

#include <stdint.h>
#include <string>

class PlotDetailModel
{
private:
    uint16_t ch;

public:
    PlotDetailModel(uint16_t ch);
    std::string getLabel();
    uint16_t getChannel();
};

#endif // PLOTDETAILMODEL_H
