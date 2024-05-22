#ifndef BINNER_H
#define BINNER_H

#include <map>
#include <vector>

class Binner
{
public:
    Binner(double start, double stop, uint32_t nBins);

    void put(double value);
    double getStep() const;
    uint32_t getNBins() const;
    std::vector<double> getKeys() const;
    std::vector<uint32_t> getValues();

private:
    int nBins;
    double start;
    double step;
    std::vector<uint32_t> accumulators;
};

#endif // BINNER_H
