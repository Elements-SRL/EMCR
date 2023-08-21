#ifndef STATISTICSRESULT_H
#define STATISTICSRESULT_H

#include <QVector>

class StatisticsResult {
public:
    StatisticsResult(int voltageChannelsNum, int currentChannelsNum);

    QVector <double> meanVoltage;
    QVector <double> stdVoltage;
    QVector <double> meanCurrent;
    QVector <double> stdCurrent;
    QVector <double> conductivity;
};


#endif // STATISTICSRESULT_H
