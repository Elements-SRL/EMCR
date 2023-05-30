#include "controller/controllerstatearray.h"
#include "view/statearray/statearraywidget.h"
#include <iostream>
#include <fstream>

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

void ControllerStateArray::writeToFile(std::string fname){
    YAML::Node node;
    node = stateArray;
    std::ofstream file;
    file.open(fname);
    file << node;
    file.close();
}
