#ifndef IVCHANNEL_H
#define IVCHANNEL_H

#include <vector>
#include "ivaccumulator.h"

class IvChannel
{
public:
    IvChannel(int nBins, double binSize, double minVoltageValue);
    ~IvChannel();

    void pushValue(int binIdx, double value);
    std::vector<double> getVoltages();
    std::vector<double> getCurrents();


private:
    int nBins;
    double binSize;
    std::vector<IvAccumulator *> ivAccumulators;
    std::vector<double> voltages;
};

#endif // IVCHANNEL_H
