#include "eventdetector.h"
#include <cmath>
#include <iostream>

EventDetector::EventDetector(int sizeHint) {
    double coef_a_high = -0.50952545;
    std::pair<double, double> coeffs_b_high = std::make_pair(0.24523728, 0.24523728);
    high = new FirstOrderIirFilter(coef_a_high, coeffs_b_high);
    double coef_a_low = -0.9994;
    std::pair<double, double> coeffs_b_low = std::make_pair(0.0003, 0.0003);
    low = new FirstOrderIirFilter(coef_a_low, coeffs_b_low);
    std::cout << "creating event detection" << std::endl;
    if (sizeHint != -1) {
        events.reserve(sizeHint);
    }
}

void EventDetector::pushEvent(Event event) {
    events.push_back(event);
}

std::vector<Event> EventDetector::getEvents() {
    return events;
}

double EventDetector::calculateThreshold(const std::vector<double>& data) {
    const auto stdDev = calcStdDev(data);
    return stdDev != -1 ? EVENT_TH * stdDev : stdDev;
}

double EventDetector::calcStdDev(const std::vector<double>& data) {
    const size_t n = data.size();
    if (n <= 1) {
        return -1.0; // Standard deviation is undefined for one or zero elements
    }

    // Calculate the mean and sum of squared differences
    double sum = 0.0;
    double sum_squared_diffs = 0.0;

#pragma omp parallel for reduction(+:sum,sum_squared_diffs)
    for (int i = 0; i < n; ++i) {
        sum += data[i];
    }

    const double mean = sum / static_cast<double>(n);

#pragma omp parallel for reduction(+:sum_squared_diffs)
    for (int i = 0; i < n; ++i) {
        const double diff = data[i] - mean;
        sum_squared_diffs += diff * diff;
    }

    // Calculate the variance
    const double variance = sum_squared_diffs / static_cast<double>(n - 1); // using Bessel's correction

    // Standard deviation is the square root of variance
    return std::sqrt(variance);
}

std::optional<std::pair<int, int>> EventDetector::analyze(double currentValue, uint32_t idx, uint32_t clipValue) {
    //TODO, For now just reinit everything
    if (idx == 0) {
        threshold = calculateThreshold(bandPassFilterData);
        //now that we have an updated value throw away the old ones
        bandPassFilterData.clear();
        eventAlreadyBegun = false;
        eventLen = 0;
        eventBeginIdx = 0;
    }
    //begin event analysis
    const auto baseline = low->sfilt(currentValue);
    const auto s_no_baseline = currentValue - baseline;
    const auto re_filtered = high->sfilt(s_no_baseline);

    // threshold is not initialized yet
    if (threshold == -1) {
        bandPassFilterData.push_back(re_filtered);
        return std::nullopt;
    }

    // WARNING THERE IS A - BEFORE THRESHOLD, ONLY WORKS WITH EVENTS GOING DOWN
    const auto isEvent = re_filtered < -threshold;
    if (isEvent && !eventAlreadyBegun) {
        eventAlreadyBegun = true;
        eventBeginIdx = idx;
        return std::nullopt;
    }
    if (isEvent && eventAlreadyBegun) {
        eventLen++;
        return std::nullopt;
    }
    bandPassFilterData.push_back(re_filtered);
    if (!eventAlreadyBegun) {
        return std::nullopt;
    }
    //event already begun
    if (eventLen >= MAX_LEN) {
        low->init(currentValue);
        return std::nullopt;
    }
    if (eventLen > MIN_LEN) {
        const auto e0 = eventBeginIdx - (EVENT_PADDING * eventLen);
        const auto e1 = eventBeginIdx + eventLen + (EVENT_PADDING * eventLen);
        return std::make_pair((e0 < 0) ? 0 : e0, (e1 >= clipValue) ? clipValue - 1 : e1);
    }
    eventAlreadyBegun = false;
    eventLen = 0;
}
