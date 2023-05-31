#include "controller/controllerstatearray.h"
#include "view/statearray/statearraywidget.h"
#include <iostream>
#include <fstream>
#include "model/state.h"
#include <filesystem>

ControllerStateArray::ControllerStateArray()
{
    stateArray.push_back({});
    std::cout << stateArray[0].triggerType << std::endl;
    stateArrayWidget = new StateArrrayWidget(nullptr, stateArray[0]);
}

void ControllerStateArray::showWidget(){
    stateArrayWidget->show();
}

void ControllerStateArray::printYaml(){
    YAML::Node node;
    node = stateArray;
    std::cout << node << std::endl;
}

void ControllerStateArray::open(std::string fname){
    std::filesystem::path path = std::filesystem::current_path() / fname;
    if (!std::filesystem::exists(path)){
        std::cout << "No file found" << std::endl;
        return;
    }
    std::ifstream inputFile(path); // Open the file for reading
    YAML::Node node = YAML::LoadFile(path.string());
    stateArray = node.as<std::vector<YAML::State>>();
}

void ControllerStateArray::writeToFile(std::string fname){
    YAML::Node node;
    node = stateArray;
    std::filesystem::path path = std::filesystem::current_path() / fname;
    std::ofstream file;
    file.open(path);
    file << node;
    file.close();
}

void ControllerStateArray::insertState(int idx, YAML::State s){
    stateArray.insert(stateArray.begin()+idx, s);
    stateArrayWidget->setStateChecboxesRanges(0, stateArray.size()-1);
    stateArrayWidget->setStateCount(stateArray.size());
    stateArrayWidget->setState(s);
}

void ControllerStateArray::deleteState(int idx){
    if (stateArray.size() <= 1){
        return;
    }
    stateArray.erase(stateArray.begin()+idx);
    stateArrayWidget->setStateChecboxesRanges(0, stateArray.size()-1);
    stateArrayWidget->setStateCount(stateArray.size());
    stateArrayWidget->setState(stateArray.front());
}
