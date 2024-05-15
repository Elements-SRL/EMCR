#include "firstorderiirfilter.h"
#include <iostream>
#include <qmath.h>

FirstOrderIirFilter::FirstOrderIirFilter(double samplingRate, double cutoffFrequency) {
    const double w = 2.0 * M_PI * cutoffFrequency;
    const double dt = 1.0 / samplingRate;
    const double commonPart = w * dt;
    const double b = commonPart / (2.0 + commonPart);
    this->a = -(2 - commonPart)/(2.0 + commonPart);
    this->b0 = b;
    this->b1 = b;
    x_prev = 0;
    y_prev = 0;
}

double FirstOrderIirFilter::sfilt(const double& x) {
    return (b0 * x) + (b1 * x_prev) - (a * y_prev);
}

std::vector<double> FirstOrderIirFilter::filt(const std::vector<double>& input) {
    auto s = input.size();
    std::vector<double> output_signal(s);
    for (int i = 0; i < s; i++) {
        auto x = input[i];
        double output = (b0 * x) + (b1 * x_prev) - (a * y_prev);
        output_signal[i] = output;
        update(x, output);
    }
    return output_signal;
}

void FirstOrderIirFilter::update(double x_prev_val, double y_prev_val) {
    x_prev = x_prev_val;
    y_prev = y_prev_val;
}

void FirstOrderIirFilter::init(double initial_status) {
    update(initial_status, initial_status);
}
