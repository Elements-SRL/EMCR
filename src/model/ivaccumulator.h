#ifndef IVACCUMULATOR_H
#define IVACCUMULATOR_H

#include <vector>

#define DEFAULT_ACCUMULATOR_SIZE 10

class IvAccumulator {
public:
    IvAccumulator();
    ~IvAccumulator();
    double getMean();
    void pushValue(double v);

private:
    double sum;
    double size;
};

#endif // IVACCUMULATOR_H
