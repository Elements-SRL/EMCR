#include "boardcontroller.h"


BoardController::BoardController(MessageDispatcher * msgDisp, MainWindow * mainWindow) :
    msgDisp(msgDisp),
    mainWindow(mainWindow) {

    if (msgDisp->hasGateVoltages() != Success && msgDisp->hasSourceVoltages() != Success) {
        return;
    }

    boardControlDockWidget = new BoardControlDockWidget(msgDisp);
    mainWindow->setBoardControlsDw(boardControlDockWidget);
    connect(boardControlDockWidget, &BoardControlDockWidget::sigGateSourceVoltagesApplied, this, [=](std::vector<uint16_t> gateVoltageBoardIndexes, std::vector<Measurement_t> gateVoltages, std::vector<uint16_t> sourceVoltageBoardIndexes, std::vector<Measurement_t> sourceVoltages){
        onGateSourceVoltagesApplied(gateVoltageBoardIndexes, gateVoltages, sourceVoltageBoardIndexes, sourceVoltages);
    });
}

BoardController::~BoardController(){
    delete boardControlDockWidget;
    boardControlDockWidget = nullptr;
    mainWindow->setBoardControlsDw(boardControlDockWidget);
}

void BoardController::onGateSourceVoltagesApplied(std::vector<uint16_t> gateVoltageBoardIndexes, std::vector<Measurement_t> gateVoltages, std::vector<uint16_t> sourceVoltageBoardIndexes, std::vector<Measurement_t> sourceVoltages){
    /*Set gate and source voltages in messageDispatcher*/
    msgDisp->setGateVoltages(gateVoltageBoardIndexes, gateVoltages, true);
    msgDisp->setSourceVoltages(sourceVoltageBoardIndexes, sourceVoltages, true);

    /*Set gate and source voltages in the model*/
    std::vector <BoardModel *> myBoards;
    msgDisp->getBoards(myBoards);
    for(int i = 0; i<gateVoltageBoardIndexes.size(); i++){
        myBoards[gateVoltageBoardIndexes[i]]->setGateVoltage(gateVoltages[i]);
    }

    for(int i = 0; i<sourceVoltageBoardIndexes.size(); i++){
        myBoards[sourceVoltageBoardIndexes[i]]->setSourceVoltage(sourceVoltages[i]);
    }

    emit sigGateSourceVoltagesApplied(gateVoltageBoardIndexes, gateVoltages, sourceVoltageBoardIndexes, sourceVoltages);
}
