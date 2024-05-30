#include "eventdetectioncontroller.h"
#include "eventdetectionwidget.h"
#include <QVector>
#include <iomanip>

using namespace H5;

void append_data(H5::DataSet& dataset, const std::vector<int16_t>& data) {
    try {
    // Get the dataspace of the dataset
    H5::DataSpace dataspace = dataset.getSpace();
    // Get the number of dimensions in the dataspace
    int ndims = dataspace.getSimpleExtentNdims();
    // Get the size of each dimension
    std::vector<hsize_t> dims(ndims);
    dataspace.getSimpleExtentDims(dims.data(), NULL);
    auto writtenData = dims[0];
    hsize_t len = data.size();
    hsize_t dimsToWrite[RANK] = { len };
    hsize_t size[RANK] = { writtenData + len };
    hsize_t offset[RANK] = { writtenData };
    dataset.extend(size);
    H5::DataSpace fspace = dataset.getSpace();
    fspace.selectHyperslab(H5S_SELECT_SET, dimsToWrite, offset);
    H5::DataSpace mspace(RANK, dimsToWrite);
    dataset.write(data.data(), H5::PredType::STD_I16LE, mspace, fspace);
    }  // end of try block
    catch (H5::GroupIException& error) {
        error.printErrorStack();
        // Handle the exception
    }
    // catch failure caused by the H5File operations
    catch (H5::FileIException error) {
        error.printErrorStack();
    }
    // catch failure caused by the DataSet operations
    catch (H5::DataSetIException error) {
        error.printErrorStack();
    }
    // catch failure caused by the DataSpace operations
    catch (H5::DataSpaceIException error) {
        error.printErrorStack();
    }
    // catch failure caused by the DataSpace operations
    catch (H5::DataTypeIException error) {
        error.printErrorStack();
    }
}

H5::DataSet createBaseline(H5::Group& parentGroup, const std::string datasetName, RangedMeasurement rm, Measurement sr) {
    try {
        //H5std_string groupName = eventName;
        //H5::Group group = parentGroup.createGroup(eventName);
        hsize_t dims[RANK] = { 0 };  // dataset dimensions at creation
        hsize_t maxdims[RANK] = { H5S_UNLIMITED };
        DataSpace mspace(RANK, dims, maxdims);
        DSetCreatPropList cparms;
        hsize_t chunk_dims[RANK] = { CHUNK_SIZE };
        cparms.setChunk(RANK, chunk_dims);
        DataSet dataset = parentGroup.createDataSet(datasetName, PredType::STD_I16LE, mspace, cparms);
        DataSpace attSpace(H5S_SCALAR);
        StrType strdatatype(0, H5T_VARIABLE);
        const double multiplier = rm.multiplier();
        const double srValue = sr.getNoPrefixValue();
        dataset.createAttribute("Uom", strdatatype, attSpace).write(strdatatype, rm.getFullUnit());
        dataset.createAttribute("Resoultion", H5::PredType::IEEE_F64LE, attSpace).write(H5::PredType::IEEE_F64LE, &rm.step);
        dataset.createAttribute("Multiplier", H5::PredType::IEEE_F64LE, attSpace).write(H5::PredType::IEEE_F64LE, &multiplier);
        dataset.createAttribute("Sampling rate", H5::PredType::IEEE_F64LE, attSpace).write(H5::PredType::IEEE_F64LE, &srValue);
        return dataset;
    }  // end of try block
    catch (H5::GroupIException& error) {
        error.printErrorStack();
        // Handle the exception
    }
    // catch failure caused by the H5File operations
    catch (H5::FileIException error) {
        error.printErrorStack();
    }
    // catch failure caused by the DataSet operations
    catch (H5::DataSetIException error) {
        error.printErrorStack();
    }
    // catch failure caused by the DataSpace operations
    catch (H5::DataSpaceIException error) {
        error.printErrorStack();
    }
    // catch failure caused by the DataSpace operations
    catch (H5::DataTypeIException error) {
        error.printErrorStack();
    }
}

