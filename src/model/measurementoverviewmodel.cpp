#include "measurementoverviewmodel.h"

MeasurementOverviewModel::MeasurementOverviewModel(std::vector<int> activeChannelsIdxs, int voltageChannelsNum, int currentChannelsNum)
{
    this->activeChannelsIdxs = activeChannelsIdxs;
    this->voltageChannelsNum = voltageChannelsNum;
    this->currentChannelsNum = currentChannelsNum;
}

StatisticsResult * MeasurementOverviewModel::getStatisticsResult(){
    return statisticsResults;
}
QVector<Measurement_t> MeasurementOverviewModel::getLiquidJunctionResults(){
    return liquidJunctionResults;
}
void MeasurementOverviewModel::setStatisticsResult(StatisticsResult * sr){
    statisticsResults = sr;
}
void MeasurementOverviewModel::setLiquidJunctionResults(QVector<Measurement_t> ljr){
    liquidJunctionResults = ljr;
}
void MeasurementOverviewModel::exportToCsv(std::string filepath){
    FILE *fpt;
    fpt = fopen(filepath.c_str(), "w+");
    fprintf(fpt,"Channel idx, Std Current, Mean Current, Mean Voltage, Conductivity, Liquid Junction\n");
    for (int i=0; i<activeChannelsIdxs.size(); i++) {
        fprintf(fpt,"%d %f, %f, %f, %f, %f\n", i+1, statisticsResults->stdCurrent[i], statisticsResults->meanCurrent[i], statisticsResults->meanVoltage[i], statisticsResults->conductivity[i], liquidJunctionResults[i].value);
    }
    fclose(fpt);
}

void MeasurementOverviewModel::setActiveChannelsIdxs(std::vector<int> updatedChIdxs){
    activeChannelsIdxs = updatedChIdxs;
}
