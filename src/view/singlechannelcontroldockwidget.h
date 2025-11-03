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
    void setOffsetRecalibrationValues(std::vector <Measurement_t> values);
    void setLiquidJunctionVoltages(std::vector <Measurement_t> voltages);
    void setOffsetTrackingValues(std::vector <Measurement_t> values);
    void onBoardMappingsLoaded();

public slots:
    void onUpdate();
    void onVcVoltageRangeSelected();
    void onVcCurrentRangeSelected();
    void onCcCurrentRangeSelected();
    void onCcVoltageRangeSelected();

protected:
    bool eventFilter(QObject * obj, QEvent * event);
    ApplicationStatus * getAppStatus();

private:
    typedef enum Operations {
        OperationHoldingStimulus,
        OperationOffsetRecalibration,
        OperationLiquidJunction,
        OperationStimulusHalf,
        OperationOffsetTracking,
        OperationInitialStimulusRamp, /*!< \note keep as last enums (the code contains some checks with inequality signs */
        OperationFinalStimulusRamp,
        OperationDurationRamp,
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

    bool widgetInitialized = false;

    QVector <QWidget *> operationButtonWidgets;
    std::vector <RangedMeasurement_t> holdingTunerRange;
    std::vector <RangedMeasurement_t> offsetRecalibrationRange;
    RangedMeasurement_t liquidJunctionRange;
    RangedMeasurement_t stimulusDurationRange;
    QVector <NoWheelSpinBox *> setAllChannelsSbxs;
    void buildOperation(QLayout * layout, Operations_t operationType, bool visibility = false);

private slots:
    void onOperationSelected(int operationIdx);
    void onApplyButtonClicked();
    void onApplyButtonClicked(int idx, bool applyAll);
    void onSetAllButtonClicked();

signals:
    void sigAppliedHoldValues(std::vector <uint16_t> channelIndexes, std::vector <Measurement_t> values);
    void sigAppliedOffsetRecalibration(std::vector <uint16_t> channelIndexes, std::vector <Measurement_t> values);
    void sigAppliedStimHalfValues(std::vector <uint16_t> channelIndexes, std::vector <Measurement_t> values);
    void sigLiquidJunctionValues(std::vector <uint16_t> channelIndexes, std::vector <Measurement_t> values);
    void sigAppliedOffsetTracking(std::vector <uint16_t> channelIndexes, std::vector <Measurement_t> values);
    void sigAppliedRamp(std::vector <uint16_t> channelIndexes, std::vector <Measurement_t> vInitial, std::vector <Measurement_t> vFinal, std::vector <Measurement_t> duration);
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
