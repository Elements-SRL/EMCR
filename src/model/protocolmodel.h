#ifndef PROTOCOLMODEL_H
#define PROTOCOLMODEL_H

#include "e384commlib_global_addendum.h"

class ProtocolModel {
public:
    ProtocolModel();

    void setStimulusRange(e384CommLib::RangedMeasurement_t range);
    e384CommLib::RangedMeasurement_t getStimulusRange();

private:
    e384CommLib::RangedMeasurement_t stimulusRange;
};

#endif // PROTOCOLMODEL_H
