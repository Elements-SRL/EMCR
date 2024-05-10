#ifndef FILTER_H
#define FILTER_H

#include <vector>
#include <memory>

// Define the interface
class Filter {
public:
    // Pure virtual function for filtering input
    virtual std::vector<double> filt(const std::vector<double>& input) = 0;

    // Pure virtual function for initialization
    virtual void init(double initial_status) = 0;

    // Virtual destructor
    virtual ~Filter() {}
};

#endif // FILTER_H
