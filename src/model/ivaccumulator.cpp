#include "ivaccumulator.h"

IvAccumulator::IvAccumulator() {

}

IvAccumulator::~IvAccumulator(){
}


void IvAccumulator::pushValue(double v){
    sum+=v;
    number_of_elements++;
}

double IvAccumulator::getMean(){
    if (sum == 0.0 || number_of_elements == 0) {
        return 0.0;
    }
    return sum/((double) number_of_elements);
}
