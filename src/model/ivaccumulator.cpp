#include "ivaccumulator.h"

IvAccumulator::IvAccumulator() {

}

IvAccumulator::~IvAccumulator(){
}


void IvAccumulator::pushValue(double v){
    sum+=v;
    size++;
}

double IvAccumulator::getMean(){
    if (sum == 0.0 || size == 0) {
        return 0.0;
    }
    return sum/((double) size);
}
