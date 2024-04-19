#ifndef MEASUREMENTOVERVIEWMODEL_H
#define MEASUREMENTOVERVIEWMODEL_H

#include "messagedispatcher.h"
#include "model/statisticsresult.h"
#include <vector>
#include <QVector>

class MeasurementOverviewModel{
private:
    std::vector<uint16_t> activeChannelsIdxs;
    int voltageChannelsNum;
    int currentChannelsNum;
    QVector<StatisticsResult> statisticsResults;
    std::vector<Measurement_t> liquidJunctionResults;

public:
    MeasurementOverviewModel(std::vector<uint16_t> activeChannelsIdxs, int voltageChannelsNum, int currentChannelsNum);
    QVector<StatisticsResult> getStatisticsResult();
    std::vector<Measurement_t> getLiquidJunctionResults();
    void setStatisticsResult(QVector<StatisticsResult>);
    void setLiquidJunctionResults(std::vector<Measurement_t>);
    void setActiveChannelsIdxs(std::vector<uint16_t>);
    void exportToCsv(std::string filepath);
};
#endif // MEASUREMENTOVERVIEWMODEL_H
