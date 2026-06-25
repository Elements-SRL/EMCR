#include "protocoldockwidget.h"

#include <QBoxLayout>
#include <QSplitter>
#include <QDesktopServices>
#include <QLabel>
#include <QSettings>
#include <QButtonGroup>

ProtocolDockWidget::ProtocolDockWidget(MessageDispatcher * msgDisp, ClampingModality_t clampingModality, QWidget * parent) :
    QDockWidget(),
    msgDisp(msgDisp),
    clampingModality(clampingModality) {

    QWidget * mainW = new QWidget;
    this->setWidget(mainW);
    this->setWindowTitle("Voltage Protocols");

    QVBoxLayout * mainVl = new QVBoxLayout;
    mainVl->setSpacing(0);
    mainVl->setContentsMargins(0, 0, 0, 0);
    mainW->setLayout(mainVl);

    // TOP BAR - CRUD Buttons
    QWidget* crudButtonsBar = new QWidget();
    crudButtonsBar->setObjectName("crudButtonsBar");

    QHBoxLayout * crudToolbarLayout = new QHBoxLayout(crudButtonsBar);
    crudToolbarLayout->setSpacing(6);
    crudToolbarLayout->setContentsMargins(4, 4, 4, 4);
    mainVl->addWidget(crudButtonsBar);

    // MIDDLE - Action bar
    QWidget* actionBar = new QWidget();
    actionBar->setObjectName("actionBar");

    QVBoxLayout * actionBarLayout = new QVBoxLayout(actionBar);
    actionBarLayout->setSpacing(6);
    actionBarLayout->setContentsMargins(4, 8, 4, 8);
    actionBarLayout->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

    mainVl->addWidget(actionBar);

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
    analysisVoltageProtocolList = new AnalysisVoltageProtocolList(msgDisp, protocolPropertyDialog, parent);
    currentProtocolList = new CurrentProtocolList(msgDisp, protocolPropertyDialog, parent);
    analysisCurrentProtocolList = new AnalysisCurrentProtocolList(msgDisp, protocolPropertyDialog, parent);

    QSplitter * mainSpl = new QSplitter(Qt::Vertical, this);
    mainVl->addWidget(mainSpl);

    mainSpl->addWidget(voltageProtocolList);
    mainSpl->addWidget(analysisVoltageProtocolList);
    mainSpl->addWidget(currentProtocolList);
    mainSpl->addWidget(analysisCurrentProtocolList);
    this->setProtocolListVisibility();
    mainSpl->addWidget(protocolPropertyDialog);

    mainSpl->setStretchFactor(0, 1);
    mainSpl->setStretchFactor(1, 1);
    mainSpl->setStretchFactor(2, 3);


    addProtocolBtn = new QPushButton; {
        addProtocolBtn->setObjectName("addProtocolBtn");
        addProtocolBtn->setFixedSize(32, 32);
        addProtocolBtn->setToolTip("Create a new protocol");
    }
    addProtocolBtn->setCheckable(false);
    connect(addProtocolBtn, &QPushButton::clicked, this, [=] () {
        if (this->clampingModality == e384CommLib::VOLTAGE_CLAMP) {
            voltageProtocolList->onAddProtocol();
        }
        else {
            currentProtocolList->onAddProtocol();
        }
    });
    crudToolbarLayout->addWidget(addProtocolBtn);

    removeProtocolBtn = new QPushButton; {
        removeProtocolBtn->setObjectName("removeProtocolBtn");
        removeProtocolBtn->setFixedSize(32, 32);
        removeProtocolBtn->setToolTip("Delete the selected protocol");
    }
    removeProtocolBtn->setCheckable(false);
    connect(removeProtocolBtn, &QPushButton::clicked, this, [=] () {
        if (this->clampingModality == e384CommLib::VOLTAGE_CLAMP) {
            voltageProtocolList->onRemoveProtocol();
        }
        else {
            currentProtocolList->onRemoveProtocol();
        }
    });
    crudToolbarLayout->addWidget(removeProtocolBtn);

    editProtocolBtn = new QPushButton; {
        editProtocolBtn->setObjectName("editProtocolBtn");
        editProtocolBtn->setFixedSize(32, 32);
        editProtocolBtn->setToolTip("Edit the selected protocol");
    }
    editProtocolBtn->setCheckable(false);
    connect(editProtocolBtn, &QPushButton::clicked, this, [=] () {
        if (this->clampingModality == e384CommLib::VOLTAGE_CLAMP) {
            voltageProtocolList->onEditProtocol();
        }
        else {
            currentProtocolList->onEditProtocol();
        }
    });
    crudToolbarLayout->addWidget(editProtocolBtn);

    copyProtocolBtn = new QPushButton; {
        copyProtocolBtn->setObjectName("copyProtocolBtn");
        copyProtocolBtn->setFixedSize(32, 32);
        copyProtocolBtn->setToolTip("Make a copy of the selected protocol");
    }
    copyProtocolBtn->setCheckable(false);
    connect(copyProtocolBtn, &QPushButton::clicked, this, [=] () {
        if (this->clampingModality == e384CommLib::VOLTAGE_CLAMP) {
            voltageProtocolList->onCopyProtocol();
        }
        else {
            currentProtocolList->onCopyProtocol();
        }
    });
    crudToolbarLayout->addWidget(copyProtocolBtn);

    setProtocolsShortCutsBtn = new QPushButton; {
        setProtocolsShortCutsBtn->setObjectName("setProtocolsShortCutsBtn");
        setProtocolsShortCutsBtn->setFixedSize(32, 32);
        setProtocolsShortCutsBtn->setToolTip("Set protocols shortcuts");
    }
    setProtocolsShortCutsBtn->setCheckable(false);
    connect(setProtocolsShortCutsBtn, &QPushButton::clicked, this, [=] () {
        if (this->clampingModality == e384CommLib::VOLTAGE_CLAMP) {
            if (analysisProtocolsFlag) {
                analysisVoltageProtocolList->onSetProtocolsShortCuts();
            }
            else {
                voltageProtocolList->onSetProtocolsShortCuts();
            }
        }
        else {
            if (analysisProtocolsFlag) {
                analysisCurrentProtocolList->onSetProtocolsShortCuts();
            }
            else {
                currentProtocolList->onSetProtocolsShortCuts();
            }
        }
    });
    crudToolbarLayout->addWidget(setProtocolsShortCutsBtn);

    importProtocolBtn = new QPushButton; {
        importProtocolBtn->setObjectName("importProtocolBtn");
        importProtocolBtn->setFixedSize(32, 32);
        importProtocolBtn->setToolTip("Import protocols");
    }
    importProtocolBtn->setCheckable(false);
    connect(importProtocolBtn, &QPushButton::clicked, this, [=] () {
        if (this->clampingModality == e384CommLib::VOLTAGE_CLAMP) {
            voltageProtocolList->onImportProtocols();
        }
        else {
            currentProtocolList->onImportProtocols();
        }
    });
    crudToolbarLayout->addWidget(importProtocolBtn);

    exportProtocolBtn = new QPushButton; {
        exportProtocolBtn->setObjectName("exportProtocolBtn");
        exportProtocolBtn->setFixedSize(32, 32);
        exportProtocolBtn->setToolTip("Export protocols");
    }
    exportProtocolBtn->setCheckable(false);
    connect(exportProtocolBtn, &QPushButton::clicked, this, [=] () {
        if (this->clampingModality == e384CommLib::VOLTAGE_CLAMP) {
            voltageProtocolList->onExportProtocols();
        }
        else {
            currentProtocolList->onExportProtocols();
        }
    });
    crudToolbarLayout->addWidget(exportProtocolBtn);


    /* ACTION BAR */

    // Row 1 - start/stop and timer
    QHBoxLayout * sweepInfoHl = new QHBoxLayout;
    sweepInfoHl->setSpacing(4);
    sweepInfoHl->setContentsMargins(0, 0, 0, 0);
    sweepInfoHl->setAlignment(Qt::AlignVCenter);
    actionBarLayout->addLayout(sweepInfoHl);

    QPushButton * startStopBtn = new QPushButton; {
        startStopBtn->setObjectName("startStopBtn");
        startStopBtn->setFixedWidth(60);
        startStopBtn->setToolTip("Start the selected protocol");
    }
    startStopBtn->setCheckable(true);
    connect(startStopBtn, &QPushButton::clicked, this, [=](bool checked) {
        if (checked) {
            startStopBtn->setProperty("running", true);
            this->onStartProtocol(true);
        } else {
            startStopBtn->setProperty("running", false);
            this->onStartProtocol(false);
        }

        startStopBtn->style()->unpolish(startStopBtn);
        startStopBtn->style()->polish(startStopBtn);
    });
    protocolTimer = new TimerDisplay(this, "hh.mm.ss");

    sweepInfoHl->addStretch();
    sweepInfoHl->addWidget(startStopBtn);
    sweepInfoHl->addWidget(protocolTimer);
    sweepInfoHl->addStretch();

    // Decomment when needed
    // QPushButton * restartProtocolBtn = new QPushButton; {
    //     restartProtocolBtn->setVisible(false);
    //     QPixmap btnPix(":/imgs/start protocol.png");
    //     QIcon btnIcon(btnPix);
    //     restartProtocolBtn->setIcon(btnIcon);
    //     restartProtocolBtn->setIconSize(QSize(30, 30));
    //     restartProtocolBtn->setFixedSize(32, 32);
    //     restartProtocolBtn->setToolTip("Restart the selected protocol");
    // }
    // restartProtocolBtn->setCheckable(false);
    // connect(restartProtocolBtn, &QPushButton::clicked, this, [=] () {
    //     this->onRestartProtocol(true);
    // });

    // Row 2 - Segmented button
    QHBoxLayout * segmentedLayout = new QHBoxLayout;
    segmentedLayout->setSpacing(0);
    segmentedLayout->setContentsMargins(4, 4, 4, 4);

    actionBarLayout->addLayout(segmentedLayout);

    QPushButton * btnAcquisition = new QPushButton("ACQUISITION", this);
    btnAcquisition->setFixedSize(85, 18);
    btnAcquisition->setCheckable(true);
    btnAcquisition->setChecked(true);
    btnAcquisition->setObjectName("btnAcquisition");

    QPushButton * btnAnalysis = new QPushButton("ESTIMATE", this);
    btnAnalysis->setFixedSize(85, 18);
    btnAnalysis->setCheckable(true);
    btnAnalysis->setObjectName("btnAnalysis");

    // Exclusive buttons
    QButtonGroup * segmentGroup = new QButtonGroup(this);
    segmentGroup->addButton(btnAcquisition);
    segmentGroup->addButton(btnAnalysis);
    segmentGroup->setExclusive(true);

    segmentedLayout->addStretch();
    segmentedLayout->addWidget(btnAcquisition);
    segmentedLayout->addWidget(btnAnalysis);
    segmentedLayout->addStretch();

    connect(btnAcquisition, &QPushButton::clicked, this, [=]() {
        this->onSetAnalysisProtocols(false);
    });
    connect(btnAnalysis, &QPushButton::clicked, this, [=]() {
        this->onSetAnalysisProtocols(true);
    });

    // Row 3 - Separation header

    QFrame * protocolsHeader = new QFrame();
    protocolsHeader->setObjectName("sectionHeaderContainer");
    QHBoxLayout* protocolsHeaderLayout = new QHBoxLayout(protocolsHeader);
    protocolsHeaderLayout->setContentsMargins(0, 0, 0, 0);

    QLabel * protocolsSectionLbl = new QLabel("PROTOCOLS");
    protocolsSectionLbl->setObjectName("sectionHeader");
    protocolsHeaderLayout->addWidget(protocolsSectionLbl);

    actionBarLayout->addWidget(protocolsHeader);

    QShortcut * sh;
    QKeyCombination startKeyOffset = Qt::ControlModifier | Qt::Key_0;
//    int recordKeyOffset = Qt::ControlModifier + Qt::ShiftModifier + Qt::Key_0;
    for (int keyIdx = 0; keyIdx < 10; keyIdx++) {
        sh = new QShortcut(QKeySequence(startKeyOffset | keyIdx), parent);
        connect(sh, &QShortcut::activated, this, [=] () {
            voltageProtocolList->startProtocolFromShortCutIndex(keyIdx);
            analysisVoltageProtocolList->startProtocolFromShortCutIndex(keyIdx);
            currentProtocolList->startProtocolFromShortCutIndex(keyIdx);
            analysisCurrentProtocolList->startProtocolFromShortCutIndex(keyIdx);
        });
        shortcuts.append(sh);

//        sh = new QShortcut(QKeySequence(recordKeyOffset+keyIdx), this);
//        connect(sh, &QShortcut::activated, this, [=] () {
//            voltageProtocolList->recordProtocol(keyIdx);
//            currentProtocolList->recordProtocol(keyIdx);
//        });
//        shortcuts.append(sh);
    }

}

