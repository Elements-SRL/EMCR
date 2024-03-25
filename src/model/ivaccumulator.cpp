#include "ivaccumulator.h"

IvAccumulator::IvAccumulator() {

}

IvAccumulator::~IvAccumulator(){
    values.clear();
}


void IvAccumulator::pushValue(double v){
    values.push_back(v);
}

double IvAccumulator::getMean(){
    double sum = 0;
    auto size = values.size();
    if (size ==0) {
        return 0;
    }
    for(auto &&v: values){
        sum+=v;
    }
    values.clear();
    return sum/size;
}
