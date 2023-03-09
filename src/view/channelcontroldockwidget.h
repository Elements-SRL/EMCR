#ifndef CHANNELCONTROLDOCKWIDGET_H
#define CHANNELCONTROLDOCKWIDGET_H

#include <QDockWidget>
#include <QComboBox>
#include <QCheckBox>
#include <QBoxLayout>
#include <QLabel>

#include "modeldevice.h"
#include "myspinbox.h"

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
    QVector <QString> operationString;

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
    void onApplyButtonClicked();
};

class SpinBoxWithChannel : public QWidget {
    Q_OBJECT

public:
    SpinBoxWithChannel(int idx, MySpinBox * sbx);

    double value();

private:
    QLabel * channelLbl;
    MySpinBox * valueSbx;
};

#endif // CHANNELCONTROLDOCKWIDGET_H
