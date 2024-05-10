#include "firstorderiirfilter.h"
#include <iostream>
FirstOrderIirFilter::FirstOrderIirFilter(double a_val, std::pair<double, double> b_val) {
    this->a = a_val;
    this->b0 = b_val.first;
    this->b1 = b_val.second;
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
