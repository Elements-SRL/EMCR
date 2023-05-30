#include "controller/controllerstatearray.h"
#include "view/statearray/statearraywidget.h"
#include <iostream>

ControllerStateArray::ControllerStateArray()
{
    stateArray.push_back({});
    std::cout << stateArray[0].triggerType << std::endl;
    stateArrayWidget = new StateArrrayWidget();
}


void ControllerStateArray::showWidget(){
    stateArrayWidget->show();
}

void ControllerStateArray::printYaml(){
    YAML::Node node;
    node = stateArray;
    std::cout << node << std::endl;
}
