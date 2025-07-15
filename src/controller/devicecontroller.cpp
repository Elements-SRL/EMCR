#include "devicecontroller.h"

DeviceController::DeviceController(ApplicationStatus * appStatus, MainWindow * mainWindow) :
    appStatus(appStatus),
    mainWindow(mainWindow) {

    MessageDispatcher * msgDisp = appStatus->getMessageDispatcher();
    msgDisp->getClampingModalitiesFeatures(clampingModalities);
    uint16_t _;
    msgDisp->getVCCurrentRanges(vcCurrentRanges, _);
    msgDisp->getVCVoltageRanges(vcVoltageRanges, _);
    msgDisp->getCCCurrentRanges(ccCurrentRanges, _);
    msgDisp->getCCVoltageRanges(ccVoltageRanges, _);
    msgDisp->getVCVoltageFilters(vcVoltageFilters);
    msgDisp->getCCCurrentFilters(ccCurrentFilters);
    msgDisp->getSamplingRatesFeatures(samplingRates);

    deviceControlDockWidget = new DeviceControlDockWidget(msgDisp);
    mainWindow->setDockWidget(MainWindow::DWDeviceControl, deviceControlDockWidget, false, Qt::LeftDockWidgetArea);

    model = new DeviceModel(appStatus, deviceControlDockWidget);

    connect(deviceControlDockWidget, &DeviceControlDockWidget::sigVcCurrentRangeSelected,     this, [=](int chIdx, int selectedVcCurrentRangeIndex) {
        onVcCurrentRangeSelected(chIdx, selectedVcCurrentRangeIndex);
    });
    connect(deviceControlDockWidget, &DeviceControlDockWidget::sigVcVoltageRangeSelected,     this, [=](int selectedVcVoltageRangeIndex) {
        onVcVoltageRangeSelected(selectedVcVoltageRangeIndex);
    });
    connect(deviceControlDockWidget, &DeviceControlDockWidget::sigCcCurrentRangeSelected,     this, [=](int selectedCcCurrentRangeIndex) {
        onCcCurrentRangeSelected(selectedCcCurrentRangeIndex);
    });
    connect(deviceControlDockWidget, &DeviceControlDockWidget::sigCcVoltageRangeSelected,     this, [=](int chIdx, int selectedCcVoltageRangeIndex) {
        onCcVoltageRangeSelected(chIdx, selectedCcVoltageRangeIndex);
    });
    connect(deviceControlDockWidget, &DeviceControlDockWidget::sigVcVoltageFilterSelected,    this, [=](int selectedVcVoltageFilterIndex) {
        onVcVoltageFilterSelected(selectedVcVoltageFilterIndex);
    });
    connect(deviceControlDockWidget, &DeviceControlDockWidget::sigCcCurrentFilterSelected,    this, [=](int selectedCcCurrentFilterIndex) {
        onCcCurrentFilterSelected(selectedCcCurrentFilterIndex);
    });
    connect(deviceControlDockWidget, &DeviceControlDockWidget::sigSamplingRateSelected,       this, [=](int selectedSamplingRateIndex) {
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
        connect(protocolDw->getCurrentProtocolList(), &ProtocolList::requestVoltageRange,  this, [=](uint16_t selectedCcVoltageRangeIndex) {
            onCcVoltageRangeSelected(appStatus->getVoltageChannelsNum(), selectedCcVoltageRangeIndex);
        });
        connect(protocolDw->getCurrentProtocolList(), &ProtocolList::requestSamplingRate,  this, [=](uint16_t selectedSamplingRateIndex) {
            onSamplingRateSelected(selectedSamplingRateIndex);
        });
        connect(protocolDw->getAnalysisCurrentProtocolList(), &ProtocolList::requestCurrentRange,  this, [=](uint16_t selectedCcCurrentRangeIndex) {
            onCcCurrentRangeSelected(selectedCcCurrentRangeIndex);
        });
        connect(protocolDw->getAnalysisCurrentProtocolList(), &ProtocolList::requestVoltageRange,  this, [=](uint16_t selectedCcVoltageRangeIndex) {
            onCcVoltageRangeSelected(appStatus->getVoltageChannelsNum(), selectedCcVoltageRangeIndex);
        });
        connect(protocolDw->getAnalysisCurrentProtocolList(), &ProtocolList::requestSamplingRate,  this, [=](uint16_t selectedSamplingRateIndex) {
            onSamplingRateSelected(selectedSamplingRateIndex);
        });

        connect(protocolDw->getVoltageProtocolList(), &ProtocolList::requestCurrentRange,  this, [=](uint16_t selectedVcCurrentRangeIndex) {
            onVcCurrentRangeSelected(appStatus->getCurrentChannelsNum(), selectedVcCurrentRangeIndex);
        } );
        connect(protocolDw->getVoltageProtocolList(), &ProtocolList::requestVoltageRange,  this, [=](uint16_t selectedVcVoltageRangeIndex) {
            onVcVoltageRangeSelected(selectedVcVoltageRangeIndex);
        });
        connect(protocolDw->getVoltageProtocolList(), &ProtocolList::requestSamplingRate,  this, [=](uint16_t selectedSamplingRateIndex) {
            onSamplingRateSelected(selectedSamplingRateIndex);
        });
        connect(protocolDw->getAnalysisVoltageProtocolList(), &ProtocolList::requestCurrentRange,  this, [=](uint16_t selectedVcCurrentRangeIndex) {
            onVcCurrentRangeSelected(appStatus->getCurrentChannelsNum(), selectedVcCurrentRangeIndex);
        } );
        connect(protocolDw->getAnalysisVoltageProtocolList(), &ProtocolList::requestVoltageRange,  this, [=](uint16_t selectedVcVoltageRangeIndex) {
            onVcVoltageRangeSelected(selectedVcVoltageRangeIndex);
        });
        connect(protocolDw->getAnalysisVoltageProtocolList(), &ProtocolList::requestSamplingRate,  this, [=](uint16_t selectedSamplingRateIndex) {
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
    deviceControlDockWidget->setVcVoltageRangesSectionEnabled(status);
    deviceControlDockWidget->setCcCurrentRangesSectionEnabled(status);

    deviceControlDockWidget->setVcCurrentRangesSectionEnabled(calcDefaultStatus(vcCurrentRanges.size(), recording));
    deviceControlDockWidget->setCcVoltageRangesSectionEnabled(calcDefaultStatus(ccVoltageRanges.size(), recording));
    deviceControlDockWidget->setSamplingRatesSectionEnabled(calcDefaultStatus(samplingRates.size(), recording));
    deviceControlDockWidget->setDownsamplingRatioSbxEnabled(!recording);
}

void DeviceController::handleProtocolStatusChanged(bool protocolRunning) {
    this->protocolRunning = protocolRunning;
    auto status = getStatusFromRecordingAndProtocol();
    deviceControlDockWidget->setVcVoltageRangesSectionEnabled(status);
    deviceControlDockWidget->setCcCurrentRangesSectionEnabled(status);
}

//return the status to set the Section when a protocol is running or a registration is being made
bool DeviceController::getStatusFromRecordingAndProtocol() {
    return !(recording || protocolRunning);
}

// Slots (actionPerformed) for current and voltage ranges
// ADC Current Range in VC
void DeviceController::onVcCurrentRangeSelected(uint16_t chIdx, uint16_t selectedVcCurrentRangeIndex) {
    if (chIdx < appStatus->getCurrentChannelsNum()) {
        appStatus->getMessageDispatcher()->setVCCurrentRange({chIdx}, {selectedVcCurrentRangeIndex}, true);
    }
    else {
        appStatus->getMessageDispatcher()->setVCCurrentRange(selectedVcCurrentRangeIndex, true);
    }

    emit sigVcCurrentRangeSelected();
}

// DAC Voltage Range in VC might be set by protocol
void DeviceController::onVcVoltageRangeSelected(uint16_t selectedVcVoltageRangeIndex) {
    appStatus->getMessageDispatcher()->setVCVoltageRange(selectedVcVoltageRangeIndex, true);

    emit sigVcVoltageRangeSelected();
}

// DAC Current Range in CC might be set by protocol
void DeviceController::onCcCurrentRangeSelected(uint16_t selectedCcCurrentRangeIndex) {
    appStatus->getMessageDispatcher()->setCCCurrentRange(selectedCcCurrentRangeIndex, true);

    emit sigCcCurrentRangeSelected();
}

// ADC Voltage Range in CC
void DeviceController::onCcVoltageRangeSelected(uint16_t chIdx, uint16_t selectedCcVoltageRangeIndex) {
    if (chIdx < appStatus->getVoltageChannelsNum()) {
        appStatus->getMessageDispatcher()->setCCVoltageRange({chIdx}, {selectedCcVoltageRangeIndex}, true);
    }
    else {
        appStatus->getMessageDispatcher()->setCCVoltageRange(selectedCcVoltageRangeIndex, true);
    }

    emit sigCcVoltageRangeSelected();
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
        emit sigVcCurrentRangeSelected();
        emit sigVcVoltageRangeSelected();

    } else {
        emit sigCcCurrentRangeSelected();
        emit sigCcVoltageRangeSelected();
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
//    if I'm not recording and there are more than 1 measurements enable the Sectiones
    return size > 1 && !recording;
}
