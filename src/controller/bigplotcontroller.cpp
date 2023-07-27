#include "bigplotcontroller.h"
#include <QDebug>

BigPlotController::BigPlotController(MessageDispatcher * msgDisp, MainWindow * mainWindow) :
    msgDisp(msgDisp),
    mainWindow(mainWindow) {
    bpw = new BigPlotWidget(msgDisp);
    bpm = new BigPlotModel();

    mainWindow->setBigPlotWidget(bpw);
    plot = bpw->getPlot();
    connect(plot, &BigPlot::zoomInRequest, this, &BigPlotController::handleZoomInRequest);
    connect(plot, &BigPlot::zoomOutRequest, this, &BigPlotController::handleZoomOutRequest);
    connect(plot, &BigPlot::zoomResetRequest, this, &BigPlotController::handleZoomResetRequest);
}

void BigPlotController::handleZoomInRequest(Rect4 r){
    bpm->pushZoomStack(plot->getRect());
    plot->setRect(r);
}

void BigPlotController::handleZoomOutRequest(){
    plot->setRect(bpm->popZoomStack());
}

void BigPlotController::handleZoomResetRequest(){
    plot->setRect(bpm->resetZoomStack());
}
