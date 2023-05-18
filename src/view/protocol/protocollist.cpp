#include "protocollist.h"

#include <QInputDialog>
#include <QSettings>
#include <QFileDialog>
#include <QMessageBox>
#include <QMenu>
#include <QDialogButtonBox>

#include "epmlmanager.h"
#include "errormanager.h"

static int createdProtocolIdx = 0;

ProtocolList::ProtocolList(e4gcl::CommLib * commLib, ProtocolPropertyDialog * protocolPropertyDialog, QWidget * parent) :
    QListWidget(),
    commLib(commLib),
    protocolPropertyDialog(protocolPropertyDialog),
    parent(parent) {

    this->setDragDropMode(QAbstractItemView::InternalMove);
    this->setIconSize(QSize(25, 25));
    connect(this, &ProtocolList::itemDoubleClicked, this, &ProtocolList::onItemDoubleClicked);
    protocols = new QVector <ProtocolWidget *>;
    protocolsNames = new QStringList;
    protocolManager = new ProtocolManager(commLib);

    connect(protocolManager, &ProtocolManager::protocolStarted, this, &ProtocolList::protocolStarted);
    connect(protocolManager, &ProtocolManager::protocolSaveRequest, this, &ProtocolList::protocolSaveRequest);
    connect(protocolManager, &ProtocolManager::currentApplied, this, &ProtocolList::currentApplied);
    connect(this, &ProtocolList::increaseProtocolId, protocolManager, &ProtocolManager::onIncreaseProtocolId);

    this->installEventFilter(this);

    protocolsSettingsDlg = new ProtocolsSettingsDialog();
    connect(protocolsSettingsDlg, &ProtocolsSettingsDialog::newRecordPath, this, &ProtocolList::newRecordPath);

    this->createActions();
    this->createShortCutsDialog();

    /*! Sort protocols and protocolsNames when they are moved around by drag&drop: this way the protocols are saved in the order wanted by the user */
    QAbstractItemModel * model = this->model();
    connect(model, &QAbstractItemModel::rowsMoved, this, [=] (const QModelIndex &, int from, int, const QModelIndex &, int to) {
        if (from < to) {
            ProtocolWidget * tempProt = protocols->at(from);
            QString tempName = protocolsNames->at(from);
            for (int idx = from; idx < to-1; idx++) {
                protocols->replace(idx, protocols->at(idx+1));
                protocolsNames->replace(idx, protocolsNames->at(idx+1));
            }
            protocols->replace(to-1, tempProt);
            protocolsNames->replace(to-1, tempName);

        } else {
            ProtocolWidget * tempProt = protocols->at(from);
            QString tempName = protocolsNames->at(from);
            for (int idx = from; idx > to; idx--) {
                protocols->replace(idx, protocols->at(idx-1));
                protocolsNames->replace(idx, protocolsNames->at(idx-1));
            }
            protocols->replace(to, tempProt);
            protocolsNames->replace(to, tempName);
        }
    });
}

ProtocolList::~ProtocolList() {
    this->onStopProtocol();

    this->exportLastProtocols();

    for (int itemIdx = 0; itemIdx < this->count(); itemIdx++) {
        ProtocolWidget * item = static_cast <ProtocolWidget *> (this->item(itemIdx));
        delete item;
    }

    protocolsNames->clear();

    if (protocolManager != nullptr) {
        delete protocolManager;
        protocolManager = nullptr;
    }

    if (nullGapfreeProtocol != nullptr) {
        delete nullGapfreeProtocol;
        nullGapfreeProtocol = nullptr;
    }

    if (nullEpisodicProtocol != nullptr) {
        delete nullEpisodicProtocol;
        nullEpisodicProtocol = nullptr;
    }

    if (vhold0Protocol != nullptr) {
        delete vhold0Protocol;
        vhold0Protocol = nullptr;
    }

    if (ihold0Protocol != nullptr) {
        delete ihold0Protocol;
        ihold0Protocol = nullptr;
    }

    if (lastRunProtocol != nullptr) {
        delete lastRunProtocol;
        lastRunProtocol = nullptr;
    }
}

QVector <ProtocolWidget *> * ProtocolList::getProtocols() {
    protocols->clear();
    for (int itemIdx = 0; itemIdx < this->count(); itemIdx++) {
        ProtocolWidget * item = static_cast <ProtocolWidget *> (this->item(itemIdx));
        protocols->push_back(item);
    }
    return protocols;
}

ProtocolsSettingsDialog * ProtocolList::getProtocolsSettingsDialog() {
    return protocolsSettingsDlg;
}

void ProtocolList::startVhold0Protocol() {
    if (vhold0Protocol != nullptr) {
        this->setNullProtocolHolding(vhold0Protocol);

        protocolManager->startProtocol(vhold0Protocol);
        this->exportLastRunProtocol(vhold0Protocol);
    }
}

void ProtocolList::startIhold0Protocol() {
    if (ihold0Protocol != nullptr) {
        this->setNullProtocolHolding(ihold0Protocol);

        protocolManager->startProtocol(ihold0Protocol);
        this->exportLastRunProtocol(ihold0Protocol);
    }
}

void ProtocolList::recordIhold0Protocol() {
    if (ihold0Protocol != nullptr) {
        this->setNullProtocolHolding(ihold0Protocol);

        protocolManager->startProtocol(ihold0Protocol, true);
        this->exportLastRunProtocol(ihold0Protocol);
    }
}

