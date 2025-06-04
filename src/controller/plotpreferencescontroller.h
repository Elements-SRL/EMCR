#ifndef PLOTPREFERENCESCONTROLLER_H
#define PLOTPREFERENCESCONTROLLER_H

#include <QObject>
#include "messagedispatcher.h"
#include "plotpreferencesdialog.h"
#include "plotpreferencesmodel.h"
#include "mainwindow.h"

class BigPlotController;
class ChessboardController;
class PlotDetailController;

class PlotPreferencesController : public QObject {
    Q_OBJECT

public:
    PlotPreferencesController(MessageDispatcher * msgDisp, MainWindow * mainWindow);
    ~PlotPreferencesController();

    void initializePlotColors();
    void connectPlotDetailController(PlotDetailController*);
    void connectChessboardController(ChessboardController*);
    void connectBigPlotController(BigPlotController*);

private:
    MessageDispatcher * msgDisp = nullptr;
    PlotPreferencesModel * model = nullptr;
    PlotPreferencesDialog * dialog = nullptr;
    MainWindow * mainWindow = nullptr;

    int voltageChannelsNum;
    int currentChannelsNum;
    int boardsNum;
    int channelsPerBoard;

signals:
    void sigCurrentColorChanged(int channelIdx, QColor color);
    void sigCurrentColorsChanged(QVector <QColor> colors);
    void sigBackgroundChanged(QColor color);
};

#endif // PLOTPREFERENCESCONTROLLER_H
