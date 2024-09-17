#include "devicecontroller.h"

DeviceController::DeviceController(ApplicationStatus * appStatus, MainWindow * mainWindow) :
    appStatus(appStatus),
    mainWindow(mainWindow) {

    MessageDispatcher * msgDisp = appStatus->getMessageDispatcher();
    msgDisp->getClampingModalitiesFeatures(clampingModalities);
    uint16_t defaultVcCurrRangeIdx;
    msgDisp->getVCCurrentRanges(vcCurrentRanges,defaultVcCurrRangeIdx);
    msgDisp->getVCVoltageRanges(vcVoltageRanges);
    msgDisp->getCCCurrentRanges(ccCurrentRanges);
    msgDisp->getCCVoltageRanges(ccVoltageRanges);
    msgDisp->getVCVoltageFilters(vcVoltageFilters);
    msgDisp->getCCCurrentFilters(ccCurrentFilters);
    msgDisp->getSamplingRatesFeatures(samplingRates);


    deviceControlDockWidget = new DeviceControlDockWidget(msgDisp);
    mainWindow->setDockWidget(MainWindow::DWDeviceControl, deviceControlDockWidget, false, Qt::LeftDockWidgetArea);

    model = new DeviceModel(appStatus, deviceControlDockWidget);

    connect(deviceControlDockWidget, &DeviceControlDockWidget::sigVcCurrentRangeSelected,     this, [=](uint16_t selectedVcCurrentRangeIndex) {
        onVcCurrentRangeSelected(selectedVcCurrentRangeIndex);
    });
    connect(deviceControlDockWidget, &DeviceControlDockWidget::sigVcVoltageRangeSelected,     this, [=](uint16_t selectedVcVoltageRangeIndex) {
        onVcVoltageRangeSelected(selectedVcVoltageRangeIndex);
    });
    connect(deviceControlDockWidget, &DeviceControlDockWidget::sigCcCurrentRangeSelected,     this, [=](uint16_t selectedCcCurrentRangeIndex) {
        onCcCurrentRangeSelected(selectedCcCurrentRangeIndex);
    });
    connect(deviceControlDockWidget, &DeviceControlDockWidget::sigCcVoltageRangeSelected,     this, [=](uint16_t selectedCcVoltageRangeIndex) {
        onCcVoltageRangeSelected(selectedCcVoltageRangeIndex);
    });
    connect(deviceControlDockWidget, &DeviceControlDockWidget::sigVcVoltageFilterSelected,    this, [=](uint16_t selectedVcVoltageFilterIndex) {
        onVcVoltageFilterSelected(selectedVcVoltageFilterIndex);
    });
    connect(deviceControlDockWidget, &DeviceControlDockWidget::sigCcCurrentFilterSelected,    this, [=](uint16_t selectedCcCurrentFilterIndex) {
        onCcCurrentFilterSelected(selectedCcCurrentFilterIndex);
    });
    connect(deviceControlDockWidget, &DeviceControlDockWidget::sigSamplingRateSelected,       this, [=](uint16_t selectedSamplingRateIndex) {
        onSamplingRateSelected(selectedSamplingRateIndex);
    });
    connect(model, &DeviceModel::sigDownsamplingRatioSelected,  this, &DeviceController::onDownsamplingRatioSelected);
    connect(model, &DeviceModel::sigDigitalFilterChanged,  this, &DeviceController::onDigitalFilterSettingsChanged);
    connect(deviceControlDockWidget, &DeviceControlDockWidget::sigClampingModalitySelected,   this, [=](ClampingModality_t selectedClampingModality) {
        onClampingModalitySelected(selectedClampingModality);
    });
    connect(deviceControlDockWidget, &DeviceControlDockWidget::sigCustomOptionSelected,   this, [=](uint32_t customOptionIdx, int selectedCustomOptionIdx) {
        onCustomOptionSelected(customOptionIdx, selectedCustomOptionIdx);
    });
    connect(deviceControlDockWidget, &DeviceControlDockWidget::sigCustomDoubleChanged,   this, [=](uint32_t customDoubleIdx, double value) {
        onCustomDoubleChanged(customDoubleIdx, value);
    });

    if (msgDisp->hasProtocols() == Success) {
        auto protocolDw = static_cast <ProtocolDockWidget *> (mainWindow->getDockWidget(MainWindow::DWProtocol));
        connect(protocolDw->getCurrentProtocolList(), &ProtocolList::requestCurrentRange,  this, [=](uint16_t selectedCcCurrentRangeIndex) {
            onCcCurrentRangeSelected(selectedCcCurrentRangeIndex);
        });
        connect(protocolDw->getCurrentProtocolList(), &ProtocolList::requestVoltageRange,  this, [=](uint16_t selectedVcVoltageRangeIndex) {
            onVcVoltageRangeSelected(selectedVcVoltageRangeIndex);
        });
        connect(protocolDw->getCurrentProtocolList(), &ProtocolList::requestSamplingRate,  this, [=](uint16_t selectedSamplingRateIndex) {
            onSamplingRateSelected(selectedSamplingRateIndex);
        });

        connect(protocolDw->getVoltageProtocolList(), &ProtocolList::requestCurrentRange,  this, [=](uint16_t selectedVcCurrentRangeIndex) {
            onVcCurrentRangeSelected(selectedVcCurrentRangeIndex);
        } );
        connect(protocolDw->getVoltageProtocolList(), &ProtocolList::requestVoltageRange,  this, [=](uint16_t selectedVcVoltageRangeIndex) {
            onVcVoltageRangeSelected(selectedVcVoltageRangeIndex);
        });
        connect(protocolDw->getVoltageProtocolList(), &ProtocolList::requestSamplingRate,  this, [=](uint16_t selectedSamplingRateIndex) {
            onSamplingRateSelected(selectedSamplingRateIndex);
        });
    }
}

