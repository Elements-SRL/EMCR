#include "bigplotcontroller.h"
#include <iostream>

BigPlotController::BigPlotController(ApplicationStatus * appStatus, DeviceDataProducer * producer, Measurement_t defaultPlotDuration, MainWindow * mainWindow) :
    appStatus(appStatus),
    mainWindow(mainWindow) {

//    we only have one widget with multiple tabs
//    todo maybe we could create a widget for each tab
    bpw = new BigPlotWidget(mainWindow);
    ivGraphWidget = new IvGraphWidget(mainWindow);

    auto ivModel = new BigPlotModel();
    auto gapFreeModel = new BigPlotModel();

    mainWindow->setBigPlotWidget(bpw);
    mainWindow->setIvGraphWidget(ivGraphWidget);
    auto ivGraphConsumer = new IvGraphConsumer(appStatus, producer);
    auto gapFreePlotConsumer = new GapFreePlotConsumer(appStatus, producer);
    gapFreePlotConsumer->onDurationChanged(defaultPlotDuration);

    auto gapFreePlot = new BigPlot("", "[s]", "", BigPlotStatus::GapFree, bpw);
    gapFreePlot->enableAxis(QwtPlot::yRight);
    bpw->setGapFreePlot(gapFreePlot);
    connect(bpw, &BigPlotWidget::tabBarClicked, this, &BigPlotController::manageStatus);
    auto ivGraph = new BigPlot("", "[V]", "", BigPlotStatus::Iv, bpw);
    bpw->setIvGraph(ivGraph);
    currentChannelsNum = appStatus->getCurrentChannelsNum();
    voltageChannelsNum = appStatus->getVoltageChannelsNum();

    currentCurves.resize(BigPlotStatus::NumberOfStatuses);
    voltageCurves.resize(BigPlotStatus::NumberOfStatuses);
//    creating curves for gapfree
    for (int i = 0; i < currentChannelsNum; i++) {
        currentCurves[BigPlotStatus::GapFree].push_back(new Curve(CurveType_t::CurveTypePlotSolid));
    }
    for (int i = 0; i < voltageChannelsNum; i++) {
        voltageCurves[BigPlotStatus::GapFree].push_back(new Curve(CurveType_t::CurveTypePlotDashed));
        voltageCurves[BigPlotStatus::GapFree][i]->setColor(QColor(Qt::red));
        voltageCurves[BigPlotStatus::GapFree][i]->setYAxis(QwtPlot::yRight);
    }
    //    creating curves for iv
    for (int i = 0; i < currentChannelsNum; i++) {
        currentCurves[BigPlotStatus::Iv].push_back(new Curve(CurveType_t::CurveTypeScatterPlot));
        voltageCurves[BigPlotStatus::Iv].push_back(new Curve());
        voltageCurves[BigPlotStatus::Iv][i]->detach();
    }
    messages.resize(BigPlotStatus::NumberOfStatuses);
    consumers = {gapFreePlotConsumer, ivGraphConsumer};
    plots = {gapFreePlot, ivGraph};
    models = {gapFreeModel, ivModel};

    for(auto p: plots) {
        connect(p, &BigPlot::zoomInRequest, this, &BigPlotController::handleZoomInRequest);
        connect(p, &BigPlot::zoomOutRequest, this, &BigPlotController::handleZoomOutRequest);
        connect(p, &BigPlot::zoomResetRequest, this, &BigPlotController::handleZoomResetRequest);
        connect(p, &BigPlot::singleAxisZoomRequest, this, &BigPlotController::handleSingleAxisZoomRequest);
        connect(p, &BigPlot::singleAxisShiftRequest, this, &BigPlotController::handleSingleAxisShiftRequest);
    }

    for(auto c: consumers){
        connect(this, &BigPlotController::durationChanged, c, &PlotConsumer::onDurationChanged);
        connect(c, &PlotConsumer::setPlotData,         this, &BigPlotController::onSetPlotData);
        connect(c, &PlotConsumer::plotDataUpdated,     this, &BigPlotController::onReplot);
        c->forceAxisUpdate();
        c->setMaxSamplesPerPlot(4096);
        c->onSelectChannels(false);
        c->onStopConsuming();
    }

    connect(ivGraphWidget, &IvGraphWidget::exportIvGraph, this, &BigPlotController::onExportIvGraph);
    currentPlot = gapFreePlot;
    currentConsumer = gapFreePlotConsumer;
    currentModel = gapFreeModel;
    bps = BigPlotStatus::GapFree;
}

void BigPlotController::manageStatus(int idx) {

    if (idx == bps) {
        return;
    }
    for(auto c: consumers){
        c->onStopConsuming();
    }
    detachCurves();
    switch (idx) {
    case 0:
        bps = BigPlotStatus::GapFree;
        break;
    case 1:
        bps = BigPlotStatus::Iv;
        ivGraphWidget->show();
        break;
    default:
        bps = BigPlotStatus::NumberOfStatuses;
        break;
    }
    currentConsumer = consumers[idx];
    currentModel = models[idx];
    onSetPlotData(messages[idx]);
    attachCurves();
    currentPlot = plots[idx];
    if (isAtLeastOneChannelExpanded()) {
        currentConsumer->onStartConsuming();
    }
}

