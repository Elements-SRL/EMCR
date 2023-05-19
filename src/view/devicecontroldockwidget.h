#ifndef DEVICECONTROLDOCKWIDGET_H
#define DEVICECONTROLDOCKWIDGET_H

#include <QDockWidget>
#include <QGroupBox>
#include <QRadioButton>
#include <QVBoxLayout>

#include "modeldevice.h"

#define DCW_CURRENT_RANGE_TITLE "Current Ranges"
#define DCW_VOLTAGE_RANGE_TITLE "Voltage Ranges"
#define DCW_SAMPLING_RATE_TITLE "Sampling Rates"
#define DCW_STIMULUS_FILTER_TITLE "Readout Filters"

class DeviceControlDockWidget : public QDockWidget{
    Q_OBJECT

public:
    DeviceControlDockWidget(ModelDevice * modelDevice);

    void forceEmit();

private:
    ModelDevice * modelDevice;
    QGroupBox * vcCurrentRangesGroupBox = nullptr;
    std::vector<QRadioButton *> vcCurrentRangesRadioButtons;
    bool vcCurrentRangesPrevioueEnableStateBeforeRecording = false;
    QGroupBox * vcVoltageRangesGroupBox = nullptr;
    std::vector<QRadioButton *> vcVoltageRangesRadioButtons;
    bool vcVoltageRangesPrevioueEnableStateBeforeRecording = false;
    QGroupBox * ccCurrentRangesGroupBox = nullptr;
    std::vector<QRadioButton *> ccCurrentRangesRadioButtons;
    bool ccCurrentRangesPrevioueEnableStateBeforeRecording = false;
    QGroupBox * ccVoltageRangesGroupBox = nullptr;
    std::vector<QRadioButton *> ccVoltageRangesRadioButtons;
    bool ccVoltageRangesPrevioueEnableStateBeforeRecording = false;
    QGroupBox * samplingRatesGroupBox = nullptr;
    std::vector<QRadioButton *> samplingRatesRadioButtons;
    bool samplingRatesPrevioueEnableStateBeforeRecording = false;

    /*! \todo MPAC da ricontrollare con calma, per il momento la si lascia commentata e si genera il widget in maniera esplicita*/
//    void testFunction(QVBoxLayout* vLayout, QGroupBox* qGroupBox, std::vector <RangedMeasurement_t> myRanges, std::vector<QRadioButton *> &qRadioButtons);

    signals:
    void sigVcCurrentRangeSelected(int idx);
    void sigVcVoltageRangeSelected(int idx);
    void sigCcCurrentRangeSelected(int idx);
    void sigCcVoltageRangeSelected(int idx);
    void sigSamplingRateSelected(int idx);

public slots:
    void onStartRecording(std::vector<uint16_t> channelIndexes, std::vector<bool> onValues);
    void onStopRecording();
};

#endif // DEVICECONTROLDOCKWIDGET_H
