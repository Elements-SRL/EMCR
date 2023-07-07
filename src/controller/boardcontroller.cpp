#include "boardcontroller.h"


BoardController::BoardController(MessageDispatcher * msgDisp, MainWindow * mainWindow) :
    msgDisp(msgDisp) {

    boardControlDockWidget = new BoardControlDockWidget(msgDisp);
    this->mainWindow = mainWindow;
    mainWindow->setBoardControlsDw(boardControlDockWidget);
    connect(boardControlDockWidget, &BoardControlDockWidget::sigGateSourceVoltagesApplied, this, [=](std::vector<uint16_t> gateVoltageBoardIndexes, std::vector<Measurement_t> gateVoltages, std::vector<uint16_t> sourceVoltageBoardIndexes, std::vector<Measurement_t> sourceVoltages){
        onGateSourceVoltagesApplied(gateVoltageBoardIndexes, gateVoltages, sourceVoltageBoardIndexes, sourceVoltages);
    });
}

void BoardController::onGateSourceVoltagesApplied(std::vector<uint16_t> gateVoltageBoardIndexes, std::vector<Measurement_t> gateVoltages, std::vector<uint16_t> sourceVoltageBoardIndexes, std::vector<Measurement_t> sourceVoltages){
    /*Set gate and source voltages in messageDispatcher*/
    msgDisp->setGateVoltagesTuner(gateVoltageBoardIndexes, gateVoltages, true);
    msgDisp->setSourceVoltagesTuner(sourceVoltageBoardIndexes, sourceVoltages, true);

    /*Set gate and source voltages in the model*/
    std::vector <ModelBoard *> myBoards;
    msgDisp->getBoards(myBoards);
    for(int i = 0; i<gateVoltageBoardIndexes.size(); i++){
        myBoards[gateVoltageBoardIndexes[i]]->setGateVoltage(gateVoltages[i]);
    }

    for(int i = 0; i<sourceVoltageBoardIndexes.size(); i++){
        myBoards[sourceVoltageBoardIndexes[i]]->setSourceVoltage(sourceVoltages[i]);
    }

    emit sigGateSourceVoltagesApplied(gateVoltageBoardIndexes, gateVoltages, sourceVoltageBoardIndexes, sourceVoltages);
}
