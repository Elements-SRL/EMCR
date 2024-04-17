#ifndef COMPENSATIONCONTROLLER_H
#define COMPENSATIONCONTROLLER_H

#include <QObject>

#include "messagedispatcher.h"
#include "compensationcontroldockwidget.h"
#include "mainwindow.h"

class CompensationController : public QObject {
    Q_OBJECT

public:
    CompensationController(MessageDispatcher * msgDisp, MainWindow * mainWindow);
    CompensationControlDockWidget * getCompensationDockWidget();
    ~CompensationController();

private:
    MessageDispatcher * msgDisp = nullptr;
    MainWindow * mainWindow = nullptr;
    CompensationControlDockWidget * compensationControlDockWidget = nullptr;

private slots:

    // Compensations
    void onCompensationApplied(std::vector<uint16_t> channelIndexes, std::vector<bool> cfastEn, std::vector<bool> cslowRsEn, std::vector<bool> rsCpEn, std::vector<bool> rsPgEn, std::vector<double> cfastValues, std::vector<double> cslowValues, std::vector<double> rsValues, std::vector<double> rsCpValues, std::vector<double> rsPgValues, std::vector<uint16_t> rsBWValueIdxs, std::vector<bool> ccCfastEn, std::vector<double> ccCfastValues);
};
#endif // COMPENSATIONCONTROLLER_H
