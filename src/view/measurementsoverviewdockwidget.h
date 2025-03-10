#ifndef MEASUREMENTSOVERVIEWDOCKWIDGET_H
#define MEASUREMENTSOVERVIEWDOCKWIDGET_H

#include <QDockWidget>
#include <QComboBox>
#include <QCheckBox>
#include <QBoxLayout>
#include <QLabel>
#include <QPushButton>
#include "copyabletable.h"

#include "statisticsresult.h"
#include "singlemeasresult.h"

class MeasurementsOverviewDockWidget : public QDockWidget {
    Q_OBJECT

private:
    typedef enum Columns {
        ColChannelIndex,
        ColMeanVoltage,
        ColVoltageRms,
        ColMeanCurrent,
        ColCurrentRms,
        ColResistance,
        ColPipetteCapacitance,
        ColOffsetRecalibration,
        ColLiquidJunction,
        ColumnsNum
    } Columns_t;

    QVBoxLayout * mainVl = nullptr;
    QWidget * mainWg = nullptr;
    CopyableTable* dataTable = nullptr;
    std::vector<QPushButton*> buttons;

    void setStatisticsResultsInRowaRow(int row, StatisticsResult &r);
    void setCellText(int row, int col, const QString text);

    std::vector<uint16_t> activeChannels;
    int voltageChannels;
    int currentChannels;

public:
    MeasurementsOverviewDockWidget(std::vector<uint16_t> activeChannels, int voltageChannels, int currentChannels, QWidget * parent = nullptr);
    void updateActiveChannels(std::vector<uint16_t> newActiveChannels);
    void setOffsetRecalibrationResult(std::vector <e384cl::Measurement_t> result);
    void setLiquidJunctionResult(std::vector <e384cl::Measurement_t> result);
    void onLiveStatisticsResult(std::vector<StatisticsResult_t> result);
    void onResistanceEstimationResult(std::vector <SingleMeasResult_t> result);
    void onPipetteCapacitanceEstimationResult(std::vector <SingleMeasResult_t> result);

public slots:
    void onUpdate();

signals:
    void sigAppliedHoldValues(std::vector<uint16_t> channelIndexes, std::vector<e384cl::Measurement_t> holdValues);
    void sigAppliedStimHalfValues(std::vector<uint16_t> channelIndexes, std::vector<e384cl::Measurement_t> halfValues);
    void extract(QString);
};

#endif // MEASUREMENTSOVERVIEWDOCKWIDGET_H