DeviceController::~DeviceController() {
    delete deviceControlDockWidget;
    deviceControlDockWidget = nullptr;
    mainWindow->setDockWidget(MainWindow::DWDeviceControl, deviceControlDockWidget);
}

void DeviceController::handleRecording(bool recording) {
    this->recording = recording;
    auto status = getStatusFromRecordingAndProtocol();
    deviceControlDockWidget->setVcVoltageRangesroupBoxEnabled(status);
    deviceControlDockWidget->setCcCurrentRangesGroupBoxEnabled(status);

    deviceControlDockWidget->setVcCurrentRangesGroupBoxEnabled(calcDefaultStatus(vcCurrentRanges.size(), recording));
    deviceControlDockWidget->setCcVoltageRangesGroupBoxEnabled(calcDefaultStatus(ccVoltageRanges.size(), recording));
    deviceControlDockWidget->setSamplingRatesGroupBoxEnabled(calcDefaultStatus(samplingRates.size(), recording));
    deviceControlDockWidget->setDownsamplingRatioSbxEnabled(!recording);
}

void DeviceController::handleProtocolStatusChanged(bool protocolRunning) {
    this->protocolRunning = protocolRunning;
    auto status = getStatusFromRecordingAndProtocol();
    deviceControlDockWidget->setVcVoltageRangesroupBoxEnabled(status);
    deviceControlDockWidget->setCcCurrentRangesGroupBoxEnabled(status);
}

//return the status to set the groupbox when a protocol is running or a registration is being made
bool DeviceController::getStatusFromRecordingAndProtocol() {
    return !(recording || protocolRunning);
}

// Slots (actionPerformed) for current and voltage ranges
// ADC Current Range in VC
void DeviceController::onVcCurrentRangeSelected(uint16_t selectedVcCurrentRangeIndex) {
    appStatus->getMessageDispatcher()->setVCCurrentRange(selectedVcCurrentRangeIndex, true);

    emit sigVcCurrentRangeSelected(selectedVcCurrentRangeIndex);
}

// DAC Voltage Range in VC might be set by protocol
void DeviceController::onVcVoltageRangeSelected(uint16_t selectedVcVoltageRangeIndex) {
    appStatus->getMessageDispatcher()->setVCVoltageRange(selectedVcVoltageRangeIndex, true);

    emit sigVcVoltageRangeSelected(selectedVcVoltageRangeIndex);
}

