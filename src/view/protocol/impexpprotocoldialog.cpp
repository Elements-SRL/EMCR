#include "impexpprotocoldialog.h"

#include <QSettings>

#include "protocols.h"

ImpExpProtocolDialog::ImpExpProtocolDialog(ClampingModality_t clampingModality, QWidget * parent) :
    QDialog(parent),
    clampingModality(clampingModality) {

    if (clampingModality == E4GCL_VOLTAGE_CLAMP_MODE) {
        stimulusName = "Voltage";

    } else {
        stimulusName = "Current";
    }

    /*! Hide the help button on the window bar */
    Qt::WindowFlags flags = windowFlags();
    Qt::WindowFlags helpFlag = Qt::WindowContextHelpButtonHint;
    flags = flags & (~helpFlag);
    this->setWindowFlags(flags);

    this->setGeometry(0, 0, 450, 300);

    QVBoxLayout * mainVl = new QVBoxLayout;
    mainVl->setContentsMargins(6, 6, 6, 6);
    mainVl->setSpacing(6);

    this->setLayout(mainVl);

    titleLbl = new QLabel("");
    mainVl->addWidget(titleLbl);

    protocolManagementLo = new QGridLayout;
    protocolManagementLo->setContentsMargins(0, 0, 0, 0);
    protocolManagementLo->setSpacing(6);
    protocolManagementLo->setColumnStretch(0, 2);
    protocolManagementLo->setColumnStretch(1, 3);
    protocolManagementLo->setColumnStretch(2, 1);
    protocolManagementLo->setColumnStretch(3, 3);

    mainVl->addLayout(protocolManagementLo);

    selectAllCbox = new QCheckBox("Select all");
    protocolManagementLo->addWidget(selectAllCbox, 0, 0);
    connect(selectAllCbox, &QCheckBox::clicked, this, &ImpExpProtocolDialog::onSelectAllCbox);

    mainVl->addItem(new QSpacerItem(0, 0, QSizePolicy::Fixed, QSizePolicy::MinimumExpanding));

    QHBoxLayout * pathHl = new QHBoxLayout;
    pathHl->setContentsMargins(0, 0, 0, 0);
    pathHl->setSpacing(6);
    mainVl->addLayout(pathHl);

    fileNameLbl = new QLabel("");
    pathHl->addWidget(fileNameLbl);

    fileNameEdit = new QLineEdit;
    pathHl->addWidget(fileNameEdit);

    browseBtn = new QPushButton("Browse ...");
    connect(browseBtn, &QPushButton::clicked, this, &ImpExpProtocolDialog::onBrowse);
    pathHl->addWidget(browseBtn);

    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &ImpExpProtocolDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &ImpExpProtocolDialog::reject);
    mainVl->addWidget(buttonBox);

    if (parent != nullptr) {
        QRect pRect = parent->geometry();
        QSize wSize = this->size();
        QRect wRect;
        wRect.setX(pRect.x()+(pRect.width()-wSize.width())/2);
        wRect.setY(pRect.y()+(pRect.height()-wSize.height())/2);
        wRect.setWidth(wSize.width());
        wRect.setHeight(wSize.height());
        this->setGeometry(wRect);
    }

    actionSelectors = new QVector <ImpExpProtocolsActionSelector *>;

//    QRegExp re("^[a-zA-Z0-9_ ]*$");
//    QRegExpValidator * validator = new QRegExpValidator(re, this);
//    ui->fileEdit->setValidator(validator);

    buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
}

ImpExpProtocolDialog::~ImpExpProtocolDialog() {
    if (actionSelectors != nullptr) {
        for (int idx = 0; idx < actionSelectors->size(); idx++) {
            delete actionSelectors->at(idx);
        }
        delete actionSelectors;
        actionSelectors = nullptr;
    }
}

void ImpExpProtocolDialog::show() {
    this->exec();
}

