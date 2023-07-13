#ifndef MEASUREMENTSOVERVIEWDOCKWIDGET_H
#define MEASUREMENTSOVERVIEWDOCKWIDGET_H

#include <QDockWidget>
#include <QComboBox>
#include <QCheckBox>
#include <QBoxLayout>
#include <QLabel>
#include <QPushButton>

//#include "messagedispatcher.h"
#include "errormanager.h"
#include "model/statisticsresult.h"

class MeasurementsOverviewDockWidget : public QDockWidget {
    Q_OBJECT

private:
//    MessageDispatcher * msgDisp = nullptr;
    QVBoxLayout * mainVl;
    QWidget * mainWg;
    QLabel * channelIndexesLabel;
    QPushButton * meanVoltageBtn;
    QPushButton * meanCurrentBtn;
    QPushButton * stdCurrentBtn;
    QPushButton * conductivityBtn;

    QGridLayout * gl;
    void updateButton(QPushButton * bt);
    std::vector<int> getActiveChannels();
    int getTotalChannelsChannels();
    std::vector<QPushButton*> buttons;
    template<typename T>
    void setAllWidgetsInvisible(const std::vector<T>& widgets);
    template<typename T>
    void setActiveChannelsVisible(const std::vector<T>& widgets, const std::vector<int> active_channels);
    template<typename T>
    void applyTextFromValuesAndaPfx(const std::vector<T>& widgets, std::vector<double> values, std::string pfx);
    std::vector<QLabel *>meanVoltageLabels;
    std::vector<QLabel *>meanCurrentLabels;
    std::vector<QLabel *>stdCurrentLabels;
    std::vector<QLabel *>conductivityLabels;
    int numberOfChannels;

public:
    MeasurementsOverviewDockWidget(int numberOfChannels, QWidget * parent = nullptr);

public slots:
    void onUpdate();
    void onNewMeasurement(std::vector<Measurement_t> measurements);
    void onResult(StatisticsResult * result);

signals:
    void testSignal();
    void sigAppliedHoldValues(std::vector<uint16_t> channelIndexes, std::vector<Measurement_t> holdValues);
};


#endif // MEASUREMENTSOVERVIEWDOCKWIDGET_H