ProtocolDockWidget::~ProtocolDockWidget() {
    if (voltageProtocolList != nullptr) {
        voltageProtocolList = nullptr;
    }

    if (analysisVoltageProtocolList != nullptr) {
        analysisVoltageProtocolList = nullptr;
    }

    if (currentProtocolList != nullptr) {
        currentProtocolList = nullptr;
    }

    if (analysisCurrentProtocolList != nullptr) {
        analysisCurrentProtocolList = nullptr;
    }

    if (protocolPropertyDialog != nullptr) {
        delete protocolPropertyDialog;
        protocolPropertyDialog = nullptr;
    }

    if (protocolTimer != nullptr) {
        delete protocolTimer;
        protocolTimer = nullptr;
    }

    for (int shortcutIdx = 0; shortcutIdx < shortcuts.size(); shortcutIdx++) {
        if (shortcuts[shortcutIdx] != nullptr) {
            delete shortcuts[shortcutIdx];
        }
    }
    shortcuts.clear();
}

bool ProtocolDockWidget::eventFilter(QObject * obj, QEvent * event) {
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent * keyEvent = static_cast <QKeyEvent *> (event);
        if ((keyEvent->key() == Qt::Key_Enter) || (keyEvent->key() == Qt::Key_Return)) {
            if ((keyEvent->modifiers() & Qt::ShiftModifier) > 0) {

            }
            else {
                this->onStartProtocol(true);
            }
        }
    }
    return QObject::eventFilter(obj, event);
}

