#ifndef MEASUREMENTOVERVIEWMODEL_H
#define MEASUREMENTOVERVIEWMODEL_H

#include "messagedispatcher.h"
#include "model/statisticsresult.h"

class MeasurementOverviewModel{
private:
    std::vector<int> activeChannelsIdxs;
    int voltageChannelsNum;
    int currentChannelsNum;
    StatisticsResult * statisticsResults;
    QVector<Measurement_t> liquidJunctionResults;

public:
    MeasurementOverviewModel(std::vector<int> activeChannelsIdxs, int voltageChannelsNum, int currentChannelsNum);
    StatisticsResult * getStatisticsResult();
    QVector<Measurement_t> getLiquidJunctionResults();
    void setStatisticsResult(StatisticsResult * );
    void setLiquidJunctionResults(QVector<Measurement_t>);
    void setActiveChannelsIdxs(std::vector<int>);
    void exportToCsv(std::string filepath);
};
#endif // MEASUREMENTOVERVIEWMODEL_H
