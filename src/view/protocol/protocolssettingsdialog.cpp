#include "protocolssettingsdialog.h"

#include <QLabel>
#include <QFileDialog>
#include <QSettings>
#include <QGroupBox>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include "globaldefines.h"

ProtocolsSettingsDialog::ProtocolsSettingsDialog() :
    QDialog() {

    this->setWindowFlags(Qt::WindowTitleHint);
    this->setWindowTitle("Recording properties");

    QVBoxLayout * mainVl = new QVBoxLayout();
    this->setLayout(mainVl);

    QGridLayout * pathGl = new QGridLayout();
    mainVl->addLayout(pathGl);

    /*! Record path settings */
    pathGl->addWidget(new QLabel("Recording path:"), 0, 0);
    recordPathEdit = new QLineEdit();
    recordPathEdit->setReadOnly(true);
    pathGl->addWidget(recordPathEdit, 0, 1);

    QPushButton * recordPathBrowseBtn = new QPushButton("Browse...");
    recordPathBrowseBtn->setCheckable(false);
    connect(recordPathBrowseBtn, &QPushButton::clicked, this, &ProtocolsSettingsDialog::onRecordPathBrowseBtnClicked);
    pathGl->addWidget(recordPathBrowseBtn, 0, 2);

    pathGl->addWidget(new QLabel("File Name:"), 1, 0);
    recordNameEdit = new QLineEdit();
    pathGl->addWidget(recordNameEdit, 1, 1);

    addDateChx = new QCheckBox("Append date to file name");
    pathGl->addWidget(addDateChx, 2, 0, 1, -1);

    QRegularExpression re("^[a-zA-Z0-9_ ]*$");
    QRegularExpressionValidator * validator = new QRegularExpressionValidator(re, this);
    recordNameEdit->setValidator(validator);

    /*! Record format settings */
    QGroupBox * recordFormatGb = new QGroupBox();
    QVBoxLayout * recordFormatVl = new QVBoxLayout();
    recordFormatGb->setLayout(recordFormatVl);
    mainVl->addWidget(recordFormatGb);

    recordFormatBg = new QButtonGroup();
    recordFormatBg->setExclusive(true);

//    recordFormatEdrfRb = new QRadioButton(".edrf");
//    recordFormatVl->addWidget(recordFormatEdrfRb);
//    recordFormatBg->addButton(recordFormatEdrfRb);

    recordFormatAbfRb = new QRadioButton(".abf");
    recordFormatVl->addWidget(recordFormatAbfRb);
    recordFormatBg->addButton(recordFormatAbfRb);

    /*! Accept and reject buttons */
    QHBoxLayout * acceptRejectHl = new QHBoxLayout();
    mainVl->addLayout(acceptRejectHl);

    QWidget * spacer = new QWidget();
    spacer->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Maximum);

    acceptRejectHl->addWidget(spacer);

    QPushButton * okButton = new QPushButton("OK");
    okButton->setCheckable(false);
    acceptRejectHl->addWidget(okButton);
    connect(okButton, &QPushButton::clicked, this, &ProtocolsSettingsDialog::onAccept);
    okButton->setDefault(true);

    QPushButton * cancButton = new QPushButton("Cancel");
    cancButton->setCheckable(false);
    acceptRejectHl->addWidget(cancButton);
    connect(cancButton, &QPushButton::clicked, this, &ProtocolsSettingsDialog::onReject);

    this->onLoadSettings();
}

QString ProtocolsSettingsDialog::getRecordPath() {
    return recordPathEdit->text();
}

QString ProtocolsSettingsDialog::getRecordName() {
    return recordNameEdit->text();
}

bool ProtocolsSettingsDialog::getAddDate() {
    return addDateChx->isChecked();
}

ProtocolsSettingsDialog::RecordFileFormat_t ProtocolsSettingsDialog::getRecordFileFormat() {
    RecordFileFormat_t format = (RecordFileFormat_t)PSD_DEFAULT_RECORD_FORMAT;

/*    if (recordFormatEdrfRb->isChecked()) {
        format = RecordFileEdrf;

    } else*/ if (recordFormatAbfRb->isChecked()) {
        format = RecordFileAbf;
    }

    return format;
}

void ProtocolsSettingsDialog::synchronizeSettings() {
    this->onLoadSettings();
}

void ProtocolsSettingsDialog::onRecordPathBrowseBtnClicked() {
    QString s = QFileDialog::getExistingDirectory(this, "Select directory", recordPathEdit->text());
    if (s != "") {
        recordPathEdit->setText(s+"/");
    }
}

void ProtocolsSettingsDialog::onLoadSettings() {
    QSettings settings;

    recordPathEdit->setText(settings.value(GLB_PROTOCOL_RECORD_PATH_TAG, PSD_DEFAULT_RECORD_PATH).toString());
    recordNameEdit->setText(settings.value(GLB_PROTOCOL_RECORD_NAME_TAG, PSD_DEFAULT_RECORD_NAME).toString());
    addDateChx->setChecked(settings.value(GLB_PROTOCOL_ADD_DATE_TAG, PSD_DEFAULT_ADD_DATE).toBool());

    switch ((RecordFileFormat_t)(settings.value(GLB_PROTOCOL_RECORD_FORMAT_TAG, PSD_DEFAULT_RECORD_FORMAT).toInt())) {
//    case RecordFileEdrf:
//        recordFormatEdrfRb->setChecked(true);
//        recordFormatAbfRb->setChecked(false);
//        break;

    case RecordFileAbf:
//        recordFormatEdrfRb->setChecked(false);
        recordFormatAbfRb->setChecked(true);
        break;

    default:
//        recordFormatEdrfRb->setChecked(false);
        recordFormatAbfRb->setChecked(true);
        break;
    }
}

void ProtocolsSettingsDialog::onSaveSettings() {
    QSettings settings;

    settings.setValue(GLB_PROTOCOL_RECORD_PATH_TAG, recordPathEdit->text());
    settings.setValue(GLB_PROTOCOL_RECORD_NAME_TAG, recordNameEdit->text());
    settings.setValue(GLB_PROTOCOL_ADD_DATE_TAG, addDateChx->isChecked());
    settings.setValue(GLB_PROTOCOL_RECORD_FORMAT_TAG, (int)(this->getRecordFileFormat()));
}

void ProtocolsSettingsDialog::onAccept() {
    this->onSaveSettings();
    emit newRecordPath();
    this->accept();
}

void ProtocolsSettingsDialog::onReject() {
    this->onLoadSettings();
    this->reject();
}
