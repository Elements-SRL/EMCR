#ifndef AUTODECLOGGERMODEL_H
#define AUTODECLOGGERMODEL_H

#include <vector>
#include <string>
#include "e384commlib_global_addendum.h"

struct AutoDecloggerModel {
    std::vector<double> thresholds;
    std::vector<double> stimuli;
    std::vector<double> msTimes;

    AutoDecloggerModel(std::vector<double> thresholds_, std::vector<double> stimuli_, std::vector<double> msTimes_)
        : thresholds(thresholds_), stimuli(stimuli_), msTimes(msTimes_) {}
};

#endif // BASELINE_H