BigPlotController::~BigPlotController() {
    this->clearCurves();

    for(int i=0; i < plots.size(); i++) {
        delete plots[i];
        plots[i] = nullptr;
    }
    for(int i=0; i < consumers.size(); i++) {
        delete consumers[i];
        consumers[i] = nullptr;
    }

    for(int i=0; i < models.size(); i++) {
        delete models[i];
        models[i] = nullptr;
    }

    if (bpw != nullptr) {
        delete bpw;
        bpw = nullptr;
        mainWindow->setBigPlotWidget(bpw);
    }

    if (currentModel != nullptr) {
        delete currentModel;
        currentModel = nullptr;
    }
}

BigPlot * BigPlotController::getPlot(){
    return currentPlot;
}

void BigPlotController::clearCurves() {
    for (int plotStatus=0; plotStatus < BigPlotStatus::NumberOfStatuses; plotStatus++) {
        for (int idx = 0; idx < currentChannelsNum; idx++) {
            currentCurves[plotStatus][idx]->detach();
            delete currentCurves[plotStatus][idx];
            delete [] currentCurves[plotStatus][idx];
        }

        for (int idx = 0; idx < voltageChannelsNum; idx++) {
            voltageCurves[plotStatus][idx]->detach();
            delete voltageCurves[plotStatus][idx];
            delete [] voltageCurves[plotStatus][idx];
        }
    }

    currentCurves.clear();
    voltageCurves.clear();
}

bool BigPlotController::isAtLeastOneChannelExpanded() {
    auto channels = appStatus->getChannels();
    for(auto c: channels){
        if(c->isExpanded()){
            return true;
        }
    }
    return false;
}

void BigPlotController::onExpandTrace(bool flag){
    currentConsumer->onStopConsuming();
    detachCurves();
    for (auto c: consumers){
        c->onSelectChannels(flag);
    }
    attachCurves();
    if (isAtLeastOneChannelExpanded()) {
        currentConsumer->onStartConsuming();
    }
}

void BigPlotController::onSetPlotData(PlotMessage plotmessage) {
    IvMessage ivMessage;
    GapFreeMessage gapFreeMessage;
    RangedMeasurement v;
    RangedMeasurement i;
    switch (plotmessage.index()) {
//    GapFree message
    case 0:
        gapFreeMessage = std::get<0>(plotmessage);
        messages[BigPlotStatus::GapFree] = gapFreeMessage;
        for (int idx = 0; idx < currentChannelsNum; idx++) {
            currentCurves[BigPlotStatus::GapFree][idx]->setRawSamples(gapFreeMessage.timeValues, gapFreeMessage.currentValues[idx], gapFreeMessage.dataSize);
            voltageCurves[BigPlotStatus::GapFree][idx]->setRawSamples(gapFreeMessage.timeValues, gapFreeMessage.voltageValues[idx], gapFreeMessage.dataSize);
        }
        break;
//    IvGraph message
    case 1:
        appStatus->getMessageDispatcher()->getVCVoltageRange(v);
        appStatus->getMessageDispatcher()->getVCCurrentRange(i);
        plots[BigPlotStatus::Iv]->setRect(models[BigPlotStatus::Iv]->initRect(v.min, v.max, i.min, i.max));
        ivMessage = std::get<1>(plotmessage);
        messages[BigPlotStatus::Iv] = ivMessage;
        if (ivMessage.currentValues.size() == 0 || ivMessage.voltageValues.size() == 0 ||  ivMessage.dataSize.size() == 0) {
            break;
        }
        for (int idx = 0; idx < currentChannelsNum; idx++) {
            currentCurves[BigPlotStatus::Iv][idx]->setRawSamples(ivMessage.voltageValues[idx], ivMessage.currentValues[idx], ivMessage.dataSize[idx]);
        }
        break;
    }
}

void BigPlotController::detachCurves(){
    for (int i=0; i<BigPlotStatus::NumberOfStatuses; i++) {
        for (int idx = 0; idx < currentChannelsNum; idx++) {
            currentCurves[i][idx]->detach();
            voltageCurves[i][idx]->detach();
        }
    }
    for (auto p: plots) {
        p->replot();
    }
}

void BigPlotController::attachCurves(){
    auto channels = appStatus->getChannels();
    for (int idx = 0; idx < currentChannelsNum; idx++) {
        if (channels[idx]->isExpanded()) {
            currentCurves[BigPlotStatus::Iv][idx]->attach(plots[BigPlotStatus::Iv]);
        }
    }
    for (int idx = 0; idx < currentChannelsNum; idx++) {
        if (channels[idx]->isExpanded()) {
            currentCurves[BigPlotStatus::GapFree][idx]->attach(plots[BigPlotStatus::GapFree]);
            voltageCurves[BigPlotStatus::GapFree][idx]->attach(plots[BigPlotStatus::GapFree]);
        }
    }
    for (auto p: plots) {
        p->replot();
    }
}

void BigPlotController::onReplot() {
//    todo CHECK INTERNAL STATUS
    if (currentPlot != nullptr) {
        currentPlot->replot();
    }
}

