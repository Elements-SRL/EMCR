#ifndef RESULTWRAPPER_H
#define RESULTWRAPPER_H

#include <vector>

#include "singlemeasresult.h"
#include "statisticsresult.h"
#include "membraneresult.h"

typedef std::vector <SingleMeasResult_t> SingleMeasResultWrapper_t;
typedef std::vector <StatisticsResult_t> StatisticsResultWrapper_t;
typedef std::vector <MembraneResult_t> MembraneResultWrapper_t;

#endif // RESULTWRAPPER_H
