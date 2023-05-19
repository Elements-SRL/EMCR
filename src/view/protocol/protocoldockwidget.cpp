#include "protocoldockwidget.h"

#include <QBoxLayout>
#include <QSplitter>
#include <QDesktopServices>
#include <QLabel>
#include <QSettings>

ProtocolDockWidget::ProtocolDockWidget(ModelDevice * mDev, ClampingModality_t clampingModality, QWidget * parent) :
    QDockWidget(),
    mDev(mDev),
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
    if (this->clampingModality == e384CommLib::VOLTAGE_CLAMP) {
        mDev->getMessageDispatcher()->getVoltageProtocolRangeFeature(0, stimulusRange);

    } else {
        mDev->getMessageDispatcher()->getCurrentProtocolRangeFeature(0, stimulusRange);
    }

    e384CommLib::RangedMeasurement_t timeRange;
    mDev->getMessageDispatcher()->getTimeProtocolRangeFeature(timeRange);
    timeRange.convertValues(e384CommLib::UnitPfxMilli);
    protocolPropertyDialog = new ProtocolPropertyDialog(mDev, timeRange, stimulusRange);

    voltageProtocolList = new VoltageProtocolList(mDev, protocolPropertyDialog, parent);
    currentProtocolList = new CurrentProtocolList(mDev, protocolPropertyDialog, parent);

#ifdef GLB_RECORD_CONTROLS_IN_PROTOCOL_WIDGET
    recordFileBtn = new QPushButton();
    recordFileBtn->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
    recordFileBtn->setStyleSheet("Text-align:left");
    if (this->clampingModality == e384CommLib::VOLTAGE_CLAMP) {
        this->setRecordFile(voltageProtocolList->getProtocolsSettingsDialog()->getRecordPath(), voltageProtocolList->getProtocolsSettingsDialog()->getRecordName());

    } else {
        this->setRecordFile(currentProtocolList->getProtocolsSettingsDialog()->getRecordPath(), currentProtocolList->getProtocolsSettingsDialog()->getRecordName());
    }
    mainVl->addWidget(recordFileBtn);
    connect(recordFileBtn, &QPushButton::clicked, this, [=] () {
        QDesktopServices::openUrl(QUrl::fromLocalFile(recordPath));
    });
#endif

    QHBoxLayout * sweepInfoHl = new QHBoxLayout;
    sweepInfoHl->setSpacing(1);
    sweepInfoHl->setContentsMargins(1, 1, 1, 1);

    protocolTimer = new TimerDisplay(this, "hh.mm.ss");
    sweepInfoHl->addWidget(protocolTimer);

    mainVl->addLayout(sweepInfoHl);

#ifdef GLB_RECORD_CONTROLS_IN_PROTOCOL_WIDGET
    connect(voltageProtocolList, &ProtocolList::newRecordPath, this, [=] () {
        this->setRecordFile(voltageProtocolList->getProtocolsSettingsDialog()->getRecordPath(), voltageProtocolList->getProtocolsSettingsDialog()->getRecordName());
        currentProtocolList->getProtocolsSettingsDialog()->synchronizeSettings();
    });

    connect(currentProtocolList, &ProtocolList::newRecordPath, this, [=] () {
        this->setRecordFile(currentProtocolList->getProtocolsSettingsDialog()->getRecordPath(), currentProtocolList->getProtocolsSettingsDialog()->getRecordName());
        voltageProtocolList->getProtocolsSettingsDialog()->synchronizeSettings();
    });
#endif

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
        if (this->clampingModality == e384CommLib::VOLTAGE_CLAMP) {
            voltageProtocolList->onStartProtocol();

        } else {
            currentProtocolList->onStartProtocol();
        }
    });
#ifdef GLB_RECORD_CONTROLS_IN_PROTOCOL_WIDGET
    btnLo->addWidget(startProtocolBtn, btnRow, btnCol++);
#else
    sweepInfoHl->insertWidget(btnCol++, startProtocolBtn);
#endif

