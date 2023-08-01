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

    connect(dialog, &PlotPreferencesDialog::buttonClicked, this, [=] (int channelIdx) {
        QColor color = QColorDialog::getColor(model->getColor(channelIdx));
        if (color.isValid()) {
            model->setColor(PlotPreferencesDialog::CurrentColor, channelIdx, color);
            dialog->setColor(PlotPreferencesDialog::CurrentColor, channelIdx, color);
            emit sigCurrentColorChanged(channelIdx, color);
        }
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

void PlotPreferencesController::onSelectChannels(bool) {
    std::vector <ChannelModel *> channels;
    msgDisp->getChannels(channels);

    QVector <QColor> colors;
    for (int idx = 0; idx < (int)channels.size(); idx++) {
        if (channels[idx]->isExpanded()) {
            colors.push_back(model->getColor(idx));
        }
    }

    emit sigSelectedColors(colors);
}
