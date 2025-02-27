#ifndef SINGLEMEASRESULTWRAPPER_H
#define SINGLEMEASRESULTWRAPPER_H

#include "singlemeasresult.h"
#include <vector>

typedef struct SingleMeasResultWrapper {
    std::vector <SingleMeasResult_t> results;
} SingleMeasResultWrapper_t;

#endif // SINGLEMEASRESULTWRAPPER_H
