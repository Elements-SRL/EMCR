#include "ivchannel.h"

IvChannel::IvChannel(int nBins, double binSize){
    this->nBins = nBins;
    this->binSize = binSize;
    ivAccumulators.resize(nBins);
    for (int i=0; i<nBins; i++){
        ivAccumulators[i] = new IvAccumulator();
    }
}

IvChannel::~IvChannel(){
    for (int i=0; i<nBins; i++){
        delete ivAccumulators[i];
    }
}

void IvChannel::pushValue(int binIdx, double value){
    ivAccumulators[binIdx]->pushValue(value);
}

std::vector<double> IvChannel::getCurrents(){
    std::vector<double> avgCurrents(nBins);
    for (int i=0; i<nBins; i++){
        avgCurrents[i] = ivAccumulators[i]->getMean();
    }
    return avgCurrents;
}
