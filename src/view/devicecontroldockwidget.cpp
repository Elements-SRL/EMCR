#define TITLE "Device controls"

#include "devicecontroldockwidget.h"
#include <QVBoxLayout>

DeviceControlDockWidget::DeviceControlDockWidget(ModelDevice *modelDevice): QDockWidget() {
    this->modelDevice = modelDevice;

    vector<int> clampingModalities;
    modelDevice->getClampingModalitiesFeatures(clampingModalities);

    vector <RangedMeasurement_t> vcCurrentRanges;
    modelDevice->getVcCurrentRangesFeatures(vcCurrentRanges);

    vector <RangedMeasurement_t> vcVoltageRanges;
    modelDevice->getVcVoltageRangesFeatures(vcVoltageRanges);

    vector <RangedMeasurement_t> ccCurrentRanges;
    modelDevice->getCcCurrentRangesFeatures(ccCurrentRanges);

    vector <RangedMeasurement_t> ccVoltageRanges;
    modelDevice->getCcVoltageRangesFeatures(ccCurrentRanges);

    vector <Measurement_t> samplingRates;
    modelDevice->getSamplingRatesFeatures(samplingRates);

    QWidget *window = new QWidget;
    this->setWidget(window);
    QVBoxLayout * vLayout = new QVBoxLayout(window);

    this->setWindowTitle(TITLE);

    /*! \todo add button for Current clamp and voltage clamp and set the ongoingClampingMOdality in modelDevice and in cascade in the real device through the messageDispatcher*/


    /*! VC Current range */
    if (vcCurrentRanges.size() > 0) {
        this->vcCurrentRangesGroupBox = new QGroupBox(DCW_CURRENT_RANGE_TITLE);

        QVBoxLayout * radioButtonsBoxLayout = new QVBoxLayout();

        vLayout->addWidget(this->vcCurrentRangesGroupBox);
        for (int idx = 0; idx < vcCurrentRanges.size(); idx++){
            auto rm = vcCurrentRanges[idx];
            QRadioButton * qrb = new QRadioButton(QString().fromStdString(rm.getMax().niceLabel()));
            radioButtonsBoxLayout->addWidget(qrb);
            this->vcCurrentRangesRadioButtons.push_back(qrb);
            connect(qrb, &QRadioButton::clicked, this, [=] (bool flag) {
                if (flag) {
                    emit sigVcCurrentRangeSelected(idx);
                }
            });
        }
        if (this->vcCurrentRangesRadioButtons.size() > 0) {
            this->vcCurrentRangesRadioButtons[0]->setChecked(true);
        }
        this->vcCurrentRangesGroupBox->setLayout(radioButtonsBoxLayout);
        if (vcCurrentRanges.size() == 1) {
            vcCurrentRangesGroupBox->setEnabled(false);
        }
    }

    /*! VC Voltage range */
    if (vcVoltageRanges.size() > 0) {
        this->vcVoltageRangesGroupBox = new QGroupBox(DCW_VOLTAGE_RANGE_TITLE);

        QVBoxLayout * radioButtonsBoxLayout = new QVBoxLayout();

        vLayout->addWidget(this->vcVoltageRangesGroupBox);
        for (int idx = 0; idx < vcVoltageRanges.size(); idx++){
            auto rm = vcVoltageRanges[idx];
            QRadioButton * qrb = new QRadioButton(QString().fromStdString(rm.getMax().niceLabel()));
            radioButtonsBoxLayout->addWidget(qrb);
            this->vcVoltageRangesRadioButtons.push_back(qrb);
            connect(qrb, &QRadioButton::clicked, this, [=] (bool flag) {
                if (flag) {
                    emit sigVcVoltageRangeSelected(idx);
                }
            });
        }
        if (this->vcVoltageRangesRadioButtons.size() > 0) {
            this->vcVoltageRangesRadioButtons[0]->setChecked(true);
        }
        this->vcVoltageRangesGroupBox->setLayout(radioButtonsBoxLayout);
        if (vcVoltageRanges.size() == 1) {
            vcVoltageRangesGroupBox->setEnabled(false);
        }
    }

    /*! Sampling rate */
    if (samplingRates.size() > 0) {
        this->samplingRatesGroupBox = new QGroupBox(DCW_SAMPLING_RATE_TITLE);

        QVBoxLayout * radioButtonsBoxLayout = new QVBoxLayout();

        vLayout->addWidget(this->samplingRatesGroupBox);
        for (int idx = 0; idx < samplingRates.size(); idx++){
            auto m = samplingRates[idx];
            QRadioButton * qrb = new QRadioButton(QString().fromStdString(m.niceLabel()));
            radioButtonsBoxLayout->addWidget(qrb);
            this->samplingRatesRadioButtons.push_back(qrb);
            connect(qrb, &QRadioButton::clicked, this, [=] (bool flag) {
                if (flag) {
                    emit sigSamplingRateSelected(idx);
                }
            });
        }
        if (this->samplingRatesRadioButtons.size() > 0) {
            this->samplingRatesRadioButtons[0]->setChecked(true);
        }
        this->samplingRatesGroupBox->setLayout(radioButtonsBoxLayout);
        if (samplingRates.size() == 1) {
            samplingRatesGroupBox->setEnabled(false);
        }
    }

    /*! \todo MPAC da ricontrollare con calma, per il momento la si lascia commentata e si genera il widget in maniera esplicita*/
//    DeviceControlDockWidget::testFunction(vLayout, this->vcCurrentRangesGroupBox, vcCurrentRanges, this->vcCurrentRangesRadioButtons);

    QWidget * spacer = new QWidget;
    spacer->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::MinimumExpanding);
    vLayout->addWidget(spacer);
}

/*! \todo MPAC da ricontrollare con calma, per il momento la si lascia commentata e si genera il widget in maniera esplicita*/
//void DeviceControlDockWidget::testFunction(QVBoxLayout* vLayout, QGroupBox* qGroupBox, vector <RangedMeasurement_t> myRanges, vector<QRadioButton *> &qRadioButtons){
//    QVBoxLayout * radioButtonsBoxLayout = new QVBoxLayout();

//    vLayout->addWidget(qGroupBox);
//    for (int idx = 0; idx < myRanges.size(); idx++){
//        auto rm = myRanges[idx];
//        QRadioButton * qrb = new QRadioButton(QString().fromStdString(rm.getMax().niceLabel()));
//        radioButtonsBoxLayout->addWidget(qrb);
//        qRadioButtons.push_back(qrb);
//        connect(qrb, &QRadioButton::clicked, this, [=] (bool flag) {
//            if (flag) {
//                emit sigVcCurrentRangeSelected(idx);
//            }
//        });
//    }
//    if (qRadioButtons.size() > 0) {
//        qRadioButtons[0]->setChecked(true);
//    }
//    qGroupBox->setLayout(radioButtonsBoxLayout);
//}
