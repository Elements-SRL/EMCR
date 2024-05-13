#include "ivgraphcontroller.h"

IvGraphController::IvGraphController(ApplicationStatus* appStatus, DeviceDataProducer* producer, Measurement_t defaultPlotDuration, BigPlotWidget* bigPlotWidget, BigPlotController* bigPlotController) :
    CentralWidgetController(appStatus, producer, bigPlotWidget) {

    model = new BigPlotModel();
    ivGraphWidget = new IvGraphWidget(currentChannelsNum, bigPlotWidget);
    consumer = new IvGraphConsumer(appStatus, producer);

    auto ivGraph = new BigPlot("", "[V]", "", BigPlotStatus::Iv, bigPlotWidget);
    //    creating curves for iv
    for (int i = 0; i < currentChannelsNum; i++) {
        currentCurves.push_back(new Curve(CurveType_t::CurveTypeScatterPlot));
    }

    connect(plot, &BigPlot::zoomInRequest, bigPlotController, &BigPlotController::handleZoomInRequest);
    connect(plot, &BigPlot::zoomOutRequest, bigPlotController, &BigPlotController::handleZoomOutRequest);
    connect(plot, &BigPlot::zoomResetRequest, bigPlotController, &BigPlotController::handleZoomResetRequest);
    connect(plot, &BigPlot::singleAxisZoomRequest, bigPlotController, &BigPlotController::handleSingleAxisZoomRequest);
    connect(plot, &BigPlot::singleAxisShiftRequest, bigPlotController, &BigPlotController::handleSingleAxisShiftRequest);

    connect(bigPlotController, &BigPlotController::durationChanged, consumer, &PlotConsumer::onDurationChanged);
    connect(consumer, &PlotConsumer::setPlotData, bigPlotController, &BigPlotController::onSetPlotData);
    connect(consumer, &PlotConsumer::plotDataUpdated, bigPlotController, &BigPlotController::onReplot);
    consumer->forceAxisUpdate();
    consumer->setMaxSamplesPerPlot(4096);
    consumer->onSelectChannels(false);
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
    currentCurves.clear();
}



void IvGraphController::onExportIvGraph() {
    QString filePath = QFileDialog::getSaveFileName(nullptr,
        "Save File",
        QDir::homePath(), // Initial directory
        "CSV Files (*.csv)");

    // Check if a file path was selected
    if (!filePath.isEmpty()) {
        IvMessage ivMessage = std::get<BigPlotStatus::Iv>(message);
        // Save data to CSV file
        saveToCSV(filePath, ivMessage);

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

    IvMessage ivMessage = std::get<BigPlotStatus::Iv>(message);

    auto selectedChannels = appStatus->getSelectedChannels();
    for (int chIdx = 0; chIdx < currentChannelsNum; chIdx++) {
        const auto nItems = ivMessage.dataSize[chIdx];
        const auto currentData = ivMessage.currentValues[chIdx];
        const auto voltageData = ivMessage.voltageValues[chIdx];
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