void writeEvent(H5::Group &parentGroup, const Event& event, const std::string eventName) {
    try {
        //H5std_string groupName = eventName;
        //H5::Group group = parentGroup.createGroup(eventName);
        hsize_t dims[RANK] = { 0 };  // dataset dimensions at creation
        hsize_t maxdims[RANK] = { H5S_UNLIMITED };
        DataSpace mspace(RANK, dims, maxdims);
        DSetCreatPropList cparms;
        hsize_t chunk_dims[RANK] = { CHUNK_SIZE };
        cparms.setChunk(RANK, chunk_dims);
        DataSet dataset = parentGroup.createDataSet(eventName, PredType::STD_I16LE, mspace, cparms);
        DataSpace attSpace(H5S_SCALAR);
        StrType strdatatype(0, H5T_VARIABLE);
        // Create an integer attribute for the dataset
        dataset.createAttribute("Resoultion", H5::PredType::IEEE_F64LE, attSpace).write(H5::PredType::IEEE_F64LE, &event.resolution);
        dataset.createAttribute("Multiplier", H5::PredType::IEEE_F64LE, attSpace).write(H5::PredType::IEEE_F64LE, &event.multiplier);
        dataset.createAttribute("Uom", strdatatype, attSpace).write(strdatatype, event.uom);
        dataset.createAttribute("Sample offset", H5::PredType::NATIVE_UINT64, attSpace).write(H5::PredType::NATIVE_UINT64, &event.eventIdx);
        dataset.createAttribute("Stimulus", H5::PredType::IEEE_F64LE, attSpace).write(H5::PredType::IEEE_F64LE, &event.stimulus);
        dataset.createAttribute("Stimulus uom", strdatatype, attSpace).write(strdatatype, event.stimulusUom);
        dataset.createAttribute("Stimulus multiplier", H5::PredType::IEEE_F64LE, attSpace).write(H5::PredType::IEEE_F64LE, &event.stimulusMultiplier);
        dataset.createAttribute("Sampling rate", H5::PredType::IEEE_F64LE, attSpace).write(H5::PredType::IEEE_F64LE, &event.samplingRate);
        dataset.createAttribute("Sampling rate uom", strdatatype, attSpace).write(strdatatype, event.samplingRateUom);
        append_data(dataset, event.rawData);
    }  // end of try block
    catch (H5::GroupIException& error) {
        error.printErrorStack();
        // Handle the exception
    }
    // catch failure caused by the H5File operations
    catch (H5::FileIException error) {
        error.printErrorStack();
    }
    // catch failure caused by the DataSet operations
    catch (H5::DataSetIException error) {
        error.printErrorStack();
    }
    // catch failure caused by the DataSpace operations
    catch (H5::DataSpaceIException error) {
        error.printErrorStack();
    }
    // catch failure caused by the DataSpace operations
    catch (H5::DataTypeIException error) {
        error.printErrorStack();
    }
}

std::tuple<H5::DataSet, H5::DataSet, H5::Group> createFile(ApplicationStatus* appStatus) {
    auto now = std::chrono::system_clock::now();
    // Convert to time_t which represents the time in seconds since epoch
    std::time_t currentTime = std::chrono::system_clock::to_time_t(now);
    // Convert to tm struct for local time
    std::tm* localTime = std::localtime(&currentTime);
    // Create a string stream to format the time
    std::ostringstream oss;
    oss << std::put_time(localTime, "_%H_%M_%S");
    // Get the string from the string stream
    std::string timeStr = oss.str();
    std::string filename = "Events" + timeStr + ".h5";
    try {
        //Exception::dontPrint();
        //Create the data space with unlimited dimensions.
        hsize_t dims[RANK] = { 0 };  // dataset dimensions at creation
        hsize_t maxdims[RANK] = { H5S_UNLIMITED };
        H5::DataSpace mspace(RANK, dims, maxdims);
        // Create a new file. If file exists its contents will be overwritten.
        H5::H5File file(filename, H5F_ACC_TRUNC);
        //Modify dataset creation properties, i.e. enable chunking.
        H5::DSetCreatPropList cparms;
        hsize_t chunk_dims[RANK] = { CHUNK_SIZE };
        cparms.setChunk(RANK, chunk_dims);
        H5::Group chGroup = file.createGroup("/ch_0");
        H5::Group baselineGroup = chGroup.createGroup("Baseline");
        H5::Group eventsGroup = chGroup.createGroup("Events");
        Measurement baselineSr = { 500.0, UnitPfx::UnitPfxNone, "Hz" };
        const auto iBaselineDataset = createBaseline(baselineGroup, "I", appStatus->getCurrentRange(), baselineSr);
        const auto vBaselineDataset = createBaseline(baselineGroup, "V", appStatus->getVoltageRange(), baselineSr);
        return std::make_tuple(iBaselineDataset, vBaselineDataset, eventsGroup);
    }  // end of try block
    // catch failure caused by the H5File operations
    catch (H5::FileIException error) {
        error.printErrorStack();
    }
    // catch failure caused by the DataSet operations
    catch (H5::DataSetIException error) {
        error.printErrorStack();
    }
    // catch failure caused by the DataSpace operations
    catch (H5::DataSpaceIException error) {
        error.printErrorStack();
    }
    // catch failure caused by the DataSpace operations
    catch (H5::DataTypeIException error) {
        error.printErrorStack();
    }
}

