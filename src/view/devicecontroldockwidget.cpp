#define TITLE "Device controls"

#include <QVBoxLayout>

#include "devicecontroldockwidget.h"

DeviceControlDockWidget::DeviceControlDockWidget(MessageDispatcher * msgDisp) :
    QDockWidget(),
    msgDisp(msgDisp) {

    setObjectName("deviceControlsDw");
    std::vector<ClampingModality_t> clampingModalities;
    msgDisp->getClampingModalitiesFeatures(clampingModalities);

    std::vector <RangedMeasurement_t> vcCurrentRanges;
    uint16_t defaultVcCurrRangeIdx;
    msgDisp->getVCCurrentRanges(vcCurrentRanges,defaultVcCurrRangeIdx);

    std::vector <RangedMeasurement_t> vcVoltageRanges;
    msgDisp->getVCVoltageRanges(vcVoltageRanges);

    std::vector <RangedMeasurement_t> ccCurrentRanges;
    msgDisp->getCCCurrentRanges(ccCurrentRanges);

    std::vector <RangedMeasurement_t> ccVoltageRanges;
    msgDisp->getCCVoltageRanges(ccVoltageRanges);

    std::vector <Measurement_t> vcVoltageFilters;
    msgDisp->getVCVoltageFilters(vcVoltageFilters);

    std::vector <Measurement_t> ccCurrentFilters;
    msgDisp->getCCCurrentFilters(ccCurrentFilters);

    std::vector <Measurement_t> samplingRates;
    msgDisp->getSamplingRatesFeatures(samplingRates);

    unsigned int maxDownsamplingRatio;
    msgDisp->getMaxDownsamplingRatioFeature(maxDownsamplingRatio);

    QWidget *window = new QWidget;
    this->setWidget(window);
    QVBoxLayout * vLayout = new QVBoxLayout(window);
    vLayout->setContentsMargins(0, 0, 0, 1);
    vLayout->setSpacing(1);

    this->setWindowTitle(TITLE);

    /*! VC Current range */
    this->vcCurrentRangesGroupBox = setupGroupBox(DCW_CURRENT_RANGE_TITLE, vcCurrentRanges, vLayout, vcCurrentRangesRadioButtons);
    for (int i = 0; i< vcCurrentRangesRadioButtons.size(); i++){
        connect(vcCurrentRangesRadioButtons[i], &QRadioButton::clicked, this, [=] (bool flag) {
            if (flag) {
                emit sigVcCurrentRangeSelected(i);
            }
        });
    }

    /*! VC Voltage range */
    this->vcVoltageRangesGroupBox = setupGroupBox(DCW_VOLTAGE_RANGE_TITLE, vcVoltageRanges, vLayout, vcVoltageRangesRadioButtons);
    for (int i = 0; i< vcVoltageRangesRadioButtons.size(); i++){
        connect(vcVoltageRangesRadioButtons[i], &QRadioButton::clicked, this, [=] (bool flag) {
            if (flag) {
                emit sigVcVoltageRangeSelected(i);
            }
        });
    }

    /*! CC Current range */
    this->ccCurrentRangesGroupBox = setupGroupBox(DCW_CC_CURRENT_RANGE_TITLE, ccCurrentRanges, vLayout, ccCurrentRangesRadioButtons);
    for (int i = 0; i< ccCurrentRangesRadioButtons.size(); i++){
        connect(ccCurrentRangesRadioButtons[i], &QRadioButton::clicked, this, [=] (bool flag) {
            if (flag) {
                emit sigCcCurrentRangeSelected(i);
            }
        });
    }

    /*! CC Voltage range */
    this->ccVoltageRangesGroupBox = setupGroupBox(DCW_CC_VOLTAGE_RANGE_TITLE, ccVoltageRanges, vLayout, ccVoltageRangesRadioButtons);
    for (int i = 0; i< ccVoltageRangesRadioButtons.size(); i++){
        connect(ccVoltageRangesRadioButtons[i], &QRadioButton::clicked, this, [=] (bool flag) {
            if (flag) {
                emit sigCcVoltageRangeSelected(i);
            }
        });
    }

    /*! VC Voltage filter */
    this->vcVoltageFiltersGroupBox = setupGroupBox(DCW_STIMULUS_FILTER_TITLE, vcVoltageFilters, vLayout, vcVoltageFiltersRadioButtons);
    for (int i = 0; i< vcVoltageFiltersRadioButtons.size(); i++){
        connect(vcVoltageFiltersRadioButtons[i], &QRadioButton::clicked, this, [=] (bool flag) {
            if (flag) {
                emit sigVcVoltageFilterSelected(i);
            }
        });
    }

    /*! CC Current filter */
    this->ccCurrentFiltersGroupBox = setupGroupBox(DCW_STIMULUS_FILTER_TITLE, ccCurrentFilters, vLayout, ccCurrentFiltersRadioButtons);
    for (int i = 0; i< ccCurrentFiltersRadioButtons.size(); i++){
        connect(ccCurrentFiltersRadioButtons[i], &QRadioButton::clicked, this, [=] (bool flag) {
            if (flag) {
                emit sigCcCurrentFilterSelected(i);
            }
        });
    }

    /*! Sampling rate */
    this->samplingRatesGroupBox = setupGroupBox(DCW_SAMPLING_RATE_TITLE,samplingRates, vLayout, samplingRatesRadioButtons);
    for (int i = 0; i< samplingRatesRadioButtons.size(); i++){
        connect(samplingRatesRadioButtons[i], &QRadioButton::clicked, this, [=] (bool flag) {
            if (flag) {
                emit sigSamplingRateSelected(i);
            }
        });
    }

    /*! Downsampling ratio */
    this->downsamplingRatiosGroupBox = new QGroupBox(DCW_DOWNSAMPLING_RATIO_TITLE);

    QVBoxLayout * downSamplingRatioVl = new QVBoxLayout();
    downSamplingRatioVl->setContentsMargins(2, 2, 2, 2);
    downSamplingRatioVl->setSpacing(2);

    vLayout->addWidget(this->downsamplingRatiosGroupBox);
    downsamplingRatioSbx = new QSpinBox;
    downsamplingRatioSbx->setRange(1, maxDownsamplingRatio);
    downsamplingRatioSbx->setValue(1);
    downSamplingRatioVl->addWidget(downsamplingRatioSbx);
    connect(downsamplingRatioSbx, QOverload <int> ::of(&QSpinBox::valueChanged), this, [=] (int value) {
        emit sigDownsamplingRatioSelected(value);
    });

    finalSamplingRateLbl = new QLabel("");
    downSamplingRatioVl->addWidget(finalSamplingRateLbl);

    this->downsamplingRatiosGroupBox->setLayout(downSamplingRatioVl);
    if (maxDownsamplingRatio <= 1) {
        downsamplingRatiosGroupBox->setEnabled(false);
    }

    /*! Clamping modality */
    if (clampingModalities.size() > 0) {
        this->clampingModalitiesGroupBox = new QGroupBox(DCW_CLMAPINGMODALITY_TITLE);

        QVBoxLayout * radioButtonsBoxLayout = new QVBoxLayout();
        radioButtonsBoxLayout->setContentsMargins(2, 2, 2, 2);
        radioButtonsBoxLayout->setSpacing(2);

        vLayout->addWidget(this->clampingModalitiesGroupBox);
        for (int idx = 0; idx < clampingModalities.size(); idx++){
            QRadioButton * qrb;
            ClampingModality_t mode = clampingModalities[idx];
            switch (mode) {
            case ClampingModality_t::VOLTAGE_CLAMP:
                qrb = new QRadioButton("Voltage clamp");
                break;

            case ClampingModality_t::ZERO_CURRENT_CLAMP:
                qrb = new QRadioButton("I0 clamp");
                break;

            case ClampingModality_t::CURRENT_CLAMP:
                qrb = new QRadioButton("Current clamp");
                break;
            }

            radioButtonsBoxLayout->addWidget(qrb);
            this->clampingModalitiesRadioButtons.push_back(qrb);
            connect(qrb, &QRadioButton::clicked, this, [=] (bool flag) {
                if (flag) {
                    emit sigClampingModalitySelected(mode);
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
    std::vector<ClampingModality_t> clampingModalities;
    msgDisp->getClampingModalitiesFeatures(clampingModalities); /*! \todo LRos si può spostare nell'appstatus */
    for (int idx = 0; idx < clampingModalitiesRadioButtons.size(); idx++) {
        QRadioButton* btn = clampingModalitiesRadioButtons[idx];
        if (btn->isChecked()) {
            emit sigClampingModalitySelected(clampingModalities[idx]);
        }
    }

    ClampingModality_t mode;
    msgDisp->getClampingModality(mode);

    if (mode == ClampingModality_t::VOLTAGE_CLAMP) {
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

        for (int idx = 0; idx < vcVoltageFiltersRadioButtons.size(); idx++) {
            QRadioButton* btn = vcVoltageFiltersRadioButtons[idx];
            if (btn->isChecked()) {
                emit sigVcVoltageFilterSelected(idx);
            }
        }
    }

    if (mode == ClampingModality_t::CURRENT_CLAMP) {
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

        for (int idx = 0; idx < ccCurrentFiltersRadioButtons.size(); idx++) {
            QRadioButton* btn = ccCurrentFiltersRadioButtons[idx];
            if (btn->isChecked()) {
                emit sigCcCurrentFilterSelected(idx);
            }
        }
    }

    for (int idx = 0; idx < samplingRatesRadioButtons.size(); idx++) {
        QRadioButton* btn = samplingRatesRadioButtons[idx];
        if (btn->isChecked()) {
            emit sigSamplingRateSelected(idx);
        }
    }

    emit sigDownsamplingRatioSelected(downsamplingRatioSbx->value());
}

void DeviceControlDockWidget::updateParameters() {
    ClampingModality_t mode;
    msgDisp->getClampingModality(mode);
    if (mode == ClampingModality_t::VOLTAGE_CLAMP) {
        this->setWidgetVisible(ccVoltageRangesGroupBox, false);
        this->setWidgetVisible(ccCurrentRangesGroupBox, false);
        this->setWidgetVisible(ccCurrentFiltersGroupBox, false);
        this->setWidgetVisible(vcVoltageRangesGroupBox, true);
        this->setWidgetVisible(vcCurrentRangesGroupBox, true);
        this->setWidgetVisible(vcVoltageFiltersGroupBox, true);

        if (vcCurrentRangesRadioButtons.size()>0){
            uint32_t idx;
            msgDisp->getVCCurrentRangeIdx(idx);
            vcCurrentRangesRadioButtons[idx]->setChecked(true);
        }

        if (vcVoltageRangesRadioButtons.size()>0){
            uint32_t idx;
            msgDisp->getVCVoltageRangeIdx(idx);
            vcVoltageRangesRadioButtons[idx]->setChecked(true);
        }

        if (vcVoltageFiltersRadioButtons.size()>0){
            uint32_t idx;
            msgDisp->getVCVoltageFilterIdx(idx);
            vcVoltageFiltersRadioButtons[idx]->setChecked(true);
        }

    } else if (mode == ClampingModality_t::CURRENT_CLAMP) {
        this->setWidgetVisible(vcVoltageRangesGroupBox, false);
        this->setWidgetVisible(vcCurrentRangesGroupBox, false);
        this->setWidgetVisible(vcVoltageFiltersGroupBox, false);
        this->setWidgetVisible(ccVoltageRangesGroupBox, true);
        this->setWidgetVisible(ccCurrentRangesGroupBox, true);
        this->setWidgetVisible(ccCurrentFiltersGroupBox, true);
        if (ccCurrentRangesRadioButtons.size()>0){
            uint32_t idx;
            msgDisp->getCCCurrentRangeIdx(idx);
            ccCurrentRangesRadioButtons[idx]->setChecked(true);
        }

        if (ccVoltageRangesRadioButtons.size()>0){
            uint32_t idx;
            msgDisp->getCCVoltageRangeIdx(idx);
            ccVoltageRangesRadioButtons[idx]->setChecked(true);
        }

        if (ccCurrentFiltersRadioButtons.size()>0){
            uint32_t idx;
            msgDisp->getCCCurrentFilterIdx(idx);
            ccCurrentFiltersRadioButtons[idx]->setChecked(true);
        }     
    }

    if (!samplingRatesRadioButtons.empty()) {
        uint32_t idx;
        msgDisp->getSamplingRateIdx(idx);
        samplingRatesRadioButtons[idx]->setChecked(true);
    }

    if (downsamplingRatioSbx != nullptr) {
        uint32_t ratio;
        msgDisp->getDownsamplingRatio(ratio);
        downsamplingRatioSbx->setValue(ratio);

        Measurement_t samplingRate;
        msgDisp->getSamplingRate(samplingRate);
        samplingRate = samplingRate/(double)ratio;
        finalSamplingRateLbl->setText("Final sampling rate: " + QString::fromStdString(samplingRate.niceLabel()));
    }

    if (!clampingModalitiesRadioButtons.empty()) {
        uint32_t idx;
        msgDisp->getClampingModalityIdx(idx);
        clampingModalitiesRadioButtons[idx]->setChecked(true);
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

void DeviceControlDockWidget::setVcVoltageRangesroupBoxEnabled(bool status){
    setWidgetEnabled(vcVoltageRangesGroupBox, status);
}
void DeviceControlDockWidget::setVcCurrentRangesGroupBoxEnabled(bool status){
    setWidgetEnabled(vcCurrentRangesGroupBox, status);
}
void DeviceControlDockWidget::setCcVoltageRangesGroupBoxEnabled(bool status){
    setWidgetEnabled(ccVoltageRangesGroupBox, status);
}
void DeviceControlDockWidget::setCcCurrentRangesGroupBoxEnabled(bool status){
    setWidgetEnabled(ccCurrentRangesGroupBox, status);
}
void DeviceControlDockWidget::setSamplingRatesGroupBoxEnabled(bool status){
    setWidgetEnabled(samplingRatesGroupBox, status);
}
void DeviceControlDockWidget::setDownsamplingRatioSbxEnabled(bool status){
    setWidgetEnabled(downsamplingRatioSbx, status);
}

template<typename T>
void DeviceControlDockWidget::setWidgetEnabled(T& widget, bool status) {
    if(widget != nullptr){
        widget->setEnabled(status);
    }
}

template<typename T>
void DeviceControlDockWidget::setWidgetVisible(T& widget, bool status) {
    if(widget != nullptr){
        widget->setVisible(status);
    }
}

QGroupBox * DeviceControlDockWidget::setupGroupBox(std::string title, std::vector<RangedMeasurement> rangedMeasurements, QVBoxLayout * parentLayout, std::vector<QRadioButton *> &radioButtons){
    if (rangedMeasurements.size() == 0) {
        return nullptr;
    }
    std::vector<Measurement> measurements;
    for(auto rm: rangedMeasurements){
        measurements.push_back(rm.getMax());
    }
    return setupGroupBox(title, measurements, parentLayout, radioButtons);
}

QGroupBox * DeviceControlDockWidget::setupGroupBox(std::string title, std::vector<Measurement> measurements, QVBoxLayout * parentLayout, std::vector<QRadioButton *> &radioButtons){
    if (measurements.size() == 0) {
        return nullptr;
    }
    auto gb = new QGroupBox(QString::fromStdString(title));
    QVBoxLayout * radioButtonsBoxLayout = new QVBoxLayout();
    radioButtonsBoxLayout->setContentsMargins(2, 2, 2, 2);
    radioButtonsBoxLayout->setSpacing(2);
    parentLayout->addWidget(gb);
    for (int idx = 0; idx < measurements.size(); idx++){
        QRadioButton * qrb = new QRadioButton(QString().fromStdString(measurements[idx].niceLabel()));
        radioButtonsBoxLayout->addWidget(qrb);
        radioButtons.push_back(qrb);
    }
    if (radioButtons.size() > 0) {
        radioButtons[0]->setChecked(true);
    }
    gb->setLayout(radioButtonsBoxLayout);
    if (measurements.size() == 1) {
        gb->setEnabled(false);
    }
    return gb;
}
