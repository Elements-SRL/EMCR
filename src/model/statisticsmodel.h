#ifndef STATISTICSMODEL_H
#define STATISTICSMODEL_H

#include <QVector>

typedef struct Result {
    QVector <double> meanVoltage;
    QVector <double> stdVoltage;
    QVector <double> meanCurrent;
    QVector <double> stdCurrent;
    QVector <double> conductivity;
} Result_t;


class StatisticsModel {
public:
    StatisticsModel();
};

#endif // STATISTICSMODEL_H
