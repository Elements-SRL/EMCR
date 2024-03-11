#include "protocoldockwidget.h"

#include <QBoxLayout>
#include <QSplitter>
#include <QDesktopServices>
#include <QLabel>
#include <QSettings>

ProtocolDockWidget::ProtocolDockWidget(MessageDispatcher * msgDisp, ClampingModality_t clampingModality, QWidget * parent) :
    QDockWidget(),
    msgDisp(msgDisp),
    clampingModality(clampingModality) {

    QWidget * mainW = new QWidget;
    this->setWidget(mainW);
    this->setWindowTitle("Voltage Protocols");

    QVBoxLayout * mainVl = new QVBoxLayout;
    mainVl->setSpacing(1);
    mainVl->setContentsMargins(1, 1, 1, 1);
    mainW->setLayout(mainVl);

    QGridLayout * btnLo = new QGridLayout;
    mainVl->addLayout(btnLo);
    btnLo->setSpacing(0);
    btnLo->setContentsMargins(1, 1, 1, 1);

    this->installEventFilter(this);

    e384CommLib::RangedMeasurement_t stimulusRange;
    if (clampingModality == e384CommLib::VOLTAGE_CLAMP) {
        msgDisp->getVoltageProtocolRangeFeature(0, stimulusRange);

    } else {
        msgDisp->getCurrentProtocolRangeFeature(0, stimulusRange);
    }

    e384CommLib::RangedMeasurement_t timeRange;
    msgDisp->getTimeProtocolRangeFeature(timeRange);
    timeRange.convertValues(e384CommLib::UnitPfxMilli);
    protocolPropertyDialog = new ProtocolPropertyDialog(msgDisp, timeRange, stimulusRange);

    voltageProtocolList = new VoltageProtocolList(msgDisp, protocolPropertyDialog, parent);
    currentProtocolList = new CurrentProtocolList(msgDisp, protocolPropertyDialog, parent);

    QHBoxLayout * sweepInfoHl = new QHBoxLayout;
    sweepInfoHl->setSpacing(1);
    sweepInfoHl->setContentsMargins(1, 1, 1, 1);

    protocolTimer = new TimerDisplay(this, "hh.mm.ss");
    sweepInfoHl->addWidget(protocolTimer);

    mainVl->addLayout(sweepInfoHl);

    QSplitter * mainSpl = new QSplitter(Qt::Vertical, this);
    mainVl->addWidget(mainSpl);

    mainSpl->addWidget(voltageProtocolList);
    mainSpl->addWidget(currentProtocolList);
    this->setProtocolListVisibility();
    mainSpl->addWidget(protocolPropertyDialog);

    mainSpl->setStretchFactor(0, 1);
    mainSpl->setStretchFactor(1, 1);
    mainSpl->setStretchFactor(2, 3);

    int btnRow = 0;
    int btnCol = 0;

    QPushButton * addProtocolBtn = new QPushButton; {
        QPixmap btnPix(":/imgs/add protocol.png");
        QIcon btnIcon(btnPix);
        addProtocolBtn->setIcon(btnIcon);
        addProtocolBtn->setIconSize(QSize(30, 30));
        addProtocolBtn->setFixedSize(32, 32);
        addProtocolBtn->setToolTip("Create a new protocol");
    }
    addProtocolBtn->setCheckable(false);
    connect(addProtocolBtn, &QPushButton::clicked, this, [=] () {
        if (this->clampingModality == e384CommLib::VOLTAGE_CLAMP) {
            voltageProtocolList->onAddProtocol();

        } else {
            currentProtocolList->onAddProtocol();
        }
    });
    btnLo->addWidget(addProtocolBtn, btnRow, btnCol++);

    QPushButton * removeProtocolBtn = new QPushButton; {
        QPixmap btnPix(":/imgs/remove protocol.png");
        QIcon btnIcon(btnPix);
        removeProtocolBtn->setIcon(btnIcon);
        removeProtocolBtn->setIconSize(QSize(30, 30));
        removeProtocolBtn->setFixedSize(32, 32);
        removeProtocolBtn->setToolTip("Delete the selected protocol");
    }
    removeProtocolBtn->setCheckable(false);
    connect(removeProtocolBtn, &QPushButton::clicked, this, [=] () {
        if (this->clampingModality == e384CommLib::VOLTAGE_CLAMP) {
            voltageProtocolList->onRemoveProtocol();

        } else {
            currentProtocolList->onRemoveProtocol();
        }
    });
    btnLo->addWidget(removeProtocolBtn, btnRow, btnCol++);

    QPushButton * editProtocolBtn = new QPushButton; {
        QPixmap btnPix(":/imgs/edit protocol.png");
        QIcon btnIcon(btnPix);
        editProtocolBtn->setIcon(btnIcon);
        editProtocolBtn->setIconSize(QSize(30, 30));
        editProtocolBtn->setFixedSize(32, 32);
        editProtocolBtn->setToolTip("Edit the selected protocol");
    }
    editProtocolBtn->setCheckable(false);
    connect(editProtocolBtn, &QPushButton::clicked, this, [=] () {
        if (this->clampingModality == e384CommLib::VOLTAGE_CLAMP) {
            voltageProtocolList->onEditProtocol();

        } else {
            currentProtocolList->onEditProtocol();
        }
    });
    btnLo->addWidget(editProtocolBtn, btnRow, btnCol++);

    QPushButton * copyProtocolBtn = new QPushButton; {
        QPixmap btnPix(":/imgs/copy protocol.png");
        QIcon btnIcon(btnPix);
        copyProtocolBtn->setIcon(btnIcon);
        copyProtocolBtn->setIconSize(QSize(30, 30));
        copyProtocolBtn->setFixedSize(32, 32);
        copyProtocolBtn->setToolTip("Make a copy of the selected protocol");
    }
    copyProtocolBtn->setCheckable(false);
    connect(copyProtocolBtn, &QPushButton::clicked, this, [=] () {
        if (this->clampingModality == e384CommLib::VOLTAGE_CLAMP) {
            voltageProtocolList->onCopyProtocol();

        } else {
            currentProtocolList->onCopyProtocol();
        }
    });
    btnLo->addWidget(copyProtocolBtn, btnRow, btnCol++);

    QPushButton * setProtocolsShortCutsBtn = new QPushButton; {
        QPixmap btnPix(":/imgs/protocols shortcuts.png");
        QIcon btnIcon(btnPix);
        setProtocolsShortCutsBtn->setIcon(btnIcon);
        setProtocolsShortCutsBtn->setIconSize(QSize(30, 30));
        setProtocolsShortCutsBtn->setFixedSize(32, 32);
        setProtocolsShortCutsBtn->setToolTip("Set protocols shortcuts");
    }
    setProtocolsShortCutsBtn->setCheckable(false);
    connect(setProtocolsShortCutsBtn, &QPushButton::clicked, this, [=] () {
        if (this->clampingModality == e384CommLib::VOLTAGE_CLAMP) {
            voltageProtocolList->onSetProtocolsShortCuts();

        } else {
            currentProtocolList->onSetProtocolsShortCuts();
        }
    });
    btnLo->addWidget(setProtocolsShortCutsBtn, btnRow, btnCol++);

    QPushButton * importProtocolBtn = new QPushButton; {
        QPixmap btnPix(":/imgs/import protocol.png");
        QIcon btnIcon(btnPix);
        importProtocolBtn->setIcon(btnIcon);
        importProtocolBtn->setIconSize(QSize(30, 30));
        importProtocolBtn->setFixedSize(32, 32);
        importProtocolBtn->setToolTip("Import protocols");
    }
    importProtocolBtn->setCheckable(false);
    connect(importProtocolBtn, &QPushButton::clicked, this, [=] () {
        if (this->clampingModality == e384CommLib::VOLTAGE_CLAMP) {
            voltageProtocolList->onImportProtocols();

        } else {
            currentProtocolList->onImportProtocols();
        }
    });
    btnLo->addWidget(importProtocolBtn, btnRow, btnCol++);

    QPushButton * exportProtocolBtn = new QPushButton; {
        QPixmap btnPix(":/imgs/export protocol.png");
        QIcon btnIcon(btnPix);
        exportProtocolBtn->setIcon(btnIcon);
        exportProtocolBtn->setIconSize(QSize(30, 30));
        exportProtocolBtn->setFixedSize(32, 32);
        exportProtocolBtn->setToolTip("Export protocols");
    }
    exportProtocolBtn->setCheckable(false);
    connect(exportProtocolBtn, &QPushButton::clicked, this, [=] () {
        if (this->clampingModality == e384CommLib::VOLTAGE_CLAMP) {
            voltageProtocolList->onExportProtocols();

        } else {
            currentProtocolList->onExportProtocols();
        }
    });
    btnLo->addWidget(exportProtocolBtn, btnRow, btnCol++);

    QWidget * spacer = new QWidget;
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    btnLo->addWidget(spacer, btnRow, btnCol++);

    /*! New row */
    btnRow++;
    btnCol = 0;

    QPushButton * startProtocolBtn = new QPushButton; {
        QPixmap btnPix(":/imgs/start protocol.png");
        QIcon btnIcon(btnPix);
        startProtocolBtn->setIcon(btnIcon);
        startProtocolBtn->setIconSize(QSize(30, 30));
        startProtocolBtn->setFixedSize(32, 32);
        startProtocolBtn->setToolTip("Start the selected protocol");
    }
    startProtocolBtn->setCheckable(false);
    connect(startProtocolBtn, &QPushButton::clicked, this, [=] () {
        this->onStartProtocol(true);
    });

    sweepInfoHl->insertWidget(btnCol++, startProtocolBtn);

    QPushButton * restartProtocolBtn = new QPushButton; {
        QPixmap btnPix(":/imgs/start protocol.png");
        QIcon btnIcon(btnPix);
        restartProtocolBtn->setIcon(btnIcon);
        restartProtocolBtn->setIconSize(QSize(30, 30));
        restartProtocolBtn->setFixedSize(32, 32);
        restartProtocolBtn->setToolTip("Retart the selected protocol");
    }
    restartProtocolBtn->setCheckable(false);
    connect(restartProtocolBtn, &QPushButton::clicked, this, [=] () {
        this->onRestartProtocol(true);
    });

    sweepInfoHl->insertWidget(btnCol++, restartProtocolBtn);

    QPushButton * stopProtocolBtn = new QPushButton; {
        QPixmap btnPix(":/imgs/stop protocol.png");
        QIcon btnIcon(btnPix);
        stopProtocolBtn->setIcon(btnIcon);
        stopProtocolBtn->setIconSize(QSize(30, 30));
        stopProtocolBtn->setFixedSize(32, 32);
        stopProtocolBtn->setToolTip("Stop the protocol currently running");
    }
    stopProtocolBtn->setCheckable(false);
    connect(stopProtocolBtn, &QPushButton::clicked, this, [=] () {
        this->onStartProtocol(false);
    });

    sweepInfoHl->insertWidget(btnCol++, stopProtocolBtn);
    {
        QWidget * spacer = new QWidget;
        spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        sweepInfoHl->insertWidget(btnCol++, spacer);
    }
}