QString ImpExpProtocolDialog::getWriteInfo(QVector <bool> &saveFlag, QVector <bool> &overwriteFlag,
                                           QStringList &names, QStringList &namesSet) {
    saveFlag.clear();
    overwriteFlag.clear();
    names.clear();
    namesSet.clear();

    for (int protIdx = 0; protIdx < protocolsNum; protIdx++) {
        ImpExpProtocolsActionSelector * sel = actionSelectors->at(protIdx);

        saveFlag.push_back(sel->toBeSaved());
        overwriteFlag.push_back(sel->overwrite());
        names.push_back(QString::fromStdString(sourceProtocolsNames[protIdx]));
        namesSet.push_back(sel->getNameSet());
    }
    return fullFileName;
}

void ImpExpProtocolDialog::onCheckAcceptability() {
    bool accept = true;
    if (!(fileNameEdit->text().isEmpty())) {
        /*! Check that path and file are set */
        for (int protIdx = 0; protIdx < protocolsNum; protIdx++) {
            if (!(actionSelectors->at(protIdx)->acceptable())) {
                accept = false;
                break;
            }
        }

    } else {
        accept = false;
    }

    buttonBox->button(QDialogButtonBox::Ok)->setEnabled(accept);
}

void ImpExpProtocolDialog::onSelectAllCbox(bool checked) {
    for (int protIdx = 0; protIdx < protocolsNum; protIdx++) {
        actionSelectors->at(protIdx)->setChecked(checked);
    }
    this->onCheckAcceptability();
}

void ImpExpProtocolDialog::onCheckSelectAllCbox() {
    bool all = true;
    for (int protIdx = 0; protIdx < protocolsNum; protIdx++) {
        all &= actionSelectors->at(protIdx)->isChecked();
    }

    selectAllCbox->setChecked(all);
}

ExportProtocolDialog::ExportProtocolDialog(QStringList pn, int clampingModality, QWidget * parent) :
    ImpExpProtocolDialog(clampingModality, parent) {

    this->setWindowTitle("Export " + stimulusName.toLower() + " protocols");

    titleLbl->setText("Select protocols to be exported");
    fileNameLbl->setText("Save to:");

    sourceProtocolsNames.clear();
    for (auto p : pn) {
        sourceProtocolsNames.push_back(p.toStdString());
    }
    protocolsNum = sourceProtocolsNames.size();

    for (int protIdx = 0; protIdx < protocolsNum; protIdx++) {
        ExportProtocolsActionSelector * sel = new ExportProtocolsActionSelector();

        sel->setText(pn[protIdx]);
        connect(sel, &ExportProtocolsActionSelector::checkAcceptability, this, &ExportProtocolDialog::onCheckAcceptability);
        connect(sel, &ExportProtocolsActionSelector::checkSelectAllCbox, this, &ExportProtocolDialog::onCheckSelectAllCbox);

        protocolManagementLo->addWidget(sel->getChBox(), protIdx+1, 0);
        protocolManagementLo->addWidget(sel->getErrorLbl(), protIdx+1, 1);
        protocolManagementLo->addWidget(sel->getCbBox(), protIdx+1, 2);
        protocolManagementLo->addWidget(sel->getLnEdit(), protIdx+1, 3);

        actionSelectors->push_back(sel);
    }
}

ExportProtocolDialog::~ExportProtocolDialog() {

}

void ExportProtocolDialog::onBrowse() {
    QSettings settings;
    fullFileName = QFileDialog::getSaveFileName(
                this, "Select export file",
                settings.value(GLB_PROTOCOL_FOLDER_TAG, EPML_DEFAULT_FOLDER).toString(),
                "*" + YAML_FILE_EXTENSION, nullptr, QFileDialog::DontConfirmOverwrite);

    if (fullFileName == "") {
        return;
    }

    QFileInfo fileInfo(fullFileName);
    folderName = fileInfo.dir().absolutePath();
    settings.setValue(GLB_PROTOCOL_FOLDER_TAG, folderName);

    this->checkFileContent();
}