#ifdef GLB_RECORD_CONTROLS_IN_PROTOCOL_WIDGET
    recordProtocolBtn = new QPushButton; {
        QPixmap btnPix(":/imgs/record protocol.png");
        QIcon btnIcon(btnPix);
        recordProtocolBtn->setIcon(btnIcon);
        recordProtocolBtn->setIconSize(QSize(30, 30));
        recordProtocolBtn->setFixedSize(32, 32);
        recordProtocolBtn->setToolTip("Start and record the selected protocol");
    }
    recordProtocolBtn->setCheckable(false);
    connect(recordProtocolBtn, &QPushButton::clicked, this, [=] () {
        if (this->clampingModality == e384CommLib::VOLTAGE_CLAMP) {
            voltageProtocolList->onRecordProtocol();

        } else {
            currentProtocolList->onRecordProtocol();
        }
    });
    btnLo->addWidget(recordProtocolBtn, btnRow, btnCol++);
#endif

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
        if (this->clampingModality == e384CommLib::VOLTAGE_CLAMP) {
            voltageProtocolList->onStopProtocol();

        } else {
            currentProtocolList->onStopProtocol();
        }
    });
#ifdef GLB_RECORD_CONTROLS_IN_PROTOCOL_WIDGET
    btnLo->addWidget(stopProtocolBtn, btnRow, btnCol++);
#else
    sweepInfoHl->insertWidget(btnCol++, stopProtocolBtn);
    {
        QWidget * spacer = new QWidget;
        spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        sweepInfoHl->insertWidget(btnCol++, spacer);
    }
#endif

#ifdef GLB_RECORD_CONTROLS_IN_PROTOCOL_WIDGET
    QPushButton * addTagBtn = new QPushButton; {
        QPixmap btnPix(":/imgs/add tag.png");
        QIcon btnIcon(btnPix);
        addTagBtn->setIcon(btnIcon);
        addTagBtn->setIconSize(QSize(30, 30));
        addTagBtn->setFixedSize(32, 32);
        addTagBtn->setToolTip("Adds a tag to the recorded trace");
    }
    addTagBtn->setCheckable(false);
    addTagBtn->setEnabled(false);
    tagDlg = new AddTagDialog;
    connect(addTagBtn, &QPushButton::clicked, tagDlg, &AddTagDialog::open);
    connect(addTagBtn, &QPushButton::clicked, this, &ProtocolDockWidget::markTagTime);
    connect(tagDlg, &QDialog::accepted, this, [=] () {
        emit saveTagString(tagDlg->getTag());
    });
    connect(tagDlg, &QDialog::rejected, this, [=] () {
        emit saveTagString("__INVALIDTAG__");
    });
    btnLo->addWidget(addTagBtn, btnRow, btnCol++);

    saveLastProtocolBtn = new QPushButton; {
        QPixmap btnPix(":/imgs/save last protocol.png");
        QIcon btnIcon(btnPix);
        saveLastProtocolBtn->setIcon(btnIcon);
        saveLastProtocolBtn->setIconSize(QSize(30, 30));
        saveLastProtocolBtn->setFixedSize(32, 32);
        saveLastProtocolBtn->setToolTip("Save the last run protocol to disk\n"
                                        "NOTE: changing the controls will invalidate\n"
                                        "the possibility to save the last protocol");
    }
    saveLastProtocolBtn->setCheckable(false);
    saveLastProtocolBtn->setEnabled(false);
    connect(saveLastProtocolBtn, &QPushButton::clicked, this, [=] () {
        if (this->clampingModality == e384CommLib::VOLTAGE_CLAMP) {
            voltageProtocolList->onSaveLastProtocol();

        } else {
            currentProtocolList->onSaveLastProtocol();
        }
    });
    connect(voltageProtocolList, &ProtocolList::enableSaveLastProtocol, saveLastProtocolBtn, &QPushButton::setEnabled);
    connect(currentProtocolList, &ProtocolList::enableSaveLastProtocol, saveLastProtocolBtn, &QPushButton::setEnabled);
    btnLo->addWidget(saveLastProtocolBtn, btnRow, btnCol++);

    QPushButton * protocolsSettingsBtn = new QPushButton; {
        QPixmap btnPix(":/imgs/folder settings.png");
        QIcon btnIcon(btnPix);
        protocolsSettingsBtn->setIcon(btnIcon);
        protocolsSettingsBtn->setIconSize(QSize(30, 30));
        protocolsSettingsBtn->setFixedSize(32, 32);
        protocolsSettingsBtn->setToolTip("Set file properties");
    }
    protocolsSettingsBtn->setCheckable(false);
    connect(protocolsSettingsBtn, &QPushButton::clicked, this, [=] () {
        if (this->clampingModality == e384CommLib::VOLTAGE_CLAMP) {
            voltageProtocolList->onProtocolsSettings();

        } else {
            currentProtocolList->onProtocolsSettings();
        }
    });
    btnLo->addWidget(protocolsSettingsBtn, btnRow, btnCol++);

    connect(this, &ProtocolDockWidget::enableTags, addTagBtn, &QPushButton::setEnabled);