EventDetectionController::EventDetectionController(ApplicationStatus* appStatus, DeviceDataProducer* producer, BigPlotWidget* bpw) :
    CentralWidgetController(appStatus, producer, bigPlotWidget) {
    //TODO THOSE NEEDS TO BE 
    auto sr = appStatus->getSamplingRate();
    auto noPrefVal = sr.getNoPrefixValue();
    minDurationInSeconds = 80.0 / noPrefVal;
    maxDurationInSeconds = 8000.0 / noPrefVal;
    minAmplitude = 0.0;
    maxAmplitude = appStatus->getCurrentRange().getMax().value / 10;
    durationBinner = new Binner(minDurationInSeconds, maxDurationInSeconds, durationBins);
    amplitudeBinner = new Binner(minAmplitude, maxAmplitude, amplitudeBins);
    const auto highCutoffFrequency = sr.getNoPrefixValue() / 4.0;
    consumer = new EventDetectionConsumer(appStatus, producer, minDurationInSeconds * noPrefVal, maxDurationInSeconds * noPrefVal, highCutoffFrequency, STD_MULTIPLIER);
    widget = new EventDetectionWidget(sr.getNoPrefixValue()/2.0, minDurationInSeconds, maxDurationInSeconds, durationBins, amplitudeBins, highCutoffFrequency, appStatus->getCurrentRange(), maxAmplitude, STD_MULTIPLIER);
    bpw->setEventDetectionTab(widget);
    connect(consumer, &PlotConsumer::setPlotData, this, &EventDetectionController::onSetPlotData);
    connect(consumer, &PlotConsumer::plotDataUpdated, this, &EventDetectionController::onReplot);
    consumer->forceAxisUpdate();
    consumer->setMaxSamplesPerPlot(4096);
    std::vector <uint16_t> allChannels(currentChannelsNum);
    for (int idx = 0; idx < currentChannelsNum; idx++) {
        allChannels[idx] = idx;
        eventCurves[idx] = new Curve();
    }
    consumer->onPlotChannels(allChannels, false);
    consumer->onStopConsuming();

    connect(widget, &EventDetectionWidget::startPressed, this, [=]() {
        initHDF5();
        consumer->onStartConsuming();
        });
    connect(widget, &EventDetectionWidget::stopPressed, this, [=]() {consumer->onStopConsuming(); });
    connect(widget, &EventDetectionWidget::minDurationChanged, this, [=](double value) {
        const auto wasThisRunning = consumer->isRunning();
        if (wasThisRunning) {
            consumer->onStopConsuming();
        }
        auto sr = appStatus->getSamplingRate();
        minDurationInSeconds = value;
        uint32_t durationInSamples = sr.getNoPrefixValue() * value;
        delete durationBinner;
        durationBinner = new Binner(minDurationInSeconds, maxDurationInSeconds, durationBins);
        totalEvents = 0;
        durationAccumulator = 0;
        amplitudeAccumulator = 0;
        consumer->setMinEventDurationInSamples(durationInSamples);
        if (wasThisRunning) {
            consumer->onStartConsuming();
        }
        });
    connect(widget, &EventDetectionWidget::maxDurationChanged, this, [=](double value) {
        const auto wasThisRunning = consumer->isRunning();
        if (wasThisRunning) {
            consumer->onStopConsuming();
        }
        auto sr = appStatus->getSamplingRate();
        maxDurationInSeconds = value;
        uint32_t durationInSamples = sr.getNoPrefixValue() * value;
        delete durationBinner;
        durationBinner = new Binner(minDurationInSeconds, maxDurationInSeconds, durationBins);
        totalEvents = 0;
        durationAccumulator = 0;
        amplitudeAccumulator = 0;
        consumer->setMaxEventDurationInSamples(durationInSamples);
        if (wasThisRunning) {
            consumer->onStartConsuming();
        }
        });
    connect(widget, &EventDetectionWidget::durationBinsChanged, this, [=](int value) {
        delete durationBinner;
        durationBins = value;
        durationBinner = new Binner(minDurationInSeconds, maxDurationInSeconds, durationBins);
        });
    connect(widget, &EventDetectionWidget::amplitudeBinsChanged, this, [=](int value) {
        delete amplitudeBinner;
        amplitudeBins = value;
        amplitudeBinner = new Binner(minAmplitude, maxAmplitude, amplitudeBins);
        });
    connect(widget, &EventDetectionWidget::maxAmplitudeChanged, this, [=](double value) {
        delete amplitudeBinner;
        maxAmplitude = value;
        amplitudeBinner = new Binner(minAmplitude, maxAmplitude, amplitudeBins);
        });
    connect(widget, &EventDetectionWidget::cutoffFrequencyChanged, this, [=](double value) {
        const auto wasThisRunning = consumer->isRunning();
        if (wasThisRunning) {
            consumer->onStopConsuming();
        }
        consumer->setHighCutoffFrequency(value);
        if (wasThisRunning) {
            consumer->onStartConsuming();
        }
        });
    connect(widget, &EventDetectionWidget::stdMultiplierChanged, this, [=](double value) {
        const auto wasThisRunning = consumer->isRunning();
        if (wasThisRunning) {
            consumer->onStopConsuming();
        }
        consumer->setStdMultiplier(value);
        if (wasThisRunning) {
            consumer->onStartConsuming();
        }
        });
}