void ExportProtocolDialog::checkFileContent() {
    if (QFile::exists(fullFileName)) {
        YAML::Node node = YAML::LoadFile(fullFileName.toStdString());
        YAML::Protocols_t yamlProtocols = node.as <YAML::Protocols_t> ();
        if (clampingModality == E4GCL_VOLTAGE_CLAMP_MODE) {
            destinationProtocolsNames = yamlProtocols.getVoltageProtocolsNames();

        } else {
            destinationProtocolsNames = yamlProtocols.getCurrentProtocolsNames();
        }
        for (int protIdx = 0; protIdx < protocolsNum; protIdx++) {
            if (std::find(destinationProtocolsNames.begin(), destinationProtocolsNames.end(), sourceProtocolsNames[protIdx]) < destinationProtocolsNames.end()) {
                actionSelectors->at(protIdx)->setNameConflict(true);

            } else {
                actionSelectors->at(protIdx)->setNameConflict(false);
            }
        }
    }
    fileNameEdit->setText(fullFileName);

    this->onCheckAcceptability();
}

ImportProtocolDialog::ImportProtocolDialog(QStringList pn, int clampingModality, QWidget * parent) :
    ImpExpProtocolDialog(clampingModality, parent) {

    this->setWindowTitle("Import " + stimulusName.toLower() + " protocols");

    titleLbl->setText("Select protocols to be imported");
    fileNameLbl->setText("Import from:");

    destinationProtocolsNames.clear();
    for (auto p : pn) {
        destinationProtocolsNames.push_back(p.toStdString());
    }
}

ImportProtocolDialog::~ImportProtocolDialog() {

}

void ImportProtocolDialog::onBrowse() {
    QSettings settings;
    fullFileName = QFileDialog::getOpenFileName(
                this, "Select import file",
                settings.value(GLB_PROTOCOL_FOLDER_TAG, EPML_DEFAULT_FOLDER).toString(),
                "*" + YAML_FILE_EXTENSION + ";;*" + EPML_FILE_EXTENSION);

    if (fullFileName == "") {
        return;
    }

    QFileInfo fileInfo(fullFileName);
    folderName = fileInfo.dir().absolutePath();
    settings.setValue(GLB_PROTOCOL_FOLDER_TAG, folderName);

    this->checkFileContent();
}