void BigPlotController::onCurrentColorsChanged(QVector <QColor> colors) {
    for (int si=0; si < BigPlotStatus::NumberOfStatuses; si++) {
        for (int idx = 0; idx < currentChannelsNum; idx++) {
            currentCurves[si][idx]->setColor(colors[idx]);
            voltageCurves[si][idx]->setColor(colors[idx]);
        }
    }
}

void BigPlotController::onCurrentColorChanged(int channelIdx, QColor color) {
    for (int si=0; si < BigPlotStatus::NumberOfStatuses; si++) {
        for (int idx = 0; idx < currentChannelsNum; idx++) {
            currentCurves[si][channelIdx]->setColor(color);
            voltageCurves[si][channelIdx]->setColor(color);
        }
    }
}

void BigPlotController::onBackgroundColorChanged(QColor color) {
    currentPlot->setCanvasBackground(color);
}

void BigPlotController::handleZoomInRequest(Rect4 r){
//    non idale, rischio di incoerenza con le altre chiamate nel model
    currentModel->updateCurrentZoom(r);
    auto zoom = currentModel->getZoom(BigPlotModel::Zoom::Current);
    currentPlot->setRect(zoom);
    emit durationChanged({zoom[QwtPlot::xBottom].width(), currentModel->getCurrentRange(QwtPlot::xBottom).prefix, "s"});
}

void BigPlotController::handleSingleAxisZoomRequest(QwtPlot::Axis axis, int zoomIn, QPointF mousePosition){
//    non idale, rischio di incoerenza con le altre chiamate nel model
    currentModel->updateCurrentZoom(currentModel->zoomOnSingleAxis(axis, zoomIn, mousePosition));
    auto zoom = currentModel->getZoom(BigPlotModel::Zoom::Current);
    currentPlot->setRect(zoom);
    if (axis == QwtPlot::Axis::xBottom){
        emit durationChanged({zoom[QwtPlot::xBottom].width(), currentModel->getCurrentRange(QwtPlot::xBottom).prefix, "s"});
    }
}

void BigPlotController::handleSingleAxisShiftRequest(QwtPlot::Axis axis, int shift){
//    non idale, rischio di incoerenza con le altre chiamate nel model
    currentModel->updateCurrentZoom(currentModel->shiftOnSingleAxis(axis, shift));
    auto zoom = currentModel->getZoom(BigPlotModel::Zoom::Current);
    currentPlot->setRect(zoom);
}

void BigPlotController::handleZoomOutRequest(){
    auto zoom = currentModel->getZoom(BigPlotModel::Zoom::Previous);
    currentPlot->setRect(zoom);
    emit durationChanged({zoom[QwtPlot::xBottom].width(), currentModel->getCurrentRange(QwtPlot::xBottom).prefix, "s"});
}

void BigPlotController::handleZoomResetRequest(){
    auto zoom = currentModel->getZoom(BigPlotModel::Zoom::Default);
    currentPlot->setRect(zoom);
    emit durationChanged({zoom[QwtPlot::xBottom].width(), currentModel->getCurrentRange(QwtPlot::xBottom).prefix, "s"});
}

void BigPlotController::onRangeUpdated(commlib::RangedMeasurement_t newRange, QwtPlot::Axis axisIdx) {
    currentModel->setCurrentRange(axisIdx, newRange);
    currentPlot->setRect(currentModel->getZoom(BigPlotModel::Zoom::Current));
    auto fullUnit = QString::fromStdString(currentModel->getCurrentRange(axisIdx).getFullUnit());
    switch (axisIdx) {
    case QwtPlot::yLeft:
        currentPlot->setLabel(fullUnit, axisIdx);
        break;

    case QwtPlot::yRight:
        currentPlot->setLabel(fullUnit, axisIdx);
        break;

    case QwtPlot::xBottom:
        currentPlot->setLabel(fullUnit, axisIdx);
        Measurement_t duration = {currentModel->getZoom(BigPlotModel::Zoom::Current)[QwtPlot::xBottom].width(), currentModel->getCurrentRange(QwtPlot::xBottom).prefix, "s"};
        emit durationChanged(duration);
        break;
    }
    currentPlot->replot();
}

std::vector<PlotConsumer *> BigPlotController::getConsumers(){
    return consumers;
}

void BigPlotController::onExportIvGraph() {
    QString filePath = QFileDialog::getSaveFileName(nullptr,
                                   "Save File",
                                   QDir::homePath(), // Initial directory
                                   "CSV Files (*.csv)");

       // Check if a file path was selected
       if (!filePath.isEmpty()) {
           IvMessage ivMessage = std::get<1>(messages[BigPlotStatus::Iv]);
           // Save data to CSV file
           saveToCSV(filePath, ivMessage);
       } else {
           // No file path selected
           qDebug() << "No file path selected.";
       }

//    std::cout << "export" << std::endl;
}



void BigPlotController::saveToCSV(const QString& filePathssasda, const IvMessage & data) {
    for (int i=0; i<currentChannelsNum; i++) {
        auto filepath = filePathssasda.toStdString();

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
