#include "binner.h"

Binner::Binner(double start, double stop, uint32_t nBins) {
    this->start = start;
    this->nBins = nBins;
    this->step = (stop - start) / ((double)(nBins - 1));
    accumulators.resize(nBins);
    std::fill(accumulators.begin(), accumulators.end(), 0);
}

void Binner::put(double value) {
    const auto k = static_cast<int>((value - start) / step);
    if (k >= 0 && k < nBins) {
        accumulators[k]++;
    }
}

double Binner::getStep() const{
    return step;
}

std::vector<double> Binner::getKeys() const {
    std::vector<double> keys(nBins);
    for (int i = 0; i < nBins; i++) {
        keys[i] = start + ((double) i) * step;
    }
    return keys;
}

std::vector<uint32_t> Binner::getValues() {
    return accumulators;
}

uint32_t Binner::getNBins() const {
    return nBins;
}

void Binner::clear() {
    for (auto& a : accumulators) {
        a = 0;
    }
}
