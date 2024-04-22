#ifndef MEASUREMENTSOVERVIEWDOCKWIDGET_H
#define MEASUREMENTSOVERVIEWDOCKWIDGET_H

#include <QDockWidget>
#include <QComboBox>
#include <QCheckBox>
#include <QBoxLayout>
#include <QLabel>
#include <QPushButton>
#include "copyabletable.h"

#include "errormanager.h"
#include "model/statisticsresult.h"

class MeasurementsOverviewDockWidget : public QDockWidget {
    Q_OBJECT

private:
    QVBoxLayout * mainVl;
    QWidget * mainWg;
    QTableWidget* dataTable;
    std::vector<QPushButton*> buttons;

    void setStatisticsResultsInRowaRow(int row, StatisticsResult r);
    std::vector<uint16_t> activeChannels;
    int voltageChannels;
    int currentChannels;

public:
    MeasurementsOverviewDockWidget(std::vector<uint16_t> activeChannels, int voltageChannels, int currentChannels, QWidget * parent = nullptr);
    void updateActiveChannels(std::vector<uint16_t> newActiveChannels);
    void setLiquidJunctionResult(std::vector <Measurement_t> result);
    void onLiveStatisticsResult(std::vector<StatisticsResult> result);

public slots:
    void onUpdate();

signals:
    void sigAppliedHoldValues(std::vector<uint16_t> channelIndexes, std::vector<Measurement_t> holdValues);
    void sigAppliedStimHalfValues(std::vector<uint16_t> channelIndexes, std::vector<Measurement_t> halfValues);
    void extract(QString);
};

#endif // MEASUREMENTSOVERVIEWDOCKWIDGET_H
