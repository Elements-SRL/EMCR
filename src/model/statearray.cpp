#include "model/statearray.h"
#include <filesystem>
#include <iostream>
#include <fstream>

void writeStateArrayToFile(YAML::StateArray sa, std::string fname) {
    YAML::Node node;
    node = sa;
    std::filesystem::path path = std::filesystem::current_path() / fname;
    std::ofstream file;
    file.open(path);
    file << node;
    file.close();
};

YAML::StateArray readStateArrayFromFile(std::string fname) {
    std::filesystem::path path = std::filesystem::current_path() / fname;
    if (!std::filesystem::exists(path)){
        std::cout << "No file found" << std::endl;
        return {};
    }
    YAML::Node node = YAML::LoadFile(path.string());
    return node.as<YAML::StateArray>();
};
