#include "measurementoverviewmodel.h"

#include <QFile>
#include <QTextStream>

MeasurementOverviewModel::MeasurementOverviewModel(std::vector <uint16_t> activeChannelsIdxs, int voltageChannelsNum, int currentChannelsNum) {
    this->activeChannelsIdxs = activeChannelsIdxs;
    this->voltageChannelsNum = voltageChannelsNum;
    this->currentChannelsNum = currentChannelsNum;
    statisticsResults.resize(currentChannelsNum);
    offsetRecalibrationResults.resize(currentChannelsNum);
    liquidJunctionResults.resize(currentChannelsNum);
}

std::vector <StatisticsResult> MeasurementOverviewModel::getStatisticsResult() {
    return statisticsResults;
}

std::vector <Measurement_t> MeasurementOverviewModel::getOffsetRecalibrationResults() {
    return offsetRecalibrationResults;
}

std::vector <Measurement_t> MeasurementOverviewModel::getLiquidJunctionResults() {
    return liquidJunctionResults;
}

void MeasurementOverviewModel::setStatisticsResult(std::vector <StatisticsResult> results) {
    statisticsResults = results;
}

void MeasurementOverviewModel::setOffsetRecalibrationResults(std::vector <Measurement_t> orr) {
    offsetRecalibrationResults = orr;
}

void MeasurementOverviewModel::setLiquidJunctionResults(std::vector <Measurement_t> ljr) {
    liquidJunctionResults = ljr;
}

void MeasurementOverviewModel::exportToCsv(std::string filepath){
    QFile file(QString::fromStdString(filepath));
    if (file.open(QIODevice::WriteOnly)) {
        QTextStream stream(&file);
        stream << "Channel idx,Mean Voltage,unit,Std Voltage,unit,Mean Current, unit,Std Current,unit,Conductivity,unit,Offset Recalibration,unit,Liquid Junction,unit\n";
        for (int i = 0; i < statisticsResults.size(); i++) {
            auto r = statisticsResults[i];
            auto orr = offsetRecalibrationResults[i];
            auto lj = liquidJunctionResults[i];
            std::vector <std::pair <QString, QString>> measurementsStrings = {
                getValueAndUnit(r.meanVoltage),
                getValueAndUnit(r.stdVoltage),
                getValueAndUnit(r.meanCurrent),
                getValueAndUnit(r.stdCurrent),
                getValueAndUnit(r.conductivity),
                getValueAndUnit(orr),
                getValueAndUnit(lj)};
            stream << r.chIdx+1;
            for (auto p : measurementsStrings) {
                stream << "," << p.first << "," << p.second;
            }
            stream << "\n";
        }
        file.close();
    }
}

void MeasurementOverviewModel::setActiveChannelsIdxs(std::vector<uint16_t> updatedChIdxs){
    activeChannelsIdxs = updatedChIdxs;
}
