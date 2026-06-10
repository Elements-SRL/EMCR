#include "globaldefines.h"

#include <QVBoxLayout>

#include "devicecontroldockwidget.h"
#include <QScrollArea>

#define TITLE "Device controls"

DeviceControlDockWidget::DeviceControlDockWidget(MessageDispatcher * msgDisp) :
    QDockWidget(),
    msgDisp(msgDisp) {
    setObjectName("deviceControlsDw");
    setAttribute(Qt::WA_TranslucentBackground);
    msgDisp->getChannelNumberFeatures(voltageChannelsNum, currentChannelsNum);
    this->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Minimum);

    QWidget* centralWidget = new QWidget();
    centralWidget->setObjectName("deviceControlsCentralWidget");
    this->setWidget(centralWidget);

    // Window border management when floating
    connect(this, &QDockWidget::topLevelChanged, this, [centralWidget](bool isFloating) {
        if (isFloating) {
            centralWidget->setStyleSheet("#deviceControlsScrollContainer { border: none; } "
                                         "#filterHeaderOutput { border-bottom: none; border-left: none; border-right: none; }"
                                         "#samplingHeaderOutput { border-left: none; border-right: none; }");
        } else {
            centralWidget->setStyleSheet("");
        }
    });

    QVBoxLayout* externalLayout = new QVBoxLayout(centralWidget);
    externalLayout->setContentsMargins(10, 0, 10, 10);
    externalLayout->setSpacing(0);

    QScrollArea* scrollArea = new QScrollArea();
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    scrollArea->setMinimumHeight(0);

    QFrame * scrollContainer = new QFrame();
    scrollContainer->setObjectName("deviceControlsScrollContainer");

    QVBoxLayout* scrollLayout = new QVBoxLayout(scrollContainer);
    scrollLayout->setContentsMargins(0, 8, 0, 0);
    scrollLayout->setSpacing(8);

    scrollArea->setWidget(scrollContainer);
    externalLayout->addWidget(scrollArea);

    std::vector <ClampingModality_t> clampingModalities;
    msgDisp->getClampingModalitiesFeatures(clampingModalities);

    std::vector <RangedMeasurement_t> vcCurrentRanges;
    std::vector <uint16_t> vcCurrentRangeDefaultIdxs;
    msgDisp->getVCCurrentRanges(vcCurrentRanges, vcCurrentRangeDefaultIdxs);
    bool independentVcCurrentRanges = (msgDisp->hasIndependentVCCurrentRanges() == Success ? true : false);

    std::vector <RangedMeasurement_t> vcVoltageRanges;
    uint16_t vcVoltageRangeDefaultIdx;
    msgDisp->getVCVoltageRanges(vcVoltageRanges, vcVoltageRangeDefaultIdx);

    std::vector <RangedMeasurement_t> ccCurrentRanges;
    uint16_t ccCurrentRangeDefaultIdx;
    msgDisp->getCCCurrentRanges(ccCurrentRanges, ccCurrentRangeDefaultIdx);

    std::vector <RangedMeasurement_t> ccVoltageRanges;
    uint16_t ccVoltageRangeDefaultIdx;
    msgDisp->getCCVoltageRanges(ccVoltageRanges, ccVoltageRangeDefaultIdx);
    bool independentCcVoltageRanges = (msgDisp->hasIndependentCCVoltageRanges() == Success ? true : false);

    std::vector <Measurement_t> vcVoltageFilters;
    msgDisp->getVCVoltageFilters(vcVoltageFilters);

    std::vector <Measurement_t> ccCurrentFilters;
    msgDisp->getCCCurrentFilters(ccCurrentFilters);

    std::vector <Measurement_t> samplingRates;
    msgDisp->getSamplingRatesFeatures(samplingRates);

    unsigned int maxDownsamplingRatio;
    msgDisp->getMaxDownsamplingRatioFeature(maxDownsamplingRatio);

    std::vector <std::string> customFlags;
    std::vector <bool> customFlagDefault;
    msgDisp->getCustomFlags(customFlags, customFlagDefault);

    std::vector <std::string> customOptions;
    std::vector <std::vector <std::string>> customOptionDescriptions;
    std::vector <uint16_t> customOptionDefault;
    msgDisp->getCustomOptions(customOptions, customOptionDescriptions, customOptionDefault);

    std::vector <std::string> customDoubles;
    std::vector <RangedMeasurement_t> customDoublesRanges;
    std::vector <double> customDoublesDefault;
    msgDisp->getCustomDoubles(customDoubles, customDoublesRanges, customDoublesDefault);

    this->setWindowTitle(TITLE);

    /*! VC Current range */
    vcCurrentRangesSections.clear();
    if (independentVcCurrentRanges) {
        for (int chIdx = 0; chIdx < currentChannelsNum; chIdx++) {
            vcCurrentRangesSections.push_back(setupSection(DCW_CURRENT_RANGE_TITLE + QString(" ch %1").arg(chIdx+1).toStdString(), vcCurrentRanges, scrollLayout, vcCurrentRangesRadioButtons, vcCurrentRangeDefaultIdxs[chIdx]));
            for (int i = 0; i < vcCurrentRangesRadioButtons[chIdx].size(); i++) {
                connect(vcCurrentRangesRadioButtons[chIdx][i], &QRadioButton::clicked, this, [=] (bool flag) {
                    if (flag) {
                        emit sigVcCurrentRangeSelected(chIdx, i);
                    }
                });
            }
        }
    }
    else {
        vcCurrentRangesSections.push_back(setupSection(DCW_CURRENT_RANGE_TITLE, vcCurrentRanges, scrollLayout, vcCurrentRangesRadioButtons, vcCurrentRangeDefaultIdxs[0]));
        for (int i = 0; i < vcCurrentRangesRadioButtons[0].size(); i++) {
            connect(vcCurrentRangesRadioButtons[0][i], &QRadioButton::clicked, this, [=] (bool flag) {
                if (flag) {
                    emit sigVcCurrentRangeSelected(currentChannelsNum, i);
                }
            });
        }
    }

    /*! VC Voltage range */
    this->vcVoltageRangesSection = setupSection(DCW_VOLTAGE_RANGE_TITLE, vcVoltageRanges, scrollLayout, vcVoltageRangesRadioButtons, vcVoltageRangeDefaultIdx);
    this->setWidgetEnabled(vcVoltageRangesSection, false);
    for (int i = 0; i < vcVoltageRangesRadioButtons.size(); i++) {
        connect(vcVoltageRangesRadioButtons[i], &QRadioButton::clicked, this, [=] (bool flag) {
            if (flag) {
                emit sigVcVoltageRangeSelected(i);
            }
        });
    }

    /*! CC Current range */
    this->ccCurrentRangesSection = setupSection(DCW_CC_CURRENT_RANGE_TITLE, ccCurrentRanges, scrollLayout, ccCurrentRangesRadioButtons, ccCurrentRangeDefaultIdx);
    this->setWidgetEnabled(ccCurrentRangesSection, false);
    for (int i = 0; i < ccCurrentRangesRadioButtons.size(); i++) {
        connect(ccCurrentRangesRadioButtons[i], &QRadioButton::clicked, this, [=] (bool flag) {
            if (flag) {
                emit sigCcCurrentRangeSelected(i);
            }
        });
    }

    /*! CC Voltage range */
    ccVoltageRangesSections.clear();
    if (independentCcVoltageRanges) {
        for (int chIdx = 0; chIdx < currentChannelsNum; chIdx++) {
            for (int i = 0; i < ccVoltageRangesRadioButtons[chIdx].size(); i++) {
                connect(ccVoltageRangesRadioButtons[chIdx][i], &QRadioButton::clicked, this, [=] (bool flag) {
                    if (flag) {
                        emit sigCcVoltageRangeSelected(chIdx, i);
                    }
                });
            }
        }
    }
    else {
        ccVoltageRangesSections.push_back(setupSection(DCW_CC_VOLTAGE_RANGE_TITLE, ccVoltageRanges, scrollLayout, ccVoltageRangesRadioButtons, ccVoltageRangeDefaultIdx));
        for (int i = 0; i < ccVoltageRangesRadioButtons[0].size(); i++) {
            connect(ccVoltageRangesRadioButtons[0][i], &QRadioButton::clicked, this, [=] (bool flag) {
                if (flag) {
                    emit sigCcVoltageRangeSelected(voltageChannelsNum, i);
                }
            });
        }
    }

    /*! VC Voltage filter */
    this->vcVoltageFiltersSection = setupSection(DCW_STIMULUS_FILTER_TITLE, vcVoltageFilters, scrollLayout, vcVoltageFiltersRadioButtons, 0);
    for (int i = 0; i < vcVoltageFiltersRadioButtons.size(); i++) {
        connect(vcVoltageFiltersRadioButtons[i], &QRadioButton::clicked, this, [=] (bool flag) {
            if (flag) {
                emit sigVcVoltageFilterSelected(i);
            }
        });
    }

    /*! CC Current filter */
    this->ccCurrentFiltersSection = setupSection(DCW_STIMULUS_FILTER_TITLE, ccCurrentFilters, scrollLayout, ccCurrentFiltersRadioButtons, 0);
    for (int i = 0; i < ccCurrentFiltersRadioButtons.size(); i++) {
        connect(ccCurrentFiltersRadioButtons[i], &QRadioButton::clicked, this, [=] (bool flag) {
            if (flag) {
                emit sigCcCurrentFilterSelected(i);
            }
        });
    }

    /*! Clamping modality */
    if (clampingModalities.size() > 0) {
        std::vector <QString> modeStrs;
        for (auto mode : clampingModalities) {
            switch (mode) {
            case ClampingModality_t::VOLTAGE_CLAMP:
                modeStrs.push_back("Voltage clamp");
                break;

            case ClampingModality_t::ZERO_CURRENT_CLAMP:
                modeStrs.push_back("I0 clamp");
                break;

            case ClampingModality_t::CURRENT_CLAMP:
                modeStrs.push_back("Current clamp");
                break;
            case e384CommLib::DYNAMIC_CLAMP:
            case e384CommLib::UNDEFINED_CLAMP:
                break;
            }
        }
        this->clampingModalitiesSection = setupSection(DCW_CLMAPINGMODALITY_TITLE, modeStrs, scrollLayout, clampingModalitiesRadioButtons, 0);
        for (int i = 0; i < clampingModalitiesRadioButtons.size(); i++) {
            connect(clampingModalitiesRadioButtons[i], &QRadioButton::clicked, this, [=] (bool flag) {
                if (flag) {
                    emit sigClampingModalitySelected(clampingModalities[i]);
                }
            });
        }
    }

    /*! Custom flags */
    for (unsigned int customFlagIdx = 0; customFlagIdx < customFlags.size(); customFlagIdx++) {
        auto btn = setupActButton(customFlags[customFlagIdx], scrollLayout, customFlagDefault[customFlagIdx]);
        customFlagsButtons.push_back(btn);
        connect(btn, &ActivationButton::clicked, this, [=] (bool flag) {
            emit sigCustomFlagSelected(customFlagIdx, flag);
        });
    }

    /*! Custom options */
    for (unsigned int customOptionIdx = 0; customOptionIdx < customOptions.size(); customOptionIdx++) {
        std::vector <QRadioButton *> radioButtons;
        this->customOptionsSections.push_back(setupSection(customOptions[customOptionIdx], customOptionDescriptions[customOptionIdx], scrollLayout, radioButtons, customOptionDefault[customOptionIdx]));
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
        this->customDoublesGroupBoxes.push_back(setupGroupBox(customDoubles[customDoubleIdx], scrollLayout, customDoublesRanges[customDoubleIdx], customDoublesDefault[customDoubleIdx], spinBox));
        customDoublesSpinBoxes.push_back(spinBox);
        emit sigCustomDoubleChanged(customDoubleIdx, spinBox->value());
        connect(customDoublesSpinBoxes[customDoubleIdx], &QDoubleSpinBox::editingFinished, this, [=] () {
            emit sigCustomDoubleChanged(customDoubleIdx, spinBox->value());
        });
    }

    /* SAMPLING SECTION */
    QFrame* samplingSectionContainer = new QFrame();

    QVBoxLayout* samplingSectionLayout = new QVBoxLayout(samplingSectionContainer);
    samplingSectionLayout->setContentsMargins(0, 0, 0, 0);
    samplingSectionLayout->setSpacing(0);
    samplingSectionLayout->setSizeConstraint(QLayout::SetMinAndMaxSize);

    QFrame* samplingHeader = new QFrame();
    samplingHeader->setObjectName("sectionHeaderContainer");
    QHBoxLayout* samplingHeaderLayout = new QHBoxLayout(samplingHeader);
    samplingHeaderLayout->setContentsMargins(0, 0, 0, 0);

    QLabel *samplingSectionLbl = new QLabel("SAMPLING");
    samplingSectionLbl->setObjectName("sectionHeader");
    samplingHeaderLayout->addWidget(samplingSectionLbl);
    samplingSectionLayout->addWidget(samplingHeader);

    QHBoxLayout* samplingRateLayout = new QHBoxLayout();
    samplingRateLayout->setContentsMargins(0, 10, 0, 8);
    samplingRateLayout->setSpacing(4);
    samplingSectionLayout->addLayout(samplingRateLayout);

    QHBoxLayout* downsamplingRateLayout = new QHBoxLayout();
    downsamplingRateLayout->setContentsMargins(8, 2, 8, 4);
    downsamplingRateLayout->setSpacing(4);
    samplingSectionLayout->addLayout(downsamplingRateLayout);

    /* Sampling rate collapsable */
    this->samplingRatesSection = setupSection(DCW_SAMPLING_RATE_TITLE, samplingRates, samplingRateLayout, samplingRatesRadioButtons, 0);
    for (int i = 0; i < samplingRatesRadioButtons.size(); i++) {
        connect(samplingRatesRadioButtons[i], &QRadioButton::clicked, this, [=] (bool flag) {
            if (flag) {
                emit sigSamplingRateSelected(i);
            }
        });
    }

    /* Downsampling */
    downsamplingToggle = new ActivationButton();
    downsamplingRateLayout->addWidget(downsamplingToggle);

    QLabel * downsamplingLbl = new QLabel("Downsampling");
    downsamplingRateLayout->addWidget(downsamplingLbl);
    downsamplingRateLayout->addStretch();

    downsamplingRatioSbx = new QSpinBox;
    downsamplingRatioSbx->setRange(1, maxDownsamplingRatio);
    downsamplingRatioSbx->setSpecialValueText(tr("Disabled"));
    downsamplingRateLayout->addWidget(downsamplingRatioSbx);

    connect(downsamplingToggle, &ActivationButton::clicked, this, &DeviceControlDockWidget::sigDownsamplingToggleClicked);

    QFrame* samplingHeaderOutput = new QFrame();
    samplingHeaderOutput->setObjectName("samplingHeaderOutput");
    QHBoxLayout* samplingHeaderOutputLayout = new QHBoxLayout(samplingHeaderOutput);
    samplingHeaderOutputLayout->setContentsMargins(0, 0, 0, 0);

    QLabel * samplingTitle = new QLabel("FINAL SAMPLING RATE");
    samplingTitle->setObjectName("deviceFinalOutputLbl");
    finalSamplingRateLbl = new QLabel("");
    finalSamplingRateLbl->setObjectName("finalSamplingRateLbl");

    samplingHeaderOutputLayout->addWidget(samplingTitle);
    samplingHeaderOutputLayout->addStretch();
    samplingHeaderOutputLayout->addWidget(finalSamplingRateLbl);

    samplingSectionLayout->addWidget(samplingHeaderOutput);
    scrollLayout->addWidget(samplingSectionContainer);

    if (maxDownsamplingRatio <= 1) {
        downsamplingToggle->clicked(false);

    }

    /* DIGITAL FILTER */
    QFrame* filterSectionContainer = new QFrame();

    QVBoxLayout * filterSectionLayout = new QVBoxLayout(filterSectionContainer);
    filterSectionLayout->setContentsMargins(0, 0, 0, 0);
    filterSectionLayout->setSpacing(0);
    filterSectionLayout->setSizeConstraint(QLayout::SetMinAndMaxSize);

    QFrame * filterHeader = new QFrame();
    filterHeader->setObjectName("sectionHeaderContainer");
    QHBoxLayout* filterHeaderLayout = new QHBoxLayout(filterHeader);
    filterHeaderLayout->setContentsMargins(0, 0, 0, 0);

    QLabel * filterSectionLbl = new QLabel("FILTER");
    filterSectionLbl->setObjectName("sectionHeader");
    filterHeaderLayout->addWidget(filterSectionLbl);
    filterSectionLayout->addWidget(filterHeader);

    QHBoxLayout * filterLayout = new QHBoxLayout();
    filterLayout->setContentsMargins(8, 4, 8, 2);
    filterLayout->setSpacing(4);
    filterSectionLayout->addLayout(filterLayout);

    digFiltBtn = new ActivationButton;
    filterLayout->addWidget(digFiltBtn);

    QLabel * digitalFilterLbl = new QLabel("Digital Filter");
    filterLayout->addWidget(digitalFilterLbl);
    filterLayout->addStretch();

    digFiltTypeCbx = new QComboBox;
    digFiltTypeCbx->addItem("Low pass");
    digFiltTypeCbx->addItem("High pass");
    filterLayout->addWidget(digFiltTypeCbx);

    /* Cut off freq. */

    QHBoxLayout* cutoffLayout = new QHBoxLayout();
    cutoffLayout->setContentsMargins(11, 2, 8, 4);
    cutoffLayout->setSpacing(4);
    filterSectionLayout->addLayout(cutoffLayout);


    QLabel * cutoffLbl = new QLabel("Cut-Off Freq.");
    cutoffLayout->addWidget(cutoffLbl);
    cutoffLayout->addStretch();

    digFiltCutoffFreqSbx = new QDoubleSpinBox;
    cutoffLayout->addWidget(digFiltCutoffFreqSbx);


    QFrame* filterHeaderOutput = new QFrame();
    filterHeaderOutput->setObjectName("filterHeaderOutput");
    QHBoxLayout* filterHeaderOutputLayout = new QHBoxLayout(filterHeaderOutput);
    filterHeaderOutputLayout->setContentsMargins(0, 0, 0, 0);

    QLabel * filterTitle = new QLabel("FINAL BANDWIDTH");
    filterTitle->setObjectName("deviceFinalOutputLbl");
    finalBandwidthLbl = new QLabel("");
    finalBandwidthLbl->setObjectName("finalBandwidthLbl");

    filterHeaderOutputLayout->addWidget(filterTitle);
    filterHeaderOutputLayout->addStretch();
    filterHeaderOutputLayout->addWidget(finalBandwidthLbl);

    //filterSectionLayout->addWidget(filterHeaderOutput);
    scrollLayout->addWidget(filterSectionContainer);

    // Stiky bottom footer
    externalLayout->addWidget(samplingHeaderOutput);
    externalLayout->addWidget(filterHeaderOutput);


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
        if (vcCurrentRangesRadioButtons.size() > 1) {
            for (int chIdx = 0; chIdx < currentChannelsNum; chIdx++) {
                for (int idx = 0; idx < vcCurrentRangesRadioButtons[chIdx].size(); idx++) {
                    QRadioButton * btn = vcCurrentRangesRadioButtons[chIdx][idx];
                    if (btn->isChecked()) {
                        emit sigVcCurrentRangeSelected(chIdx, idx);
                    }
                }
            }
        }
        else {
            for (int idx = 0; idx < vcCurrentRangesRadioButtons[0].size(); idx++) {
                QRadioButton * btn = vcCurrentRangesRadioButtons[0][idx];
                if (btn->isChecked()) {
                    emit sigVcCurrentRangeSelected(currentChannelsNum, idx);
                }
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

        if (ccVoltageRangesRadioButtons.size() > 1) {
            for (int chIdx = 0; chIdx < voltageChannelsNum; chIdx++) {
                for (int idx = 0; idx < ccVoltageRangesRadioButtons[chIdx].size(); idx++) {
                    QRadioButton * btn = ccVoltageRangesRadioButtons[chIdx][idx];
                    if (btn->isChecked()) {
                        emit sigCcVoltageRangeSelected(voltageChannelsNum, idx);
                    }
                }
            }
        }
        else {
            for (int idx = 0; idx < ccVoltageRangesRadioButtons[0].size(); idx++) {
                QRadioButton * btn = ccVoltageRangesRadioButtons[0][idx];
                if (btn->isChecked()) {
                    emit sigCcVoltageRangeSelected(voltageChannelsNum, idx);
                }
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

    for (int customFlagIdx = 0; customFlagIdx < customFlagsButtons.size(); customFlagIdx++) {
        emit sigCustomFlagSelected(customFlagIdx, customFlagsButtons[customFlagIdx]->isChecked());
    }

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
        for (auto & section : ccVoltageRangesSections) {
            this->setWidgetVisible(section, false);
        }
        this->setWidgetVisible(ccCurrentRangesSection, false);
        this->setWidgetVisible(ccCurrentFiltersSection, false);
        this->setWidgetVisible(vcVoltageRangesSection, true);
        for (auto & section : vcCurrentRangesSections) {
            this->setWidgetVisible(section, true);
        }
        this->setWidgetVisible(vcVoltageFiltersSection, true);

        if (!vcCurrentRangesRadioButtons.empty()){
            if (vcCurrentRangesRadioButtons.size() > 1) {
                std::vector <uint32_t> idxs;
                msgDisp->getVCCurrentRangeIdx(idxs);
                for (int chIdx = 0; chIdx < currentChannelsNum; chIdx++) {
                    vcCurrentRangesRadioButtons[chIdx][idxs[chIdx]]->setChecked(true);
                }
            }
            else {
                uint32_t idx;
                msgDisp->getVCCurrentRangeIdx(idx);
                vcCurrentRangesRadioButtons[0][idx]->setChecked(true);
            }
        }

        if (!vcVoltageRangesRadioButtons.empty()){
            uint32_t idx;
            msgDisp->getVCVoltageRangeIdx(idx);
            vcVoltageRangesRadioButtons[idx]->setChecked(true);
        }

        if (!vcVoltageFiltersRadioButtons.empty()){
            uint32_t idx;
            msgDisp->getVCVoltageFilterIdx(idx);
            vcVoltageFiltersRadioButtons[idx]->setChecked(true);
        }
    }
    else if (mode == ClampingModality_t::CURRENT_CLAMP || mode == ClampingModality_t::ZERO_CURRENT_CLAMP) {
        this->setWidgetVisible(vcVoltageRangesSection, false);
        for (auto & section : vcCurrentRangesSections) {
            this->setWidgetVisible(section, false);
        }
        this->setWidgetVisible(vcVoltageFiltersSection, false);
        for (auto & section : ccVoltageRangesSections) {
            this->setWidgetVisible(section, true);
        }
        this->setWidgetVisible(ccCurrentRangesSection, true);
        this->setWidgetVisible(ccCurrentFiltersSection, true);
        if (!ccCurrentRangesRadioButtons.empty()){
            uint32_t idx;
            msgDisp->getCCCurrentRangeIdx(idx);
            ccCurrentRangesRadioButtons[idx]->setChecked(true);
        }

        if (!ccVoltageRangesRadioButtons.empty()){
            if (ccVoltageRangesRadioButtons.size() > 1) {
                std::vector <uint32_t> idxs;
                msgDisp->getCCVoltageRangeIdx(idxs);
                for (int chIdx = 0; chIdx < voltageChannelsNum; chIdx++) {
                    ccVoltageRangesRadioButtons[chIdx][idxs[chIdx]]->setChecked(true);
                }
            }
            else {
                uint32_t idx;
                msgDisp->getCCVoltageRangeIdx(idx);
                ccVoltageRangesRadioButtons[0][idx]->setChecked(true);
            }
        }

        if (!ccCurrentFiltersRadioButtons.empty()){
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

        if (ratio <= 1) {
            this->setDownsamplingVisualState(false, 1, false);
        } else {
            this->setDownsamplingVisualState(true, ratio, true);
        }
    }

    if (!clampingModalitiesRadioButtons.empty()) {
        uint32_t idx;
        msgDisp->getClampingModalityIdx(idx);
        clampingModalitiesRadioButtons[idx]->setChecked(true);
    }

    /*! \todo FCON aggiungere controlli per DAC filters */
}

void DeviceControlDockWidget::setVcVoltageRangesSectionEnabled(bool status){
    if (vcVoltageRangesSection == nullptr) {
        return;
    }
    if (vcVoltageRangesSection->getSingleOption()) {
        /*! Cannot enable controls with a single option */
        status = false;
    }
    this->setWidgetEnabled(vcVoltageRangesSection, status);
}

void DeviceControlDockWidget::setVcCurrentRangesSectionEnabled(bool status){
    for (auto & section : vcCurrentRangesSections) {
        if (section == nullptr) {
            continue;
        }
        if (section->getSingleOption()) {
            /*! Cannot enable controls with a single option */
            status = false;
        }
        this->setWidgetEnabled(section, status);
    }
}

void DeviceControlDockWidget::setCcVoltageRangesSectionEnabled(bool status){
    for (auto & section : ccVoltageRangesSections) {
        if (section == nullptr) {
            continue;
        }
        if (section->getSingleOption()) {
            /*! Cannot enable controls with a single option */
            status = false;
        }
        this->setWidgetEnabled(section, status);
    }
}

void DeviceControlDockWidget::setCcCurrentRangesSectionEnabled(bool status){
    if (ccCurrentRangesSection == nullptr) {
        return;
    }
    if (ccCurrentRangesSection->getSingleOption()) {
        /*! Cannot enable controls with a single option */
        status = false;
    }
    this->setWidgetEnabled(ccCurrentRangesSection, status);
}

void DeviceControlDockWidget::setSamplingRatesSectionEnabled(bool status){
    if (samplingRatesSection == nullptr) {
        return;
    }
    if (samplingRatesSection->getSingleOption()) {
        /*! Cannot enable controls with a single option */
        status = false;
    }
    this->setWidgetEnabled(samplingRatesSection, status);
}

void DeviceControlDockWidget::setDownsamplingRatioSbxEnabled(bool status){
    this->setWidgetEnabled(downsamplingRatioSbx, status);
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

CollapsibleSection * DeviceControlDockWidget::setupSection(std::string title, std::vector <RangedMeasurement> rangedMeasurements, QBoxLayout * parentLayout, std::vector <QRadioButton *> &radioButtons, int defaultIdx) {
    if (rangedMeasurements.empty()) {
        return nullptr;
    }
    std::vector <QString> texts;
    for (auto rm : rangedMeasurements) {
        texts.push_back(QString::fromStdString(rm.getMax().niceLabel()));
    }
    return setupSection(title, texts, parentLayout, radioButtons, defaultIdx);
}

CollapsibleSection * DeviceControlDockWidget::setupSection(std::string title, std::vector <RangedMeasurement> rangedMeasurements, QBoxLayout * parentLayout, std::vector <std::vector <QRadioButton *>> &radioButtons, int defaultIdx) {
    std::vector <QRadioButton *> buttons;
    auto ret = setupSection(title, rangedMeasurements, parentLayout, buttons, defaultIdx);
    radioButtons.push_back(buttons);
    return ret;
}

CollapsibleSection * DeviceControlDockWidget::setupSection(std::string title, std::vector <Measurement> measurements, QBoxLayout * parentLayout, std::vector <QRadioButton *> &radioButtons, int defaultIdx) {
    if (measurements.empty()) {
        return nullptr;
    }
    std::vector <QString> texts;
    for (auto m : measurements) {
        texts.push_back(QString::fromStdString(m.niceLabel()));
    }
    return setupSection(title, texts, parentLayout, radioButtons, defaultIdx);
}

CollapsibleSection * DeviceControlDockWidget::setupSection(std::string title, std::vector <std::string> strings, QBoxLayout * parentLayout, std::vector <QRadioButton *> &radioButtons, int defaultIdx) {
    if (strings.empty()) {
        return nullptr;
    }
    std::vector <QString> texts;
    for (auto &s : strings) {
        texts.push_back(QString::fromStdString(s));
    }
    return setupSection(title, texts, parentLayout, radioButtons, defaultIdx);
}

CollapsibleSection * DeviceControlDockWidget::setupSection(std::string title, std::vector <QString> texts, QBoxLayout * parentLayout, std::vector <QRadioButton *> &radioButtons, int defaultIdx) {
    if (texts.empty()) {
        return nullptr;
    }
    auto sec = new CollapsibleSection(QString::fromStdString(title), Qt::Vertical);
    parentLayout->addWidget(sec);
    if (texts.size() == 1) {
        sec->setSingleOption(texts[0]);
        sec->setEnabled(false);
    }
    else {
        sec->setParametricTitle(true);
        for (int idx = 0; idx < texts.size(); idx++){
            sec->addRadioButton(texts[idx], idx == defaultIdx);
        }
        sec->setExclusiveControls(true);
    }
    auto buttons = sec->getButtons();
    for (auto &&btn : buttons) {
        radioButtons.push_back(static_cast <QRadioButton *> (btn));
    }
    return sec;
}

ActivationButton * DeviceControlDockWidget::setupActButton(std::string title, QBoxLayout * parentLayout, bool defaultFlag) {
    QWidget * wid = new QWidget;
    parentLayout->addWidget(wid);

    QHBoxLayout * hl = new QHBoxLayout;
    hl->setContentsMargins(0, 0, 0, 0);
    wid->setLayout(hl);

    auto btn = new ActivationButton();
    btn->setChecked(defaultFlag);

    QLabel * lbl = new QLabel(QString::fromStdString(title));

    hl->addWidget(btn);
    hl->addWidget(lbl);

    return btn;
}

QGroupBox * DeviceControlDockWidget::setupGroupBox(std::string title, QVBoxLayout * parentLayout, RangedMeasurement_t range, double valueDefault, QDoubleSpinBox * &spinbox) {
    auto gb = new QGroupBox(QString::fromStdString(title));
    QHBoxLayout * layout = new QHBoxLayout();
    layout->setContentsMargins(10, 4, 10, 4);
    layout->setSpacing(6);
    parentLayout->addWidget(gb);
    spinbox = new QDoubleSpinBox;
    spinbox->setRange(range.min, range.max);
    spinbox->setDecimals(range.decimals());
    spinbox->setValue(valueDefault);
    spinbox->setMinimumHeight(24);
    spinbox->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
    layout->addWidget(spinbox);
    layout->addWidget(new QLabel(QString::fromStdString(range.getFullUnit())));
    gb->setLayout(layout);
    return gb;
}

/* Manages downsapling widget visibility */
void DeviceControlDockWidget::setDownsamplingVisualState(bool checked, int value, bool spinboxEnabled) {
    downsamplingToggle->setChecked(checked);
    downsamplingRatioSbx->setValue(value);
    downsamplingRatioSbx->setEnabled(spinboxEnabled);
}