void ProtocolList::setStopProtocolHold(e4gcl::Measurement_t hold) {
    nullGapfreeProtocol->setHold(hold);
    nullEpisodicProtocol->setHold(hold);
}

void ProtocolList::inhibitProtocols(bool inhibitFlag) {
    for (int protocolIdx = 0; protocolIdx < protocols->size(); protocolIdx++) {
        protocols->at(protocolIdx)->setInhibition(inhibitFlag);
    }
}

void ProtocolList::startProtocol(int shortCutIdx) {
    if (clampingModality == clampingModalitySet) {
        ProtocolWidget * protocol = shortCutsProtocols[shortCutIdx];
        if (protocol != nullptr) {
            this->setCurrentItem(protocol);
            this->onStartProtocol();
        }
    }
}

void ProtocolList::recordProtocol(int shortCutIdx) {
    if (clampingModality == clampingModalitySet) {
        ProtocolWidget * protocol = shortCutsProtocols[shortCutIdx];
        if (protocol != nullptr) {
            this->setCurrentItem(protocol);
            this->onRecordProtocol();
        }
    }
}

void ProtocolList::setClampingModality(int clampingModalitySet) {
    this->clampingModalitySet = clampingModalitySet;
    bool visible = clampingModality == clampingModalitySet;
    this->setVisible(visible);
    if (visible) {
        /*! If one specific modality is never used do not update its saved protocols */
        exportLastProtocolsFlag = true;
    }
}

void ProtocolList::saveAndClosePropertyDialog() {
    QString openedProtocolName;
    if (protocolPropertyDialog->isOpened(openedProtocolName)) {
        ProtocolWidget * openedProtocol = findProtocolByName(openedProtocolName);
        if (openedProtocol != nullptr) {
            openedProtocol->onAcceptPropertyDialog();
        }
    }
}

void ProtocolList::setSecondaryDevice(bool flag) {
    protocolManager->setSecondaryDevice(flag);
}

bool ProtocolList::eventFilter(QObject * obj, QEvent * event) {
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent * keyEvent = static_cast <QKeyEvent *> (event);
        if (keyEvent->key() == Qt::Key_Delete) {
            this->onRemoveProtocol();
        }
    }
    return QObject::eventFilter(obj, event);
}

void ProtocolList::contextMenuEvent(QContextMenuEvent * event) {
    QMenu menu(this);
    menu.addAction(startProtocolAct);
    menu.addAction(recordProtocolAct);
    menu.addSeparator();
    menu.addAction(copyProtocolAct);
    menu.addAction(editProtocolAct);
    menu.addAction(openProtocolPropertiesAct);
    menu.addAction(removeProtocolAct);
    menu.exec(event->globalPos());
}

void ProtocolList::onStartProtocol(bool recordFlag) {
    ProtocolWidget * protocol = static_cast <ProtocolWidget *> (this->currentItem());
    if (protocol == nullptr) {
        ErrorManager e(ErrorNoProtocolSelected);
        return;
    }

    int currentRangeIndex = protocol->getCurrentRangeIndex();
    if (currentRangeIndex >= 0) {
        emit requestCurrentRange(currentRangeIndex);
    }

    int voltageRangeIndex = protocol->getVoltageRangeIndex();
    if (voltageRangeIndex >= 0) {
        emit requestVoltageRange(voltageRangeIndex);
    }

    emit protocolAppliedRange(protocol->getAppliedRange());

    int samplingRateIndex = protocol->getSamplingRateIndex();
    if (samplingRateIndex >= 0) {
        emit requestSamplingRate(samplingRateIndex);
    }

    this->setNullProtocolHolding(protocol);

    if (!(protocol->isNullProtocol())) {
        this->exportLastRunProtocol(protocol);
    }

    ProtocolManager::ProtocolApplicationStatus_t ret = protocolManager->startProtocol(protocol, recordFlag);
    if (ret != ProtocolManager::Success) {
        ErrorManager e(ret);
    }
}

void ProtocolList::onRecordProtocol() {
    QSettings settings;

    QString fileName = settings.value(GLB_PROTOCOL_RECORD_NAME_TAG, "").toString();
    if (fileName == "") {
        this->onProtocolsSettings();
    }

    this->onStartProtocol(true);
}

void ProtocolList::onSaveLastProtocol() {
    QSettings settings;

    QString fileName = settings.value(GLB_PROTOCOL_RECORD_NAME_TAG, "").toString();
    if (fileName == "") {
        this->onProtocolsSettings();
    }

    this->importLastRunProtocol();

    protocolManager->saveLast(lastRunProtocol);
}

void ProtocolList::onStopProtocol() {
    if (protocolManager->getLastStartedType() == ProtocolTypeGapfree) {
        if (nullGapfreeProtocol != nullptr) {
            int currentRangeIndex = nullGapfreeProtocol->getCurrentRangeIndex();
            if (currentRangeIndex >= 0) {
                emit requestCurrentRange(currentRangeIndex);
            }

            int voltageRangeIndex = nullGapfreeProtocol->getVoltageRangeIndex();
            if (voltageRangeIndex >= 0) {
                emit requestVoltageRange(voltageRangeIndex);
            }

            protocolManager->startProtocol(nullGapfreeProtocol);
        }

    } else {
        if (nullEpisodicProtocol != nullptr) {
            int currentRangeIndex = nullEpisodicProtocol->getCurrentRangeIndex();
            if (currentRangeIndex >= 0) {
                emit requestCurrentRange(currentRangeIndex);
            }

            int voltageRangeIndex = nullEpisodicProtocol->getVoltageRangeIndex();
            if (voltageRangeIndex >= 0) {
                emit requestVoltageRange(voltageRangeIndex);
            }

            protocolManager->startProtocol(nullEpisodicProtocol);
        }
    }
}

