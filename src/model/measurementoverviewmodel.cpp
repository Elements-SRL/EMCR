#include "measurementoverviewmodel.h"

#include <QFile>
#include <QTextStream>

MeasurementOverviewModel::MeasurementOverviewModel(std::vector<uint16_t> activeChannelsIdxs, int voltageChannelsNum, int currentChannelsNum) {
    this->activeChannelsIdxs = activeChannelsIdxs;
    this->voltageChannelsNum = voltageChannelsNum;
    this->currentChannelsNum = currentChannelsNum;
    liquidJunctionResults.resize(currentChannelsNum);
    liquidJunctionResults.resize(currentChannelsNum);
}

QVector<StatisticsResult> MeasurementOverviewModel::getStatisticsResult(){
    return statisticsResults;
}
std::vector<Measurement_t> MeasurementOverviewModel::getLiquidJunctionResults(){
    return liquidJunctionResults;
}
void MeasurementOverviewModel::setStatisticsResult(QVector<StatisticsResult> results){
    statisticsResults = results;
}
void MeasurementOverviewModel::setLiquidJunctionResults(std::vector<Measurement_t> ljr){
    liquidJunctionResults = ljr;
}
void MeasurementOverviewModel::exportToCsv(std::string filepath){
    QFile file(QString::fromStdString(filepath));
    if (file.open(QIODevice::WriteOnly)) {
        QTextStream stream(&file);
        stream << "Channel idx,Mean Voltage, unit, Mean Current, unit,Std Current, unit,Conductivity, unit,Liquid Junction, unit\n";
        for (auto ch: activeChannelsIdxs) {
            stdCurrent.value = statisticsResults->stdCurrent[ch];
            meanCurrent.value = statisticsResults->meanCurrent[ch];
            meanVoltage.value = statisticsResults->meanVoltage[ch];
            conductivity.value = statisticsResults->conductivity[ch];

            stream << ch+1 << "," << QString::fromStdString(meanVoltage.niceLabel()) << "," << QString::fromStdString(meanCurrent.niceLabel()) << "," << QString::fromStdString(stdCurrent.niceLabel()) << "," << QString::fromStdString(conductivity.niceLabel()) << "," << QString::fromStdString(liquidJunctionResults[ch].niceLabel()) << "\n";
        }
        file.close();
    }
}

void MeasurementOverviewModel::setActiveChannelsIdxs(std::vector<uint16_t> updatedChIdxs){
    activeChannelsIdxs = updatedChIdxs;
}
