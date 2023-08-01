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
            emit buttonClicked(idx);
        });

        currentButtonsLo->addWidget(button, row, col);
        if (++row >= channelsPerBoard) {
            row = 0;
            col++;
        }
    }

    QHBoxLayout * otherButtonsHl = new QHBoxLayout;

//    QPushButton * darkModeBtn = new QPushButton("Dark mode");
//    darkModeBtn->setCheckable(true);
//    darkModeBtn->setChecked(darkModeFlag);
//    otherButtonsHl->addWidget(darkModeBtn);

//    /*! Reset to defaults button */
//    QPushButton * restoreDefaultBtn = new QPushButton("Restore defaults");
//    restoreDefaultBtn->setCheckable(false);
//    otherButtonsHl->addWidget(restoreDefaultBtn);

    /*! Adds sub layouts in main layout */
    mainVl->addWidget(new QLabel("Current channels colors:"));
    mainVl->addLayout(currentButtonsLo);
    mainVl->addLayout(otherButtonsHl);
}

void PlotPreferencesDialog::setColor(SettingType_t type, int channelIdx, QColor color) {
    switch (type) {
    case CurrentColor:
        currentBtns[channelIdx]->setColor(color);
        break;

    case VoltageColor:
    case DarkMode:
        break;
    }
}