void ProtocolList::onAddProtocol() {
    QDialog * dialog = new QDialog();

    dialog->setWindowFlags(Qt::WindowTitleHint);
    dialog->setWindowTitle("New Protocol");

    QVBoxLayout * dialogLo = new QVBoxLayout();
    dialog->setLayout(dialogLo);

    /*! Get new protocol name */
    QHBoxLayout * nameLo = new QHBoxLayout();
    dialogLo->addLayout(nameLo);

    QLabel * nameLbl = new QLabel("Protocol name:");
    QLineEdit * nameEdit = new QLineEdit(QString("Protocol %1").arg(createdProtocolIdx));
    nameEdit->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Minimum);

    nameLo->addWidget(nameLbl);
    nameLo->addWidget(nameEdit);

    /*! Get new protocol type (gap-free or episodic) */
    QComboBox * protocolTypeEdit = new QComboBox();
    protocolTypeEdit->addItem("Gap-Free");
    protocolTypeEdit->addItem("Episodic");

    dialogLo->addWidget(protocolTypeEdit);

    /*! OK / Cancel buttons */
    QHBoxLayout * buttonsLo = new QHBoxLayout();
    dialogLo->addLayout(buttonsLo);

    QPushButton * okButton = new QPushButton("OK");
    okButton->setCheckable(false);
    buttonsLo->addWidget(okButton);

    connect(okButton, &QPushButton::clicked, dialog, &QDialog::accept);
    okButton->setDefault(true);

    QPushButton * cancButton = new QPushButton("Cancel");
    cancButton->setCheckable(false);
    buttonsLo->addWidget(cancButton);
    connect(cancButton, &QPushButton::clicked, dialog, &QDialog::reject);

    bool ok = (dialog->exec() == QDialog::Accepted ? true : false);

    QString name = nameEdit->text();

    if (ok && !name.isEmpty()) {
        if (protocolsNames->contains(name)) {
            ErrorManager e(ErrorProtocolAlreadyExists);

        } else {
            ProtocolWidget * protocol;
            if (protocolTypeEdit->currentIndex() == 0) {
                protocol = newGapfreeProtocol(name);

            } else {
                protocol = newEpisodicProtocol(name);
            }

            this->addItem(protocol);
            protocols->push_back(protocol);
            protocolsNames->push_back(name);

            if (name == QString("Protocol %1").arg(createdProtocolIdx)) {
                /*! If name Protocol x is actually used increment x to set a different name next time */
                createdProtocolIdx++;
            }
            connect(protocol, &ProtocolWidget::nameChanged, this, &ProtocolList::onProtocolNameChanged);
            int dlgRet = protocol->openProtocolEditor();
            if (dlgRet == QDialog::Rejected) {
                this->removeProtocol(protocol, protocol->getName());
            }
        }
    }
}

void ProtocolList::onRemoveProtocol() {
    ProtocolWidget * protocol = static_cast <ProtocolWidget *> (this->currentItem());
    /*! \todo FCON chiedere conferma */
    if (protocol == nullptr) {
        ErrorManager e(ErrorNoProtocolSelected);
        return;
    }

    if (protocolPropertyDialog->getOwner() == protocol) {
        protocol->onRejectPropertyDialog();
    }

    QString name = protocol->getName();

    this->removeProtocol(protocol, name);
}

void ProtocolList::onEditProtocol() {
    ProtocolWidget * protocol = static_cast <ProtocolWidget *> (this->currentItem());
    if (protocol == nullptr) {
        ErrorManager e(ErrorNoProtocolSelected);
        return;
    }

    if (protocolPropertyDialog->getOwner() == protocol) {
        QMessageBox msgBox;
        msgBox.setText("Before editing a protocol you\nneed to close its property panel");
        msgBox.addButton("OK", QMessageBox::NoRole);

        msgBox.exec();

    } else {
        this->copyTempProtocol(protocol, protocol->getName(), protocol->getShortCutIdx());
        int dlgRet = protocol->openProtocolEditor();
        if (dlgRet == QDialog::Accepted) {
            this->deleteTempProtocol();

        } else {
            this->removeProtocol(protocol, protocol->getName());
            this->pasteTempProtocol();
        }
    }
}

void ProtocolList::onCopyProtocol() {
    ProtocolWidget * protocol = static_cast <ProtocolWidget *> (this->currentItem());
    if (protocol == nullptr) {
        ErrorManager e(ErrorNoProtocolSelected);
        return;
    }

    QString name = this->availableProtocolName("Copy of " + protocol->getName());

    this->copyTempProtocol(protocol, name, -1);
    this->pasteTempProtocol();
}

void ProtocolList::onSetProtocolsShortCuts() {
    this->populateShortCutsDialog();
    shortCutsDlg->open();
}

void ProtocolList::onImportProtocols() {
    QString openedProtocolName;
    bool importProtocolsFlag = true;
    if (protocolPropertyDialog->isOpened(openedProtocolName)) {
        ProtocolWidget * openedProtocol = findProtocolByName(openedProtocolName);
        ProtocolWidget::ClosureRequestReply_t reply = openedProtocol->askForPropertyDialogClosure();
        if (reply == ProtocolWidget::ClosureRequestRejected) {
            importProtocolsFlag = false;
        }
    }

    if (importProtocolsFlag) {
        ImportProtocolDialog * ipd = new ImportProtocolDialog(* protocolsNames, clampingModality, parent);
        if (ipd->exec() == QDialog::Accepted) {
            this->importProtocols(ipd);
        }

        delete ipd;
    }
}

