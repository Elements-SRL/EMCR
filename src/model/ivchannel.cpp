#include "ivchannel.h"

IvChannel::IvChannel(int nBins, double binSize, double minVoltageValue){
    this->nBins = nBins;
    this->binSize = binSize;
    for (int i=0; i<nBins; i++){
        ivAccumulators.push_back(new IvAccumulator());
        voltages.push_back(minVoltageValue + (binSize * i));
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
    std::vector<double> avgCurrents;
    for (auto &&acc: ivAccumulators){
        avgCurrents.push_back(acc->getMean());
    }
    return avgCurrents;
}

std::vector<double> IvChannel::getVoltages(){
    return this->voltages;
}
