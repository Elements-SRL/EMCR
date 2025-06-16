#ifndef AXISINFO_H
#define AXISINFO_H

#include "e384commlib_global_addendum.h"
#include <optional>

struct AxisInfo {
    AxisInfo() = default;
    AxisInfo(e384CommLib::RangedMeasurement_t r, std::optional<double> m = std::nullopt, bool l = false, bool i = false):
        range(r), fixedMinimum(m), log(l), inverted(i) {}

    // if log is false then scale is linear
    e384CommLib::RangedMeasurement_t range;
    std::optional<double> fixedMinimum = std::nullopt;
    bool log = false;
    bool inverted = false;
};
#endif // AXISINFO_H