void ProtocolList::onExportProtocols() {
    QString openedProtocolName;
    bool exportProtocolsFlag = true;
    if (protocolPropertyDialog->isOpened(openedProtocolName)) {
        ProtocolWidget * openedProtocol = findProtocolByName(openedProtocolName);
        ProtocolWidget::ClosureRequestReply_t reply = openedProtocol->askForPropertyDialogClosure();
        if (reply == ProtocolWidget::ClosureRequestRejected) {
            exportProtocolsFlag = false;
        }
    }

    if (exportProtocolsFlag) {
        ExportProtocolDialog * epd = new ExportProtocolDialog(* protocolsNames, clampingModality, parent);
        if (epd->exec() == QDialog::Accepted) {

            QVector <bool> saveFlag;
            QVector <bool> overwriteFlag;
            QStringList names;
            QStringList namesSet;

            QString fullFileName = epd->getWriteInfo(saveFlag, overwriteFlag, names, namesSet);
            if (fullFileName == "") {
                return;
            }

            EpmlManager * epmlManager = new EpmlManager(fullFileName, QIODevice::ReadWrite);
            EpmlManager * tempEpmlManager = new EpmlManager();
            EpmlStatus_t epmlStatus;

            int protocolsNum = protocolsNames->size();
            for (int protIdx = 0; protIdx < protocolsNum; protIdx++) {
                if (saveFlag[protIdx]) {
                    if (!tempEpmlManager->createProtocolNameSection(namesSet[protIdx], protocolsGroupName, epmlStatus)) {
                        /*! \todo FCON gestire uscita */
                        return;
                    }

                    QString parentTag = "protocol";
                    QString tag = "shortcutindex";
                    int depth = EPML_PROTOCOL_PARAM_DEPTH;
                    tempEpmlManager->addIntValue(tag, depth, this->getShortCutByProtocol(protocols->at(protIdx)));

                    protocols->at(protIdx)->exportEpml(tempEpmlManager, epmlStatus);
                    epmlManager->copyProtocol(tempEpmlManager, namesSet[protIdx], protocolsGroupName,
                                              overwriteFlag[protIdx], epmlStatus);
                }
            }

            if (!(epmlManager->writeToFile())) {
                /*! \todo FCON gestire l'uscita */
                return;
            }

            delete epmlManager;
            delete tempEpmlManager;
        }

        delete epd;
    }
}

void ProtocolList::onProtocolsSettings() {
    protocolsSettingsDlg->exec();
}

void ProtocolList::onHoldingDeltaChanged(e4gcl::Measurement_t newHoldingDelta) {
    holdingDelta = newHoldingDelta;

    for (int protIdx = 0; protIdx < protocols->size(); protIdx++) {
        protocols->at(protIdx)->setHoldingDelta(holdingDelta);
    }
}

void ProtocolList::onProtocolNameChanged(QString oldName, QString newName) {
    protocolsNames->replace(protocolsNames->indexOf(oldName), newName);
}

void ProtocolList::onPlotting(bool flag, ProtocolType_t) {
    plottingFlag = flag;
    if (plottingFlag == true) {
        /*! Whenever a plot starts controlsChangeFlag is reset */
        controlsChangedFlag = false;
    }

    /*! The save last button is enbaled if we are not plotting anymore and there has been no change in the controls in the meanwhile */
    emit enableSaveLastProtocol((!plottingFlag) && (!controlsChangedFlag));
}

void ProtocolList::onControlsChanged() {
    controlsChangedFlag = true;
    if (!plottingFlag) {
        /*! If the controls change after the plot ends disable the save last protocol button */
        emit enableSaveLastProtocol(false);
    }
}

void ProtocolList::onItemDoubleClicked(QListWidgetItem * item) {
    QString openedProtocolName;
    bool openProtocolPropertyDialog = true;
    if (protocolPropertyDialog->isOpened(openedProtocolName)) {
        ProtocolWidget * openedProtocol = findProtocolByName(openedProtocolName);
        ProtocolWidget::ClosureRequestReply_t reply = openedProtocol->askForPropertyDialogClosure();
        if (reply == ProtocolWidget::ClosureRequestRejected) {
            openProtocolPropertyDialog = false;
        }
    }

    if (openProtocolPropertyDialog) {
        ProtocolWidget * protocol = static_cast <ProtocolWidget *> (item);

        QStringList otherProtocolsNames = * protocolsNames;
        otherProtocolsNames.removeOne(protocol->getName());
        protocol->openPropertyDialog(otherProtocolsNames);
    }
}

void ProtocolList::onProtocolEnded() {
    protocolManager->onProtocolEnded();
}

