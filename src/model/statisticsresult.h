#ifndef STATISTICSRESULT_H
#define STATISTICSRESULT_H

#include "e384commlib_global_addendum.h"
using namespace e384CommLib;

struct StatisticsResult {
    int chIdx;
    Measurement_t meanVoltage;
    //Measurement_t stdVoltage;
    Measurement_t meanCurrent;
    Measurement_t stdCurrent;
    Measurement_t conductivity;
};


#endif // STATISTICSRESULT_H
