#ifndef DEVICECONTROLDOCKWIDGET_H
#define DEVICECONTROLDOCKWIDGET_H

#include <QDockWidget>
#include <QGroupBox>
#include "model/modeldevice.h"
#include <QRadioButton>
#include <QVBoxLayout>

#define DCW_CURRENT_RANGE_TITLE "Current Ranges"
#define DCW_VOLTAGE_RANGE_TITLE "Voltage Ranges"
#define DCW_SAMPLING_RATE_TITLE "Sampling Rates"
#define DCW_STIMULUS_FILTER_TITLE "Readout Filters"

class DeviceControlDockWidget : public QDockWidget{
    Q_OBJECT

public:
    DeviceControlDockWidget(ModelDevice * modelDevice);

private:
    ModelDevice * modelDevice;
    QGroupBox * vcCurrentRangesGroupBox;
    vector<QRadioButton *> vcCurrentRangesRadioButtons;
    QGroupBox * vcVoltageRangesGroupBox;
    vector<QRadioButton *> vcVoltageRangesRadioButtons;
    QGroupBox * ccCurrentRangesGroupBox;
    vector<QRadioButton *> ccCurrentRangesRadioButtons;
    QGroupBox * ccVoltageRangesGroupBox;
    vector<QRadioButton *> ccVoltageRangesRadioButtons;
    QGroupBox * samplingRatesGroupBox;
    vector<QRadioButton *> samplingRatesRadioButtons;

    /*! \todo MPAC da ricontrollare con calma, per il momento la si lascia commentata e si genera il widget in maniera esplicita*/
//    void testFunction(QVBoxLayout* vLayout, QGroupBox* qGroupBox, vector <RangedMeasurement_t> myRanges, vector<QRadioButton *> &qRadioButtons);

signals:
    void sigVcCurrentRangeSelected(int idx);
    void sigVcVoltageRangeSelected(int idx);
    void sigCcCurrentRangeSelected(int idx);
    void sigCcVoltageRangeSelected(int idx);
    void sigSamplingRateSelected(int idx);
};

#endif // DEVICECONTROLDOCKWIDGET_H