void ProtocolList::createActions() {
    startProtocolAct = new QAction("Start protocol", this);
    startProtocolAct->setIcon(QIcon(":/imgs/start protocol.png"));
    connect(startProtocolAct, &QAction::triggered, this, &ProtocolList::onStartProtocol);

    recordProtocolAct = new QAction("Record protocol", this);
    recordProtocolAct->setIcon(QIcon(":/imgs/record protocol.png"));
    connect(recordProtocolAct, &QAction::triggered, this, &ProtocolList::onRecordProtocol);

    copyProtocolAct = new QAction("Copy protocol", this);
    copyProtocolAct->setIcon(QIcon(":/imgs/copy protocol.png"));
    connect(copyProtocolAct, &QAction::triggered, this, &ProtocolList::onCopyProtocol);

    editProtocolAct = new QAction("Edit protocol", this);
    editProtocolAct->setIcon(QIcon(":/imgs/edit protocol.png"));
    connect(editProtocolAct, &QAction::triggered, this, &ProtocolList::onEditProtocol);

    openProtocolPropertiesAct = new QAction("Edit parameters", this);
    openProtocolPropertiesAct->setIcon(QIcon(":/imgs/edit parameters.png"));
    connect(openProtocolPropertiesAct, &QAction::triggered, this, [=] () {
        this->onItemDoubleClicked(this->currentItem());
    });

    removeProtocolAct = new QAction("Remove protocol", this);
    removeProtocolAct->setIcon(QIcon(":/imgs/remove protocol.png"));
    connect(removeProtocolAct, &QAction::triggered, this, &ProtocolList::onRemoveProtocol);
}

void ProtocolList::createShortCutsDialog() {
    shortCutsDlg = new QDialog();

    shortCutsDlg->setWindowFlags(Qt::WindowTitleHint);
    shortCutsDlg->setWindowTitle("Protocols shortcuts");

    QVBoxLayout * mainShortCutsDlgVl = new QVBoxLayout;
    shortCutsDlg->setLayout(mainShortCutsDlgVl);

    QLabel * description = new QLabel("Set shortcuts to run or record protocols.\n"
                                      "\nEach protocol can be assigned a digit d from 0 to 9.\n"
                                      "CTRL+d will run the protocol.\n"
                                      "CTRL+SHIFT+d will run and record the protocol.");
    mainShortCutsDlgVl->addWidget(description);

    shortCutsDlgLo = new QGridLayout;
    mainShortCutsDlgVl->addLayout(shortCutsDlgLo);
    shortCutsProtocols.resize(10);
    shortCutsProtocols.fill(nullptr);

    QDialogButtonBox * buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &ProtocolList::onAcceptShortCutsDialog);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &ProtocolList::onRejectShortCutsDialog);
    mainShortCutsDlgVl->addWidget(buttonBox);
}

void ProtocolList::populateShortCutsDialog() {
    shortCutIdxCbxs.clear(); /*!< List of the comboboxes, we collect them first and create cross-connections later */
    shortCutsIndexes.resize(protocols->size()); /*!< Maps protocol indexes to shortcut indexes (-1 for "None") */
    shortCutsIndexes.fill(-1);

    /*! Create a combobox for each protocol with a "None" field plus 0-9 fields */
    for (int protocolIdx = 0; protocolIdx < protocols->size(); protocolIdx++) {
        shortCutsDlgLo->addWidget(new QLabel(protocolsNames->at(protocolIdx)), protocolIdx, 0);
        QComboBox * shortCutIdxCbx = new QComboBox();

        shortCutIdxCbx->addItem("None");
        shortCutIdxCbx->setCurrentIndex(0);

        for (int shortCutIdx = 0; shortCutIdx < 10; shortCutIdx++) {
            shortCutIdxCbx->addItem(QString("%1").arg(shortCutIdx));

            if (shortCutsProtocols[shortCutIdx] != nullptr) {
                /*! If one shortcut is already assigned... */
                if (shortCutsProtocols[shortCutIdx] == protocols->at(protocolIdx)) {
                    /*! Set the current index and assign shortCutsIndexes if it's assigned to the current protocol... */
                    shortCutIdxCbx->setCurrentIndex(shortCutIdx+1);
                    shortCutsIndexes[protocolIdx] = shortCutIdx;

                } else {
                    /*! Otherwise disable the field for the current protocol */
                    QStandardItemModel * model = qobject_cast <QStandardItemModel *> (shortCutIdxCbx->model());
                    QStandardItem * item = model->item(shortCutIdx+1);
                    item->setFlags(item->flags() & ~Qt::ItemIsEnabled);
                }
            }
        }
        shortCutsDlgLo->addWidget(shortCutIdxCbx, protocolIdx, 1);
        shortCutIdxCbxs.append(shortCutIdxCbx);
    }

    /*! Cross-connect all comboboxes with one another, so that... */
    for (int idx1 = 0; idx1 < protocols->size(); idx1++) {
        connect(shortCutIdxCbxs[idx1], QOverload <int> ::of(&QComboBox::currentIndexChanged), this, [=](int index) {
            QStandardItemModel * model;
            QStandardItem * item;
            for (int idx2 = 0; idx2 < protocols->size(); idx2++) {
                if (idx1 != idx2) {
                    model = qobject_cast <QStandardItemModel *> (shortCutIdxCbxs[idx2]->model());
                    if (shortCutsIndexes[idx1] >= 0) {
                        /*! If one was set and gets changed the old shortcut is enabled for other protocols... */
                        item = model->item(shortCutsIndexes[idx1]+1);
                        item->setFlags(item->flags() | Qt::ItemIsEnabled);
                    }

                    if (index != 0) {
                        /*! While the new shortcut is disabled if it's not "None" */
                        item = model->item(index);
                        item->setFlags(item->flags() & ~Qt::ItemIsEnabled);
                    }
                }
            }
            shortCutsIndexes[idx1] = index-1;
        });
    }
}

