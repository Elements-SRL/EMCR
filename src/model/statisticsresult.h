#ifndef STATISTICSRESULT_H
#define STATISTICSRESULT_H

#include <QVector>

class StatisticsResult {
private:
    QVector <double> meanVoltage;
    QVector <double> stdVoltage;
    QVector <double> meanCurrent;
    QVector <double> stdCurrent;
    QVector <double> conductivity;

public:

    StatisticsResult(int voltageChannelsNum, int currentChannelsNum);

    QVector <double> getMeanVoltage();
    QVector <double> getStdVoltage();
    QVector <double> getMeanCurrent();
    QVector <double> getStdCurrent();
    QVector <double> getConductivity();
};

#endif // STATISTICSRESULT_H
