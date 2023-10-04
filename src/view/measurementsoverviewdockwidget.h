#ifndef MEASUREMENTSOVERVIEWDOCKWIDGET_H
#define MEASUREMENTSOVERVIEWDOCKWIDGET_H

#include <QDockWidget>
#include <QComboBox>
#include <QCheckBox>
#include <QBoxLayout>
#include <QLabel>
#include <QPushButton>

#include "errormanager.h"
#include "model/statisticsresult.h"

class MeasurementsOverviewDockWidget : public QDockWidget {
    Q_OBJECT

private:
    QVBoxLayout * mainVl;
    QWidget * mainWg;
    QLabel * channelIndexesLabel;
    QPushButton * meanVoltageBtn;
    QPushButton * meanCurrentBtn;
    QPushButton * stdCurrentBtn;
    QPushButton * conductivityBtn;
    QPushButton * liquidJunctionBtn;

    QGridLayout * gl;
    void updateButton(QPushButton * bt);
    int getTotalChannelsChannels();
    std::vector<QPushButton*> buttons;
    template<typename T>
    void setAllWidgetsInvisible(const std::vector<T>& widgets);
    template<typename T>
    void setActiveChannelsVisible(const std::vector<T>& widgets, const std::vector<int> active_channels);
    template<typename T>
    void applyTextFromValuesAndPfx(const std::vector<T>& widgets, std::vector<double> values, std::string pfx);
    template<typename T>
    void applyTextFromMeasurements(const std::vector<T>& widgets, QVector<Measurement_t> meas);
    std::vector<QLabel *>activeChannelsLabels;
    std::vector<QLabel *>meanVoltageLabels;
    std::vector<QLabel *>meanCurrentLabels;
    std::vector<QLabel *>stdCurrentLabels;
    std::vector<QLabel *>conductivityLabels;
    std::vector<QLabel *>liquidJunctionLabels;
    std::vector<int> activeChannels;
    int voltageChannels;
    int currentChannels;

public:
    MeasurementsOverviewDockWidget(std::vector<int> activeChannels, int voltageChannels, int currentChannels, QWidget * parent = nullptr);
    void updateActiveChannels(std::vector<int> newActiveChannels);
    void setLiquidJunctionResult(QVector <Measurement_t> result);
    void onLiveStatisticsResult(StatisticsResult * result);

public slots:
    void onUpdate();

signals:
    void testSignal();
    void sigAppliedHoldValues(std::vector<uint16_t> channelIndexes, std::vector<Measurement_t> holdValues);
    void sigAppliedStimHalfValues(std::vector<uint16_t> channelIndexes, std::vector<Measurement_t> halfValues);
    void extract(QString);
};

#endif // MEASUREMENTSOVERVIEWDOCKWIDGET_H
