#include "devicemodel.h"

DeviceModel::DeviceModel(ApplicationStatus * appStatus, DeviceControlDockWidget * view) :
    QObject(),
    view(view),
    appStatus(appStatus) {

    appStatus->getMessageDispatcher()->getSamplingRatesFeatures(samplingRates);

    samplingRate = samplingRates[0];
    samplingRate.convertValue(DIG_FILT_CUTOFF_FREQ_UNIT_PFX);
    finalSamplingRate = samplingRate;
    maxCutoffFrequency = finalSamplingRate/4.0;
    cutoffFrequency = maxCutoffFrequency;

    view->digFiltCutoffFreqSbx->setDecimals(3);
    view->digFiltCutoffFreqSbx->setSuffix(QString::fromStdString(" " + finalSamplingRate.getFullUnit()));
    view->digFiltCutoffFreqSbx->setRange(1.0/finalSamplingRateMultiplier, maxCutoffFrequency.value);
    view->digFiltCutoffFreqSbx->setValue(cutoffFrequency.value);

    this->updateDownsamplingAndFilteringSettings();

    connect(view, &DeviceControlDockWidget::sigSamplingRateSelected, this, &DeviceModel::onSamplingrateSelected);
    connect(view->downsamplingRatioSbx, &QSpinBox::editingFinished, this, &DeviceModel::onDownsamplingRatioChanged);
    connect(view->digFiltBtn, &ActivationButton::clicked, this, &DeviceModel::onDigFiltEnabled);
    connect(view->digFiltTypeCbx, QOverload <int>::of(&QComboBox::currentIndexChanged), this, &DeviceModel::onDigFiltSettingsChanged);
    connect(view->digFiltCutoffFreqSbx, &QSpinBox::editingFinished, this, &DeviceModel::onDigFiltSettingsChanged);
}

void DeviceModel::onSamplingrateSelected(int idx) {
    if (samplingRate == samplingRates[idx]) {
        return;
    }
    samplingRate = samplingRates[idx];
    samplingRate.convertValue(DIG_FILT_CUTOFF_FREQ_UNIT_PFX);
    finalSamplingRate = samplingRate / (double)downsamplingRatio;
    this->updateDownsamplingAndFilteringSettings();
}

void DeviceModel::onDownsamplingRatioChanged() {
    if (downsamplingRatio == view->downsamplingRatioSbx->value()) {
        return;
    }
    downsamplingRatio = view->downsamplingRatioSbx->value();
    downsamplingFlag = downsamplingRatio > 1;
    finalSamplingRate = samplingRate/(double)downsamplingRatio;
    this->updateDownsamplingAndFilteringSettings();
    emit sigDownsamplingRatioSelected(downsamplingRatio);
}

void DeviceModel::onDigFiltEnabled() {
    if (digFiltEnabledFlag == view->digFiltBtn->isChecked()) {
        return;
    }
    digFiltEnabledFlag = view->digFiltBtn->isChecked();
    digFiltEnabledUserFlag = digFiltEnabledFlag;
    this->updateDownsamplingAndFilteringSettings();
    emit sigDigitalFilterChanged(cutoffFrequency, digFiltLowPassFlag, digFiltEnabledFlag);
}

void DeviceModel::onDigFiltSettingsChanged() {
    if (cutoffFrequency.value == view->digFiltCutoffFreqSbx->value()
            && (digFiltLowPassFlag == (view->digFiltTypeCbx->currentIndex() == 0))) {
        return;
    }
    cutoffFrequency = {view->digFiltCutoffFreqSbx->value(), DIG_FILT_CUTOFF_FREQ_UNIT_PFX, "Hz"};
    digFiltLowPassFlag = view->digFiltTypeCbx->currentIndex() == 0;
    digFiltLowPassUserFlag = digFiltLowPassFlag;
    this->updateDownsamplingAndFilteringSettings();
    emit sigDigitalFilterChanged(cutoffFrequency, digFiltLowPassFlag, digFiltEnabledFlag);
}

void DeviceModel::updateDownsamplingAndFilteringSettings() {
    if (downsamplingFlag) {
        if (!digFiltEnabledFlag) {
            digFiltEnabledFlag = true;
            view->digFiltBtn->setChecked(true);
        }
        view->digFiltBtn->setEnabled(false);
        if (!digFiltLowPassFlag) {
            digFiltLowPassFlag = true;
            view->digFiltTypeCbx->setCurrentIndex(0);
        }
        view->digFiltTypeCbx->setEnabled(false);
    }
    else {
        view->digFiltBtn->setEnabled(true);
        if (digFiltEnabledUserFlag != digFiltEnabledFlag) {
            digFiltEnabledFlag = digFiltEnabledUserFlag;
            view->digFiltBtn->setChecked(digFiltEnabledFlag);
        }
        view->digFiltTypeCbx->setEnabled(true);
        if (digFiltLowPassUserFlag != digFiltLowPassFlag) {
            digFiltLowPassFlag = digFiltLowPassUserFlag;
            view->digFiltTypeCbx->setCurrentIndex(digFiltLowPassFlag ? 0 : 1);
        }
    }

    maxCutoffFrequency = finalSamplingRate/4.0;
    if (digFiltEnabledFlag) {
        if (cutoffFrequency > maxCutoffFrequency) {
            maxCutoffFrequency.convertValue(cutoffFrequency.prefix);
            cutoffFrequency.value = maxCutoffFrequency.value;
            view->digFiltCutoffFreqSbx->setValue(cutoffFrequency.value);
        }
        view->digFiltCutoffFreqSbx->setRange(1.0/finalSamplingRateMultiplier, maxCutoffFrequency.value);

        if (digFiltLowPassFlag) {
            view->finalBandwidthLbl->setText(QString::fromStdString("0 - " + cutoffFrequency.niceLabel()));
        }
        else {
            view->finalBandwidthLbl->setText(QString::fromStdString(cutoffFrequency.niceLabel() + " - " + maxCutoffFrequency.niceLabel()));
        }
    }
    else {
        view->finalBandwidthLbl->setText(QString::fromStdString("0 - " + maxCutoffFrequency.niceLabel()));
    }
    view->finalSamplingRateLbl->setText("Final sampling rate: " + QString::fromStdString(finalSamplingRate.niceLabel()));
}
