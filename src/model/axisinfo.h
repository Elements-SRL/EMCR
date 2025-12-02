#ifndef AXISINFO_H
#define AXISINFO_H

#include "e384commlib_global_addendum.h"
#include <optional>

struct AxisInfo {
    AxisInfo() = default;
    AxisInfo(
        e384CommLib::RangedMeasurement_t r,
        std::optional<double> m = std::nullopt,
        bool l = false,
        bool i = false,
        std::optional<std::string> ls = std::nullopt
    ):
        range(r), fixedMinimum(m), log(l), inverted(i), labelSuffix(ls) {}

    // if log is false then scale is linear
    e384CommLib::RangedMeasurement_t range;
    std::optional<double> fixedMinimum = std::nullopt;
    bool log = false;
    bool inverted = false;
    std::optional<std::string> labelSuffix = std::nullopt;

    void setRange(e384CommLib::RangedMeasurement_t r) {
        this->range = r;
    }
};
#endif // AXISINFO_H
