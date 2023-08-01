#ifndef PLOTPREFERENCESCONTROLLER_H
#define PLOTPREFERENCESCONTROLLER_H

#include <QObject>

#include "messagedispatcher.h"
#include "plotpreferencesdialog.h"
#include "plotpreferencesmodel.h"
#include "mainwindow.h"

class PlotPreferencesController : public QObject {
    Q_OBJECT

public:
    PlotPreferencesController(MessageDispatcher * msgDisp, MainWindow * mainWindow);
    ~PlotPreferencesController();

public slots:
    void onSelectChannels(bool flag);

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
    void sigSelectedColors(QVector <QColor> colors);
};

#endif // PLOTPREFERENCESCONTROLLER_H
