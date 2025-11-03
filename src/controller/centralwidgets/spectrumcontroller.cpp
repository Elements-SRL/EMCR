#include "spectrumcontroller.h"
#include <qwt_date_scale_engine.h>
#include "operationmode.h"

SpectrumController::SpectrumController(
    ApplicationStatus * appStatus,
    DeviceDataProducer * producer,
    RangedMeasurement_t defaultPlotBandwidth,
    BigPlotWidget * bigPlotWidget,
    MainWindow * mainWindow
    ):
    CentralWidgetController(appStatus, producer, bigPlotWidget),
    mainWindow(mainWindow){

    consumer = new SpectrumConsumer(appStatus, producer);
    consumer->onIntegrationWindowChanged({1.0, UnitPfxNone, "s"});

    std::map<QwtPlot::Axis, AxisInfo> m;
    //TODO lrossi correctly initialize these with filo
    m[QwtPlot::Axis::yLeft] = {appStatus->getCurretRange(), std::nullopt, true};
    m[QwtPlot::Axis::yRight] = {appStatus->getVoltageRange(), std::nullopt, true};
    m[QwtPlot::Axis::xBottom] = {defaultPlotBandwidth, std::make_optional(0.0)};

    pc = std::make_unique<PlotController>(m, mainWindow);

    spectrumWidget = new SpectrumWidget(currentChannelsNum, pc->getPlot(), bigPlotWidget);

    bigPlotWidget->setSpectrumPlot(spectrumWidget);
    //    creating curves for spectra
    for (int i = 0; i < currentChannelsNum; i++) {
        psdCurves.push_back(new Curve(CurveType_t::CurveTypePlotSolid));
        irmsCurves.push_back(new Curve(CurveType_t::CurveTypePlotDashed));
        irmsCurves[i]->setYAxis(QwtPlot::yRight);
    }

    connect(consumer, &PlotConsumer::setPlotData, this, &SpectrumController::onSetPlotData);
    connect(consumer, &PlotConsumer::plotDataUpdated, this, &SpectrumController::onReplot);
    connect(consumer, &SpectrumConsumer::sigRangeUpdate, this, &SpectrumController::onRangeUpdated);
    connect(spectrumWidget, &SpectrumWidget::sigIntegrationWindowChanged, this, [=] (double windowS) {
        consumer->onIntegrationWindowChanged({windowS, UnitPfxNone, "s"});
    });
    connect(spectrumWidget, &SpectrumWidget::sigStartPressed, consumer, &SpectrumConsumer::onStartConsuming);
    connect(spectrumWidget, &SpectrumWidget::sigStopPressed, consumer, &SpectrumConsumer::onStopConsuming);
    connect(spectrumWidget, &SpectrumWidget::sigExportSpectrum, this, &SpectrumController::onExportSpectrum);
    connect(spectrumWidget, &SpectrumWidget::sigAutoZoom, pc.get(), &PlotController::onAutoZoom);
    connect(this, &SpectrumController::sigLabelsOverride, pc.get(), &PlotController::onLabelsOverride);
    consumer->forceAxisUpdate();
    consumer->onStopConsuming();
}

SpectrumController::~SpectrumController() {
    if (consumer != nullptr) {
        delete consumer;
        consumer = nullptr;
    }
    psdCurves.clear();
    irmsCurves.clear();
}

void SpectrumController::detachCurves(const std::vector <uint16_t>& channelIndexes) {
    for (auto ch : channelIndexes) {
        psdCurves[ch]->detach();
        irmsCurves[ch]->detach();
    }
    pc->getPlot()->replot();
}

void SpectrumController::attachCurves(const std::vector <uint16_t>& channelIndexes) {
    auto plot = pc->getPlot();
    for (auto ch : channelIndexes) {
        psdCurves[ch]->attach(plot);
        irmsCurves[ch]->attach(plot);
    }
    plot->replot();
}

void SpectrumController::start() {
    if (!isAtLeastOneChannelExpanded()) {
        return;
    }
    attachCurves(appStatus->getExpandedChannelsIndexes());
    consumer->onStartConsuming();
}

void SpectrumController::stop() {
    consumer->onStopConsuming();
    std::vector <uint16_t> allChannels(currentChannelsNum);
    for (int idx = 0; idx < currentChannelsNum; idx++) {
        allChannels[idx] = idx;
    }
    detachCurves(allChannels);
}

void SpectrumController::onCurrentColorsChanged(QVector <QColor> colors) {
    for (int idx = 0; idx < currentChannelsNum; idx++) {
        psdCurves[idx]->setColor(colors[idx]);
        irmsCurves[idx]->setColor(colors[idx]);
    }
}

