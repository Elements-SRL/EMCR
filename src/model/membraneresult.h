#ifndef MEMBRANERESULT_H
#define MEMBRANERESULT_H

#include "e384commlib_global_addendum.h"

namespace e384cl = e384CommLib;

typedef struct MembraneResult {
    int chIdx;
    e384cl::Measurement_t membraneCapacitance;
    e384cl::Measurement_t accessResistance;
    e384cl::Measurement_t membraneResistance;
} MembraneResult_t;

#endif // MEMBRANERESULT_H
