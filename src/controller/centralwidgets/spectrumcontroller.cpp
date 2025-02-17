#include "spectrumcontroller.h"

#include <qwt_date_scale_engine.h>

SpectrumController::SpectrumController(ApplicationStatus * appStatus, DeviceDataProducer * producer, Measurement_t defaultPlotBandwidth, BigPlotWidget * bigPlotWidget, BigPlotController * bigPlotController, MainWindow * mainWindow):
    CentralWidgetController(appStatus, producer, bigPlotWidget),
    mainWindow(mainWindow){

    model = new BigPlotModel(BigPlot::Spectrum);
    consumer = new SpectrumConsumer(appStatus, producer);
    consumer->onIntegrationWindowChanged({1.0, UnitPfxNone, "s"});

    plot = new BigPlot("", "[Hz]", "", BigPlot::Spectrum, bigPlotWidget);
    plot->enableAxis(QwtPlot::yRight);
    plot->setAxisAutoScale(QwtPlot::xBottom, false);
    plot->setAxisAutoScale(QwtPlot::yLeft, false);
    plot->setAxisAutoScale(QwtPlot::yRight, false);
    plot->setAxisScaleEngine(QwtPlot::xBottom, new QwtLogScaleEngine(10));
    plot->setAxisScaleEngine(QwtPlot::yLeft, new QwtLogScaleEngine(10));

    spectrumWidget = new SpectrumWidget(currentChannelsNum, plot, bigPlotWidget);

    bigPlotWidget->setSpectrumPlot(spectrumWidget);
    //    creating curves for spectra
    for (int i = 0; i < currentChannelsNum; i++) {
        psdCurves.push_back(new Curve(CurveType_t::CurveTypePlotSolid));
        irmsCurves.push_back(new Curve(CurveType_t::CurveTypePlotDashed));
        irmsCurves[i]->setYAxis(QwtPlot::yRight);
    }

    connect(plot, &BigPlot::zoomInRequest, bigPlotController, [=](Rect4 r) {
        bigPlotController->handleZoomInRequest(model, plot, r);
    });
    connect(plot, &BigPlot::zoomOutRequest, bigPlotController, [=]() {
        bigPlotController->handleZoomOutRequest(model, plot);
    });
    connect(plot, &BigPlot::zoomResetRequest, bigPlotController, [=]() {
        bigPlotController->handleZoomResetRequest(model, plot);
    });
    connect(plot, &BigPlot::singleAxisZoomRequest, bigPlotController, [=](QwtPlot::Axis axis, int zoomIn, QPointF mousePosition) {
        bigPlotController->handleSingleAxisZoomRequest(model, plot, axis, zoomIn, mousePosition);
    });
    connect(plot, &BigPlot::singleAxisShiftRequest, bigPlotController, [=](QwtPlot::Axis axis, int shift) {
        bigPlotController->handleSingleAxisShiftRequest(model, plot, axis, shift);
    });

    connect(consumer, &PlotConsumer::setPlotData, this, &SpectrumController::onSetPlotData);
    connect(consumer, &PlotConsumer::plotDataUpdated, this, &SpectrumController::onReplot);
    connect(consumer, &SpectrumConsumer::sigRangeUpdate, this, &SpectrumController::onRangeUpdated);
    connect(spectrumWidget, &SpectrumWidget::sigIntegrationWindowChanged, this, [=] (double windowS) {
        consumer->onIntegrationWindowChanged({windowS, UnitPfxNone, "s"});
    });
    connect(spectrumWidget, &SpectrumWidget::sigStartPressed, consumer, &SpectrumConsumer::onStartConsuming);
    connect(spectrumWidget, &SpectrumWidget::sigStopPressed, consumer, &SpectrumConsumer::onStopConsuming);
    connect(spectrumWidget, &SpectrumWidget::sigExportSpectrum, this, &SpectrumController::onExportSpectrum);
    connect(spectrumWidget, &SpectrumWidget::sigAutoZoom, this, [=] () {
        plot->onAutoZoom({QwtPlot::yLeft, QwtPlot::yRight});
    });
    consumer->forceAxisUpdate();
    consumer->onStopConsuming();
}

SpectrumController::~SpectrumController() {
    if (model != nullptr) {
        delete model;
        model = nullptr;
    }
    if (consumer != nullptr) {
        delete consumer;
        consumer = nullptr;
    }
    if (plot != nullptr) {
        delete plot;
        plot = nullptr;
    }
    psdCurves.clear();
    irmsCurves.clear();
}

void SpectrumController::detachCurves(const std::vector <uint16_t>& channelIndexes) {
    for (auto ch : channelIndexes) {
        psdCurves[ch]->detach();
        irmsCurves[ch]->detach();
    }
    plot->replot();
}

void SpectrumController::attachCurves(const std::vector <uint16_t>& channelIndexes) {
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
    if (plot != nullptr) {
        plot->setCanvasBackground(color);
    }
}

void SpectrumController::onReplot() {
    if (plot != nullptr) {
        if (plotInitializedFlag) {
            plot->replot();
        }
        else {
            plot->onAutoZoom({QwtPlot::yLeft, QwtPlot::yRight});
            plotInitializedFlag = true;
        }
    }
}

void SpectrumController::onRangeUpdated(commlib::RangedMeasurement_t newRange) {
    ClampingModality_t mode;
    appStatus->getMessageDispatcher()->getClampingModality(mode);
    std::string unit = "";
    switch (mode) {
    case e384CommLib::VOLTAGE_CLAMP:
    case e384CommLib::CURRENT_CLAMP_CURRENT_READ:
        unit = "A";
        break;
    case e384CommLib::CURRENT_CLAMP:
    case e384CommLib::ZERO_CURRENT_CLAMP:
    case e384CommLib::VOLTAGE_CLAMP_VOLTAGE_READ:
        unit = "V";
        break;
    }
    QwtPlot::Axis axisIdx;
    if (newRange.unit == unit) {
        axisIdx = QwtPlot::yLeft;
        model->setCurrentRangeLog(axisIdx, newRange);
        plot->setLabel(QString::fromStdString(model->getCurrentRange(axisIdx).getFullUnit()) + "^2/Hz", axisIdx);

        axisIdx = QwtPlot::yRight;
        model->setCurrentRangeLog(axisIdx, newRange);
        plot->setLabel(QString::fromStdString(model->getCurrentRange(axisIdx).getFullUnit()) + "rms", axisIdx);
    }
    else if (newRange.unit == "Hz") {
        axisIdx = QwtPlot::xBottom;
        model->setCurrentRange(axisIdx, newRange);
        plot->setLabel(QString::fromStdString(model->getCurrentRange(axisIdx).getFullUnit()), axisIdx);
    }
    else {
        return;
    }
    plot->setRect(model->getZoom(BigPlotModel::Zoom::Current));
    plot->replot();
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
    message = std::get <PMS_SPECTRUM> (plotmessage);
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
        qDebug() << "No file path selected.";
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
