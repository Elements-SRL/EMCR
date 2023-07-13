#include "statisticsresult.h"

StatisticsResult::StatisticsResult(int voltageChannelsNum, int currentChannelsNum){
    meanVoltage.resize(voltageChannelsNum);
    stdVoltage.resize(voltageChannelsNum);
    meanCurrent.resize(currentChannelsNum);
    stdCurrent.resize(currentChannelsNum);
    conductivity.resize(currentChannelsNum);
}


QVector <double> StatisticsResult::getMeanVoltage(){
    return meanVoltage;
}
QVector <double> StatisticsResult::getStdVoltage(){
    return stdVoltage;
}
QVector <double> StatisticsResult::getMeanCurrent(){
    return meanCurrent;
}
QVector <double> StatisticsResult::getStdCurrent(){
    return stdCurrent;
}
QVector <double> StatisticsResult::getConductivity(){
    return conductivity;
}