void ProtocolDockWidget::onStartProtocol(bool flag) {
    protocolTimer->onStopTimer();
    if (flag) {
        protocolTimer->onStartTimer();
        emit startProtocol();
    }
    else {
        emit stopProtocol();
    }
}

void ProtocolDockWidget::onRestartProtocol(bool flag) {
    protocolTimer->onStopTimer();
    if (flag) {
        protocolTimer->onStartTimer();
        emit restartProtocol();
    }
    else {
        emit stopProtocol();
    }
}

ProtocolList * ProtocolDockWidget::getVoltageProtocolList() {
    return voltageProtocolList;
}

ProtocolList * ProtocolDockWidget::getAnalysisVoltageProtocolList() {
    return analysisVoltageProtocolList;
}

ProtocolList * ProtocolDockWidget::getCurrentProtocolList() {
    return currentProtocolList;
}

ProtocolList * ProtocolDockWidget::getAnalysisCurrentProtocolList() {
    return analysisCurrentProtocolList;
}

void ProtocolDockWidget::onSetClampingModality(ClampingModality_t clampingModality) {
    this->clampingModality = clampingModality;

    this->setProtocolListVisibility();
}

void ProtocolDockWidget::onSetAnalysisProtocols(bool analysisProtocols) {
    this->analysisProtocolsFlag = analysisProtocols;
    addProtocolBtn->setEnabled(!analysisProtocols);
    removeProtocolBtn->setEnabled(!analysisProtocols);
    editProtocolBtn->setEnabled(!analysisProtocols);
    copyProtocolBtn->setEnabled(!analysisProtocols);
    importProtocolBtn->setEnabled(!analysisProtocols);
    exportProtocolBtn->setEnabled(!analysisProtocols);

    this->setProtocolListVisibility();
}