ProtocolDockWidget::~ProtocolDockWidget() {
    if (voltageProtocolList != nullptr) {
        delete voltageProtocolList;
        voltageProtocolList = nullptr;
    }

    if (currentProtocolList != nullptr) {
        delete currentProtocolList;
        currentProtocolList = nullptr;
    }

    if (protocolPropertyDialog != nullptr) {
        delete protocolPropertyDialog;
        protocolPropertyDialog = nullptr;
    }

    if (protocolTimer != nullptr) {
        delete protocolTimer;
        protocolTimer = nullptr;
    }
}

bool ProtocolDockWidget::eventFilter(QObject * obj, QEvent * event) {
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent * keyEvent = static_cast <QKeyEvent *> (event);
        if ((keyEvent->key() == Qt::Key_Enter) || (keyEvent->key() == Qt::Key_Return)) {
            if ((keyEvent->modifiers() & Qt::ShiftModifier) > 0) {

            } else {
                this->onStartProtocol(true);
            }
        }
    }
    return QObject::eventFilter(obj, event);
}

void ProtocolDockWidget::onStartProtocol(bool flag) {
    if (flag) {
        protocolTimer->onStopTimer();
        protocolTimer->onStartTimer();
        emit startProtocol();

    } else {
        protocolTimer->onStopTimer();
        emit stopProtocol();
    }
}

