#define TITLE "Device controls"

#include "devicecontroldockwidget.h"
#include <QVBoxLayout>

DeviceControlDockWidget::DeviceControlDockWidget(ModelDevice * mDev): QDockWidget() {
    this->mDev = mDev;

    std::vector<int> clampingModalities;
    mDev->getClampingModalitiesFeatures(clampingModalities);

    std::vector <RangedMeasurement_t> vcCurrentRanges;
    uint16_t defaultVcCurrRangeIdx;
    mDev->getVcCurrentRangesFeatures(vcCurrentRanges,defaultVcCurrRangeIdx);

    std::vector <RangedMeasurement_t> vcVoltageRanges;
    mDev->getVcVoltageRangesFeatures(vcVoltageRanges);

    std::vector <RangedMeasurement_t> ccCurrentRanges;
    mDev->getCcCurrentRangesFeatures(ccCurrentRanges);

    std::vector <RangedMeasurement_t> ccVoltageRanges;
    mDev->getCcVoltageRangesFeatures(ccVoltageRanges);

    std::vector <Measurement_t> samplingRates;
    mDev->getSamplingRatesFeatures(samplingRates);

    QWidget *window = new QWidget;
    this->setWidget(window);
    QVBoxLayout * vLayout = new QVBoxLayout(window);
    vLayout->setContentsMargins(0, 0, 0, 1);
    vLayout->setSpacing(1);

    this->setWindowTitle(TITLE);

    /*! VC Current range */
    if (vcCurrentRanges.size() > 0) {
        this->vcCurrentRangesGroupBox = new QGroupBox(DCW_CURRENT_RANGE_TITLE);

        QVBoxLayout * radioButtonsBoxLayout = new QVBoxLayout();
        radioButtonsBoxLayout->setContentsMargins(2, 2, 2, 2);
        radioButtonsBoxLayout->setSpacing(2);

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
//            this->vcCurrentRangesRadioButtons[0]->setChecked(true);
            this->vcCurrentRangesRadioButtons[defaultVcCurrRangeIdx]->setChecked(true);
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
        radioButtonsBoxLayout->setContentsMargins(2, 2, 2, 2);
        radioButtonsBoxLayout->setSpacing(2);

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

    /*! CC Current range */
    if (ccCurrentRanges.size() > 0) {
        this->ccCurrentRangesGroupBox = new QGroupBox(DCW_CC_CURRENT_RANGE_TITLE);

        QVBoxLayout * radioButtonsBoxLayout = new QVBoxLayout();
        radioButtonsBoxLayout->setContentsMargins(2, 2, 2, 2);
        radioButtonsBoxLayout->setSpacing(2);

        vLayout->addWidget(this->ccCurrentRangesGroupBox);
        for (int idx = 0; idx < ccCurrentRanges.size(); idx++){
            auto rm = ccCurrentRanges[idx];
            QRadioButton * qrb = new QRadioButton(QString().fromStdString(rm.getMax().niceLabel()));
            radioButtonsBoxLayout->addWidget(qrb);
            this->ccCurrentRangesRadioButtons.push_back(qrb);
            connect(qrb, &QRadioButton::clicked, this, [=] (bool flag) {
                if (flag) {
                    emit sigCcCurrentRangeSelected(idx);
                }
            });
        }
        if (this->ccCurrentRangesRadioButtons.size() > 0) {
            this->ccCurrentRangesRadioButtons[0]->setChecked(true);
        }
        this->ccCurrentRangesGroupBox->setLayout(radioButtonsBoxLayout);
        if (ccCurrentRanges.size() == 1) {
            ccCurrentRangesGroupBox->setEnabled(false);
        }
    }


    /*! CC Voltage range */
    if (ccVoltageRanges.size() > 0) {
        this->ccVoltageRangesGroupBox = new QGroupBox(DCW_CC_VOLTAGE_RANGE_TITLE);

        QVBoxLayout * radioButtonsBoxLayout = new QVBoxLayout();
        radioButtonsBoxLayout->setContentsMargins(2, 2, 2, 2);
        radioButtonsBoxLayout->setSpacing(2);

        vLayout->addWidget(this->ccVoltageRangesGroupBox);
        for (int idx = 0; idx < ccVoltageRanges.size(); idx++){
            auto rm = ccVoltageRanges[idx];
            QRadioButton * qrb = new QRadioButton(QString().fromStdString(rm.getMax().niceLabel()));
            radioButtonsBoxLayout->addWidget(qrb);
            this->ccVoltageRangesRadioButtons.push_back(qrb);
            connect(qrb, &QRadioButton::clicked, this, [=] (bool flag) {
                if (flag) {
                    emit sigCcVoltageRangeSelected(idx);
                }
            });
        }
        if (this->ccVoltageRangesRadioButtons.size() > 0) {
            this->ccVoltageRangesRadioButtons[0]->setChecked(true);
        }
        this->ccVoltageRangesGroupBox->setLayout(radioButtonsBoxLayout);
        if (ccVoltageRanges.size() == 1) {
            ccVoltageRangesGroupBox->setEnabled(false);
        }
    }

    /*! Sampling rate */
    if (samplingRates.size() > 0) {
        this->samplingRatesGroupBox = new QGroupBox(DCW_SAMPLING_RATE_TITLE);

        QVBoxLayout * radioButtonsBoxLayout = new QVBoxLayout();
        radioButtonsBoxLayout->setContentsMargins(2, 2, 2, 2);
        radioButtonsBoxLayout->setSpacing(2);

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

    /*! Sampling rate */
    if (clampingModalities.size() > 0) {
        this->clampingModalitiesGroupBox = new QGroupBox(DCW_CLMAPINGMODALITY_TITLE);

        QVBoxLayout * radioButtonsBoxLayout = new QVBoxLayout();
        radioButtonsBoxLayout->setContentsMargins(2, 2, 2, 2);
        radioButtonsBoxLayout->setSpacing(2);

        vLayout->addWidget(this->clampingModalitiesGroupBox);
        for (int idx = 0; idx < clampingModalities.size(); idx++){
            QRadioButton * qrb;
            switch (clampingModalities[idx]) {
            case ClampingModality_t::VOLTAGE_CLAMP:
                qrb = new QRadioButton("Voltage clamp");
                break;

            case ClampingModality_t::CURRENT_CLAMP:
                qrb = new QRadioButton("Current clamp");
                break;
            }

            radioButtonsBoxLayout->addWidget(qrb);
            this->clampingModalitiesRadioButtons.push_back(qrb);
            connect(qrb, &QRadioButton::clicked, this, [=] (bool flag) {
                if (flag) {
                    emit sigClampingModalitySelected(idx);
                }
            });
        }
        if (this->clampingModalitiesRadioButtons.size() > 0) {
            this->clampingModalitiesRadioButtons[0]->setChecked(true);
        }
        this->clampingModalitiesGroupBox->setLayout(radioButtonsBoxLayout);
        if (clampingModalities.size() == 1) {
            clampingModalitiesGroupBox->setEnabled(false);
        }
    }

    /*! \todo MPAC da ricontrollare con calma, per il momento la si lascia commentata e si genera il widget in maniera esplicita*/
//    DeviceControlDockWidget::testFunction(vLayout, this->vcCurrentRangesGroupBox, vcCurrentRanges, this->vcCurrentRangesRadioButtons);

    QWidget * spacer = new QWidget;
    spacer->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::MinimumExpanding);
    vLayout->addWidget(spacer);
}

void DeviceControlDockWidget::forceEmit() {
    for (int idx = 0; idx < clampingModalitiesRadioButtons.size(); idx++) {
        QRadioButton* btn = clampingModalitiesRadioButtons[idx];
        if (btn->isChecked()) {
            emit sigClampingModalitySelected(idx);
        }
    }

    if (mDev->getOngoingClampingModality() == ClampingModality_t::VOLTAGE_CLAMP) {
        for (int idx = 0; idx < vcCurrentRangesRadioButtons.size(); idx++) {
            QRadioButton* btn = vcCurrentRangesRadioButtons[idx];
            if (btn->isChecked()) {
                emit sigVcCurrentRangeSelected(idx);
            }
        }

        for (int idx = 0; idx < vcVoltageRangesRadioButtons.size(); idx++) {
            QRadioButton* btn = vcVoltageRangesRadioButtons[idx];
            if (btn->isChecked()) {
                emit sigVcVoltageRangeSelected(idx);
            }
        }
    }

    if (mDev->getOngoingClampingModality() == ClampingModality_t::CURRENT_CLAMP) {
        for (int idx = 0; idx < ccCurrentRangesRadioButtons.size(); idx++) {
            QRadioButton* btn = ccCurrentRangesRadioButtons[idx];
            if (btn->isChecked()) {
                emit sigCcCurrentRangeSelected(idx);
            }
        }

        for (int idx = 0; idx < ccVoltageRangesRadioButtons.size(); idx++) {
            QRadioButton* btn = ccVoltageRangesRadioButtons[idx];
            if (btn->isChecked()) {
                emit sigCcVoltageRangeSelected(idx);
            }
        }
    }

    for (int idx = 0; idx < samplingRatesRadioButtons.size(); idx++) {
        QRadioButton* btn = samplingRatesRadioButtons[idx];
        if (btn->isChecked()) {
            emit sigSamplingRateSelected(idx);
        }
    }
}

void DeviceControlDockWidget::updateParameters() {
    if (mDev->getOngoingClampingModality() == ClampingModality_t::VOLTAGE_CLAMP) {
        if(vcCurrentRangesRadioButtons.size()>0){
            vcCurrentRangesRadioButtons[mDev->getVcCurrentRangeIdx()]->setChecked(true);
        }

        if(vcVoltageRangesRadioButtons.size()>0){
            vcVoltageRangesRadioButtons[mDev->getVcVoltageRangeIdx()]->setChecked(true);
        }
    }

    if (mDev->getOngoingClampingModality() == ClampingModality_t::CURRENT_CLAMP) {
        if(ccCurrentRangesRadioButtons.size()>0){
            ccCurrentRangesRadioButtons[mDev->getCcCurrentRangeIdx()]->setChecked(true);
        }

        if(ccVoltageRangesRadioButtons.size()>0){
            ccVoltageRangesRadioButtons[mDev->getCcVoltageRangeIdx()]->setChecked(true);
        }
    }

    if(samplingRatesRadioButtons.size()>0){
        samplingRatesRadioButtons[mDev->getSamplingRateIdx()]->setChecked(true);
    }

    if(clampingModalitiesRadioButtons.size()>0){
        clampingModalitiesRadioButtons[mDev->getOngoingClampingModalityIdx()]->setChecked(true);
    }

    /*! \todo FCON aggiungere controlli per DAC filters */
}

/*! \todo MPAC da ricontrollare con calma, per il momento la si lascia commentata e si genera il widget in maniera esplicita*/
//void DeviceControlDockWidget::testFunction(QVBoxLayout* vLayout, QGroupBox* qGroupBox, std::vector <RangedMeasurement_t> myRanges, std::vector<QRadioButton *> &qRadioButtons){
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


void DeviceControlDockWidget::onStartRecording(std::vector<uint16_t> channelIndexes, std::vector<bool> onValues){
    /*! \todo MPAC at the moment miccing ccCurrent and ccVoltage*/
    if(this->vcCurrentRangesGroupBox != nullptr){
        vcCurrentRangesPrevioueEnableStateBeforeRecording = this->vcCurrentRangesGroupBox->isEnabled();
        this->vcCurrentRangesGroupBox->setEnabled(false);
    }

    if(this->vcVoltageRangesGroupBox != nullptr){
        vcVoltageRangesPrevioueEnableStateBeforeRecording = this->vcVoltageRangesGroupBox->isEnabled();
        this->vcVoltageRangesGroupBox->setEnabled(false);
    }

    if(this->ccCurrentRangesGroupBox != nullptr){
        ccCurrentRangesPrevioueEnableStateBeforeRecording = this->ccCurrentRangesGroupBox->isEnabled();
        this->ccCurrentRangesGroupBox->setEnabled(false);
    }

    if(this->ccVoltageRangesGroupBox != nullptr){
        ccVoltageRangesPrevioueEnableStateBeforeRecording = this->ccVoltageRangesGroupBox->isEnabled();
        this->ccVoltageRangesGroupBox->setEnabled(false);
    }

    if(this->samplingRatesGroupBox != nullptr){
        samplingRatesPrevioueEnableStateBeforeRecording = this->samplingRatesGroupBox->isEnabled();
        this->samplingRatesGroupBox->setEnabled(false);
    }
}

void DeviceControlDockWidget::onStopRecording(){
    if(this->vcCurrentRangesGroupBox != nullptr){
        this->vcCurrentRangesGroupBox->setEnabled(vcCurrentRangesPrevioueEnableStateBeforeRecording);
    }

    if(this->vcVoltageRangesGroupBox != nullptr){
        this->vcVoltageRangesGroupBox->setEnabled(vcVoltageRangesPrevioueEnableStateBeforeRecording);
    }

    if(this->ccCurrentRangesGroupBox != nullptr){
        this->ccCurrentRangesGroupBox->setEnabled(ccCurrentRangesPrevioueEnableStateBeforeRecording);
    }

    if(this->ccVoltageRangesGroupBox != nullptr){
        this->ccVoltageRangesGroupBox->setEnabled(ccVoltageRangesPrevioueEnableStateBeforeRecording);
    }

    if(this->samplingRatesGroupBox != nullptr){
        this->samplingRatesGroupBox->setEnabled(samplingRatesPrevioueEnableStateBeforeRecording);
    }
}
