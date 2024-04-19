#ifndef STATISTICSRESULTWRAPPER_H
#define STATISTICSRESULTWRAPPER_H

#include "statisticsresult.h"
#include <QVector>

using namespace e384CommLib;

struct StatisticsResultWrapper {
    QVector<StatisticsResult> results;
};


#endif // STATISTICSRESULTWRAPPER_H
