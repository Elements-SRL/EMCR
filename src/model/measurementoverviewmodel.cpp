#include "measurementoverviewmodel.h"

#include <QFile>
#include <QTextStream>

MeasurementOverviewModel::MeasurementOverviewModel(std::vector <uint16_t> activeChannelsIdxs, int voltageChannelsNum, int currentChannelsNum) {
    this->activeChannelsIdxs = activeChannelsIdxs;
    this->voltageChannelsNum = voltageChannelsNum;
    this->currentChannelsNum = currentChannelsNum;
    statisticsResults.resize(currentChannelsNum);
    resistanceEstimationResults.resize(currentChannelsNum);
    offsetRecalibrationResults.resize(currentChannelsNum);
    liquidJunctionResults.resize(currentChannelsNum);
}

std::vector <StatisticsResult_t> MeasurementOverviewModel::getStatisticsResults() {
    return statisticsResults;
}

std::vector <SingleMeasResult_t> MeasurementOverviewModel::getResistanceEstimationResults() {
    return resistanceEstimationResults;
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

void MeasurementOverviewModel::setResistanceEstimationResult(std::vector <SingleMeasResult_t> results) {
    resistanceEstimationResults = results;
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
        auto r = statisticsResults[0];
        auto res = resistanceEstimationResults[0];
        auto orr = offsetRecalibrationResults[0];
        auto lj = liquidJunctionResults[0];
        stream << "Channel idx,Mean Voltage [" << getValueAndUnit(r.meanVoltage).second << "],";
        stream << "Std Voltage [" << getValueAndUnit(r.stdVoltage).second << "],";
        stream << "Mean Current [" << getValueAndUnit(r.meanCurrent).second << "],";
        stream << "Std Current [" << getValueAndUnit(r.stdCurrent).second << "],";
        stream << "Resistance [" << getValueAndUnit(res.meas).second << "],";
        stream << "Offset Recalibration [" << getValueAndUnit(orr).second << "],";
        stream << "Liquid Junction [" << getValueAndUnit(lj).second << "]\n";
        for (int i = 0; i < statisticsResults.size(); i++) {
            r = statisticsResults[i];
            res = resistanceEstimationResults[i];
            orr = offsetRecalibrationResults[i];
            lj = liquidJunctionResults[i];
            std::vector <std::pair <QString, QString>> measurementsStrings = {
                getValueAndUnit(r.meanVoltage),
                getValueAndUnit(r.stdVoltage),
                getValueAndUnit(r.meanCurrent),
                getValueAndUnit(r.stdCurrent),
                getValueAndUnit(res.meas),
                getValueAndUnit(orr),
                getValueAndUnit(lj)};
            stream << r.chIdx+1;
            for (auto p : measurementsStrings) {
                stream << "," << p.first;
            }
            stream << "\n";
        }
        file.close();
    }
}

void MeasurementOverviewModel::setActiveChannelsIdxs(std::vector<uint16_t> updatedChIdxs){
    activeChannelsIdxs = updatedChIdxs;
}
