#define TITLE "Device controls"

#define CURRENT_RANGE "Current Ranges"
#define STIMULUS_FILTER "Readout Filters"

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


    this->vcCurrentRangesGroupBox = new QGroupBox(CURRENT_RANGE);

//    QVBoxLayout * radioButtonsBoxLayout = new QVBoxLayout();

//    vLayout->addWidget(this->vcCurrentRangesGroupBox);
//    for (int idx = 0; idx < vcCurrentRanges.size(); idx++){
//        auto rm = vcCurrentRanges[idx];
//        QRadioButton * qrb = new QRadioButton(QString().fromStdString(rm.getMax().niceLabel()));
//        radioButtonsBoxLayout->addWidget(qrb);
//        this->vcCurrentRangesRadioButtons.push_back(qrb);
//        connect(qrb, &QRadioButton::clicked, this, [=] (bool flag) {
//            if (flag) {
//                emit sigVcCurrentRangeSelected(idx);
//            }
//        });
//    }
//    if (this->vcCurrentRangesRadioButtons.size() > 0) {
//        this->vcCurrentRangesRadioButtons[0]->setChecked(true);
//    }
//    this->vcCurrentRangesGroupBox->setLayout(radioButtonsBoxLayout);

    DeviceControlDockWidget::testFunction(vLayout, this->vcCurrentRangesGroupBox, vcCurrentRanges, this->vcCurrentRangesRadioButtons);

    QWidget * spacer = new QWidget;
    spacer->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::MinimumExpanding);
    vLayout->addWidget(spacer);
}

void DeviceControlDockWidget::testFunction(QVBoxLayout* vLayout, QGroupBox* qGroupBox, vector <RangedMeasurement_t> myRanges, vector<QRadioButton *> &qRadioButtons){
    QVBoxLayout * radioButtonsBoxLayout = new QVBoxLayout();

    vLayout->addWidget(qGroupBox);
    for (int idx = 0; idx < myRanges.size(); idx++){
        auto rm = myRanges[idx];
        QRadioButton * qrb = new QRadioButton(QString().fromStdString(rm.getMax().niceLabel()));
        radioButtonsBoxLayout->addWidget(qrb);
        qRadioButtons.push_back(qrb);
        connect(qrb, &QRadioButton::clicked, this, [=] (bool flag) {
            if (flag) {
                emit sigVcCurrentRangeSelected(idx);
            }
        });
    }
    if (qRadioButtons.size() > 0) {
        qRadioButtons[0]->setChecked(true);
    }
    qGroupBox->setLayout(radioButtonsBoxLayout);
}
