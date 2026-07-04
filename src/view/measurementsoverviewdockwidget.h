#ifndef MEASUREMENTSOVERVIEWDOCKWIDGET_H
#define MEASUREMENTSOVERVIEWDOCKWIDGET_H

#include <QDockWidget>
#include <QComboBox>
#include <QCheckBox>
#include <QBoxLayout>
#include <QLabel>
#include <QPushButton>
#include "copyabletable.h"

#include "resultwrapper.h"

class MeasurementsOverviewDockWidget : public QDockWidget {
    Q_OBJECT

private:
    typedef enum Columns {
        ColMeanVoltage,
        ColVoltageRms,
        ColMeanCurrent,
        ColCurrentRms,
        ColResistance,
        ColPipetteCapacitance,
        ColMembraneCapacitance,
        ColAccessResistance,
        ColMembraneResistance,
        ColOffsetRecalibration,
        ColLiquidJunction,
        ColumnsNum
    } Columns_t;

    QVBoxLayout * mainVl = nullptr;
    QWidget * mainWg = nullptr;
    CopyableTable* dataTable = nullptr;
    std::vector<QPushButton*> buttons;
    QLabel * emptyStateLabel = nullptr;

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
    void onLiveStatisticsResult(StatisticsResultWrapper_t result);
    void onResistanceEstimationResult(SingleMeasResultWrapper_t result);
    void onPipetteCapacitanceEstimationResult(SingleMeasResultWrapper_t result);
    void onMembraneEstimationResult(MembraneResultWrapper_t result);

public slots:
    void onUpdate();

signals:
    void sigAppliedHoldValues(std::vector<uint16_t> channelIndexes, std::vector<e384cl::Measurement_t> holdValues);
    void sigAppliedStimHalfValues(std::vector<uint16_t> channelIndexes, std::vector<e384cl::Measurement_t> halfValues);
    void extract(QString);
};

#endif // MEASUREMENTSOVERVIEWDOCKWIDGET_H