void SpectrumController::onCurrentColorChanged(int channelIdx, QColor color) {
    psdCurves[channelIdx]->setColor(color);
    irmsCurves[channelIdx]->setColor(color);
}

void SpectrumController::onBackgroundColorChanged(QColor color) {
    auto plot = pc->getPlot();
    if (plot != nullptr) {
        plot->setCanvasBackground(color);
    }
}

void SpectrumController::onReplot() {
    auto plot = pc->getPlot();
    if (plot != nullptr) {
        if (plotInitializedFlag) {
            plot->replot();
        }
        else {
            pc->onAutoZoom();
            plotInitializedFlag = true;
        }
    }
}

void SpectrumController::onRangeUpdated(commlib::RangedMeasurement_t newRange) {
    auto plot = pc->getPlot();
    auto model = pc->getModel();
    ClampingModality_t mode;
    appStatus->getMessageDispatcher()->getClampingModality(mode);
    std::string unit = "";
    switch (mode) {
    case e384CommLib::VOLTAGE_CLAMP:
        unit = "A";
        break;
    case e384CommLib::CURRENT_CLAMP:
    case e384CommLib::ZERO_CURRENT_CLAMP:
        unit = "V";
        break;
    }
    QwtPlot::Axis axisIdx;
    if (newRange.unit == unit) {
        std::map<QwtPlot::Axis, std::string> labels;
        axisIdx = QwtPlot::yLeft;
        auto pm = pc->getModel();
        auto rm = pm->getAxisRangedMeasurement(axisIdx);
        labels[axisIdx] = rm.getFullUnit() + "^2/Hz";
        axisIdx = QwtPlot::yRight;
        rm = pm->getAxisRangedMeasurement(axisIdx);
        labels[axisIdx] = rm.getFullUnit() + "rms";

    } else if (newRange.unit == "Hz") {
        axisIdx = QwtPlot::xBottom;
        pc->setRangedMeasurement(axisIdx, newRange);
    }
    else {
        return;
    }
}

void SpectrumController::onExpandTrace(bool flag) {
    auto wasRunning = consumer->isRunning();
    if (wasRunning) {
        stop();
        consumer->onPlotSelectedChannels(flag);
        start();
    }
    else {
        consumer->onPlotSelectedChannels(flag);
    }
}

void SpectrumController::onSetPlotData(PlotMessage plotmessage) {
    message = std::get <OperationMode_t::Spectrum> (plotmessage);
    for (int idx = 0; idx < currentChannelsNum; idx++) {
        psdCurves[idx]->setRawSamples(message.frequencyValues, message.psdValues[idx], message.dataSize);
        irmsCurves[idx]->setRawSamples(message.frequencyValues, message.irmsValues[idx], message.dataSize);
    }
}

PlotConsumer * SpectrumController::getConsumer() {
    return consumer;
}

std::vector <DeviceDataConsumer*> SpectrumController::getConsumers() {
    return {consumer};
}

void SpectrumController::onExportSpectrum() {
    QString filePath = QFileDialog::getSaveFileName(nullptr,
        "Save File",
        QDir::homePath(), // Initial directory
        "CSV Files (*.csv)");

    // Check if a file path was selected
    if (!filePath.isEmpty()) {
        // Save data to CSV file
        saveToCSV(filePath, message);

    }
    else {
        // No file path selected
        QString err = "No file path selected.";
        QString info = "";
        ErrorManager e(err, info);
    }
}

void SpectrumController::saveToCSV(const QString& originalFilePath, const SpectrumMessage& data) {
    auto selectedChannels = appStatus->getSelectedChannels();
    for (int i = 0; i < currentChannelsNum; i++) {
        //        save to file only selected channels
        if (!selectedChannels[i]) {
            continue;
        }
        auto filepath = originalFilePath.toStdString();
        //        append the channel number
        size_t pos = filepath.find_last_of('.');
        if (pos != std::string::npos && filepath.substr(pos) == ".csv") {
            filepath.insert(pos, "_" + std::to_string(i));
        }

        QFile file(QString::fromStdString(filepath));

        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QMessageBox::warning(nullptr, "Warning", "File could not be saved.");
            return;
        }

        QTextStream out(&file);
        RangedMeasurement iRange;
        appStatus->getMessageDispatcher()->getCurrentRange(iRange);
        // Write header
        out << "frequency [Hz], PSD [" << QString::fromStdString(iRange.getFullUnit()) << "^2/Hz], irms [" << QString::fromStdString(iRange.getFullUnit()) << "rms]\n";
        // Write data
        int numRows = data.dataSize;
        for (int row = 0; row < numRows; row++) {
            out << data.frequencyValues[row] << "," << data.psdValues[i][row] << "," << data.irmsValues[i][row] << "\n";
        }
        file.close();
    }
}