void ProtocolList::clearShortCutsDialog() {
    QLayoutItem * item;
    int rowIdx;
    for (rowIdx = 0; rowIdx < protocols->size(); rowIdx++) {
        for (int colIdx = 0; colIdx < 2; colIdx++) {
            item = shortCutsDlgLo->itemAtPosition(rowIdx, colIdx);
            if (item != nullptr) {
                shortCutsDlgLo->removeItem(item);
                delete item->widget();
            }
        }
    }
}

int ProtocolList::getShortCutByProtocol(ProtocolWidget * protocol) {
    int ret = -1;
    for (int shortCutsIdx = 0; shortCutsIdx < 10; shortCutsIdx++) {
        if (shortCutsProtocols[shortCutsIdx] == protocol) {
            ret = shortCutsIdx;
            break;
        }
    }
    return ret;
}

void ProtocolList::setNullProtocolHolding(ProtocolWidget * protocol) {
    if (protocol->getType() == ProtocolTypeGapfree) {
        if (nullGapfreeProtocol != nullptr) {
            nullGapfreeProtocol->setStimulusRangeIndex(protocol->getStimulusRangeIndex());
            nullGapfreeProtocol->setHold(protocol->getHold());
        }

    } else {
        if (nullEpisodicProtocol != nullptr) {
            nullEpisodicProtocol->setStimulusRangeIndex(protocol->getStimulusRangeIndex());
            nullEpisodicProtocol->setHold(protocol->getHold());
        }
    }
}

void ProtocolList::exportLastRunProtocol(ProtocolWidget * protocol) {
    EpmlManager * epmlManager = new EpmlManager(EPML_LAST_PROTOCOL_FULL_FILE, QIODevice::WriteOnly);
    EpmlStatus_t epmlStatus;

    epmlManager->clear();

    if (!epmlManager->createProtocolNameSection("LAST", protocolsGroupName, epmlStatus)) {
        /*! \todo FCON gestire uscita, magari qui basta non rendere disponibile il tasto per il write last */
        return;
    }

    QString parentTag = "protocol";
    QString tag = "shortcutindex";
    int depth = EPML_PROTOCOL_PARAM_DEPTH;
    epmlManager->addIntValue(tag, depth, 0);

    protocol->exportEpml(epmlManager, epmlStatus);

    if (!(epmlManager->writeToFile())) {
        /*! \todo FCON gestire l'uscita */
        return;
    }

    delete epmlManager;
}

void ProtocolList::exportLastProtocols() {
    if (exportLastProtocolsFlag) {
        EpmlManager * epmlManager = new EpmlManager(EPML_LAST_FULL_FILE, QIODevice::ReadWrite);
        EpmlStatus_t epmlStatus;

        if (!(epmlManager->clearProtocolList(protocolsGroupName, epmlStatus))) {
            /*! \todo FCON gestire uscita */
            return;
        }

        int protocolsNum = protocolsNames->size();
        for (int protIdx = 0; protIdx < protocolsNum; protIdx++) {
            if (!epmlManager->createProtocolNameSection(protocolsNames->at(protIdx), protocolsGroupName, epmlStatus)) {
                /*! \todo FCON gestire uscita */
                return;
            }

            QString parentTag = "protocol";
            QString tag = "shortcutindex";
            int depth = EPML_PROTOCOL_PARAM_DEPTH;
            epmlManager->addIntValue(tag, depth, this->getShortCutByProtocol(protocols->at(protIdx)));

            protocols->at(protIdx)->exportEpml(epmlManager, epmlStatus);
        }

        if (!(epmlManager->writeToFile())) {
            /*! \todo FCON gestire l'uscita */
            return;
        }

        delete epmlManager;
    }
}

void ProtocolList::importNullProtocol() {
    nullProtocolFlag = true;
    if (!(this->importProtocols(EPML_NULL_FULL_FILE))) {
        ErrorManager e(ErrorLoadNullProtocolsFail);
    }
    nullProtocolFlag = false;
}

void ProtocolList::importVhold0Protocol() {
    vhold0ProtocolFlag = true;
    if (!(this->importProtocols(EPML_VHOLD0_FULL_FILE))) {
        ErrorManager e(ErrorLoadOffsetCompensationProtocolFail);
    }
    vhold0ProtocolFlag = false;
}

void ProtocolList::importIhold0Protocol() {
    ihold0ProtocolFlag = true;
    if (!(this->importProtocols(EPML_IHOLD0_FULL_FILE))) {
        ErrorManager e(ErrorLoadRestingPotentialProtocolFail);
    }
    ihold0ProtocolFlag = false;
}

void ProtocolList::importLastRunProtocol() {
    /*! Import the last run protocol */
    lastRunProtocolFlag = true;
    if (!(this->importProtocols(EPML_LAST_PROTOCOL_FULL_FILE))) {
        ErrorManager e(ErrorLoadLastExecutedProtocolFail);
    }
    lastRunProtocolFlag = false;
}

void ProtocolList::importLastProtocols() {
    /*! Import the default protocols if the last saved file is not found */
    if (!(this->importProtocols(EPML_LAST_FULL_FILE))) {
        ErrorManager e(ErrorLoadLastExecutionProtocolsFail);
        if (!(this->importProtocols())) {
            ErrorManager e(ErrorLoadDefaultProtocolsFail);
        }
    }
}

