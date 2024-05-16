#ifndef FIRSTORDERIIRFILTER_H
#define FIRSTORDERIIRFILTER_H

#include "filter.h"

// Concrete implementation of the interface
class FirstOrderIirFilter : public Filter {
private:
    double a;
    double b0;
    double b1;
    double x_prev;
    double y_prev;

    void update(double x_prev_val, double y_prev_val);
public:
    FirstOrderIirFilter(double samplingRate, double cutoffFrequency);

    // Implementing the interface functions
    std::vector<double> filt(const std::vector<double>& input) override;

    double sfilt(const double& x);

    void init(double initial_status) override;
};
#endif // FIRSTORDERIIRFILTER_H
