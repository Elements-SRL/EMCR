#ifndef MEASUREMENTOVERVIEWMODEL_H
#define MEASUREMENTOVERVIEWMODEL_H

#include "messagedispatcher.h"
#include "statisticsresult.h"
#include "singlemeasresult.h"
#include <vector>
#include <utility>
#include <QString>

class MeasurementOverviewModel{
private:
    std::vector<uint16_t> activeChannelsIdxs;
    int voltageChannelsNum;
    int currentChannelsNum;
    std::vector <StatisticsResult_t> statisticsResults;
    std::vector <SingleMeasResult_t> resistanceEstimationResults;
    std::vector <Measurement_t> offsetRecalibrationResults;
    std::vector <Measurement_t> liquidJunctionResults;
    std::pair <QString, QString> getValueAndUnit(Measurement m) {
        return std::make_pair(QString::number(m.value), QString::fromStdString(m.getFullUnit()));
    }

public:
    MeasurementOverviewModel(std::vector<uint16_t> activeChannelsIdxs, int voltageChannelsNum, int currentChannelsNum);
    std::vector <StatisticsResult_t> getStatisticsResults();
    std::vector <SingleMeasResult_t> getResistanceEstimationResults();
    std::vector <Measurement_t> getOffsetRecalibrationResults();
    std::vector <Measurement_t> getLiquidJunctionResults();
    void setStatisticsResult(std::vector <StatisticsResult>);
    void setResistanceEstimationResult(std::vector <SingleMeasResult_t>);
    void setOffsetRecalibrationResults(std::vector <Measurement_t>);
    void setLiquidJunctionResults(std::vector <Measurement_t>);
    void setActiveChannelsIdxs(std::vector <uint16_t>);
    void exportToCsv(std::string filepath);
};
#endif // MEASUREMENTOVERVIEWMODEL_H
