#include "devicemodel.h"

DeviceModel::DeviceModel(ApplicationStatus * appStatus, DeviceControlDockWidget * view) :
    QObject(),
    view(view),
    appStatus(appStatus) {

    appStatus->getMessageDispatcher()->getSamplingRatesFeatures(samplingRates);

    finalSamplingRate = samplingRates[0];
    maxCutoffFrequency = finalSamplingRate/4.0;

    view->digFiltCutoffFreqSbx->setRange(1.0/finalSamplingRate.multiplier(), maxCutoffFrequency.value);
    view->digFiltCutoffFreqSbx->setValue(finalSamplingRate.value/4.0);

    for (unsigned int pfx = UnitPfxNone; pfx <= finalSamplingRate.prefix; pfx++) {
        Measurement_t sr = {1.0, (UnitPfx_t)pfx, "Hz"};
        view->digFiltUnitCbx->addItem(QString::fromStdString(sr.getFullUnit()));
    }

    connect(view, &DeviceControlDockWidget::sigSamplingRateSelected, this, &DeviceModel::onSamplingrateSelected);
    connect(view->downsamplingRatioSbx, &QSpinBox::editingFinished, this, &DeviceModel::onDownsamplingRatioChanged);
    connect(view->digFiltBtn, &ActivationButton::clicked, this, &DeviceModel::onDigFiltEnabled);
    connect(view->digFiltTypeCbx, QOverload <int>::of(&QComboBox::currentIndexChanged), this, &DeviceModel::onDigFiltSettingsChanged);
    connect(view->digFiltCutoffFreqSbx, &QSpinBox::editingFinished, this, &DeviceModel::onDigFiltSettingsChanged);
    connect(view->digFiltUnitCbx, QOverload <int>::of(&QComboBox::currentIndexChanged), this, &DeviceModel::onDigFiltSettingsChanged);
}

void DeviceModel::onSamplingrateSelected(int idx) {
    if (samplingRate == samplingRates[idx]) {
        return;
    }
    samplingRate = samplingRates[idx];
    this->updateDownsamplingAndFilteringSettings();
    finalSamplingRate = samplingRate/(double)downsamplingRatio;
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
            && cutoffFrequency.prefix == (UnitPfx_t)(view->digFiltUnitCbx->currentIndex()+(int)UnitPfxNone)
            && (digFiltLowPassFlag == (view->digFiltTypeCbx->currentIndex() == 0))) {
        return;
    }
    cutoffFrequency = {view->digFiltCutoffFreqSbx->value(), (UnitPfx_t)(view->digFiltUnitCbx->currentIndex()+(int)UnitPfxNone), "Hz"};
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
            view->digFiltBtn->setEnabled(false);
        }
        if (!digFiltLowPassFlag) {
            digFiltLowPassFlag = true;
            view->digFiltTypeCbx->setCurrentIndex(0);
            view->digFiltTypeCbx->setEnabled(false);
        }
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

    if (digFiltEnabledFlag) {
        maxCutoffFrequency = samplingRate/4.0;
        if (cutoffFrequency > maxCutoffFrequency) {
            maxCutoffFrequency.convertValue(cutoffFrequency.prefix);
            cutoffFrequency.value = maxCutoffFrequency.value;
            view->digFiltCutoffFreqSbx->setValue(cutoffFrequency.value);
        }

        if (digFiltLowPassFlag) {
            view->finalBandwidthLbl->setText(QString::fromStdString("0 - " + cutoffFrequency.niceLabel()));
        }
        else {
            view->finalBandwidthLbl->setText(QString::fromStdString(cutoffFrequency.niceLabel() + " - " + maxCutoffFrequency.niceLabel()));
        }
    }
    view->finalBandwidthLbl->setText(QString::fromStdString("0 - " + maxCutoffFrequency.niceLabel()));
}
