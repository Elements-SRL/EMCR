#include "eventdetectioncontroller.h"
#include "eventdetectionwidget.h"
#include <QVector>
using namespace H5;

//const H5std_string GROUP_NAME("/events/");
void append_data(H5::DataSet dataset, const std::vector<int16_t>& data) {
    // Get the dataspace of the dataset
    H5::DataSpace dataspace = dataset.getSpace();
    // Get the number of dimensions in the dataspace
    int ndims = dataspace.getSimpleExtentNdims();
    // Get the size of each dimension
    std::vector<hsize_t> dims(ndims);
    dataspace.getSimpleExtentDims(dims.data(), NULL);
    auto writtenData = dims[0];
    int len = data.size();
    hsize_t dimsToWrite[RANK] = { len };
    hsize_t size[RANK] = { writtenData + len };
    hsize_t offset[RANK] = { writtenData };
    dataset.extend(size);
    H5::DataSpace fspace = dataset.getSpace();
    fspace.selectHyperslab(H5S_SELECT_SET, dimsToWrite, offset);
    H5::DataSpace mspace(RANK, dimsToWrite);
    dataset.write(data.data(), H5::PredType::STD_I16LE, mspace, fspace);
}

void pippo(H5::Group& parentGroup, const std::string eventName) {
    H5::Group group = parentGroup.createGroup(eventName);
}

void writeEvent(H5::Group &parentGroup, const Event& event, const std::string eventName) {
    try {
        //H5std_string groupName = eventName;
        H5::Group group = parentGroup.createGroup(eventName);
        hsize_t dims[RANK] = { 0 };  // dataset dimensions at creation
        hsize_t maxdims[RANK] = { H5S_UNLIMITED };
        DataSpace mspace(RANK, dims, maxdims);
        DSetCreatPropList cparms;
        hsize_t chunk_dims[RANK] = { CHUNK_SIZE };
        cparms.setChunk(RANK, chunk_dims);
        const H5std_string DATASET_NAME("Event");
        DataSet dataset = group.createDataSet(DATASET_NAME, PredType::STD_I16LE, mspace, cparms);
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
        append_data(dataset, event.event);
        group.close();
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
    widget = new EventDetectionWidget();
    bpw->setEventDetectionTab(widget);
    consumer = new EventDetectionConsumer(appStatus, producer);
    // creating curves for eventdetection
    //for (int i = 0; i < currentChannelsNum; i++) {
    //    currentCurves.push_back(new Curve(CurveType_t::CurveTypePlotSolid));
    //}
    connect(consumer, &PlotConsumer::setPlotData, this, &EventDetectionController::onSetPlotData);
    connect(consumer, &PlotConsumer::plotDataUpdated, this, &EventDetectionController::onReplot);
    consumer->forceAxisUpdate();
    consumer->setMaxSamplesPerPlot(4096);
    consumer->onSelectChannels(false);
    consumer->onStopConsuming();
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
    eventsInfo.clear();
}

void EventDetectionController::detachCurves() {
    for (auto curvesInChannel: eventCurves) {
        for (auto c : curvesInChannel.second) {
            c->detach();
        }
    }
    widget->getPlot()->replot();
}

void EventDetectionController::attachCurves() {
    //for (auto c : currentCurves) {
    //    c->attach(plot);
    //}
    //plot->replot();
}

void EventDetectionController::start() {
    if (!isAtLeastOneChannelExpanded()) {
        return;
    }
    attachCurves();
    consumer->onStartConsuming();
}

void EventDetectionController::stop() {
    consumer->onStopConsuming();
    detachCurves();
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
        consumer->onSelectChannels(flag);
        start();
    }
    else {
        consumer->onSelectChannels(flag);
    }
}

void EventDetectionController::onSetPlotData(PlotMessage plotmessage) {
    message = std::get<2>(plotmessage);
    auto plot = widget->getPlot();
    detachCurves();
    for (int i = 0; i < currentChannelsNum; i++) {
        eventCurves[i].clear();
    }
    eventsInfo = message.eventsInfo;
    for (const auto& pair : eventsInfo) {
        auto chIdx = pair.first;
        uint64_t acc = 0;
        const auto& eventsAndBaseline = pair.second;
        const auto& events = eventsAndBaseline.first;
        const auto& baseline = eventsAndBaseline.second;
        uint32_t len = events.size();
        for (int eventIdx = 0; eventIdx < events.size(); eventIdx++) {
            const auto e = events[eventIdx];
            const std::vector<int16_t> data = e.event;
            const auto resolution = events[eventIdx].resolution;
            acc += data.size();
            writeEvent(parentGroup, e, "e_"+std::to_string(eventCounter++));
            if (eventIdx < 10) {
                QwtPlotCurve* curve = new QwtPlotCurve();
                eventCurves[chIdx].push_back(curve);
                QVector<double> yData(data.size());
                std::copy(data.begin(), data.end(), yData.begin());
                QVector<double> xData;
                for (int i = 0; i < yData.size(); i++) {
                    yData[i] = ((double) data[i]) * resolution;
                    xData << i;
                }
                curve->setSamples(xData, yData);
                curve->attach(plot);
            }
        }
        if (len > 0) {
            widget->setAvgLen((double)((double) acc / (double) len / appStatus->getSamplingRate().value));
            widget->setNumberOfEvents(len);
        }
    }
    plot->show();
    plot->replot();
}

PlotConsumer* EventDetectionController::getConsumer() {
    return consumer;
}