void ProtocolDockWidget::onRestartProtocol(bool flag) {
    if (flag) {
        protocolTimer->onStopTimer();
        protocolTimer->onStartTimer();
        emit restartProtocol();

    } else {
        protocolTimer->onStopTimer();
        emit stopProtocol();
    }
}

ProtocolList * ProtocolDockWidget::getProtocolList() {
    if (clampingModality == e384CommLib::VOLTAGE_CLAMP) {
        return voltageProtocolList;

    } else {
        return currentProtocolList;
    }
}

ProtocolList * ProtocolDockWidget::getVoltageProtocolList() {
    return voltageProtocolList;
}

ProtocolList * ProtocolDockWidget::getCurrentProtocolList() {
    return currentProtocolList;
}

void ProtocolDockWidget::onSetClampingModality(ClampingModality_t clampingModality) {
    this->clampingModality = clampingModality;

    this->setProtocolListVisibility();
}

void ProtocolDockWidget::setProtocolListVisibility() {
    if (clampingModality == e384CommLib::VOLTAGE_CLAMP) {
        this->setWindowTitle("Voltage Protocols");
        voltageProtocolList->setClampingModality(clampingModality);
        voltageProtocolList->saveAndClosePropertyDialog();
        currentProtocolList->setClampingModality(clampingModality);
        currentProtocolList->saveAndClosePropertyDialog();

    } else if (clampingModality == e384CommLib::ZERO_CURRENT_CLAMP) {
        /*! Nothing to do */

    } else if (clampingModality == e384CommLib::CURRENT_CLAMP) {
        this->setWindowTitle("Current Protocols");
        voltageProtocolList->setClampingModality(clampingModality);
        voltageProtocolList->saveAndClosePropertyDialog();
        currentProtocolList->setClampingModality(clampingModality);
        currentProtocolList->saveAndClosePropertyDialog();
    }
}