EventDetectionController::~EventDetectionController() {
    if (consumer != nullptr) {
        delete consumer;
        consumer = nullptr;
    }
    //for (auto eventsInChannel : events) {
    //    eventsInChannel.clear();
    //}
    eventPackets.clear();
}

void EventDetectionController::detachCurves(const std::vector <uint16_t>& channelIndexes) {
    for (const auto& curvesInChannel: eventCurves) {
        curvesInChannel.second->detach();
    }
    widget->getPlot()->replot();
}

void EventDetectionController::attachCurves(const std::vector <uint16_t>& channelIndexes) {
    auto plot = widget->getPlot();
    for (const auto& curvesInChannel : eventCurves) {
        curvesInChannel.second->attach(plot);
    }
    plot->replot();
}

void EventDetectionController::start() {
    initHDF5();
    if (!isAtLeastOneChannelExpanded()) {
        return;
    }
    attachCurves(appStatus->getExpandedChannelsIndexes());
    consumer->onStartConsuming();
}

void EventDetectionController::stop() {
    consumer->onStopConsuming();
    std::vector <uint16_t> allChannels(currentChannelsNum);
    for (int idx = 0; idx < currentChannelsNum; idx++) {
        allChannels[idx] = idx;
    }
    detachCurves(allChannels);
}

void EventDetectionController::onCurrentColorsChanged(QVector <QColor> colors) {
    for (int idx = 0; idx < currentChannelsNum; idx++) {
        eventCurves[idx]->setColor(colors[idx]);
    }
}

void EventDetectionController::onCurrentColorChanged(int channelIdx, QColor color) {
    eventCurves[channelIdx]->setColor(color);
}

void EventDetectionController::onBackgroundColorChanged(QColor color) {
    const auto& plot = widget->getPlot();
    if (plot != nullptr) {
        plot->setCanvasBackground(color);
    }
}

void EventDetectionController::onReplot() {
    //if (plot != nullptr) {
    //    plot->replot();
    //}
}

//todo Check clamping modality too
void EventDetectionController::onRangeUpdated(commlib::RangedMeasurement_t newRange) {
    //QwtPlot::Axis axisIdx;
    //if (newRange.unit == "s") {
    //    axisIdx = QwtPlot::xBottom;
    //    model->setCurrentRange(axisIdx, newRange);
    //    Measurement_t duration = { model->getZoom(BigPlotModel::Zoom::Current)[axisIdx].width(), model->getCurrentRange(axisIdx).prefix, "s" };
    //    emit durationChanged(duration);

    //}
    //else if (newRange.unit == "V") {
    //    axisIdx = QwtPlot::yRight;
    //    model->setCurrentRange(axisIdx, newRange);

    //}
    //else if (newRange.unit == "A") {
    //    axisIdx = QwtPlot::yLeft;
    //    model->setCurrentRange(axisIdx, newRange);
    //}
    //plot->setRect(model->getZoom(BigPlotModel::Zoom::Current));
    //plot->setLabel(QString::fromStdString(model->getCurrentRange(axisIdx).getFullUnit()), axisIdx);
    //plot->replot();
}

