#include "ivaccumulator.h"

IvAccumulator::IvAccumulator() {

}

void IvAccumulator::pushValue(double v){
    sum+=v;
    number_of_elements++;
}

std::optional<double> IvAccumulator::getMean(){
    if (sum == 0.0 || number_of_elements < 20) {
        return std::nullopt;
    }
    return sum/((double) number_of_elements);
}