#endif
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

#ifdef GLB_RECORD_CONTROLS_IN_PROTOCOL_WIDGET
    if (tagDlg != nullptr) {
        delete tagDlg;
        tagDlg = nullptr;
    }
#endif

    if (protocolTimer != nullptr) {
        delete protocolTimer;
        protocolTimer = nullptr;
    }
}

void ProtocolDockWidget::onPlotting(bool flag, ProtocolType_t type) {
    if (flag) {
        /*! Gapfree protocols can be pause and restart the timer, while episodic ones always reset the timer */
        if (type == ProtocolTypeGapfree) {
            protocolTimer->onRestartTimer();

        } else {
            protocolTimer->onStartTimer();
        }

    } else {
        if (type == ProtocolTypeGapfree) {
            protocolTimer->onStopTimer(true);

        } else {
            protocolTimer->onStopTimer(false);
        }
    }
}

void ProtocolDockWidget::onNullProtocol() {
    protocolTimer->onStopTimer(false);
}

#ifdef GLB_RECORD_CONTROLS_IN_PROTOCOL_WIDGET
void ProtocolDockWidget::onNewRecordFile(QString fileName) {
    if (clampingModality == e384CommLib::VOLTAGE_CLAMP) {
        this->setRecordFile(voltageProtocolList->getProtocolsSettingsDialog()->getRecordPath(), fileName);

    } else {
        this->setRecordFile(currentProtocolList->getProtocolsSettingsDialog()->getRecordPath(), fileName);
    }
}

void ProtocolDockWidget::onRecording(bool flag) {
    if (flag) {
        QPixmap btnPix(":/imgs/recording protocol.png");
        QIcon btnIcon(btnPix);
        recordProtocolBtn->setIcon(btnIcon);
        protocolTimer->onStartTimer();

    } else {
        QPixmap btnPix(":/imgs/record protocol.png");
        QIcon btnIcon(btnPix);
        recordProtocolBtn->setIcon(btnIcon);
        protocolTimer->onStopTimer(false);
    }
}
#endif

bool ProtocolDockWidget::eventFilter(QObject * obj, QEvent * event) {
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent * keyEvent = static_cast <QKeyEvent *> (event);
        if ((keyEvent->key() == Qt::Key_Enter) || (keyEvent->key() == Qt::Key_Return)) {
            if ((keyEvent->modifiers() & Qt::ShiftModifier) > 0) {
#ifdef GLB_RECORD_CONTROLS_IN_PROTOCOL_WIDGET
                if (clampingModality == e384CommLib::VOLTAGE_CLAMP) {
                    voltageProtocolList->onRecordProtocol();

                } else {
                    currentProtocolList->onRecordProtocol();
                }
#endif

            } else {
                if (clampingModality == e384CommLib::VOLTAGE_CLAMP) {
                    voltageProtocolList->onStartProtocol();

                } else {
                    currentProtocolList->onStartProtocol();
                }
            }
        }
    }
    return QObject::eventFilter(obj, event);
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
#ifdef GLB_RECORD_CONTROLS_IN_PROTOCOL_WIDGET
    saveLastProtocolBtn->setEnabled(false);
#endif
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

#ifdef GLB_RECORD_CONTROLS_IN_PROTOCOL_WIDGET
void ProtocolDockWidget::setRecordFile(QString path, QString name) {
    recordPath = path;
    recordFileBtn->setText("Recording path: " + recordPath + "\nFile: " + name);
    recordFileBtn->setToolTip(recordPath + name);
}
#endif
