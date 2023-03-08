#include "channelcontroldockwidget.h"

#include <QBoxLayout>

ChannelControlDockWidget::ChannelControlDockWidget(ModelDevice * mDev, QWidget * parent) :
    QDockWidget(parent),
    mDev(mDev) {

    QWidget * mainWg = new QWidget();
    mainWg->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    this->setWindowTitle("Channels controls");

    this->setWidget(mainWg);

    QVBoxLayout * mainVl = new QVBoxLayout();
    mainVl->setContentsMargins(0, 0, 0, 0);
    mainVl->setSpacing(1);
    mainWg->setLayout(mainVl);

    mDev->getChannelsNumberFeatures(voltageChannelsNum, currentChannelsNum);

    operationCbx = new QComboBox;
    operationCbx->addItem("Turn channels on/off");
    operationCbx->addItem("Turn stimulus on/off");
    operationCbx->addItem("Holding stimulus");

    mainVl->addWidget(operationCbx);
    mainVl->addWidget(this->createTurnChannelsOnOffWidget());

    QWidget * spacer = new QWidget;
    spacer->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::MinimumExpanding);
    mainVl->addWidget(spacer);
}

void ChannelControlDockWidget::onUpdate() {
    mDev->get
}

QWidget * ChannelControlDockWidget::createTurnChannelsOnOffWidget() {
    turnChannelsOnOffWidget = new QWidget;

    QVBoxLayout * vl = new QVBoxLayout;
    vl->setContentsMargins(0, 0, 0, 0);
    vl->setSpacing(1);
    turnChannelsOnOffWidget->setLayout(vl);

    turnChannelsOnOffBtns.resize(currentChannelsNum);
    for (int idx = 0; idx < currentChannelsNum; idx++) {
        QCheckBox * btn = new QCheckBox(QString("Ch%1 On").arg(idx+1));
        btn->setChecked(true);
        vl->addWidget(btn);
        turnChannelsOnOffBtns[idx] = btn;
    }
}