// DAC Current Range in CC might be set by protocol
void DeviceController::onCcCurrentRangeSelected(uint16_t selectedCcCurrentRangeIndex) {
    appStatus->getMessageDispatcher()->setCCCurrentRange(selectedCcCurrentRangeIndex, true);

    emit sigCcCurrentRangeSelected(selectedCcCurrentRangeIndex);
}

// ADC Voltage Range in CC
void DeviceController::onCcVoltageRangeSelected(uint16_t selectedCcVoltageRangeIndex) {
    appStatus->getMessageDispatcher()->setCCVoltageRange(selectedCcVoltageRangeIndex, true);

    emit sigCcVoltageRangeSelected(selectedCcVoltageRangeIndex);
}

// Slots (actionPerformed) for current and voltage filters
// ADC Current Filter in VC set by Sampling Rate

// DAC Voltage Filter in VC
void DeviceController::onVcVoltageFilterSelected(uint16_t selectedVcVoltageFilterIndex) {
    appStatus->getMessageDispatcher()->setVoltageStimulusLpf(selectedVcVoltageFilterIndex, true);

    emit sigVcVoltageFilterSelected(selectedVcVoltageFilterIndex);
}

// DAC Current Filter in CC
void DeviceController::onCcCurrentFilterSelected(uint16_t selectedCcCurrentFilterIndex) {
    appStatus->getMessageDispatcher()->setCurrentStimulusLpf(selectedCcCurrentFilterIndex, true);

    emit sigCcCurrentFilterSelected(selectedCcCurrentFilterIndex);
}

// Sampling rate
void DeviceController::onSamplingRateSelected(uint16_t selectedSamplingRateIndex) {
    appStatus->getMessageDispatcher()->setSamplingRate(selectedSamplingRateIndex, true);

    emit sigSamplingRateSelected(selectedSamplingRateIndex);
}

// Downsampling ratio
void DeviceController::onDownsamplingRatioSelected(uint16_t selectedDownsamplingRatioIndex) {
    appStatus->getMessageDispatcher()->setDownsamplingRatio(selectedDownsamplingRatioIndex);
    deviceControlDockWidget->updateParameters();

    emit sigDownsamplingRatioSelected(selectedDownsamplingRatioIndex);
}

// Digital filter
void DeviceController::onDigitalFilterSettingsChanged(Measurement_t cutoffrequency, bool lowPassFlag, bool activeFlag) {
    appStatus->getMessageDispatcher()->setRawDataFilter(cutoffrequency, lowPassFlag, activeFlag);
}
// ADC Voltage Filter in CC set by Sampling rate

void DeviceController::onClampingModalitySelected(ClampingModality_t mode) {
    MessageDispatcher * msgDisp = appStatus->getMessageDispatcher();
    msgDisp->setClampingModality(mode, true, true);

    if (mode == ClampingModality_t::VOLTAGE_CLAMP) {
        uint32_t idx;
        msgDisp->getVCCurrentRangeIdx(idx);
        emit sigVcCurrentRangeSelected(idx);

        msgDisp->getVCVoltageRangeIdx(idx);
        emit sigVcVoltageRangeSelected(idx);

    } else {
        uint32_t idx;
        msgDisp->getCCCurrentRangeIdx(idx);
        emit sigCcCurrentRangeSelected(idx);

        msgDisp->getCCVoltageRangeIdx(idx);
        emit sigCcVoltageRangeSelected(idx);
    }

    emit sigClampingModalitySelected(mode);
}

void DeviceController::onCustomOptionSelected(uint32_t customOptionIdx, int idx) {
    appStatus->getMessageDispatcher()->setCustomOption(customOptionIdx, idx, true);
}

void DeviceController::onCustomDoubleChanged(uint32_t customDoubleIdx, double value) {
    appStatus->getMessageDispatcher()->setCustomDouble(customDoubleIdx, value, true);
}

bool DeviceController::calcDefaultStatus(int size, bool recording) {
//    if I'm not recording and there are more than 1 measurements enable the groupboxes
    return size > 1 && !recording;
}
