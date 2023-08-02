#include "plotpreferencescontroller.h"

#include <QColorDialog>

PlotPreferencesController::PlotPreferencesController(MessageDispatcher * msgDisp, MainWindow * mainWindow) :
    msgDisp(msgDisp),
    mainWindow(mainWindow) {

    msgDisp->getChannelNumberFeatures(voltageChannelsNum, currentChannelsNum);
    msgDisp->getBoardsNumberFeatures(boardsNum);
    channelsPerBoard = currentChannelsNum/boardsNum;

    model = new PlotPreferencesModel(currentChannelsNum);
    dialog = new PlotPreferencesDialog(currentChannelsNum, channelsPerBoard);

    for (int idx = 0; idx < currentChannelsNum; idx++) {
        dialog->setColor(PlotPreferencesDialog::CurrentColor, idx, model->getColor(idx));
    }

    mainWindow->setPlotPreferencesDialog(dialog);

    connect(dialog, &PlotPreferencesDialog::channelButtonClicked, this, [=] (int channelIdx) {
        QColor color = QColorDialog::getColor(model->getColor(channelIdx));
        if (color.isValid()) {
            model->setColor(PlotPreferencesDialog::CurrentColor, channelIdx, color);
            dialog->setColor(PlotPreferencesDialog::CurrentColor, channelIdx, color);
            emit sigCurrentColorChanged(channelIdx, color);
        }
    });

    connect(dialog, &PlotPreferencesDialog::restoreDefaultButtonClicked, this, [=] () {
        model->restoreDefaultColors();
        QVector <QColor> colors = model->getColors();
        for (int idx = 0; idx < currentChannelsNum; idx++) {
            dialog->setColor(PlotPreferencesDialog::CurrentColor, idx, colors[idx]);
        }
        emit sigCurrentColorsChanged(colors);
    });
}

PlotPreferencesController::~PlotPreferencesController() {
    if (model != nullptr) {
        delete model;
        model = nullptr;
    }

    if (dialog != nullptr) {
        delete dialog;
        dialog = nullptr;
    }
}

void PlotPreferencesController::initializePlotColors() {
    QVector <QColor> colors(currentChannelsNum);
    for (int idx = 0; idx < currentChannelsNum; idx++) {
        colors[idx] = model->getColor(idx);
    }

    emit sigCurrentColorsChanged(colors);
}
