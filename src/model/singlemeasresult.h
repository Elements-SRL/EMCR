#ifndef SINGLEMEASRESULT_H
#define SINGLEMEASRESULT_H

#include "e384commlib_global_addendum.h"

namespace e384cl = e384CommLib;

typedef struct SingleMeasResult {
    int chIdx;
    e384cl::Measurement_t meas;
} SingleMeasResult_t;

#endif // SINGLEMEASRESULT_H
