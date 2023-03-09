#ifndef CHANNELCONTROLDOCKWIDGET_H
#define CHANNELCONTROLDOCKWIDGET_H

#include <QDockWidget>
#include <QComboBox>
#include <QCheckBox>
#include <QDoubleSpinBox>
#include <QBoxLayout>
#include <QLabel>

#include "modeldevice.h"

class SpinBoxWithChannel;

class ChannelControlDockWidget : public QDockWidget {
    Q_OBJECT

public:
    ChannelControlDockWidget(ModelDevice * mDev, QWidget * parent = nullptr);

public slots:
    void onUpdate();

private:
    typedef enum Operations {
        OperationTurnChannelsOnOff,
        OperationTurnStimulusOnOff,
        OperationStartStopDigitalOffsetCompensation,
        OperationHoldingStimulus,
        OperationsNum
    } Operations_t;

    QVector <QString> operationTitles;

    QWidget * createOperationWidget(int idx);
    QVBoxLayout * getLayoutWithScrollBar(QWidget * widget);

    ModelDevice * mDev = nullptr;

    int voltageChannelsNum;
    int currentChannelsNum;
    QComboBox * operationCbx = nullptr;

    QVector <QWidget *> operationWidgets;

    QVector <QVector <QWidget *>> operationEdits;

private slots:
    void onOperationSelected(int operationIdx);
};

class SpinBoxWithChannel : public QWidget {
    Q_OBJECT

public:
    SpinBoxWithChannel(int idx, QDoubleSpinBox * sbx);

    double value();

private:
    QLabel * channelLbl;
    QDoubleSpinBox * valueSbx;
};

#endif // CHANNELCONTROLDOCKWIDGET_H
