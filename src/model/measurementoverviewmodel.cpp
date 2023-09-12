#include "measurementoverviewmodel.h"

#include <QFile>
#include <QTextStream>

MeasurementOverviewModel::MeasurementOverviewModel(std::vector<int> activeChannelsIdxs, int voltageChannelsNum, int currentChannelsNum) {
    this->activeChannelsIdxs = activeChannelsIdxs;
    this->voltageChannelsNum = voltageChannelsNum;
    this->currentChannelsNum = currentChannelsNum;
    liquidJunctionResults.resize(currentChannelsNum);
    liquidJunctionResults.fill({0.0, UnitPfxNone, "V"});
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
    QFile file(QString::fromStdString(filepath));
    if (file.open(QIODevice::WriteOnly)) {
        QTextStream stream(&file);
        stream << "Channel idx,Mean Voltage,Mean Current,Std Current,Conductivity,Liquid Junction\n";
        Measurement_t meanVoltage = {0.0, UnitPfxNone, "V"};
        Measurement_t meanCurrent = {0.0, UnitPfxNone, "A"};
        Measurement_t stdCurrent = {0.0, UnitPfxNone, "A"};
        Measurement_t conductivity = {0.0, UnitPfxNone, "S"};
        for (int i = 0; i < activeChannelsIdxs.size(); i++) {
            stdCurrent.value = statisticsResults->stdCurrent[i];
            meanCurrent.value = statisticsResults->meanCurrent[i];
            meanVoltage.value = statisticsResults->meanVoltage[i];
            conductivity.value = statisticsResults->conductivity[i];

            stream << i+1 << "," << QString::fromStdString(meanVoltage.niceLabel()) << "," << QString::fromStdString(meanCurrent.niceLabel()) << "," << QString::fromStdString(stdCurrent.niceLabel()) << "," << QString::fromStdString(conductivity.niceLabel()) << "," << QString::fromStdString(liquidJunctionResults[i].niceLabel()) << "\n";
        }
        file.close();
    }
}

void MeasurementOverviewModel::setActiveChannelsIdxs(std::vector<int> updatedChIdxs){
    activeChannelsIdxs = updatedChIdxs;
}
