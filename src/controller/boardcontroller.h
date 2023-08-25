#ifndef BOARDCONTROLLER_H
#define BOARDCONTROLLER_H

#include <QObject>

#include "messagedispatcher.h"
#include "boardcontroldockwidget.h"
#include "mainwindow.h"

class BoardController : public QObject {
    Q_OBJECT

public:
    BoardController(MessageDispatcher * msgDisp, MainWindow * mainWindow);
    ~BoardController();
private:
    MessageDispatcher * msgDisp = nullptr;
    BoardControlDockWidget * boardControlDockWidget = nullptr;

public slots:
    void onGateSourceVoltagesApplied(std::vector<uint16_t> gateVoltageBoardIndexes, std::vector<Measurement_t> gateVoltages, std::vector<uint16_t> sourceVoltageBoardIndexes, std::vector<Measurement_t> sourceVoltages);

signals:
    void sigGateSourceVoltagesApplied(std::vector<uint16_t> gateVoltageBoardIndexes, std::vector<Measurement_t> gateVoltages, std::vector<uint16_t> sourceVoltageBoardIndexes, std::vector<Measurement_t> sourceVoltages);
};

#endif // BOARDCONTROLLER_H
