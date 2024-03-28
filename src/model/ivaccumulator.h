#ifndef IVACCUMULATOR_H
#define IVACCUMULATOR_H

#define DEFAULT_ACCUMULATOR_SIZE 10

class IvAccumulator {
public:
    IvAccumulator();
    ~IvAccumulator();
    double getMean();
    void pushValue(double v);

private:
    double sum = 0.0;
    unsigned long number_of_elements = 0;
};

#endif // IVACCUMULATOR_H
