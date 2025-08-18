#include "plotdetailcontroller.h"
#include "plotdetailmodel.h"
#include <cassert>

PlotDetailController::PlotDetailController(ApplicationStatus * appStatus, Measurement_t defaultPlotDuration, MainWindow* mainWindow, MultipleChannelController * mcc, ChessboardController * cc, DeviceDataProducer* p) :
    ControllerWithConsumer(appStatus), mainWindow(mainWindow) {
    connect(mcc, &MultipleChannelController::sigAddRemovePlotDetail, this, &PlotDetailController::onPlotDetailAction);
    connect(this, &PlotDetailController::sigAddState, cc, &ChessboardController::onPlotDetailCreation);
    connect(this, &PlotDetailController::sigRemoveState, cc, &ChessboardController::onPlotDetailDeletion);
    consumer = new GapFreePlotConsumer(appStatus, p);
    consumer->onDurationChanged(defaultPlotDuration);

    auto crs = appStatus->getCurrentRanges();
    for (int i = 0; i < crs.size(); i++) {
        pdms.push_back(new PlotDetailModel(i, crs[i]));
    }

    connect(consumer, &PlotConsumer::setPlotData, this, &PlotDetailController::onSetPlotData);
    consumer->forceAxisUpdate();
    consumer->setMaxSamplesPerPlot(4096);
    connect(consumer, &PlotConsumer::plotDataUpdated, this, &PlotDetailController::onReplot);
    connect(consumer, &PlotConsumer::endOfPlotReached, this, &PlotDetailController::onHandleEndOfPlot);
    connect(cc, &ChessboardController::sigAllChannelsClicked, this, [=](bool newChannelState) {
        if (appStatus->isPlotDetailAuto()) {
            onPlotDetailAction(newChannelState);
        }
    });
    connect(cc, &ChessboardController::sigOneBoardClicked, this, [=](uint16_t changedBoardIndex, bool newChannelState) {
        if (appStatus->isPlotDetailAuto()) {
            onPlotDetailAction(newChannelState);
        }    });
    connect(cc, &ChessboardController::sigOneRowClicked, this, [=](uint16_t changedRowIndex, bool newChannelState) {
        if (appStatus->isPlotDetailAuto()) {
            onPlotDetailAction(newChannelState);
        }    });
    connect(cc, &ChessboardController::sigSingleChannelClicked, this, [=](uint16_t changedChannelIndex, QMouseEvent * event) {
        bool newState = event->button() == Qt::LeftButton;
        if (appStatus->isPlotDetailAuto()) {
            onPlotDetailAction(newState);
        }
    });
}

PlotDetailController::~PlotDetailController() {
    consumer->onStopConsuming();
    if (consumer != nullptr) {
        delete consumer;
        consumer = nullptr;
    }
    appStatus->clearPlotDetails();
    manageDeletion();
}


// build a map where the keys are the vec entry and the values the flag
std::map<uint16_t, bool> buildCoherentMap(std::vector<uint16_t> chs, bool flag) {
    std::map<uint16_t, bool> plotDetails;
    for (auto &s : chs) {
        plotDetails[s] = flag;
    }
    return plotDetails;
}

void PlotDetailController::onPlotDetailAction(bool flag) {
    if (appStatus->isPlotDetailAuto()) {
        // clear the plot details to keep them consistent
        appStatus->clearPlotDetails();
        // delete current plot detail
        manageDeletion();
    }
    // get selected channels
    const auto sChs = appStatus->getSelectedChannelsIndexes();
    const auto pds = buildCoherentMap(sChs, flag);
    // update the plot details
    appStatus->setDetailedPlots(pds);
    if (flag) {
        manageCreation();
    } else {
        manageDeletion();
    }
    manageComsuner();
}

void PlotDetailController::manageCreation(){
    auto detailedPlots = appStatus->getDetailedPlotIndexes();
    for (auto &ch: detailedPlots) {
        // if not already created
        if(pds.find(ch) == pds.end()){
            auto pd = new PlotDetail(pdms[ch], mainWindow);
            pd->show();
            pds[ch] = pd;
            // manage deletion of the widget pressing x
            connect(pd, &PlotDetail::close, this, [=]() {
                const auto upd = buildCoherentMap({pd->getChannel()}, false);
                appStatus->setDetailedPlots(upd);
                manageDeletion();
            });
        };
    }
    // signal to chessboard that plots have been added
    emit sigAddState(detailedPlots);
}

void PlotDetailController::manageDeletion(){
    auto detailedPlots = appStatus->getDetailedPlotIndexes();
    std::vector<uint16_t> toRemove;
    for (auto &pair : pds) {
        if (std::find(detailedPlots.begin(), detailedPlots.end(), pair.first) == detailedPlots.end()) {
            toRemove.push_back(pair.first);
            pdms[pair.first]->getCurve()->detach();
            pair.second->deleteLater();
        }
    }
    appStatus->setDetailedPlots(buildCoherentMap(toRemove, false));
    // signal to chessboard that plots have been deleted
    emit sigRemoveState(toRemove);
    for (auto &tr : toRemove) {
        pds.erase(tr);
    }
}

void PlotDetailController::manageComsuner() {
    auto plotDetails = appStatus->getDetailedPlotIndexes();
    consumer->onStopConsuming();
    consumer->forceAxisUpdate();
    if (plotDetails.size()) {
        consumer->onStartConsuming();
    }
}

void PlotDetailController::onSetPlotData(PlotMessage plotmessage) {
    GapFreeMessage gapFreeMessage = std::get<GapFreeMessage>(plotmessage);
    for (int i=0; i< pdms.size(); ++i) {
        pdms[i]->getCurve()->setRawSamples(gapFreeMessage.timeValues, gapFreeMessage.currentValues[i], gapFreeMessage.dataSize);
    }
}

void PlotDetailController::onReplot() {
    for (auto &p: pds) {
        p.second->replot();
    }
}

void PlotDetailController::onCurrentRangeChanged(){
    auto crs = appStatus->getCurrentRanges();
    for (int i = 0; i < pdms.size(); i++) {
        pdms[i]->setCurrentRange(crs[i]);
    }
    for (auto pd : pds) {
        pd.second->updateLabel();
    }
}

std::vector <DeviceDataConsumer*> PlotDetailController::getConsumers() {
    return {consumer};
}

void PlotDetailController::onHandleEndOfPlot() {
    for (auto &pdm : pdms) {
        pdm->updateMargins();
    }

    for (auto &pair : pds) {
        pair.second->updatePlot();
    }
}

void PlotDetailController::onCurrentColorsChanged(QVector <QColor> colors) {
    assert(colors.size() == pdms.size());
    for (int i=0; i<colors.size(); i++) {
        pdms[i]->setCurveColor(colors[i]);
    }
}

void PlotDetailController::onCurrentColorChanged(int channelIdx, QColor c) {
    assert(channelIdx < pdms.size());
    pdms[channelIdx]->setCurveColor(c);
}

void PlotDetailController::onBackgroundColorChanged(QColor color) {
    for (auto &pd: pds) {
        pd.second->setBackgroundColor(color);
    }
}
