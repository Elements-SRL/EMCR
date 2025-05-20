#include "plotdetailcontroller.h"
#include "plotdetailmodel.h"

PlotDetailController::PlotDetailController(ApplicationStatus * appStatus, MainWindow* mainWindow, MultipleChannelController * mcc, ChessboardController * cc) :
    appStatus(appStatus), mainWindow(mainWindow) {
    connect(mcc, &MultipleChannelController::sigAddRemovePlotDetail, this, &PlotDetailController::plotDetailAction);
    connect(this, &PlotDetailController::addState, cc, &ChessboardController::onPlotDetailCreation);
    connect(this, &PlotDetailController::removeState, cc, &ChessboardController::onPlotDetailDeletion);
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
}

void PlotDetailController::manageCreation(){
    auto detailedPlots = appStatus->getDetailedPlots();
    for (auto &ch: detailedPlots) {
        // if not already created
        if(pds.find(ch) == pds.end()){
            auto pm = new PlotDetailModel(ch);
            auto pd = new PlotDetail(pm);
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
