#include "ivgraphcontroller.h"

IvGraphController::IvGraphController(ApplicationStatus* appStatus, DeviceDataProducer* producer, BigPlotWidget* bigPlotWidget, BigPlotController* bigPlotController, MainWindow * mainWindow) :
    CentralWidgetController(appStatus, producer, bigPlotWidget) {
    this->mainWindow = mainWindow;
    model = new BigPlotModel();
    ivGraphWidget = new IvGraphWidget(currentChannelsNum, bigPlotWidget);
    consumer = new IvGraphConsumer(appStatus, producer);
    this->mainWindow->setIvGraphWidget(ivGraphWidget);
    plot = new BigPlot("", "[V]", "", BigPlotStatus::Iv, bigPlotWidget);
    bigPlotWidget->setIvGraph(plot);
    // creating curves for iv
    for (int i = 0; i < currentChannelsNum; i++) {
        currentCurves.push_back(new Curve(CurveType_t::CurveTypeScatterPlot));
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

    connect(bigPlotController, &BigPlotController::durationChanged, consumer, &PlotConsumer::onDurationChanged);
    connect(consumer, &PlotConsumer::setPlotData, this, &IvGraphController::onSetPlotData);
    connect(consumer, &PlotConsumer::plotDataUpdated, this, &IvGraphController::onReplot);
    consumer->forceAxisUpdate();
    consumer->setMaxSamplesPerPlot(4096);
    std::vector <uint16_t> allChannels(currentChannelsNum);
    for (int idx = 0; idx < currentChannelsNum; idx++) {
        allChannels[idx] = idx;
    }
    consumer->onPlotChannels(allChannels, false);
    consumer->onStopConsuming();

    connect(ivGraphWidget, &IvGraphWidget::exportIvGraph, this, &IvGraphController::onExportIvGraph);
    connect(ivGraphWidget, &IvGraphWidget::calcMeanSquared, this, &IvGraphController::onCalcMeanSquared);
    connect(ivGraphWidget, &IvGraphWidget::startIvGraph, this, &IvGraphController::onStartIvGraph);
    connect(ivGraphWidget, &IvGraphWidget::stopIvGraph, this, &IvGraphController::onStopIvGraph);
}

IvGraphController::~IvGraphController() {
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
    if (ivGraphWidget != nullptr) {
        delete ivGraphWidget;
        ivGraphWidget = nullptr;
        mainWindow->setIvGraphWidget(ivGraphWidget);
    }
    currentCurves.clear();
}

void IvGraphController::onExportIvGraph() {
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

void IvGraphController::saveToCSV(const QString& originalFilePath, const IvMessage& data) {
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
        RangedMeasurement vRange;
        RangedMeasurement iRange;
        appStatus->getMessageDispatcher()->getVoltageRange(vRange);
        appStatus->getMessageDispatcher()->getCurrentRange(iRange);
        // Write header
        out << "Voltage " << QString::fromStdString(vRange.label()) << ", Current " << QString::fromStdString(iRange.label()) << "\n";
        // Write data
        int numRows = data.dataSize[i];
        for (int row = 0; row < numRows; row++) {
            out << data.voltageValues[i][row] << "," << data.currentValues[i][row] << "\n";
        }
        file.close();
    }
}

void IvGraphController::onCalcMeanSquared() {
    std::map<std::uint32_t, std::vector<Measurement>> myMap;
    RangedMeasurement vRange;
    RangedMeasurement iRange;
    appStatus->getMessageDispatcher()->getVoltageRange(vRange);
    appStatus->getMessageDispatcher()->getCurrentRange(iRange);
    auto vUnitPfx = vRange.prefix;
    auto iUnitPfx = iRange.prefix;

    auto selectedChannels = appStatus->getSelectedChannels();
    for (int chIdx = 0; chIdx < currentChannelsNum; chIdx++) {
        const auto nItems = message.dataSize[chIdx];
        const auto currentData = message.currentValues[chIdx];
        const auto voltageData = message.voltageValues[chIdx];
        // calc regression only for active channels
        if (!selectedChannels[chIdx] || nItems == 0) {
            continue;
        }
        double xSum = 0.0;
        double ySum = 0.0;

        for (int i = 0; i < nItems; i++) {
            xSum += voltageData[i];
            ySum += currentData[i];
        }
        const auto nItemsD = (double)nItems;
        const double meanX = xSum / nItemsD;
        const double meanY = ySum / nItemsD;

        double bDenom = 0.0;
        double bNum = 0.0;
        for (int i = 0; i < nItems; i++) {
            const auto xMinusMean = voltageData[i] - meanX;
            const auto yMinusMean = currentData[i] - meanY;
            bDenom += pow(xMinusMean, 2.0);
            bNum += xMinusMean * yMinusMean;
        }
        const auto b = bNum / bDenom;
        const auto a = meanY - (b * meanX);

        const Measurement conductance = { b, iUnitPfx / vUnitPfx, "S" };
        const Measurement resistance = { ((double)1) / conductance.value, UnitPfx::UnitPfxNone / conductance.prefix, "Ohm" };
        const Measurement invPot = { ((0.0 - a) / b), vUnitPfx, "V" };
        const Measurement iOffset = { a, iUnitPfx, "A" };

        std::vector<Measurement> vals = { conductance, resistance, invPot, iOffset };
        myMap.insert(std::make_pair(chIdx, vals));
    }
    ivGraphWidget->setParams(myMap);
}

void IvGraphController::onStartIvGraph() {
    consumer->onStopConsuming();
    RangedMeasurement r;
    appStatus->getMessageDispatcher()->getVCVoltageRange(r);
    //sending this only to reset the data
    consumer->onVoltageRangeChanged(r);
    consumer->onStartConsuming();
}

void IvGraphController::onStopIvGraph() {
    consumer->onStopConsuming();
}

void IvGraphController::start() {
    if (!isAtLeastOneChannelExpanded()) {
        return;
    }
    ivGraphWidget->show();
    attachCurves();
    consumer->onStartConsuming();
}

void IvGraphController::stop() {
    consumer->onStopConsuming();
    detachCurves();
}

void IvGraphController::detachCurves() {
    for (auto c : currentCurves) {
        c->detach();
    }
    plot->replot();
}

void IvGraphController::attachCurves() {
    for (auto c : currentCurves) {
        c->attach(plot);
    }
    plot->replot();
}

IvGraphWidget* IvGraphController::getIvGraphWidget() {
    return ivGraphWidget;
}

void IvGraphController::onCurrentColorsChanged(QVector <QColor> colors) {
    for (int idx = 0; idx < currentChannelsNum; idx++) {
        currentCurves[idx]->setColor(colors[idx]);
    }
}

void IvGraphController::onCurrentColorChanged(int channelIdx, QColor color) {
    for (int idx = 0; idx < currentChannelsNum; idx++) {
        currentCurves[channelIdx]->setColor(color);
    }
}

void IvGraphController::onBackgroundColorChanged(QColor color) {
    if (plot != nullptr) {
        plot->setCanvasBackground(color);
    }
}

void IvGraphController::onReplot() {
    if (plot != nullptr) {
        plot->replot();
    }
}

//todo Check clampingmodality as well
void IvGraphController::onRangeUpdated(commlib::RangedMeasurement_t newRange) {
    QwtPlot::Axis axisIdx;
    if (newRange.unit == "s") {
        return;
    }
    else if (newRange.unit == "V") {
        axisIdx = QwtPlot::xBottom;
    }
    else if (newRange.unit == "A") {
        axisIdx = QwtPlot::yLeft;
    }
    model->setCurrentRange(axisIdx, newRange);
    plot->setRect(model->getZoom(BigPlotModel::Zoom::Current));
    plot->setLabel(QString::fromStdString(model->getCurrentRange(axisIdx).getFullUnit()), axisIdx);
    plot->replot();
}

void IvGraphController::onExpandTrace(bool flag) {
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

void IvGraphController::onSetPlotData(PlotMessage plotmessage) {
    RangedMeasurement v;
    RangedMeasurement i;
    // IvGraph message
    appStatus->getMessageDispatcher()->getVCVoltageRange(v);
    appStatus->getMessageDispatcher()->getVCCurrentRange(i);
    plot->setRect(model->initRect(v.min, v.max, i.min, i.max));
    message = std::get<1>(plotmessage);
    if (message.currentValues.size() == 0 || message.voltageValues.size() == 0 || message.dataSize.size() == 0) {
        return;
    }
    for (int idx = 0; idx < currentChannelsNum; idx++) {
        currentCurves[idx]->setRawSamples(message.voltageValues[idx], message.currentValues[idx], message.dataSize[idx]);
    }
}

PlotConsumer* IvGraphController::getConsumer() {
    return consumer;
}