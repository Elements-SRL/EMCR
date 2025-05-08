#ifndef IVCHANNEL_H
#define IVCHANNEL_H

#include <vector>
#include "ivaccumulator.h"
#include <optional>

class IvChannel
{
public:
    IvChannel(int nBins, double binSize);

    void pushValue(int binIdx, double value);
    std::vector<std::optional<double>> getCurrents();


private:
    int nBins;
    double binSize;
    std::vector<IvAccumulator> ivAccumulators;
};

#endif // IVCHANNEL_H
