#ifndef PLOTDETAILCONTROLLER_H
#define PLOTDETAILCONTROLLER_H

#include <QObject>
#include "plotdetail.h"
#include "multiplechannelcontroller.h"
#include "chessboardcontroller.h"

class PlotDetailController: public QObject {
    Q_OBJECT

private:
    std::map<uint16_t, PlotDetail *> pds;
    ApplicationStatus * appStatus;
    MainWindow* mainWindow;
    void manageCreation();
    void manageDeletion();

public:
    PlotDetailController(ApplicationStatus * appStatus, MainWindow* mainWindow, MultipleChannelController * mcc, ChessboardController * cc);

private slots:
    void plotDetailAction(bool);

signals:
    void removeState(std::vector<uint16_t>);
    void addState(std::vector<uint16_t>);
};

#endif // PLOTDETAILCONTROLLER_H