bool ProtocolList::importProtocols(QString fullFileName) {
    bool ret;
    EpmlManager * epmlManager = new EpmlManager(fullFileName, QIODevice::ReadOnly);

    if (epmlManager->fileExists()) {
        QString tag = protocolsGroupName;
        int depth = EPML_PROTOCOL_LIST_DEPTH;
        QString parentTag = "";

        EpmlStatus_t epmlStatus;

        if (epmlManager->getNext(tag, depth, parentTag, epmlStatus)) {
            parentTag = tag;
            depth = EPML_PROTOCOL_DEPTH;
            tag = "protocol";
            while (epmlManager->getNext(tag, depth, parentTag, epmlStatus)) {
                this->importProtocol(epmlManager, tag, epmlStatus);
            }
        }
        ret = true;

    } else {
        ret = false;
    }
    delete epmlManager;
    return ret;
}

bool ProtocolList::importProtocols(ImportProtocolDialog * ipd) {
    bool ret;
    QVector <bool> saveFlag;
    QVector <bool> overwriteFlag;
    QStringList names;
    QStringList namesSet;

    QString fullFileName = ipd->getWriteInfo(saveFlag, overwriteFlag, names, namesSet);
    if (fullFileName == "") {
        return false;
    }

    EpmlManager * epmlManager = new EpmlManager(fullFileName, QIODevice::ReadOnly);

    int protocolsNum = namesSet.size();

    QString tag = protocolsGroupName;
    int depth = EPML_PROTOCOL_LIST_DEPTH;
    QString parentTag = "";

    EpmlStatus_t epmlStatus;

    if (epmlManager->getNext(tag, depth, parentTag, epmlStatus)) {
        for (int protIdx = 0; protIdx < protocolsNum; protIdx++) {
            if (saveFlag[protIdx]) {
                if (overwriteFlag[protIdx]) {
                    this->removeProtocolByName(namesSet[protIdx]);
                }
                parentTag = tag;
                depth = EPML_PROTOCOL_DEPTH;
                tag = "protocol";
                if (epmlManager->getNextSectionByName(tag, depth, names[protIdx], parentTag, epmlStatus)) {
                    this->importProtocolAs(epmlManager, namesSet[protIdx], epmlStatus);
                }
            }
        }
        ret = true;

    } else {
        ret = false;
    }

    delete epmlManager;
    return ret;
}

void ProtocolList::importProtocol(EpmlManager * epmlManager, QString parentTag, EpmlStatus_t &epmlStatus) {
    int depth = EPML_PROTOCOL_PARAM_DEPTH;
    QString tag = "name";
    epmlManager->getNext(tag, depth, parentTag, epmlStatus);

    QString name = epmlManager->getString(tag, depth, epmlStatus);
    if (nullProtocolFlag || vhold0ProtocolFlag || ihold0ProtocolFlag) {
        name = "";
    }

    if (protocolsNames->contains(name)) {
        this->removeProtocolByName(name);
    }

    if (epmlStatus == EpmlValueFound) {
        this->importProtocolAs(epmlManager, name, epmlStatus);

    } else {
        ErrorManager e("Failed to load protocol", "Protocol missing name keyword");
    }
}

void ProtocolList::importProtocolAs(EpmlManager * epmlManager, QString name, EpmlStatus_t &epmlStatus) {
    int depth = EPML_PROTOCOL_PARAM_DEPTH;

    QString tag = "shortcutindex";
    QString parentTag = "protocol";
    epmlManager->getNext(tag, depth, parentTag, epmlStatus);
    int shortCutIdx = epmlManager->getInt(tag, depth, epmlStatus);

    tag = "type";
    epmlManager->getNext(tag, depth, parentTag, epmlStatus);

    bool addProtocolToListFlag = true;

    QString type = epmlManager->getString(tag, depth, epmlStatus);
    if (epmlStatus == EpmlValueFound) {
        ProtocolWidget * protocol;
        /*! Forcing to import the last run protocol as an episodic ensures that if it gets saved to disk
         *  The recording stops when the protocol stops */
        if ((type == "Gap Free") && (!lastRunProtocolFlag)) {
            protocol = this->newGapfreeProtocol(name);

        } else {
            protocol = this->newEpisodicProtocol(name);
        }

        if (protocol->importEpml(epmlManager, epmlStatus)) {
            if (nullProtocolFlag) {
                protocol->setNullProtocol(true);
                if (protocol->getType() == ProtocolTypeGapfree) {
                    nullGapfreeProtocol = protocol;

                } else {
                    nullEpisodicProtocol = protocol;
                }
                addProtocolToListFlag = false;
            }

            if (vhold0ProtocolFlag) {
                vhold0Protocol = protocol;
                addProtocolToListFlag = false;
            }

            if (ihold0ProtocolFlag) {
                ihold0Protocol = protocol;
                addProtocolToListFlag = false;
            }

            if (lastRunProtocolFlag) {
                if (lastRunProtocol != nullptr) {
                    delete lastRunProtocol;
                    lastRunProtocol = nullptr;
                }
                lastRunProtocol = protocol;
                addProtocolToListFlag = false;
            }

            if (addProtocolToListFlag) {
                this->addItem(protocol);
                protocols->push_back(protocol);
                protocolsNames->push_back(name);
                if ((shortCutIdx >= 0) && (shortCutsProtocols[shortCutIdx] == nullptr)) {
                    shortCutsProtocols[shortCutIdx] = protocol;
                    protocol->setShortCutIdx(shortCutIdx);
                }
                connect(protocol, &ProtocolWidget::nameChanged, this, &ProtocolList::onProtocolNameChanged);
            }

        } else {
            ErrorManager e("Failed to load protocol " + name, "Protocol format corrupted");
            delete protocol;
        }

    } else {
        ErrorManager e("Failed to load protocol " + name, "Protocol missing type keyword");
    }
}

