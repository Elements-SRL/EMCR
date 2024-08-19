#include "eventdetectioncontroller.h"
#include "eventdetectionwidget.h"
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

H5::DataSet createBaseline(H5::Group& parentGroup, const std::string datasetName, Measurement sr) {
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
        const double srValue = sr.getNoPrefixValue();
        const double spValue = 1.0 / srValue;
        dataset.createAttribute("Sampling rate (Hz)", H5::PredType::IEEE_F64LE, attSpace).write(H5::PredType::IEEE_F64LE, &srValue);
        dataset.createAttribute("Sampling period (s)", H5::PredType::IEEE_F64LE, attSpace).write(H5::PredType::IEEE_F64LE, &spValue);
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
        dataset.createAttribute("Sample offset", H5::PredType::NATIVE_UINT64, attSpace).write(H5::PredType::NATIVE_UINT64, &event.eventIdx);
        dataset.createAttribute("Stimulus", H5::PredType::STD_I16LE, attSpace).write(H5::PredType::IEEE_F64LE, &event.stimulus);
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

std::tuple<std::optional<H5::DataSet>, std::optional<H5::DataSet>, std::optional<H5::Group>, std::optional<H5File>> createFile(ApplicationStatus* appStatus, std::string filename) {
    auto now = std::chrono::system_clock::now();
    // Convert to time_t which represents the time in seconds since epoch
    std::time_t currentTime = std::chrono::system_clock::to_time_t(now);
    // Convert to tm struct for local time
    std::tm* localTime = std::localtime(&currentTime);
    // Create a string stream to format the time
    std::ostringstream oss, oss_date_time;
    oss << std::put_time(localTime, "_%H_%M_%S");
    // Get the string from the string stream
    std::string timeStr = oss.str();
    filename += timeStr + ".h5";
    oss_date_time << std::put_time(localTime, "%Y-%m-%d %H:%M:%S");
    std::string dateTimeStr = oss_date_time.str();
    DataSpace attSpace(H5S_SCALAR);
    StrType strdatatype(0, H5T_VARIABLE);
    try {
        //Exception::dontPrint();
        //Create the data space with unlimited dimensions.
        hsize_t dims[RANK] = { 0 };  // dataset dimensions at creation
        hsize_t maxdims[RANK] = { H5S_UNLIMITED };
        H5::DataSpace mspace(RANK, dims, maxdims);
        // Create a new file. If file exists its contents will be overwritten.
        H5::H5File file(filename, H5F_ACC_TRUNC);
        uint16_t version = 1;
        std::string acq_mod = "Events";
        std::string device_info = appStatus->getDeviceInfoString();
        auto cms = appStatus->getClampingModalityString();
        auto sn = appStatus->getSerialNumber();
        auto software_name = GLB_SOFTWARE_NAME.toStdString();
        auto device_name = appStatus->getMessageDispatcher()->getDeviceName();
        file.createAttribute("Date time (Year-Month-Day Hour:Minute:Second)", strdatatype, attSpace).write(strdatatype, dateTimeStr);
        file.createAttribute("Version", H5::PredType::STD_I16LE, attSpace).write(H5::PredType::NATIVE_UINT16, &version);
        file.createAttribute("Acquisition modality", strdatatype, attSpace).write(strdatatype, acq_mod);
        file.createAttribute("Clamping modality", strdatatype, attSpace).write(strdatatype, cms);
        file.createAttribute("Device info", strdatatype, attSpace).write(strdatatype, device_info);
        file.createAttribute("Serial number", strdatatype, attSpace).write(strdatatype, sn);
        file.createAttribute("Acquisition software", strdatatype, attSpace).write(strdatatype, software_name);
        file.createAttribute("Device name", strdatatype, attSpace).write(strdatatype, device_name);
        
        //Modify dataset creation properties, i.e. enable chunking.
        H5::DSetCreatPropList cparms;
        hsize_t chunk_dims[RANK] = { CHUNK_SIZE };
        cparms.setChunk(RANK, chunk_dims);
        H5::Group electrophysiologyGroup = file.createGroup("/electrophysiology");
        H5::Group chGroup = electrophysiologyGroup.createGroup("/electrophysiology/ch_0");
        H5::Group baselineGroup = chGroup.createGroup("/electrophysiology/ch_0/Baseline");
        auto sr = appStatus->getSamplingRate();
        const auto srNoPref = sr.getNoPrefixValue();
        const auto period = 1.0 / srNoPref;
        auto cr = appStatus->getCurrentRange();
        auto crMultiplier = cr.multiplier();
        auto vr = appStatus->getVoltageRange();
        auto vrMultiplier = vr.multiplier();
        auto crUnit = cr.getFullUnit();
        baselineGroup.createAttribute("Current uom", strdatatype, attSpace).write(strdatatype, cr.getFullUnit());
        baselineGroup.createAttribute("Current resolution", H5::PredType::IEEE_F64LE, attSpace).write(H5::PredType::IEEE_F64LE, &cr.step);
        baselineGroup.createAttribute("Current multiplier", H5::PredType::IEEE_F64LE, attSpace).write(H5::PredType::IEEE_F64LE, &crMultiplier);
        baselineGroup.createAttribute("Voltage uom", strdatatype, attSpace).write(strdatatype, vr.getFullUnit());
        baselineGroup.createAttribute("Voltage resolution", H5::PredType::IEEE_F64LE, attSpace).write(H5::PredType::IEEE_F64LE, &vr.step);
        baselineGroup.createAttribute("Voltage multiplier", H5::PredType::IEEE_F64LE, attSpace).write(H5::PredType::IEEE_F64LE, &vrMultiplier);
        H5::Group eventsGroup = chGroup.createGroup("/electrophysiology/ch_0/Events");
        eventsGroup.createAttribute("Current uom", strdatatype, attSpace).write(strdatatype, cr.getFullUnit());
        eventsGroup.createAttribute("Current resolution", H5::PredType::IEEE_F64LE, attSpace).write(H5::PredType::IEEE_F64LE, &cr.step);
        eventsGroup.createAttribute("Current multiplier", H5::PredType::IEEE_F64LE, attSpace).write(H5::PredType::IEEE_F64LE, &crMultiplier);
        eventsGroup.createAttribute("Voltage uom", strdatatype, attSpace).write(strdatatype, vr.getFullUnit());
        eventsGroup.createAttribute("Voltage resolution", H5::PredType::IEEE_F64LE, attSpace).write(H5::PredType::IEEE_F64LE, &vr.step);
        eventsGroup.createAttribute("Voltage multiplier", H5::PredType::IEEE_F64LE, attSpace).write(H5::PredType::IEEE_F64LE, &vrMultiplier);
        eventsGroup.createAttribute("Sampling rate (Hz)", H5::PredType::IEEE_F64LE, attSpace).write(H5::PredType::IEEE_F64LE, &srNoPref);
        eventsGroup.createAttribute("Sampling period (s)", H5::PredType::IEEE_F64LE, attSpace).write(H5::PredType::IEEE_F64LE, &period);

        //TODO this could be a user parameter
        Measurement baselineSr = { 500.0, UnitPfx::UnitPfxNone, "Hz" };
        const auto iBaselineDataset = createBaseline(baselineGroup, "I", baselineSr);
        const auto vBaselineDataset = createBaseline(baselineGroup, "V", baselineSr);
        return std::make_tuple(iBaselineDataset, vBaselineDataset, eventsGroup, file);
    }  // end of try block
    // catch failure caused by the H5File operations
    catch (H5::FileIException error) {
        return std::make_tuple(std::nullopt, std::nullopt, std::nullopt, std::nullopt);
        //error.printErrorStack();
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
    minDuration = { 10.0, UnitPfx::UnitPfxMicro, "s" };
    maxDuration = { 500.0, UnitPfx::UnitPfxMicro, "s"};
    minAmplitude = 0.0;
    maxAmplitude = appStatus->getCurrentRange().getMax().value / 10;
    durationBinner = new Binner(minDuration.getNoPrefixValue(), maxDuration.getNoPrefixValue(), durationBins);
    amplitudeBinner = new Binner(minAmplitude, maxAmplitude, amplitudeBins);
    const auto highCutoffFrequency = sr.getNoPrefixValue() / 4.0;
    eventsDirection = EventsDirection::DOWN;
    const auto minSamples = minDuration.getNoPrefixValue() * sr.getNoPrefixValue();
    const auto maxSamples = maxDuration.getNoPrefixValue() * sr.getNoPrefixValue();

    consumer = new EventDetectionConsumer(appStatus, producer, minSamples, maxSamples, highCutoffFrequency, maxAmplitude, STD_MULTIPLIER, eventsDirection);
    widget = new EventDetectionWidget(sr.getNoPrefixValue()/2.0, minDuration, maxDuration, durationBins, amplitudeBins, highCutoffFrequency, appStatus->getCurrentRange(), maxAmplitude, STD_MULTIPLIER, eventsDirection);
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
        consumer->onStartConsuming();
        });
    //ADD FILE CLOSING OPERATION
    connect(widget, &EventDetectionWidget::stopPressed, this, [=]() {
        consumer->onStopConsuming();
        });
    connect(widget, &EventDetectionWidget::minDurationChanged, this, [=](Measurement m) {
        const auto wasThisRunning = consumer->isRunning();
        if (wasThisRunning) {
            consumer->onStopConsuming();
        }
        auto sr = appStatus->getSamplingRate();
        minDuration = m;
        const auto mNoPref = m.getNoPrefixValue();
        uint32_t durationInSamples = sr.getNoPrefixValue() * mNoPref;
        delete durationBinner;
        durationBinner = new Binner(mNoPref, maxDuration.getNoPrefixValue(), durationBins);
        resetStats();
        consumer->setMinEventDurationInSamples(durationInSamples);
        if (wasThisRunning) {
            consumer->onStartConsuming();
        }
        });
    connect(widget, &EventDetectionWidget::maxDurationChanged, this, [=](Measurement duration) {
        const auto wasThisRunning = consumer->isRunning();
        if (wasThisRunning) {
            consumer->onStopConsuming();
        }
        auto sr = appStatus->getSamplingRate();
        maxDuration = duration;
        const auto mNoPref = duration.getNoPrefixValue();
        uint32_t durationInSamples = sr.getNoPrefixValue() * mNoPref;
        delete durationBinner;
        durationBinner = new Binner(minDuration.getNoPrefixValue(), mNoPref, durationBins);
        resetStats();
        consumer->setMaxEventDurationInSamples(durationInSamples);
        if (wasThisRunning) {
            consumer->onStartConsuming();
        }
        });
    connect(widget, &EventDetectionWidget::durationBinsChanged, this, [=](int value) {
        const auto wasThisRunning = consumer->isRunning();
        if (wasThisRunning) {
            consumer->onStopConsuming();
        }
        delete durationBinner;
        durationBins = value;
        durationBinner = new Binner(minDuration.getNoPrefixValue(), maxDuration.getNoPrefixValue(), durationBins);
        if (wasThisRunning) {
            consumer->onStartConsuming();
        }
        });
    connect(widget, &EventDetectionWidget::amplitudeBinsChanged, this, [=](int value) {
        const auto wasThisRunning = consumer->isRunning();
        if (wasThisRunning) {
            consumer->onStopConsuming();
        }
        delete amplitudeBinner;
        amplitudeBins = value;
        amplitudeBinner = new Binner(minAmplitude, maxAmplitude, amplitudeBins);
        if (wasThisRunning) {
            consumer->onStartConsuming();
        }
        });
    connect(widget, &EventDetectionWidget::maxAmplitudeChanged, this, [=](double value) {
        const auto wasThisRunning = consumer->isRunning();
        if (wasThisRunning) {
            consumer->onStopConsuming();
        }
        delete amplitudeBinner;
        maxAmplitude = value;
        consumer->setMaxAmplitude(value);
        resetStats();
        amplitudeBinner = new Binner(minAmplitude, maxAmplitude, amplitudeBins);
        if (wasThisRunning) {
            consumer->onStartConsuming();
        }
        });
    connect(widget, &EventDetectionWidget::cutoffFrequencyChanged, this, [=](double value) {
        const auto wasThisRunning = consumer->isRunning();
        if (wasThisRunning) {
            consumer->onStopConsuming();
        }
        consumer->setHighCutoffFrequency(value);
        resetStats();
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
        resetStats();
        if (wasThisRunning) {
            consumer->onStartConsuming();
        }
        });
    connect(widget, &EventDetectionWidget::recordingStarted, this, [=]() {
        const auto wasThisRunning = consumer->isRunning();
        if (wasThisRunning) {
            consumer->onStopConsuming();
        }
        widget->setRecordingStatus(true);
        initHDF5();
        if (wasThisRunning) {
            consumer->onStartConsuming();
        }
        });
    connect(widget, &EventDetectionWidget::recordingStopped, this, [=]() {
        const auto wasThisRunning = consumer->isRunning();
        if (wasThisRunning) {
            consumer->onStopConsuming();
        }
        widget->setRecordingStatus(false);
        closeHDF5();
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
    //initHDF5();
    if (!isAtLeastOneChannelExpanded()) {
        return;
    }
    attachCurves(appStatus->getExpandedChannelsIndexes());
    consumer->onStartConsuming();
}

void EventDetectionController::stop() {
    consumer->onStopConsuming();
    closeHDF5();
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
//todo reset stats and reinit file
void EventDetectionController::onRangeUpdated(commlib::RangedMeasurement_t newRange) {
    if (newRange.unit == "A") {
        widget->setCurrentRange(newRange);
    }
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
    if (!file.has_value()) {
        return;
    }
    message = std::get<2>(plotmessage);
    auto plot = widget->getPlot();
    auto sr = appStatus->getSamplingRate();
    std::vector <uint16_t> allChannels(currentChannelsNum);
    for (int idx = 0; idx < currentChannelsNum; idx++) {
        allChannels[idx] = idx;
    }
    uint32_t eventDurationAcc = 0;
    uint32_t len;
    for (const auto& pair : message.eventPackets) {
        auto chIdx = pair.first;
        uint64_t acc = 0;
        const auto& eventPacket = pair.second;
        const auto& eventsInfo = eventPacket.eventsinfo;
        const auto& ib = eventPacket.iBaseline;
        const auto& iv = eventPacket.vBaseline;
        if (iBaselineDataset.has_value()) {
            append_data(iBaselineDataset.value(), ib.baseline);
        }
        if (vBaselineDataset.has_value()) {
            append_data(vBaselineDataset.value(), iv.baseline);
        }
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
            const auto resolution = appStatus->getCurrentRange().step;
            acc += data.size();
            if (eventsGroup.has_value()) {
                writeEvent(eventsGroup.value(), event, "e_" + std::to_string(eventCounter++));
            }
            if (eventIdx == 0) {
                const auto & curve = eventCurves[chIdx];
                //let's display only the middle part of the event and less baseline
                //The event is long n, n/5 is the actual length of the event while the other
                // (4/5)N are baseline, we'll take only 3/5 of n, in the middle part of course
                auto onlyEvent = (data.size() / 7);
                auto eventToDisplayLen = onlyEvent * 3;
                //There is 
                QVector<double> yData(eventToDisplayLen);
                QVector<double> xData;
                //rescaling x axis
                const auto noPrefSr = (1.0 / sr.getNoPrefixValue()) * 1e6;
                for (int i = 0; i < yData.size(); i++) {
                    yData[i] = ((double) data[(onlyEvent * 2) + i]) * resolution;
                    xData << i * noPrefSr;
                }
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
            const auto scaleFactor = 1.0 / maxDuration.multiplier();
            for (int i = 0; i < durationBinner->getNBins(); i++) {
                const auto k = keys[i];
                durationSamples.append(QPointF(keys[i] * scaleFactor, accs[i]));
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
    auto wasConsumerRunning = consumer->isRunning();
    //file reinitialization
    if (iBaselineDataset.has_value()) {
        iBaselineDataset.value().close();
    }
    if (vBaselineDataset.has_value()) {
        vBaselineDataset.value().close();
    }
    if (eventsGroup.has_value()) {
        eventsGroup.value().close();
    }
    consumer->onStopConsuming();
    const auto filepath = widget->getFilePath();
    std::string filepathEndingInBackslash = (filepath.back() == '\\') ? filepath : filepath + '\\';
    std::string filename = filepathEndingInBackslash + widget->getFileName();
    const auto baselineAndEvents = createFile(appStatus, filename);
    closeHDF5();
    iBaselineDataset = std::get<0>(baselineAndEvents);
    vBaselineDataset = std::get<1>(baselineAndEvents);
    eventsGroup = std::get<2>(baselineAndEvents);
    file = std::get<3>(baselineAndEvents);
    if (wasConsumerRunning) {
        consumer->onStartConsuming();
    }
}

void EventDetectionController::closeHDF5() {
    if (file.has_value()) {
        iBaselineDataset.value().close();
        vBaselineDataset.value().close();
        eventsGroup.value().close();
        file.value().close();
    }
}

void EventDetectionController::resetStats() {
    amplitudeBinner->clear();
    durationBinner->clear();
    totalEvents = 0;
    durationAccumulator = 0;
    amplitudeAccumulator = 0;
}

void EventDetectionController::onSamplingRateChanged(Measurement sr) {
    consumer->onSamplingRateChanged(sr);
    samplingRateChangedroutine(sr);
}

void EventDetectionController::onDownsamplingRatioChanged(uint32_t newRatio) {
    consumer->onDownsamplingRatioChanged(newRatio);
    auto sr = appStatus->getSamplingRate();
    samplingRateChangedroutine(sr / (double) newRatio);
}

void EventDetectionController::samplingRateChangedroutine(Measurement sr) {
    const auto wasThisRunning = consumer->isRunning();
    if (wasThisRunning) {
        consumer->onStopConsuming();
    }
    resetStats();
    const auto srHalf = sr.getNoPrefixValue() / 2.0;
    const auto newHighCutoffFreq = sr.getNoPrefixValue() / 4.0;
    widget->setMaxSamplingRate(srHalf);
    consumer->reinitFilters(newHighCutoffFreq);
    widget->setCutoffFrequency(newHighCutoffFreq);
    if (wasThisRunning) {
        consumer->onStartConsuming();
    }
}