#ifndef BIGPLOTCONTROLLER_H
#define BIGPLOTCONTROLLER_H

#include "bigplotwidget.h"
#include "mainwindow.h"
#include "messagedispatcher.h"
#include "bigplotmodel.h"

class BigPlotController : public QObject {
    Q_OBJECT
public:
    BigPlotController(MessageDispatcher * msgDisp, MainWindow * mainWindow);

private:
    MessageDispatcher * msgDisp = nullptr;
    MainWindow * mainWindow = nullptr;
    BigPlotWidget * bpw = nullptr;
    BigPlotModel * bpm= nullptr;
    BigPlot * plot;

private slots:
    void handleZoomInRequest(Rect4 r);
    void handleZoomOutRequest();
    void handleZoomResetRequest();
};

#endif // BIGPLOTCONTROLLER_H
