#include "globaldefines.h"

#include <QVBoxLayout>

#include "devicecontroldockwidget.h"

#define TITLE "Device controls"

DeviceControlDockWidget::DeviceControlDockWidget(MessageDispatcher * msgDisp) :
    QDockWidget(),
    msgDisp(msgDisp) {

    setObjectName("deviceControlsDw");
    std::vector <ClampingModality_t> clampingModalities;
    msgDisp->getClampingModalitiesFeatures(clampingModalities);

    std::vector <RangedMeasurement_t> vcCurrentRanges;
    uint16_t _;
    msgDisp->getVCCurrentRanges(vcCurrentRanges, _);

    std::vector <RangedMeasurement_t> vcVoltageRanges;
    msgDisp->getVCVoltageRanges(vcVoltageRanges, _);

    std::vector <RangedMeasurement_t> ccCurrentRanges;
    msgDisp->getCCCurrentRanges(ccCurrentRanges, _);

    std::vector <RangedMeasurement_t> ccVoltageRanges;
    msgDisp->getCCVoltageRanges(ccVoltageRanges, _);

    std::vector <Measurement_t> vcVoltageFilters;
    msgDisp->getVCVoltageFilters(vcVoltageFilters);

    std::vector <Measurement_t> ccCurrentFilters;
    msgDisp->getCCCurrentFilters(ccCurrentFilters);

    std::vector <Measurement_t> samplingRates;
    msgDisp->getSamplingRatesFeatures(samplingRates);

    unsigned int maxDownsamplingRatio;
    msgDisp->getMaxDownsamplingRatioFeature(maxDownsamplingRatio);

    std::vector <std::string> customOptions;
    std::vector <std::vector <std::string>> customOptionDescriptions;
    std::vector <uint16_t> customOptionDefault;
    msgDisp->getCustomOptions(customOptions, customOptionDescriptions, customOptionDefault);

    std::vector <std::string> customDoubles;
    std::vector <RangedMeasurement_t> customDoublesRanges;
    std::vector <double> customDoublesDefault;
    msgDisp->getCustomDoubles(customDoubles, customDoublesRanges, customDoublesDefault);

    QWidget * window = new QWidget;
    this->setWidget(window);
    QVBoxLayout * vLayout = new QVBoxLayout(window);
    vLayout->setContentsMargins(0, 0, 0, 1);
    vLayout->setSpacing(1);

    this->setWindowTitle(TITLE);

    /*! VC Current range */
    this->vcCurrentRangesGroupBox = setupGroupBox(DCW_CURRENT_RANGE_TITLE, vcCurrentRanges, vLayout, vcCurrentRangesRadioButtons);
    for (int i = 0; i < vcCurrentRangesRadioButtons.size(); i++) {
        connect(vcCurrentRangesRadioButtons[i], &QRadioButton::clicked, this, [=] (bool flag) {
            if (flag) {
                emit sigVcCurrentRangeSelected(i);
            }
        });
    }

    /*! VC Voltage range */
    this->vcVoltageRangesGroupBox = setupGroupBox(DCW_VOLTAGE_RANGE_TITLE, vcVoltageRanges, vLayout, vcVoltageRangesRadioButtons);
    for (int i = 0; i < vcVoltageRangesRadioButtons.size(); i++) {
        connect(vcVoltageRangesRadioButtons[i], &QRadioButton::clicked, this, [=] (bool flag) {
            if (flag) {
                emit sigVcVoltageRangeSelected(i);
            }
        });
    }

    /*! CC Current range */
    this->ccCurrentRangesGroupBox = setupGroupBox(DCW_CC_CURRENT_RANGE_TITLE, ccCurrentRanges, vLayout, ccCurrentRangesRadioButtons);
    for (int i = 0; i < ccCurrentRangesRadioButtons.size(); i++) {
        connect(ccCurrentRangesRadioButtons[i], &QRadioButton::clicked, this, [=] (bool flag) {
            if (flag) {
                emit sigCcCurrentRangeSelected(i);
            }
        });
    }

    /*! CC Voltage range */
    this->ccVoltageRangesGroupBox = setupGroupBox(DCW_CC_VOLTAGE_RANGE_TITLE, ccVoltageRanges, vLayout, ccVoltageRangesRadioButtons);
    for (int i = 0; i < ccVoltageRangesRadioButtons.size(); i++) {
        connect(ccVoltageRangesRadioButtons[i], &QRadioButton::clicked, this, [=] (bool flag) {
            if (flag) {
                emit sigCcVoltageRangeSelected(i);
            }
        });
    }

    /*! VC Voltage filter */
    this->vcVoltageFiltersGroupBox = setupGroupBox(DCW_STIMULUS_FILTER_TITLE, vcVoltageFilters, vLayout, vcVoltageFiltersRadioButtons);
    for (int i = 0; i < vcVoltageFiltersRadioButtons.size(); i++) {
        connect(vcVoltageFiltersRadioButtons[i], &QRadioButton::clicked, this, [=] (bool flag) {
            if (flag) {
                emit sigVcVoltageFilterSelected(i);
            }
        });
    }

    /*! CC Current filter */
    this->ccCurrentFiltersGroupBox = setupGroupBox(DCW_STIMULUS_FILTER_TITLE, ccCurrentFilters, vLayout, ccCurrentFiltersRadioButtons);
    for (int i = 0; i < ccCurrentFiltersRadioButtons.size(); i++) {
        connect(ccCurrentFiltersRadioButtons[i], &QRadioButton::clicked, this, [=] (bool flag) {
            if (flag) {
                emit sigCcCurrentFilterSelected(i);
            }
        });
    }

    /*! Sampling rate */
    this->samplingRatesGroupBox = setupGroupBox(DCW_SAMPLING_RATE_TITLE, samplingRates, vLayout, samplingRatesRadioButtons);
    for (int i = 0; i < samplingRatesRadioButtons.size(); i++) {
        connect(samplingRatesRadioButtons[i], &QRadioButton::clicked, this, [=] (bool flag) {
            if (flag) {
                emit sigSamplingRateSelected(i);
            }
        });
    }

    /*! Downsampling ratio */
    downsamplingRatiosGroupBox = new QGroupBox(DCW_DOWNSAMPLING_RATIO_TITLE);

    /*! Digital filter */
    digitalFilterGroupBox = new QGroupBox(DCW_DIGITAL_FILTER_TITLE);

    /*! Custom options */
    for (unsigned int customOptionIdx = 0; customOptionIdx < customOptions.size(); customOptionIdx++) {
        std::vector <QRadioButton *> radioButtons;
        this->customOptionsGroupBoxes.push_back(setupGroupBox(customOptions[customOptionIdx], customOptionDescriptions[customOptionIdx], vLayout, radioButtons));
        customOptionsRadioButtons.push_back(radioButtons);
        for (int i = 0; i < radioButtons.size(); i++) {
            connect(radioButtons[i], &QRadioButton::clicked, this, [=] (bool flag) {
                if (flag) {
                    emit sigCustomOptionSelected(customOptionIdx, i);
                }
            });
        }
    }

    /*! Custom doubles */
    for (unsigned int customDoubleIdx = 0; customDoubleIdx < customDoubles.size(); customDoubleIdx++) {
        QDoubleSpinBox * spinBox;
        this->customDoublesGroupBoxes.push_back(setupGroupBox(customDoubles[customDoubleIdx], vLayout, customDoublesRanges[customDoubleIdx], customDoublesDefault[customDoubleIdx], spinBox));
        customDoublesSpinBoxes.push_back(spinBox);
        emit sigCustomDoubleChanged(customDoubleIdx, spinBox->value());
        connect(customDoublesSpinBoxes[customDoubleIdx], &QDoubleSpinBox::editingFinished, this, [=] () {
            emit sigCustomDoubleChanged(customDoubleIdx, spinBox->value());
        });
    }

    QVBoxLayout * downSamplingRatioVl = new QVBoxLayout();
    downSamplingRatioVl->setContentsMargins(2, 2, 2, 2);
    downSamplingRatioVl->setSpacing(2);

    vLayout->addWidget(this->downsamplingRatiosGroupBox);
    downsamplingRatioSbx = new QSpinBox;
    downsamplingRatioSbx->setRange(1, maxDownsamplingRatio);
    downsamplingRatioSbx->setValue(1);
    downSamplingRatioVl->addWidget(downsamplingRatioSbx);

    finalSamplingRateLbl = new QLabel("");
    downSamplingRatioVl->addWidget(finalSamplingRateLbl);

    downsamplingRatiosGroupBox->setLayout(downSamplingRatioVl);
    if (maxDownsamplingRatio <= 1) {
        downsamplingRatiosGroupBox->setEnabled(false);
    }

    QVBoxLayout * digitalFilterVl = new QVBoxLayout();
    digitalFilterVl->setContentsMargins(2, 2, 2, 2);
    digitalFilterVl->setSpacing(2);

    vLayout->addWidget(digitalFilterGroupBox);

    QHBoxLayout * digFiltSettingsHl = new QHBoxLayout;
    digitalFilterVl->addLayout(digFiltSettingsHl);
    digFiltBtn = new ActivationButton;
    digFiltSettingsHl->addWidget(digFiltBtn);
    digFiltTypeCbx = new QComboBox;
    digFiltTypeCbx->addItem("Low pass");
    digFiltTypeCbx->addItem("High pass");
    digFiltSettingsHl->addWidget(digFiltTypeCbx);

    digitalFilterVl->addWidget(new QLabel("Cut-off frequency"));
    digFiltCutoffFreqSbx = new QDoubleSpinBox;
    digitalFilterVl->addWidget(digFiltCutoffFreqSbx);

    QHBoxLayout * digFiltFinalBandiwdthHl = new QHBoxLayout;
    digitalFilterVl->addLayout(digFiltFinalBandiwdthHl);
    digFiltFinalBandiwdthHl->addWidget(new QLabel("Final Bandwidth"));
    finalBandwidthLbl = new QLabel("");
    digFiltFinalBandiwdthHl->addWidget(finalBandwidthLbl);

    digitalFilterGroupBox->setLayout(digitalFilterVl);

    /*! Clamping modality */
    if (clampingModalities.size() > 0) {
        this->clampingModalitiesGroupBox = new QGroupBox(DCW_CLMAPINGMODALITY_TITLE);

        QVBoxLayout * radioButtonsBoxLayout = new QVBoxLayout();
        radioButtonsBoxLayout->setContentsMargins(2, 2, 2, 2);
        radioButtonsBoxLayout->setSpacing(2);

        vLayout->addWidget(this->clampingModalitiesGroupBox);
        for (int idx = 0; idx < clampingModalities.size(); idx++) {
            QRadioButton * qrb = nullptr;
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

            if (qrb == nullptr) {
                continue;
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

    QWidget * spacer = new QWidget;
    spacer->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    vLayout->addWidget(spacer);
}

void DeviceControlDockWidget::forceEmit() {
    std::vector <ClampingModality_t> clampingModalities;
    msgDisp->getClampingModalitiesFeatures(clampingModalities); /*! \todo LRos si può spostare nell'appstatus */
    for (int idx = 0; idx < clampingModalitiesRadioButtons.size(); idx++) {
        QRadioButton * btn = clampingModalitiesRadioButtons[idx];
        if (btn->isChecked()) {
            emit sigClampingModalitySelected(clampingModalities[idx]);
        }
    }

    ClampingModality_t mode;
    msgDisp->getClampingModality(mode);

    if (mode == ClampingModality_t::VOLTAGE_CLAMP) {
        for (int idx = 0; idx < vcCurrentRangesRadioButtons.size(); idx++) {
            QRadioButton * btn = vcCurrentRangesRadioButtons[idx];
            if (btn->isChecked()) {
                emit sigVcCurrentRangeSelected(idx);
            }
        }

        for (int idx = 0; idx < vcVoltageRangesRadioButtons.size(); idx++) {
            QRadioButton * btn = vcVoltageRangesRadioButtons[idx];
            if (btn->isChecked()) {
                emit sigVcVoltageRangeSelected(idx);
            }
        }

        for (int idx = 0; idx < vcVoltageFiltersRadioButtons.size(); idx++) {
            QRadioButton * btn = vcVoltageFiltersRadioButtons[idx];
            if (btn->isChecked()) {
                emit sigVcVoltageFilterSelected(idx);
            }
        }
    }

    if (mode == ClampingModality_t::CURRENT_CLAMP) {
        for (int idx = 0; idx < ccCurrentRangesRadioButtons.size(); idx++) {
            QRadioButton * btn = ccCurrentRangesRadioButtons[idx];
            if (btn->isChecked()) {
                emit sigCcCurrentRangeSelected(idx);
            }
        }

        for (int idx = 0; idx < ccVoltageRangesRadioButtons.size(); idx++) {
            QRadioButton * btn = ccVoltageRangesRadioButtons[idx];
            if (btn->isChecked()) {
                emit sigCcVoltageRangeSelected(idx);
            }
        }

        for (int idx = 0; idx < ccCurrentFiltersRadioButtons.size(); idx++) {
            QRadioButton * btn = ccCurrentFiltersRadioButtons[idx];
            if (btn->isChecked()) {
                emit sigCcCurrentFilterSelected(idx);
            }
        }
    }

    for (int idx = 0; idx < samplingRatesRadioButtons.size(); idx++) {
        QRadioButton * btn = samplingRatesRadioButtons[idx];
        if (btn->isChecked()) {
            emit sigSamplingRateSelected(idx);
        }
    }

//    emit sigDownsamplingRatioSelected(downsamplingRatioSbx->value());

    for (int customOptionIdx = 0; customOptionIdx < customOptionsRadioButtons.size(); customOptionIdx++) {
        for (int idx = 0; idx < customOptionsRadioButtons[customOptionIdx].size(); idx++) {
            QRadioButton * btn = customOptionsRadioButtons[customOptionIdx][idx];
            if (btn->isChecked()) {
                emit sigCustomOptionSelected(customOptionIdx, idx);
            }
        }
    }

    for (int customDoubleIdx = 0; customDoubleIdx < customDoublesSpinBoxes.size(); customDoubleIdx++) {
        emit sigCustomDoubleChanged(customDoubleIdx, customDoublesSpinBoxes[customDoubleIdx]->value());
    }
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
    }

    if (!clampingModalitiesRadioButtons.empty()) {
        uint32_t idx;
        msgDisp->getClampingModalityIdx(idx);
        clampingModalitiesRadioButtons[idx]->setChecked(true);
    }

    /*! \todo FCON aggiungere controlli per DAC filters */
}

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

void DeviceControlDockWidget::setWidgetEnabled(QWidget * widget, bool status) {
    if (widget != nullptr) {
        widget->setEnabled(status);
    }
}

void DeviceControlDockWidget::setWidgetVisible(QWidget * widget, bool status) {
    if (widget != nullptr) {
        widget->setVisible(status);
    }
}

QGroupBox * DeviceControlDockWidget::setupGroupBox(std::string title, std::vector <RangedMeasurement> rangedMeasurements, QVBoxLayout * parentLayout, std::vector <QRadioButton *> &radioButtons) {
    if (rangedMeasurements.empty()) {
        return nullptr;
    }
    std::vector <QString> texts;
    for (auto rm : rangedMeasurements) {
        texts.push_back(QString::fromStdString(rm.getMax().niceLabel()));
    }
    return setupGroupBox(title, texts, parentLayout, radioButtons);
}

QGroupBox * DeviceControlDockWidget::setupGroupBox(std::string title, std::vector <Measurement> measurements, QVBoxLayout * parentLayout, std::vector <QRadioButton *> &radioButtons) {
    if (measurements.empty()) {
        return nullptr;
    }
    std::vector <QString> texts;
    for (auto m : measurements) {
        texts.push_back(QString::fromStdString(m.niceLabel()));
    }
    return setupGroupBox(title, texts, parentLayout, radioButtons);
}

QGroupBox * DeviceControlDockWidget::setupGroupBox(std::string title, std::vector <std::string> strings, QVBoxLayout * parentLayout, std::vector <QRadioButton *> &radioButtons) {
    if (strings.empty()) {
        return nullptr;
    }
    std::vector <QString> texts;
    for (auto s : strings) {
        texts.push_back(QString::fromStdString(s));
    }
    return setupGroupBox(title, texts, parentLayout, radioButtons);
}

QGroupBox * DeviceControlDockWidget::setupGroupBox(std::string title, std::vector <QString> texts, QVBoxLayout * parentLayout, std::vector <QRadioButton *> &radioButtons) {
    if (texts.empty()) {
        return nullptr;
    }
    auto gb = new QGroupBox(QString::fromStdString(title));
    QVBoxLayout * layout = new QVBoxLayout();
    layout->setContentsMargins(2, 2, 2, 2);
    layout->setSpacing(2);
    parentLayout->addWidget(gb);
    for (int idx = 0; idx < texts.size(); idx++){
        QRadioButton * qrb = new QRadioButton(texts[idx]);
        layout->addWidget(qrb);
        radioButtons.push_back(qrb);
    }
    if (radioButtons.size() > 0) {
        radioButtons[0]->setChecked(true);
    }
    gb->setLayout(layout);
    if (texts.size() == 1) {
        gb->setEnabled(false);
    }
    return gb;
}

QGroupBox * DeviceControlDockWidget::setupGroupBox(std::string title, QVBoxLayout * parentLayout, RangedMeasurement_t range, double valueDefault, QDoubleSpinBox * &spinbox) {
    auto gb = new QGroupBox(QString::fromStdString(title));
    QHBoxLayout * layout = new QHBoxLayout();
    layout->setContentsMargins(2, 2, 2, 2);
    layout->setSpacing(2);
    parentLayout->addWidget(gb);
    spinbox = new QDoubleSpinBox;
    spinbox->setRange(range.min, range.max);
    spinbox->setDecimals(range.decimals());
    spinbox->setValue(valueDefault);
    layout->addWidget(spinbox);
    layout->addWidget(new QLabel(QString::fromStdString(range.getFullUnit())));
    gb->setLayout(layout);
    return gb;
}