void ProtocolDockWidget::setProtocolListVisibility() {
    voltageProtocolList->setClampingModality(clampingModality);
    voltageProtocolList->saveAndClosePropertyDialog();
    analysisVoltageProtocolList->setClampingModality(clampingModality);
    analysisVoltageProtocolList->saveAndClosePropertyDialog();
    currentProtocolList->setClampingModality(clampingModality);
    currentProtocolList->saveAndClosePropertyDialog();
    analysisCurrentProtocolList->setClampingModality(clampingModality);
    analysisCurrentProtocolList->saveAndClosePropertyDialog();

    switch (clampingModality) {
    case VOLTAGE_CLAMP:
        if (analysisProtocolsFlag) {
            voltageProtocolList->setVisible(false);
            analysisVoltageProtocolList->setVisible(true);
            this->setWindowTitle("Analysis Voltage Protocols");
        }
        else {
            voltageProtocolList->setVisible(true);
            analysisVoltageProtocolList->setVisible(false);
            this->setWindowTitle("Voltage Protocols");
        }
        break;

    case ZERO_CURRENT_CLAMP:
        this->setWindowTitle("Current Protocols");
        break;

    case CURRENT_CLAMP:
        if (analysisProtocolsFlag) {
            currentProtocolList->setVisible(false);
            analysisCurrentProtocolList->setVisible(true);
            this->setWindowTitle("Analysis Current Protocols");
        }
        else {
            currentProtocolList->setVisible(true);
            analysisCurrentProtocolList->setVisible(false);
            this->setWindowTitle("Current Protocols");
        }
        break;
    }

    if (clampingModality == e384CommLib::VOLTAGE_CLAMP) {
    }
    else if (clampingModality == e384CommLib::ZERO_CURRENT_CLAMP || clampingModality == e384CommLib::CURRENT_CLAMP) {
    }
}
