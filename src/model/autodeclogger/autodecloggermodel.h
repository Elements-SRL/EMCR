#ifndef AUTODECLOGGERMODEL_H
#define AUTODECLOGGERMODEL_H

#include <vector>

struct AutoDecloggerModel {
    std::vector<double> thresholds;
    std::vector<double> stimuli;
    std::vector<double> msTimes;

    AutoDecloggerModel(std::vector<double> thresholds_, std::vector<double> stimuli_, std::vector<double> msTimes_)
        : thresholds(thresholds_), stimuli(stimuli_), msTimes(msTimes_) {}
};

#endif // AUTODECLOGGERMODEL_H