#include "protocolmodel.h"

ProtocolModel::ProtocolModel() {

}

void ProtocolModel::setStimulusRange(e384CommLib::RangedMeasurement_t range) {
    stimulusRange = range;
}

e384CommLib::RangedMeasurement_t ProtocolModel::getStimulusRange() {
    return stimulusRange;
}
