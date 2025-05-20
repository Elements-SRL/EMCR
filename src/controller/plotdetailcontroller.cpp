#include "plotdetailcontroller.h"
#include "plotdetailmodel.h"

//todo: passare al controller direttamente il controller della chessboard e attaccarsi a quello
// per gestire signal/slot
PlotDetailController::PlotDetailController(ApplicationStatus * appStatus, MainWindow* mainWindow, MultipleChannelController * mcc, ChessboardController * cc) :
    appStatus(appStatus), mainWindow(mainWindow) {
    connect(mcc, &MultipleChannelController::sigAddRemovePlotDetail, this, &PlotDetailController::createPlotDetail);
    connect(this, &PlotDetailController::addState, cc, &ChessboardController::onPlotDetailCreation);
    connect(this, &PlotDetailController::removeState, cc, &ChessboardController::onPlotDetailDeletion);
}

std::map<uint16_t, bool> buildCoherentMap(std::vector<uint16_t> chs, bool flag) {
    std::map<uint16_t, bool> plotDetails;
    for (auto &s : chs) {
        plotDetails[s] = flag;
    }
    return plotDetails;
}

void PlotDetailController::createPlotDetail(bool flag) {
    const auto sChs = appStatus->getSelectedChannelsIndexes();
    const auto pds = buildCoherentMap(sChs, flag);
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
        if(pds.find(ch) == pds.end()){
            auto pm = new PlotDetailModel(ch);
            auto pd = new PlotDetail(pm);
            pd->show();
            pds[ch] = pd;
            connect(pd, &PlotDetail::close, this, [=]() {
                const auto upd = buildCoherentMap({pd->getChannel()}, false);
                appStatus->setDetailedPlots(upd);
                manageDeletion();
            });
        };
    }
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
    emit removeState(toRemove);
    for (auto &tr : toRemove) {
        pds.erase(tr);
    }
}