void ImportProtocolDialog::checkFileContent() {
    if (fullFileName.endsWith(YAML_FILE_EXTENSION)) {
        if (QFile::exists(fullFileName)) {
            YAML::Node node = YAML::LoadFile(fullFileName.toStdString());
            YAML::Protocols_t yamlProtocols = node.as <YAML::Protocols_t> ();

            fileNameEdit->setText(fullFileName);

            QLayoutItem * item;
            for (int protIdx = 0; protIdx < protocolsNum; protIdx++) {
                for (int colIdx = 0; colIdx < 4; colIdx++) { /*! \todo FCON mettere una macro al posto di questo 4 */
                    item = protocolManagementLo->itemAtPosition(protIdx+1, colIdx);
                    protocolManagementLo->removeItem(item);
                    delete item->widget();
                }
                delete actionSelectors->at(protIdx);
            }
            actionSelectors->clear();

            if (clampingModality == ClampingModality_t::VOLTAGE_CLAMP) {
                sourceProtocolsNames = yamlProtocols.getVoltageProtocolsNames();

            } else {
                sourceProtocolsNames = yamlProtocols.getCurrentProtocolsNames();
            }

            protocolsNum = sourceProtocolsNames.size();

            for (int protIdx = 0; protIdx < protocolsNum; protIdx++) {
                ImportProtocolsActionSelector * sel = new ImportProtocolsActionSelector();

                sel->setText(QString::fromStdString(sourceProtocolsNames[protIdx]));
                connect(sel, &ImportProtocolsActionSelector::checkAcceptability, this, &ImportProtocolDialog::onCheckAcceptability);
                connect(sel, &ImportProtocolsActionSelector::checkSelectAllCbox, this, &ImportProtocolDialog::onCheckSelectAllCbox);

                protocolManagementLo->addWidget(sel->getChBox(), protIdx+1, 0);
                protocolManagementLo->addWidget(sel->getErrorLbl(), protIdx+1, 1);
                protocolManagementLo->addWidget(sel->getCbBox(), protIdx+1, 2);
                protocolManagementLo->addWidget(sel->getLnEdit(), protIdx+1, 3);

                if (std::find(destinationProtocolsNames.begin(), destinationProtocolsNames.end(), sourceProtocolsNames[protIdx]) < destinationProtocolsNames.end()) {
                    sel->setNameConflict(true);
                }
                actionSelectors->push_back(sel);
            }

            this->onCheckAcceptability();
        }

        if (epmlManager != nullptr) {
            delete epmlManager;
            epmlManager = nullptr;
        }

    } else {
        epmlManager = new EpmlManager(fullFileName, QIODevice::ReadWrite);

        if (epmlManager->fileExists()) {
            fileNameEdit->setText(fullFileName);

            QLayoutItem * item;
            for (int protIdx = 0; protIdx < protocolsNum; protIdx++) {
                for (int colIdx = 0; colIdx < 4; colIdx++) { /*! \todo FCON mettere una macro al posto di questo 4 */
                    item = protocolManagementLo->itemAtPosition(protIdx+1, colIdx);
                    protocolManagementLo->removeItem(item);
                    delete item->widget();
                }
                delete actionSelectors->at(protIdx);
            }
            actionSelectors->clear();

            QStringList namesList = epmlManager->getProtocolsList(stimulusName.toLower() + "protocols");
            sourceProtocolsNames.clear();
            for (auto name : namesList) {
                sourceProtocolsNames.push_back(name.toStdString());
            }
            protocolsNum = sourceProtocolsNames.size();

            for (int protIdx = 0; protIdx < protocolsNum; protIdx++) {
                ImportProtocolsActionSelector * sel = new ImportProtocolsActionSelector();

                sel->setText(QString::fromStdString(sourceProtocolsNames[protIdx]));
                connect(sel, &ImportProtocolsActionSelector::checkAcceptability, this, &ImportProtocolDialog::onCheckAcceptability);
                connect(sel, &ImportProtocolsActionSelector::checkSelectAllCbox, this, &ImportProtocolDialog::onCheckSelectAllCbox);

                protocolManagementLo->addWidget(sel->getChBox(), protIdx+1, 0);
                protocolManagementLo->addWidget(sel->getErrorLbl(), protIdx+1, 1);
                protocolManagementLo->addWidget(sel->getCbBox(), protIdx+1, 2);
                protocolManagementLo->addWidget(sel->getLnEdit(), protIdx+1, 3);

                if (std::find(destinationProtocolsNames.begin(), destinationProtocolsNames.end(), sourceProtocolsNames[protIdx]) < destinationProtocolsNames.end()) {
                    sel->setNameConflict(true);
                }
                actionSelectors->push_back(sel);
            }

            this->onCheckAcceptability();
        }

        if (epmlManager != nullptr) {
            delete epmlManager;
            epmlManager = nullptr;
        }
    }
}

ImpExpProtocolsActionSelector::ImpExpProtocolsActionSelector() : QObject() {
    chBox = new QCheckBox;
    connect(chBox, &QCheckBox::clicked, this, &ImpExpProtocolsActionSelector::checkAcceptability);
    connect(chBox, &QCheckBox::clicked, this, &ImpExpProtocolsActionSelector::checkSelectAllCbox);

    errorLbl = new QLabel;
    errorLbl->setVisible(false);

    cbBox = new QComboBox;
    cbBox->insertItem(ComboBoxNoSelection, "<Select action>");
    cbBox->insertItem(ComboBoxDontSave, "Don't save/Don't import");
    cbBox->insertItem(ComboBoxOverwrite, "Overwrite");
    cbBox->insertItem(ComboBoxRename, "Rename to");
    cbBox->setVisible(false);
    connect(cbBox, QOverload <int> ::of(&QComboBox::activated), this, &ImpExpProtocolsActionSelector::onComboBoxChanged);
    connect(cbBox, QOverload <int> ::of(&QComboBox::activated), this, &ImpExpProtocolsActionSelector::checkAcceptability);

    lnEdit = new QLineEdit;
    lnEdit->setPlaceholderText("type new protocol name");
    lnEdit->setVisible(false);
    lnEdit->setEnabled(false);
    connect(lnEdit, &QLineEdit::textEdited, this, &ImpExpProtocolsActionSelector::checkAcceptability);
}

