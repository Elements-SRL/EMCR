#ifndef BASELINE_H
#define BASELINE_H

#include <vector>
#include <string>

struct Baseline {
    double resolution;
    std::string label;
    std::vector<int16_t> baseline;

    Baseline(double resolution_, std::vector<int16_t> baseline_, std::string label_)
        : resolution(resolution_), baseline(baseline_), label(label_) {}
};

#endif // BASELINE_H