void ProtocolList::copyTempProtocol(ProtocolWidget * protocol, QString name, int shortCutIdx) {
    this->deleteTempProtocol();
    tempEpmlManager = new EpmlManager();
    EpmlStatus_t epmlStatus;

    if (!tempEpmlManager->createProtocolNameSection(name, protocolsGroupName, epmlStatus)) {
        /*! \todo FCON gestire uscita */
        return;
    }

    QString parentTag = "protocol";
    QString tag = "shortcutindex";
    int depth = EPML_PROTOCOL_PARAM_DEPTH;
    tempEpmlManager->addIntValue(tag, depth, shortCutIdx);

    protocol->exportEpml(tempEpmlManager, epmlStatus);
}

void ProtocolList::pasteTempProtocol() {
    EpmlStatus_t epmlStatus;

    tempEpmlManager->reset();
    QString tag = "protocol";
    this->importProtocol(tempEpmlManager, tag, epmlStatus);

    this->deleteTempProtocol();
}

void ProtocolList::deleteTempProtocol() {
    if (tempEpmlManager != nullptr) {
        delete tempEpmlManager;
        tempEpmlManager = nullptr;
    }
}

void ProtocolList::removeProtocolByName(QString name) {
    ProtocolWidget * protocol = this->findProtocolByName(name);
    if (protocol == nullptr) {
        return;
    }

    this->removeProtocol(protocol, name);
}

void ProtocolList::removeProtocol(ProtocolWidget * protocol, QString name) {

    /*! If the protocol was assigned a shortcut remove it */
    int shortCutIdx = this->getShortCutByProtocol(protocol);
    if (shortCutIdx >= 0) {
        shortCutsProtocols[shortCutIdx] = nullptr;
    }

    delete protocol;
    protocols->removeOne(protocol);
    protocolsNames->removeOne(name);
}

ProtocolWidget * ProtocolList::findProtocolByName(QString name) {
    ProtocolWidget * protocol;
    for (int protIdx = 0; protIdx < protocols->size(); protIdx++) {
        protocol = protocols->at(protIdx);
        if (protocol->getName() == name) {
            return protocol;
        }
    }
    return nullptr;
}

QString ProtocolList::availableProtocolName(QString name) {
    QString ret = name;
    int idx = 1;
    while (protocolsNames->contains(ret)) {
        ret = name + QString::number(idx++);
    }
    return ret;
}

void ProtocolList::onAcceptShortCutsDialog() {
    shortCutsProtocols.fill(nullptr);
    for (int protocolIdx = 0; protocolIdx < protocols->size(); protocolIdx++) {
        if (shortCutIdxCbxs[protocolIdx]->currentIndex() != 0) {
            shortCutsProtocols[shortCutsIndexes[protocolIdx]] = protocols->at(protocolIdx);
        }
        protocols->at(protocolIdx)->setShortCutIdx(shortCutsIndexes[protocolIdx]);
    }
    this->clearShortCutsDialog();
    shortCutsDlg->accept();
}

void ProtocolList::onRejectShortCutsDialog() {
    this->clearShortCutsDialog();
    shortCutsDlg->reject();
}

VoltageProtocolList::VoltageProtocolList(e4gcl::CommLib * commLib, ProtocolPropertyDialog * protocolPropertyDialog, QWidget * parent) :
    ProtocolList(commLib, protocolPropertyDialog, parent) {

    clampingModality = E4GCL_VOLTAGE_CLAMP_MODE;
    protocolsGroupName = "voltageprotocols";
    this->importNullProtocol();
    this->importVhold0Protocol();
    this->importLastProtocols();
    this->onStopProtocol();
    QThread::msleep(100);
}

VoltageProtocolList::~VoltageProtocolList() {

}

ProtocolWidget * VoltageProtocolList::newGapfreeProtocol(QString name) {
    ProtocolWidget * protocol = new GapfreeVoltageProtocolWidget(commLib, name, protocolPropertyDialog);
    return protocol;
}

ProtocolWidget * VoltageProtocolList::newEpisodicProtocol(QString name) {
    ProtocolWidget * protocol = new EpisodicVoltageProtocolWidget(commLib, name, protocolPropertyDialog);
    return protocol;
}

CurrentProtocolList::CurrentProtocolList(e4gcl::CommLib * commLib, ProtocolPropertyDialog * protocolPropertyDialog, QWidget * parent) :
    ProtocolList(commLib, protocolPropertyDialog, parent) {

    clampingModality = E4GCL_CURRENT_CLAMP_MODE;
    protocolsGroupName = "currentprotocols";

    this->importNullProtocol();
    this->importIhold0Protocol();
    this->importLastProtocols();
    this->onStopProtocol();
    QThread::msleep(100);
}

CurrentProtocolList::~CurrentProtocolList() {

}

ProtocolWidget * CurrentProtocolList::newGapfreeProtocol(QString name) {
    ProtocolWidget * protocol = new GapfreeCurrentProtocolWidget(commLib, name, protocolPropertyDialog);
    return protocol;
}

ProtocolWidget * CurrentProtocolList::newEpisodicProtocol(QString name) {
    ProtocolWidget * protocol = new EpisodicCurrentProtocolWidget(commLib, name, protocolPropertyDialog);
    return protocol;
}
