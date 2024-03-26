#include "ivaccumulator.h"

IvAccumulator::IvAccumulator() {

}

IvAccumulator::~IvAccumulator(){
}


void IvAccumulator::pushValue(double v){
    sum+=v;
    sum+=1.0;
}

double IvAccumulator::getMean(){
    return sum/size;
}
