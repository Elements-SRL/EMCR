#include "protocolpropertydialog.h"

#include <QGroupBox>
#include <QScrollArea>
#include <QSplitter>

#include "protocolwidget.h"

ProtocolPropertyDialog::ProtocolPropertyDialog(MessageDispatcher * msgDisp, RangedMeasurement_t timeRange, RangedMeasurement_t stimulusRange) {
    QVBoxLayout * mainVl = new QVBoxLayout;
    mainVl->setContentsMargins(0, 0, 0, 0);
    this->setLayout(mainVl);

    QSplitter * mainSpl = new QSplitter(Qt::Vertical, this);
    mainVl->addWidget(mainSpl);
    mainSpl->setContentsMargins(0, 0, 0, 0);

    QWidget * propertyW = new QWidget();
    mainSpl->addWidget(propertyW);
    propertyW->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);

    QVBoxLayout * propertyVl = new QVBoxLayout();
    propertyVl->setContentsMargins(0, 0, 0, 0);
    propertyVl->setSpacing(0);
    propertyW->setLayout(propertyVl);

    QScrollArea * controlsSa = new QScrollArea;
    controlsSa->setWidgetResizable(true);
    controlsSa->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    controlsSa->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    propertyVl->addWidget(controlsSa);

    QGroupBox * propertyFrame = new QGroupBox("Protocol properties panel");
    controlsSa->setWidget(propertyFrame);
    propertyFrame->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);

    controlsLo = new QGridLayout();
    controlsLo->setContentsMargins(1, 1, 1, 1);
    controlsLo->setSpacing(2);
    propertyFrame->setLayout(controlsLo);

    buttonsHl = new QHBoxLayout();
    buttonsHl->setContentsMargins(1, 1, 1, 1);
    buttonsHl->setSpacing(2);
    propertyVl->addLayout(buttonsHl);

    controlsLo->setColumnStretch(0, 3);
    controlsLo->setColumnStretch(1, 2);
    controlsLo->setColumnStretch(2, 1);

    /*! Protocol preview */
    preview = new MinimalProtocolPreview(msgDisp, timeRange, stimulusRange, "Preview");
    preview->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    mainSpl->addWidget(preview);

    mainSpl->setStretchFactor(0, 3);
    mainSpl->setStretchFactor(1, 2);

    this->setVisible(false);
}

ProtocolPropertyDialog::~ProtocolPropertyDialog() {
    populated = false;
    if (preview != nullptr) {
        delete preview;
        preview = nullptr;
    }
}

QGridLayout * ProtocolPropertyDialog::getControlsLayout() {
    return controlsLo;
}

QHBoxLayout * ProtocolPropertyDialog::getButtonsLayout() {
    return buttonsHl;
}

ProtocolPreview * ProtocolPropertyDialog::getProtocolPreview() {
    return preview;
}

bool ProtocolPropertyDialog::isOpened(QString &protocolName) {
    bool ret = this->isVisible();
    if (ret) {
        protocolName = ownerPtr->getName();

    } else {
        protocolName = "";
    }
    return ret;
}

void ProtocolPropertyDialog::setOwner(ProtocolWidget * owner) {
    ownerPtr = owner;
}

ProtocolWidget * ProtocolPropertyDialog::getOwner() {
    return ownerPtr;
}
