#include "plotpreferencesdialog.h"

#include <QBoxLayout>
#include <QLabel>

PlotPreferencesDialog::PlotPreferencesDialog(int channelsNum, int channelsPerBoard, QWidget * parent) :
    QDialog(parent) {

    QVBoxLayout * mainVl = new QVBoxLayout;
    this->setLayout(mainVl);

    /*! Buttons for selecting current channels colors */
    QGridLayout * currentButtonsLo = new QGridLayout();

    int row = 0;
    int col = 0;

    for (int idx = 0; idx < channelsNum; idx++) {
        ColorSelectionButton * button = new ColorSelectionButton(nullptr);
        button->setText(QString("%1").arg(idx+1));
        currentBtns.push_back(button);
        connect(button, &QPushButton::clicked, this, [=] () {
            emit channelButtonClicked(idx);
        });

        currentButtonsLo->addWidget(button, row, col);
        if (++row >= channelsPerBoard) {
            row = 0;
            col++;
        }
    }

    QHBoxLayout * otherButtonsHl = new QHBoxLayout;

    darkModeBtn = new QPushButton("Dark mode");
    darkModeBtn->setCheckable(true);
    otherButtonsHl->addWidget(darkModeBtn);

    connect(darkModeBtn, &QPushButton::clicked, this, &PlotPreferencesDialog::darkModeButtonClicked);

    /*! Reset to defaults button */
    QPushButton * restoreDefaultBtn = new QPushButton("Restore defaults");
    restoreDefaultBtn->setCheckable(false);
    otherButtonsHl->addWidget(restoreDefaultBtn);

    connect(restoreDefaultBtn, &QPushButton::clicked, this, &PlotPreferencesDialog::restoreDefaultButtonClicked);

    /*! Adds sub layouts in main layout */
    mainVl->addWidget(new QLabel("Current channels colors:"));
    mainVl->addLayout(currentButtonsLo);
}

void PlotPreferencesDialog::setColor(SettingType_t type, int channelIdx, QColor color) {
    switch (type) {
    case CurrentColor: {
        currentBtns[channelIdx]->setColor(color);
        currentBtns[channelIdx]->setStyleSheet(QString("background: %1;").arg(color.name()));
        break;
    }
    case VoltageColor:
    case DarkMode:
        break;
    }
}

void PlotPreferencesDialog::setDarkMode(bool flag) {
    darkModeBtn->setChecked(flag);
}
