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


class MeasurementsOverviewDockWidget : public QDockWidget {
    Q_OBJECT

private:
//    MessageDispatcher * msgDisp = nullptr;
    QVBoxLayout * mainVl;
    QWidget * mainWg;
    QPushButton * b1;
    QPushButton * b2;
    QPushButton * b3;
    QGridLayout * gl;
    void updateButton(QPushButton * bt);
    std::vector<int> getActiveChannels();
    int getTotalChannelsChannels();
    std::vector<QPushButton*> buttons;
    void setAllButtonsInvisible(std::vector<QPushButton *> bts);
    void setActiveChannelsVisible(std::vector<QPushButton *> bts, std::vector<int> active_channels);
    std::vector<QPushButton *>col1;
    std::vector<QPushButton *>col2;
    std::vector<QPushButton *>col3;

public:
    MeasurementsOverviewDockWidget(QWidget * parent = nullptr);

public slots:
    void onUpdate();
    void onNewMeasurement(std::vector<Measurement_t> measurements);

signals:
    void testSignal();
    void sigAppliedHoldValues(std::vector<uint16_t> channelIndexes, std::vector<Measurement_t> holdValues);
};


#endif // MEASUREMENTSOVERVIEWDOCKWIDGET_H
