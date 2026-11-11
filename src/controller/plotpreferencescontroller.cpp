#include "plotpreferencescontroller.h"

#include <QColorDialog>
#include "bigplotcontroller.h"
#include "chessboardcontroller.h"
#include "plotdetailcontroller.h"
#include "themecontroller.h"

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

    // On theme change the channels with a default color switch to the new
    // theme palette; the ones customized by the user are left unchanged
    connect(&ThemeController::getInstance(), &ThemeController::sigThemeUpdated, this, [=] () {
        if (!model->refreshDefaultColors()) {
            return;
        }
        QVector <QColor> colors = model->getColors();
        for (int idx = 0; idx < currentChannelsNum; idx++) {
            dialog->setColor(PlotPreferencesDialog::CurrentColor, idx, colors[idx]);
        }
        emit sigCurrentColorsChanged(colors);
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


void PlotPreferencesController::connectPlotDetailController(PlotDetailController* pdc) {
    connect(this, &PlotPreferencesController::sigCurrentColorsChanged, pdc, &PlotDetailController::onCurrentColorsChanged);
    connect(this, &PlotPreferencesController::sigCurrentColorChanged, pdc, &PlotDetailController::onCurrentColorChanged);
}

void PlotPreferencesController::connectChessboardController(ChessboardController* cc) {
    connect(this, &PlotPreferencesController::sigCurrentColorsChanged, cc, &ChessboardController::onCurrentColorsChanged);
    connect(this, &PlotPreferencesController::sigCurrentColorChanged, cc, &ChessboardController::onCurrentColorChanged);
}

void PlotPreferencesController::connectBigPlotController(BigPlotController* bpc) {
    connect(this, &PlotPreferencesController::sigCurrentColorsChanged, bpc, &BigPlotController::onCurrentColorsChanged);
    connect(this, &PlotPreferencesController::sigCurrentColorChanged, bpc, &BigPlotController::onCurrentColorChanged);
}
