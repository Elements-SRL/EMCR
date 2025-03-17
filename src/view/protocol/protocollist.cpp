#include "protocollist.h"

#include <fstream>

#include <QInputDialog>
#include <QSettings>
#include <QFileDialog>
#include <QMessageBox>
#include <QMenu>
#include <QDialogButtonBox>

#include "messagedispatcher.h"
#include "errormanager.h"

namespace e384cl = e384CommLib;

static int createdProtocolIdx = 0;

ProtocolList::ProtocolList(MessageDispatcher * msgDisp, ProtocolPropertyDialog * protocolPropertyDialog, QWidget * parent) :
    QListWidget(parent),
    msgDisp(msgDisp),
    protocolPropertyDialog(protocolPropertyDialog),
    parent(parent) {

    this->setDragDropMode(QAbstractItemView::InternalMove);
    this->setIconSize(QSize(25, 25));
    connect(this, &ProtocolList::itemDoubleClicked, this, &ProtocolList::onItemDoubleClicked);
    protocols = new QVector <ProtocolWidget *>;
    protocolsNames = new QStringList;

    this->installEventFilter(this);

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

    for (int itemIdx = 0; itemIdx < this->count(); itemIdx++) {
        ProtocolWidget * item = static_cast <ProtocolWidget *> (this->item(itemIdx));
        delete item;
    }

    protocolsNames->clear();

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

void ProtocolList::startVhold0Protocol() {
    if (vhold0Protocol != nullptr) {
        lastStartedType = vhold0Protocol->getType();
        emit startProtocolRequest(vhold0Protocol);
    }
}

void ProtocolList::startIhold0Protocol() {
    if (ihold0Protocol != nullptr) {
        lastStartedType = ihold0Protocol->getType();
        emit startProtocolRequest(ihold0Protocol);
    }
}

void ProtocolList::inhibitProtocols(bool inhibitFlag) {
    for (int protocolIdx = 0; protocolIdx < protocols->size(); protocolIdx++) {
        protocols->at(protocolIdx)->setInhibition(inhibitFlag);
    }
}

void ProtocolList::startProtocolFromShortCutIndex(int shortCutIdx) {
    if (clampingModality == clampingModalitySet && this->isVisible()) {
        ProtocolWidget * protocol = shortCutsProtocols[shortCutIdx];
        if (protocol != nullptr) {
            this->setCurrentItem(protocol);
            this->onStartProtocol();
        }
    }
}

void ProtocolList::setClampingModality(ClampingModality_t clampingModalitySet) {
    this->clampingModalitySet = clampingModalitySet;
    bool visible = clampingModality == clampingModalitySet;
    this->setVisible(visible);
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
    menu.addSeparator();
    menu.addAction(copyProtocolAct);
    menu.addAction(editProtocolAct);
    menu.addAction(openProtocolPropertiesAct);
    menu.addAction(removeProtocolAct);
    menu.exec(event->globalPos());
}

void ProtocolList::onStartProtocol(bool) {
    if (clampingModality != clampingModalitySet) {
        return;
    }

    if (!(this->isVisible())) {
        return;
    }

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

    lastStartedType = protocol->getType();
    emit startProtocolRequest(protocol);
}

void ProtocolList::onStopProtocol() {
    if (clampingModality != clampingModalitySet) {
        return;
    }
    msgDisp->stopProtocol();
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

    QComboBox * protocolTypeEdit = nullptr;
    if (msgDisp->isEpisodic() == e384cl::Success) {
        /*! Get new protocol type (gap-free or episodic) */
        protocolTypeEdit = new QComboBox();
        protocolTypeEdit->addItem("Gap-Free");
        protocolTypeEdit->addItem("Episodic");

        dialogLo->addWidget(protocolTypeEdit);
    }

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
        }
        else {
            ProtocolWidget * protocol;
            if (protocolTypeEdit != nullptr) {
                if (protocolTypeEdit->currentIndex() == 0) {
                    protocol = newGapfreeProtocol(name);
                }
                else {
                    protocol = newEpisodicProtocol(name);
                }
            }
            else {
                protocol = newGapfreeProtocol(name);
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
        if (clampingModality == ClampingModality_t::VOLTAGE_CLAMP) {
            YAML::VoltageProtocol_t yamlProtocol = this->copyVoltageProtocol(protocol);
            int dlgRet = protocol->openProtocolEditor();
            if (dlgRet == QDialog::Accepted) {

            } else {
                this->removeProtocol(protocol, protocol->getName());
                this->pasteVoltageProtocol(yamlProtocol);
            }

        } else {
            YAML::CurrentProtocol_t yamlProtocol = this->copyCurrentProtocol(protocol);
            int dlgRet = protocol->openProtocolEditor();
            if (dlgRet == QDialog::Accepted) {

            } else {
                this->removeProtocol(protocol, protocol->getName());
                this->pasteCurrentProtocol(yamlProtocol);
            }
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

    if (clampingModality == ClampingModality_t::VOLTAGE_CLAMP) {
        YAML::VoltageProtocol_t yamlProtocol = this->copyVoltageProtocol(protocol);
        yamlProtocol.name = name.toStdString();
        yamlProtocol.shortcutindex = -1;

        this->pasteVoltageProtocol(yamlProtocol);

    } else {
        YAML::CurrentProtocol_t yamlProtocol = this->copyCurrentProtocol(protocol);
        yamlProtocol.name = name.toStdString();
        yamlProtocol.shortcutindex = -1;

        this->pasteCurrentProtocol(yamlProtocol);
    }
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

            YAML::Node node;
            node = this->getYamlProtocols();

            YAML::Protocols_t yamlProtocols = node.as <YAML::Protocols_t> ();
            YAML::Protocols_t yamlTempProtocols;

            if (QFile::exists(fullFileName)) {
                node = YAML::LoadFile(fullFileName.toStdString());
                yamlTempProtocols = node.as <YAML::Protocols_t> ();
            }

            if (clampingModality == ClampingModality_t::VOLTAGE_CLAMP) {
                for (unsigned int protIdx = 0; protIdx < yamlProtocols.voltageprotocols.size(); protIdx++) {
                    if (saveFlag[protIdx]) {
                        auto yamlProtocol = yamlProtocols.voltageprotocols[protIdx];
                        yamlProtocol.name = namesSet[protIdx].toStdString();
                        if (overwriteFlag[protIdx]) {
                            yamlTempProtocols.replaceProtocol(yamlProtocol);

                        } else {
                            yamlTempProtocols.addProtocol(yamlProtocol);
                        }
                    }
                }

            } else {
                for (unsigned int protIdx = 0; protIdx < yamlProtocols.currentprotocols.size(); protIdx++) {
                    if (saveFlag[protIdx]) {
                        auto yamlProtocol = yamlProtocols.currentprotocols[protIdx];
                        yamlProtocol.name = namesSet[protIdx].toStdString();
                        if (overwriteFlag[protIdx]) {
                            yamlTempProtocols.replaceProtocol(yamlProtocol);

                        } else {
                            yamlTempProtocols.addProtocol(yamlProtocol);
                        }
                    }
                }
            }

            std::ofstream fout(fullFileName.toStdString());
            node = yamlTempProtocols;
            fout << node;
            fout.close();
        }

        delete epd;
    }
}

void ProtocolList::onHoldingDeltaChanged(Measurement_t newHoldingDelta) {
    holdingDelta = newHoldingDelta;

    for (int protIdx = 0; protIdx < protocols->size(); protIdx++) {
        protocols->at(protIdx)->setHoldingDelta(holdingDelta);
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

void ProtocolList::onProtocolNameChanged(QString oldName, QString newName) {
    protocolsNames->replace(protocolsNames->indexOf(oldName), newName);
}

void ProtocolList::onProtocolRequestOutcome(ProtocolApplicationStatus_t status) {
    if (status != ProtocolApplicationSuccess) {
        ErrorManager e(status);
    }
}

void ProtocolList::createActions() {
    startProtocolAct = new QAction("Start protocol", this);
    startProtocolAct->setIcon(QIcon(":/imgs/start protocol.png"));
    connect(startProtocolAct, &QAction::triggered, this, &ProtocolList::onStartProtocol);

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
                                      "CTRL+d will run the protocol.");
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

void ProtocolList::exportLastProtocols() {
    QString fullFileName = YAML_LAST_FULL_FILE;
    YAML::Protocols yamlProtocols;
    YAML::Node node;

    if (QFile::exists(fullFileName)) {
        node = YAML::LoadFile(fullFileName.toStdString());
        yamlProtocols = node.as <YAML::Protocols_t> ();
    }

    if (clampingModality == ClampingModality_t::VOLTAGE_CLAMP) {
        auto prots = this->getYamlProtocols().voltageprotocols;
        if (!prots.empty()) {
            yamlProtocols.voltageprotocols = prots;
        }
    }
    else {
        auto prots = this->getYamlProtocols().currentprotocols;
        if (!prots.empty()) {
            yamlProtocols.currentprotocols = prots;
        }
    }

    node = yamlProtocols;
    std::ofstream fout(fullFileName.toStdString());
    fout << node;
    fout.close();
}

void ProtocolList::exportAnalysisProtocols() {
    QString fullFileName = YAML_ANALYSIS_FULL_FILE;
    YAML::Protocols yamlProtocols;
    YAML::Node node;

    if (QFile::exists(fullFileName)) {
        node = YAML::LoadFile(fullFileName.toStdString());
        yamlProtocols = node.as <YAML::Protocols_t> ();
    }

    if (clampingModality == ClampingModality_t::VOLTAGE_CLAMP) {
        auto prots = this->getYamlProtocols().voltageprotocols;
        if (!prots.empty()) {
            yamlProtocols.voltageprotocols = prots;
        }
    }
    else {
        auto prots = this->getYamlProtocols().currentprotocols;
        if (!prots.empty()) {
            yamlProtocols.currentprotocols = prots;
        }
    }

    node = yamlProtocols;
    std::ofstream fout(fullFileName.toStdString());
    fout << node;
    fout.close();
}

void ProtocolList::importVhold0Protocol() {
    vhold0ProtocolFlag = true;
    if (!(this->importProtocols(YAML_VHOLD0_FULL_FILE))) {
        ErrorManager e(ErrorLoadOffsetCompensationProtocolFail);
    }
    vhold0ProtocolFlag = false;
}

void ProtocolList::importIhold0Protocol() {
    ihold0ProtocolFlag = true;
    if (!(this->importProtocols(YAML_IHOLD0_FULL_FILE))) {
        ErrorManager e(ErrorLoadRestingPotentialProtocolFail);
    }
    ihold0ProtocolFlag = false;
}

void ProtocolList::importLastRunProtocol() {
    /*! Import the last run protocol */
    lastRunProtocolFlag = true;
    if (!(this->importProtocols(YAML_LAST_PROTOCOL_FULL_FILE))) {
        ErrorManager e(ErrorLoadLastExecutedProtocolFail);
    }
    lastRunProtocolFlag = false;
}

void ProtocolList::importLastProtocols() {
    /*! Import the default protocols if the last saved file is not found */
    if (!(this->importProtocols(YAML_LAST_FULL_FILE))) {
        ErrorManager e(ErrorLoadLastExecutionProtocolsFail);
        if (!(this->importProtocols())) {
            ErrorManager e(ErrorLoadDefaultProtocolsFail);
        }
    }
}

void ProtocolList::importAnalysisProtocols() {
    /*! Import the protocols used to perform analysis */
    if (!(this->importProtocols(YAML_ANALYSIS_FULL_FILE))) {
        ErrorManager e(ErrorLoadAnalysisProtocolsFail);
    }
}

bool ProtocolList::importProtocols(QString fullFileName) {
    QString yamlFileName = fullFileName;
    if (QFile::exists(yamlFileName)) {
        fullFileName = yamlFileName;

        YAML::Node node = YAML::LoadFile(fullFileName.toStdString());
        YAML::Protocols yamlProtocols = node.as <YAML::Protocols> ();

        if (clampingModality == ClampingModality_t::VOLTAGE_CLAMP) {
            for (auto yamlProtocol : yamlProtocols.voltageprotocols) {
                this->importProtocol(yamlProtocol);
            }

        } else {
            for (auto yamlProtocol : yamlProtocols.currentprotocols) {
                this->importProtocol(yamlProtocol);
            }
        }
        return true;
    }
    else {
        return false;
    }
}

bool ProtocolList::importProtocols(ImportProtocolDialog * ipd) {
    bool ret = true;
    QVector <bool> saveFlag;
    QVector <bool> overwriteFlag;
    QStringList names;
    QStringList namesSet;

    QString fullFileName = ipd->getWriteInfo(saveFlag, overwriteFlag, names, namesSet);
    if (fullFileName == "") {
        return false;
    }

    if (fullFileName.endsWith(YAML_FILE_EXTENSION)) {
        YAML::Node node = YAML::LoadFile(fullFileName.toStdString());
        YAML::Protocols yamlProtocols = node.as <YAML::Protocols> ();

        if (clampingModality == ClampingModality_t::VOLTAGE_CLAMP) {
            for (unsigned int protIdx = 0; protIdx < yamlProtocols.voltageprotocols.size(); protIdx++) {
                YAML::VoltageProtocol_t yamlProtocol = yamlProtocols.voltageprotocols[protIdx];
                if (yamlProtocol.sweeps == 1 || (msgDisp->isEpisodic() != e384cl::Success)) { /*! Don't import if the protocol is episodic, but the device is not */
                    if (saveFlag[protIdx]) {
                        if (overwriteFlag[protIdx]) {
                            this->removeProtocolByName(namesSet[protIdx]);
                        }
                        this->importProtocolAs(yamlProtocol, namesSet[protIdx]);
                    }
                }
            }

        } else {
            for (unsigned int protIdx = 0; protIdx < yamlProtocols.currentprotocols.size(); protIdx++) {
                YAML::CurrentProtocol_t yamlProtocol = yamlProtocols.currentprotocols[protIdx];
                if (yamlProtocol.sweeps == 1 || (msgDisp->isEpisodic() != e384cl::Success)) { /*! Don't import if the protocol is episodic, but the device is not */
                    if (saveFlag[protIdx]) {
                        if (overwriteFlag[protIdx]) {
                            this->removeProtocolByName(namesSet[protIdx]);
                        }
                        this->importProtocolAs(yamlProtocol, namesSet[protIdx]);
                    }
                }
            }
        }
    }
    return ret;
}

void ProtocolList::importProtocol(const YAML::VoltageProtocol &yamlProtocol) {
    QString name = QString::fromStdString(yamlProtocol.name);
    if (vhold0ProtocolFlag || ihold0ProtocolFlag) {
        name = "";
    }

    if (protocolsNames->contains(name)) {
        this->removeProtocolByName(name);
    }

    this->importProtocolAs(yamlProtocol, name);
}

void ProtocolList::importProtocol(const YAML::CurrentProtocol &yamlProtocol) {
    QString name = QString::fromStdString(yamlProtocol.name);
    if (vhold0ProtocolFlag || ihold0ProtocolFlag) {
        name = "";
    }

    if (protocolsNames->contains(name)) {
        this->removeProtocolByName(name);
    }

    this->importProtocolAs(yamlProtocol, name);
}

void ProtocolList::importProtocolAs(const YAML::VoltageProtocol &yamlProtocol, QString name) {
    bool addProtocolToListFlag = true;

    ProtocolWidget * protocol;
    /*! Forcing to import the last run protocol as an episodic ensures that if it gets saved to disk
         *  The recording stops when the protocol stops */
    if ((yamlProtocol.operationmode == YAML::GapFree) && (!lastRunProtocolFlag)) {
        protocol = this->newGapfreeProtocol(name);

    } else {
        protocol = this->newEpisodicProtocol(name);
    }

    protocol->setProtocolFromYaml(yamlProtocol);

    if (true) {
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
            if ((yamlProtocol.shortcutindex >= 0) && (shortCutsProtocols[yamlProtocol.shortcutindex] == nullptr)) {
                shortCutsProtocols[yamlProtocol.shortcutindex] = protocol;
                protocol->setShortCutIdx(yamlProtocol.shortcutindex);
            }
            connect(protocol, &ProtocolWidget::nameChanged, this, &ProtocolList::onProtocolNameChanged);
        }

    } else {
        ErrorManager e("Failed to load protocol " + name, "Protocol format corrupted");
        delete protocol;
    }
}

void ProtocolList::importProtocolAs(const YAML::CurrentProtocol &yamlProtocol, QString name) {
    bool addProtocolToListFlag = true;

    ProtocolWidget * protocol;
    /*! Forcing to import the last run protocol as an episodic ensures that if it gets saved to disk
         *  The recording stops when the protocol stops */
    if ((yamlProtocol.operationmode == YAML::GapFree) && (!lastRunProtocolFlag)) {
        protocol = this->newGapfreeProtocol(name);

    } else {
        protocol = this->newEpisodicProtocol(name);
    }

    protocol->setProtocolFromYaml(yamlProtocol);

    if (true) {
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
            if ((yamlProtocol.shortcutindex >= 0) && (shortCutsProtocols[yamlProtocol.shortcutindex] == nullptr)) {
                shortCutsProtocols[yamlProtocol.shortcutindex] = protocol;
                protocol->setShortCutIdx(yamlProtocol.shortcutindex);
            }
            connect(protocol, &ProtocolWidget::nameChanged, this, &ProtocolList::onProtocolNameChanged);
        }

    } else {
        ErrorManager e("Failed to load protocol " + name, "Protocol format corrupted");
        delete protocol;
    }
}

YAML::VoltageProtocol_t ProtocolList::copyVoltageProtocol(ProtocolWidget * protocol) {
    return protocol->getYamlVoltageProtocol();
}

YAML::CurrentProtocol_t ProtocolList::copyCurrentProtocol(ProtocolWidget * protocol) {
    return protocol->getYamlCurrentProtocol();
}

void ProtocolList::pasteVoltageProtocol(const YAML::VoltageProtocol_t &yamlProtocol) {
    this->importProtocol(yamlProtocol);
}

void ProtocolList::pasteCurrentProtocol(const YAML::CurrentProtocol_t &yamlProtocol) {
    this->importProtocol(yamlProtocol);
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
    for (auto protocol : protocols[0]) {
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

int ProtocolList::getProtocolIndexFromAnalysis(YAML::AnalysisType_t type) {
    for (int idx = 0; idx < protocols->size(); idx++) {
        auto protocol = protocols->at(idx);
        YAML::AnalysisType_t protType;
        if (protocol->getAnalysisType(protType)) {
            if (type == protType) {
                return idx;
            }
        }
    }
    return -1;
}

YAML::Protocols_t ProtocolList::getYamlProtocols() {
    YAML::Protocols_t yamlProtocols;

    int protocolsNum = protocolsNames->size();
    for (int protIdx = 0; protIdx < protocolsNum; protIdx++) {
        if (clampingModality == ClampingModality_t::VOLTAGE_CLAMP) {
            yamlProtocols.addProtocol(protocols->at(protIdx)->getYamlVoltageProtocol());

        } else {
            yamlProtocols.addProtocol(protocols->at(protIdx)->getYamlCurrentProtocol());
        }
    }

    return yamlProtocols;
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

VoltageProtocolList::VoltageProtocolList(MessageDispatcher * msgDisp, ProtocolPropertyDialog * protocolPropertyDialog, QWidget * parent) :
    ProtocolList(msgDisp, protocolPropertyDialog, parent) {

    clampingModality = ClampingModality_t::VOLTAGE_CLAMP;
    protocolsGroupName = "voltageprotocols";

    std::vector <ClampingModality_t> clampingModalities;
    msgDisp->getClampingModalitiesFeatures(clampingModalities);
    if (std::find(clampingModalities.begin(), clampingModalities.end(), clampingModality) != clampingModalities.end()) {
        this->importVhold0Protocol();
        this->importLastProtocols();
        this->onStopProtocol();
        QThread::msleep(100);
    }
}

VoltageProtocolList::~VoltageProtocolList() {
    this->exportLastProtocols();
}

ProtocolWidget * VoltageProtocolList::newGapfreeProtocol(QString name) {
    ProtocolWidget * protocol = new GapfreeVoltageProtocolWidget(msgDisp, name, protocolPropertyDialog);
    return protocol;
}

ProtocolWidget * VoltageProtocolList::newEpisodicProtocol(QString name) {
    ProtocolWidget * protocol = new EpisodicVoltageProtocolWidget(msgDisp, name, protocolPropertyDialog);
    return protocol;
}

AnalysisVoltageProtocolList::AnalysisVoltageProtocolList(MessageDispatcher * msgDisp, ProtocolPropertyDialog * protocolPropertyDialog, QWidget * parent) :
    ProtocolList(msgDisp, protocolPropertyDialog, parent) {

    clampingModality = ClampingModality_t::VOLTAGE_CLAMP;
    protocolsGroupName = "analysisvoltageprotocols";

    std::vector <ClampingModality_t> clampingModalities;
    msgDisp->getClampingModalitiesFeatures(clampingModalities);
    if (std::find(clampingModalities.begin(), clampingModalities.end(), clampingModality) != clampingModalities.end()) {
        this->importAnalysisProtocols();
        this->onStopProtocol();
        QThread::msleep(100);
    }
}

AnalysisVoltageProtocolList::~AnalysisVoltageProtocolList() {
    this->exportAnalysisProtocols();
}

ProtocolWidget * AnalysisVoltageProtocolList::newGapfreeProtocol(QString name) {
    ProtocolWidget * protocol = new GapfreeVoltageProtocolWidget(msgDisp, name, protocolPropertyDialog);
    return protocol;
}

ProtocolWidget * AnalysisVoltageProtocolList::newEpisodicProtocol(QString name) {
    ProtocolWidget * protocol = new EpisodicVoltageProtocolWidget(msgDisp, name, protocolPropertyDialog);
    return protocol;
}

void AnalysisVoltageProtocolList::contextMenuEvent(QContextMenuEvent * event) {
    QMenu menu(this);
    menu.addAction(startProtocolAct);
    menu.addSeparator();
    menu.addAction(openProtocolPropertiesAct);
    menu.exec(event->globalPos());
}

CurrentProtocolList::CurrentProtocolList(MessageDispatcher * msgDisp, ProtocolPropertyDialog * protocolPropertyDialog, QWidget * parent) :
    ProtocolList(msgDisp, protocolPropertyDialog, parent) {

    clampingModality = ClampingModality_t::CURRENT_CLAMP;
    protocolsGroupName = "currentprotocols";

    std::vector <ClampingModality_t> clampingModalities;
    msgDisp->getClampingModalitiesFeatures(clampingModalities);
    if (std::find(clampingModalities.begin(), clampingModalities.end(), clampingModality) != clampingModalities.end()) {
        this->importIhold0Protocol();
        this->importLastProtocols();
        this->onStopProtocol();
        QThread::msleep(100);
    }
}

CurrentProtocolList::~CurrentProtocolList() {
    this->exportLastProtocols();
}

ProtocolWidget * CurrentProtocolList::newGapfreeProtocol(QString name) {
    ProtocolWidget * protocol = new GapfreeCurrentProtocolWidget(msgDisp, name, protocolPropertyDialog);
    return protocol;
}

ProtocolWidget * CurrentProtocolList::newEpisodicProtocol(QString name) {
    ProtocolWidget * protocol = new EpisodicCurrentProtocolWidget(msgDisp, name, protocolPropertyDialog);
    return protocol;
}

AnalysisCurrentProtocolList::AnalysisCurrentProtocolList(MessageDispatcher * msgDisp, ProtocolPropertyDialog * protocolPropertyDialog, QWidget * parent) :
    ProtocolList(msgDisp, protocolPropertyDialog, parent) {

    clampingModality = ClampingModality_t::CURRENT_CLAMP;
    protocolsGroupName = "analysiscurrentprotocols";

    std::vector <ClampingModality_t> clampingModalities;
    msgDisp->getClampingModalitiesFeatures(clampingModalities);
    if (std::find(clampingModalities.begin(), clampingModalities.end(), clampingModality) != clampingModalities.end()) {
        this->importAnalysisProtocols();
        this->onStopProtocol();
        QThread::msleep(100);
    }
}

AnalysisCurrentProtocolList::~AnalysisCurrentProtocolList() {
    this->exportAnalysisProtocols();
}

ProtocolWidget * AnalysisCurrentProtocolList::newGapfreeProtocol(QString name) {
    ProtocolWidget * protocol = new GapfreeCurrentProtocolWidget(msgDisp, name, protocolPropertyDialog);
    return protocol;
}

ProtocolWidget * AnalysisCurrentProtocolList::newEpisodicProtocol(QString name) {
    ProtocolWidget * protocol = new EpisodicCurrentProtocolWidget(msgDisp, name, protocolPropertyDialog);
    return protocol;
}

void AnalysisCurrentProtocolList::contextMenuEvent(QContextMenuEvent * event) {
    QMenu menu(this);
    menu.addAction(startProtocolAct);
    menu.addSeparator();
    menu.addAction(openProtocolPropertiesAct);
    menu.exec(event->globalPos());
}
