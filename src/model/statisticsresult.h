#ifndef STATISTICSRESULT_H
#define STATISTICSRESULT_H

#include "e384commlib_global_addendum.h"

namespace e384cl = e384CommLib;

typedef struct StatisticsResult {
    int chIdx;
    e384cl::Measurement_t meanVoltage;
    e384cl::Measurement_t stdVoltage;
    e384cl::Measurement_t meanCurrent;
    e384cl::Measurement_t stdCurrent;
    e384cl::Measurement_t conductivity;
} StatisticsResult_t;

#endif // STATISTICSRESULT_H