void EventDetectionController::onExpandTrace(bool flag) {
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

void EventDetectionController::onSetPlotData(PlotMessage plotmessage) {
    message = std::get<2>(plotmessage);
    auto plot = widget->getPlot();
    auto sr = appStatus->getSamplingRate();
    std::vector <uint16_t> allChannels(currentChannelsNum);
    for (int idx = 0; idx < currentChannelsNum; idx++) {
        allChannels[idx] = idx;
    }
    //detachCurves(allChannels);
    uint32_t eventDurationAcc = 0;
    uint32_t len;
    for (const auto& pair : message.eventPackets) {
        auto chIdx = pair.first;
        uint64_t acc = 0;
        const auto& eventPacket = pair.second;
        const auto& eventsInfo = eventPacket.eventsinfo;
        const auto& ib = eventPacket.iBaseline;
        const auto& iv = eventPacket.vBaseline;
        append_data(iBaselineDataset, ib.baseline);
        append_data(vBaselineDataset, iv.baseline);
        len = eventsInfo.size();
        totalEvents += len;
        for (int eventIdx = 0; eventIdx < eventsInfo.size(); eventIdx++) {
            const auto& ei = eventsInfo[eventIdx];
            const auto& event = ei.event;
            durationAccumulator += ei.duration;
            durationBinner->put(ei.duration);
            amplitudeBinner->put(ei.amplitude);
            amplitudeAccumulator += ei.amplitude;
            const std::vector<int16_t>& data = event.rawData;
            eventDurationAcc += data.size();
            const auto resolution = event.resolution;
            acc += data.size();
            writeEvent(eventsGroup, event, "e_"+std::to_string(eventCounter++));
            if (eventIdx == 0) {
                const auto & curve = eventCurves[chIdx];
                QVector<double> yData(data.size());
                QVector<double> xData;
                const auto noPrefSr = 1.0 / sr.getNoPrefixValue();
                for (int i = 0; i < yData.size(); i++) {
                    yData[i] = ((double) data[i]) * resolution;
                    xData << i * noPrefSr;
                }
                //NOT PLOTTING DATA ANYMORE
                curve->setSamples(xData, yData);
                plot->show();
                plot->replot();
            }
        }
        const auto ciccia = ((double)len) * (appStatus->getSamplingRate().getNoPrefixValue()/((double)durationAccumulator));
        QVector<QPointF> durationSamples; 
        {
            const auto& keys = durationBinner->getKeys();
            const auto& accs = durationBinner->getValues();
            for (int i = 0; i < durationBinner->getNBins(); i++) {
                const auto k = keys[i];
                durationSamples.append(QPointF(keys[i], accs[i]));
            }
        }

        QVector<QPointF> amplitudeSamples; 
        const auto& keys = amplitudeBinner->getKeys();
        {
            const auto& accs = amplitudeBinner->getValues();

            for (int i = 0; i < amplitudeBinner->getNBins(); i++) {
                const auto k = keys[i];
                amplitudeSamples.append(QPointF(keys[i], accs[i]));
            }
        }

        if (len > 0) {
            widget->setAvgLen(durationAccumulator / ((double) totalEvents));
            widget->setEventsPerSecond(eventPacket.eventPerSecond);
            widget->setTotalNumberOfEvents(totalEvents);
            widget->setAvgAmplitude(amplitudeAccumulator / ((double)totalEvents));
            widget->setDurationData(durationSamples);
            widget->setAmplitudeData(amplitudeSamples);
        }
    }
}

PlotConsumer* EventDetectionController::getConsumer() {
    return consumer;
}

void EventDetectionController::initHDF5() {
    const auto baselineAndEvents = createFile(appStatus);
    iBaselineDataset= std::get<0>(baselineAndEvents);
    vBaselineDataset= std::get<1>(baselineAndEvents);
    eventsGroup = std::get<2>(baselineAndEvents);
}
