#ifndef SINGLECHANNELCONTROLDOCKWIDGET_H
#define SINGLECHANNELCONTROLDOCKWIDGET_H

#include <QDockWidget>
#include <QComboBox>
#include <QBoxLayout>
#include <QLabel>
#include <QPushButton>

#include "application_status.h"
#include "nowheelspinbox.h"
#include "errormanager.h"

class SpinBoxWithChannel;

class SingleChannelControlDockWidget : public QDockWidget {
    Q_OBJECT

public:
    SingleChannelControlDockWidget(ApplicationStatus * appStatus, QWidget * parent = nullptr);
    void setLiquidJunctionVoltages(std::vector <Measurement_t> voltages);
    void onBoardMappingsLoaded();

public slots:
    void onUpdate();
    void onVcVoltageRangeSelected(int idx);
    void onCcCurrentRangeSelected(int idx);

protected:
    bool eventFilter(QObject * obj, QEvent * event);

private:
    typedef enum Operations {
        OperationHoldingStimulus,
        OperationLiquidJunction,
        OperationStimulusHalf,
        OperationsNum
    } Operations_t;

    ApplicationStatus * appStatus = nullptr;

    QVector <QString> operationTitles;

    QWidget * createOperationWidget(int idx);
    QWidget * createOperationButtonWidget(int idx);
    QVBoxLayout * getLayoutWithScrollBar(QWidget * widget);

    int voltageChannelsNum;
    int currentChannelsNum;
    QComboBox * operationCbx = nullptr;
    QVector <SpinBoxWithChannel *> setAllWidgets;

    QPushButton* startRecordingBtn = nullptr;
    QPushButton* stopRecordingBtn = nullptr;
    QPushButton* applyBtn = nullptr;

    QVector <QWidget *> operationWidgets;

    QVector <QVector <QWidget *>> operationEdits;

    bool anyOperationActive = false;

    QVector <QWidget *> operationButtonWidgets;
    RangedMeasurement_t holdingTunerRange;
    RangedMeasurement_t liquidJunctionRange;
    QVector <NoWheelSpinBox *> setAllChannelsSbxs;
    void buildOperation(QLayout * layout, int operationType, bool visibility = false);

private slots:
    void onOperationSelected(int operationIdx);
    void onApplyButtonClicked();
    void onApplyButtonClicked(int idx, bool applyAll);
    void onAllButtonClicked(bool);
    void onSetAllButtonClicked();

signals:
    void sigAppliedHoldValues(std::vector <uint16_t> channelIndexes, std::vector <Measurement_t> values);
    void sigAppliedStimHalfValues(std::vector <uint16_t> channelIndexes, std::vector <Measurement_t> values);
    void sigLiquidJunctionValues(std::vector <uint16_t> channelIndexes, std::vector <Measurement_t> values);
};

class SpinBoxWithChannel : public QWidget {
    Q_OBJECT

public:
    SpinBoxWithChannel(std::string title, NoWheelSpinBox * sbx);
    void setName(std::string title);
    NoWheelSpinBox * getSpinBox();

private:
    QLabel * channelLbl;
    NoWheelSpinBox * valueSbx;
};

#endif // SINGLECHANNELCONTROLDOCKWIDGET_H
