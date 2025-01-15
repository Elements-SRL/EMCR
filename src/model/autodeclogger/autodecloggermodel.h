#ifndef AUTODECLOGGERMODEL_H
#define AUTODECLOGGERMODEL_H

#include <vector>

struct AutoDecloggerModel {
    std::vector<double> thresholds;
    std::vector<double> stimuli;
    std::vector<double> decloggingTimes;
    std::vector<double> belowThresholdsTimes;

    AutoDecloggerModel(std::vector<double> thresholds_, std::vector<double> stimuli_, std::vector<double> decloggingTimes_, std::vector<double> belowThresholdsTimes_)
        : thresholds(thresholds_), stimuli(stimuli_), decloggingTimes(decloggingTimes_), belowThresholdsTimes(belowThresholdsTimes_){}
};

#endif // AUTODECLOGGERMODEL_H