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
    qDebug() <<"tete";
}

void BigPlotController::handleZoomInRequest(Rect4 r){
    bpm->pushZoomStack(r);
    plot->setRect(r);
}

void BigPlotController::handleZoomOutRequest(){
    plot->setRect(bpm->popZoomStack());
}

void BigPlotController::handleZoomResetRequest(){
    qDebug() <<"culo";
    plot->setRect(bpm->resetZoomStack());
}
