#ifndef STATISTICSRESULT_H
#define STATISTICSRESULT_H

#include <QVector>

class StatisticsResult {
public:
    QVector <double> meanVoltage;
    QVector <double> stdVoltage;
    QVector <double> meanCurrent;
    QVector <double> stdCurrent;
    QVector <double> conductivity;

public:
    StatisticsResult(int voltageChannelsNum, int currentChannelsNum);
};

#endif // STATISTICSRESULT_H
