#include "ivchannel.h"

IvChannel::IvChannel(int nBins, double binSize){
    this->nBins = nBins;
    this->binSize = binSize;
    ivAccumulators.resize(nBins);
}

void IvChannel::pushValue(int binIdx, double value){
    ivAccumulators[binIdx].pushValue(value);
}

std::vector<std::optional<double>> IvChannel::getCurrents(){
    std::vector<std::optional<double>> avgCurrents(nBins);
    for (int i=0; i<nBins; i++){
        avgCurrents[i] = ivAccumulators[i].getMean();
    }
    return avgCurrents;
}
