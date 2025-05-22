#include "plotdetailcontroller.h"
#include "plotdetailmodel.h"

PlotDetailController::PlotDetailController(ApplicationStatus * appStatus, Measurement_t defaultPlotDuration, MainWindow* mainWindow, MultipleChannelController * mcc, ChessboardController * cc, DeviceDataProducer* p) :
    ControllerWithConsumer(appStatus), mainWindow(mainWindow) {
    connect(mcc, &MultipleChannelController::sigAddRemovePlotDetail, this, &PlotDetailController::plotDetailAction);
    connect(this, &PlotDetailController::addState, cc, &ChessboardController::onPlotDetailCreation);
    connect(this, &PlotDetailController::removeState, cc, &ChessboardController::onPlotDetailDeletion);
    consumer = new GapFreePlotConsumer(appStatus, p);
    consumer->onDurationChanged(defaultPlotDuration);

    for (auto cm : appStatus->getChannels()) {
        pdms.push_back(new PlotDetailModel(cm->getId()));
    }
    connect(consumer, &PlotConsumer::setPlotData, this, &PlotDetailController::onSetPlotData);
    consumer->forceAxisUpdate();
    consumer->setMaxSamplesPerPlot(4096);
    connect(consumer, &PlotConsumer::plotDataUpdated, this, &PlotDetailController::onReplot);
}

// build a map where the keys are the vec entry and the values the flag
std::map<uint16_t, bool> buildCoherentMap(std::vector<uint16_t> chs, bool flag) {
    std::map<uint16_t, bool> plotDetails;
    for (auto &s : chs) {
        plotDetails[s] = flag;
    }
    return plotDetails;
}

void PlotDetailController::plotDetailAction(bool flag) {
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
    manageComsuner(flag);
}

void PlotDetailController::manageCreation(){
    auto detailedPlots = appStatus->getDetailedPlots();
    for (auto &ch: detailedPlots) {
        // if not already created
        if(pds.find(ch) == pds.end()){
            auto pd = new PlotDetail(pdms[ch]);
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
    emit addState(detailedPlots);
}

void PlotDetailController::manageDeletion(){
    auto detailedPlots = appStatus->getDetailedPlots();
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
    emit removeState(toRemove);
    for (auto &tr : toRemove) {
        pds.erase(tr);
    }
}

void PlotDetailController::manageComsuner(bool flag) {
    auto plotDetails = appStatus->getDetailedPlots();
    consumer->onStopConsuming();
    consumer->forceAxisUpdate();
    if (plotDetails.size()) {
        consumer->onStartConsuming();
    }
}

void PlotDetailController::onSetPlotData(PlotMessage plotmessage) {
    GapFreeMessage gapFreeMessage = std::get<BigPlot::BigPlotStatus::GapFree>(plotmessage);
    for (int i=0; i< pdms.size(); ++i) {
        pdms[i]->getCurve()->setRawSamples(gapFreeMessage.timeValues, gapFreeMessage.currentValues[i], gapFreeMessage.dataSize);
    }
}

void PlotDetailController::onReplot() {
    for (auto &p: pds) {
        p.second->replot();
    }
}

std::vector <DeviceDataConsumer*> PlotDetailController::getConsumers() {
    return {consumer};
}