void ImpExpProtocolsActionSelector::setText(QString text) {
    chBox->setText(text);
}

void ImpExpProtocolsActionSelector::setNameConflict(bool b) {
    nameConflict = b;
    if (nameConflict) {
        errorLbl->setVisible(true);
        cbBox->setVisible(true);
        lnEdit->setVisible(true);
        if (cbBox->currentIndex() == ComboBoxRename) {
            lnEdit->setEnabled(true);
        }

    } else {
        errorLbl->setVisible(false);
        cbBox->setVisible(false);
        lnEdit->setVisible(false);
        lnEdit->setEnabled(false);
    }
}

QCheckBox * ImpExpProtocolsActionSelector::getChBox() {
    return chBox;
}

QLabel * ImpExpProtocolsActionSelector::getErrorLbl() {
    return errorLbl;
}

QComboBox * ImpExpProtocolsActionSelector::getCbBox() {
    return cbBox;
}

QLineEdit * ImpExpProtocolsActionSelector::getLnEdit() {
    return lnEdit;
}

QString ImpExpProtocolsActionSelector::getNameSet() {
    QString name;
    if (cbBox->currentIndex() != ComboBoxRename) {
        name = chBox->text();

    } else {
        name = lnEdit->text();
    }
    return name;
}

bool ImpExpProtocolsActionSelector::acceptable() {
    bool ret;
    if (nameConflict && chBox->isChecked()) {
        /*! If name does not already exist or it must no be processed it's ok */
        if ((cbBox->currentIndex() == ComboBoxNoSelection)
                || ((cbBox->currentIndex() == ComboBoxRename) && lnEdit->text().isEmpty())) {
            /*! Else, if combobox isn't set or it is set to rename and name is empty it's not ok */
            ret = false;

        } else {
            ret = true;
        }

    } else {
        ret = true;
    }
    return ret;
}

void ImpExpProtocolsActionSelector::setChecked(bool checked) {
    chBox->setChecked(checked);
}

bool ImpExpProtocolsActionSelector::isChecked() {
    return chBox->isChecked();
}

bool ImpExpProtocolsActionSelector::toBeSaved() {
    bool flag;
    if (chBox->isChecked() && (cbBox->currentIndex() != ComboBoxDontSave)) {
        flag = true;

    } else {
        flag = false;
    }
    return flag;
}

bool ImpExpProtocolsActionSelector::overwrite() {
    bool flag;
    if (cbBox->currentIndex() == ComboBoxOverwrite) {
        flag = true;

    } else {
        flag = false;
    }
    return flag;
}

void ImpExpProtocolsActionSelector::onComboBoxChanged(int idx) {
    if (idx == 3) {
        lnEdit->setEnabled(true);

    } else {
        lnEdit->setEnabled(false);
    }
}

ExportProtocolsActionSelector::ExportProtocolsActionSelector() : ImpExpProtocolsActionSelector() {
    errorLbl->setText("Already in destination file");

    cbBox->setItemText(ComboBoxDontSave, "Don't save");
}

ImportProtocolsActionSelector::ImportProtocolsActionSelector() : ImpExpProtocolsActionSelector() {
    errorLbl->setText("Already in GUI");

    cbBox->setItemText(ComboBoxDontSave, "Don't import");
}
