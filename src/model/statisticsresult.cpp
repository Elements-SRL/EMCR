#include "statisticsresult.h"

StatisticsResult::StatisticsResult(int voltageChannelsNum, int currentChannelsNum){
    meanVoltage.resize(voltageChannelsNum);
    stdVoltage.resize(voltageChannelsNum);
    meanCurrent.resize(currentChannelsNum);
    stdCurrent.resize(currentChannelsNum);
    conductivity.resize(currentChannelsNum);
}
