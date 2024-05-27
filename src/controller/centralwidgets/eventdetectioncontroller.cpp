#include "eventdetectioncontroller.h"
#include "eventdetectionwidget.h"
#include <QVector>
using namespace H5;

//const H5std_string GROUP_NAME("/events/");
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
        H5::Attribute attr1 = dataset.createAttribute("Uom", strdatatype, attSpace);
        //const char* description = "This is a dataset of integers.";
        attr1.write(strdatatype, event.uom);
        // Create an integer attribute for the dataset
        H5::Attribute attr2 = dataset.createAttribute("Version", H5::PredType::NATIVE_INT, attSpace);
        int version = 1;
        attr2.write(H5::PredType::NATIVE_INT, &version);
        H5::Attribute attr3 = dataset.createAttribute("Current resoultion", H5::PredType::IEEE_F64LE, attSpace);
        attr3.write(H5::PredType::IEEE_F64LE, &event.resolution);
        H5::Attribute attr4 = dataset.createAttribute("Sample Offset", H5::PredType::NATIVE_UINT64, attSpace);
        attr4.write(H5::PredType::NATIVE_UINT64, &event.eventIdx);
        H5::Attribute attr5 = dataset.createAttribute("Stimulus", H5::PredType::IEEE_F64LE, attSpace);
        attr5.write(H5::PredType::IEEE_F64LE, &event.stimulus);
        H5::Attribute attr6 = dataset.createAttribute("Stimulus Uom", strdatatype, attSpace);
        //const char* description = "This is a dataset of integers.";
        attr6.write(strdatatype, event.stimulusUom);
        H5::Attribute attr7 = dataset.createAttribute("Sampling Rate", H5::PredType::IEEE_F64LE, attSpace);
        attr7.write(H5::PredType::IEEE_F64LE, &event.stimulus);
        H5::Attribute attr8 = dataset.createAttribute("Sampling Rate Uom", strdatatype, attSpace);
        //const char* description = "This is a dataset of integers.";
        attr8.write(strdatatype, event.samplingRateUom);
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

    consumer = new EventDetectionConsumer(appStatus, producer, minDurationInSeconds * noPrefVal, maxDurationInSeconds * noPrefVal);
    widget = new EventDetectionWidget(sr.getNoPrefixValue()/2.0, minDurationInSeconds, maxDurationInSeconds, durationBins, amplitudeBins, sr.getNoPrefixValue(), appStatus->getCurrentRange(), maxAmplitude);
    bpw->setEventDetectionTab(widget);
    connect(consumer, &PlotConsumer::setPlotData, this, &EventDetectionController::onSetPlotData);
    connect(consumer, &PlotConsumer::plotDataUpdated, this, &EventDetectionController::onReplot);
    consumer->forceAxisUpdate();
    consumer->setMaxSamplesPerPlot(4096);
    std::vector <uint16_t> allChannels(currentChannelsNum);
    for (int idx = 0; idx < currentChannelsNum; idx++) {
        allChannels[idx] = idx;
        eventCurves[idx] = new QwtPlotCurve();
    }
    consumer->onPlotChannels(allChannels, false);
    consumer->onStopConsuming();

    connect(widget, &EventDetectionWidget::startPressed, this, [=]() {consumer->onStartConsuming(); });
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

    try {
        /*
         * Turn off the auto-printing when failure occurs so that we can
         * handle the errors appropriately
         */
        //Exception::dontPrint();
        /*
        * Create the data space with unlimited dimensions.
        */
        hsize_t dims[RANK] = { 0 };  // dataset dimensions at creation
        hsize_t maxdims[RANK] = { H5S_UNLIMITED };
        H5::DataSpace mspace(RANK, dims, maxdims);
        /*
         * Create a new file. If file exists its contents will be overwritten.
         */
        H5::H5File file("Events.h5", H5F_ACC_TRUNC);
         /*
         * Modify dataset creation properties, i.e. enable chunking.
         */
        H5::DSetCreatPropList cparms;
        hsize_t chunk_dims[RANK] = { CHUNK_SIZE };
        cparms.setChunk(RANK, chunk_dims);
        parentGroup = file.createGroup("/events");
        baselineDataset = parentGroup.createDataSet("Baseline", PredType::STD_I16LE, mspace, cparms);
    }  // end of try block
// catch failure caused by the H5File operations
    catch (H5::FileIException error){
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
    //for (int idx = 0; idx < currentChannelsNum; idx++) {
    //    events[idx]->setColor(colors[idx]);
    //}
}

void EventDetectionController::onCurrentColorChanged(int channelIdx, QColor color) {
    //for (int idx = 0; idx < currentChannelsNum; idx++) {
    //    currentCurves[channelIdx]->setColor(color);
    //}
}

void EventDetectionController::onBackgroundColorChanged(QColor color) {
    //if (plot != nullptr) {
    //    plot->setCanvasBackground(color);
    //}
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
        const auto& baseline = eventPacket.baseline;
        append_data(baselineDataset, baseline.baseline);
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
            writeEvent(parentGroup, event, "e_"+std::to_string(eventCounter++));
            if (eventIdx == 0) {
                const auto & curve = eventCurves[chIdx];
                QVector<double> yData(data.size());
                QVector<double> xData;
                for (int i = 0; i < yData.size(); i++) {
                    yData[i] = ((double) data[i]) * resolution;
                    xData << i;
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
