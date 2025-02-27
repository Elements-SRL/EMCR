#ifndef STATISTICSRESULTWRAPPER_H
#define STATISTICSRESULTWRAPPER_H

#include "statisticsresult.h"
#include <vector>

typedef struct StatisticsResultWrapper {
    std::vector <StatisticsResult_t> results;
} StatisticsResultWrapper_t;

#endif // STATISTICSRESULTWRAPPER_H
