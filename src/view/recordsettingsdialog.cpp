#include "recordsettingsdialog.h"

#include <QFileDialog>
#include <QSettings>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QDateTime>

#include "globaldefines.h"

RecordSettingsDialog::RecordSettingsDialog() :
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
    connect(recordPathBrowseBtn, &QPushButton::clicked, this, &RecordSettingsDialog::onRecordPathBrowseBtnClicked);
    pathGl->addWidget(recordPathBrowseBtn, 0, 2);

    pathGl->addWidget(new QLabel("File Name:"), 1, 0);
    recordNameEdit = new QLineEdit();
    pathGl->addWidget(recordNameEdit, 1, 1);

    addDateChx = new QCheckBox("Append date to file name");
    pathGl->addWidget(addDateChx, 3, 0, 1, -1);

    QRegularExpression re("^[a-zA-Z0-9_ ]*$");
    QRegularExpressionValidator * validator = new QRegularExpressionValidator(re, this);
    recordNameEdit->setValidator(validator);

    /*! Record format settings */
    QGroupBox * recordFormatGb = new QGroupBox();
    recordFormatGb->setTitle("Data format");
    QVBoxLayout * recordFormatVl = new QVBoxLayout();
    recordFormatGb->setLayout(recordFormatVl);
    mainVl->addWidget(recordFormatGb);

    recordFormatBg = new QButtonGroup();
    recordFormatBg->setExclusive(true);

//    recordFormatDatRb = new QRadioButton(".dat");
//    recordFormatVl->addWidget(recordFormatDatRb);
//    recordFormatBg->addButton(recordFormatDatRb);

//    recordFormatEdrfRb = new QRadioButton(".edrf");
//    recordFormatVl->addWidget(recordFormatEdrfRb);
//    recordFormatBg->addButton(recordFormatEdrfRb);

    recordFormatAbfRb = new QRadioButton(".abf");
    recordFormatVl->addWidget(recordFormatAbfRb);
    recordFormatBg->addButton(recordFormatAbfRb);

    /*! Record size settings */
    QGroupBox * recordSizeGb = new QGroupBox();
    recordSizeGb->setTitle("Data size");
    QVBoxLayout * recordSizeVl = new QVBoxLayout();
    recordSizeGb->setLayout(recordSizeVl);
    mainVl->addWidget(recordSizeGb);

    QHBoxLayout * recordSizeHl = new QHBoxLayout();
    recordSizeVl->addLayout(recordSizeHl);

    QLabel * recordDurationLbl = new QLabel("Record for");
    recordSizeHl->addWidget(recordDurationLbl);
    recordDurationEdit = new QDoubleSpinBox;
    recordDurationEdit->setRange(0.0, 1.0e6);
    recordDurationEdit->setSingleStep(0.1);
    recordDurationEdit->setValue(PSD_DEFAULT_RECORD_DURATION);
    recordDurationEdit->setDecimals(1);
    recordDurationEdit->setSpecialValueText("Unlimited");
    recordSizeHl->addWidget(recordDurationEdit);
    QLabel * recordDurationUnitLbl = new QLabel("s");
    recordSizeHl->addWidget(recordDurationUnitLbl);

    QHBoxLayout * chunkSelectionHl = new QHBoxLayout();
    recordSizeVl->addLayout(chunkSelectionHl);

    QLabel * chunkDurationLbl = new QLabel("Split file in chunks");
    chunkSelectionHl->addWidget(chunkDurationLbl);
    chunkDurationEdit = new QSpinBox;
    chunkDurationEdit->setRange(0, 1000000);
    chunkDurationEdit->setValue(PSD_DEFAULT_CHUNK_DURATION);
    chunkDurationEdit->setSpecialValueText("Disabled");
    chunkSelectionHl->addWidget(chunkDurationEdit);
    QLabel * chunkDurationUnitLbl = new QLabel("s");
    chunkSelectionHl->addWidget(chunkDurationUnitLbl);

    recordSizeLbl = new QLabel("Recording size on disk: 0 MB (0 MB per chunk)");
    recordSizeVl->addWidget(recordSizeLbl);

//    connect(recordFormatDatRb, &QRadioButton::clicked, this, &RecordSettingsDialog::onDataFormatSelected);

    /*! Accept and reject buttons */
    QHBoxLayout * acceptRejectHl = new QHBoxLayout();
    mainVl->addLayout(acceptRejectHl);

    QWidget * spacer = new QWidget();
    spacer->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Maximum);

    acceptRejectHl->addWidget(spacer);

    QPushButton * okButton = new QPushButton("OK");
    okButton->setCheckable(false);
    acceptRejectHl->addWidget(okButton);
    connect(okButton, &QPushButton::clicked, this, &RecordSettingsDialog::onAccept);
    okButton->setDefault(true);

    QPushButton * cancButton = new QPushButton("Cancel");
    cancButton->setCheckable(false);
    acceptRejectHl->addWidget(cancButton);
    connect(cancButton, &QPushButton::clicked, this, &RecordSettingsDialog::onReject);

    this->onLoadSettings();
}

RecordSettingsDialog::RecordFileFormat_t RecordSettingsDialog::getRecordFileFormat() {
    RecordFileFormat_t format = (RecordFileFormat_t)PSD_DEFAULT_RECORD_FORMAT;

    if (recordFormatAbfRb->isChecked()) {
        format = RecordFileAbf;
    }

    return format;
}

void RecordSettingsDialog::onRecordPathBrowseBtnClicked() {
    QString s = QFileDialog::getExistingDirectory(this, "Select directory", recordPathEdit->text());
    if (s != "") {
        recordPathEdit->setText(s + "/");
    }
}

void RecordSettingsDialog::onLoadSettings() {
    QSettings settings;

    recordPathEdit->setText(settings.value(GLB_PROTOCOL_RECORD_PATH_TAG, PSD_DEFAULT_RECORD_PATH).toString());
    recordNameEdit->setText(settings.value(GLB_PROTOCOL_RECORD_NAME_TAG, PSD_DEFAULT_RECORD_NAME).toString());
    addDateChx->setChecked(settings.value(GLB_PROTOCOL_ADD_DATE_TAG, PSD_DEFAULT_ADD_DATE).toBool());

    format = (RecordFileFormat_t)(settings.value(GLB_PROTOCOL_RECORD_FORMAT_TAG, PSD_DEFAULT_RECORD_FORMAT).toInt());
    switch (format) {
//    case RecordFileDat:
//        recordFormatDatRb->setChecked(true);
//        recordFormatEdrfRb->setChecked(false);
//        recordFormatAbfRb->setChecked(false);
//        break;

//    case RecordFileEdrf:
//        recordFormatDatRb->setChecked(false);
//        recordFormatEdrfRb->setChecked(true);
//        recordFormatAbfRb->setChecked(false);
//        break;

    case RecordFileAbf:
//        recordFormatDatRb->setChecked(false);
//        recordFormatEdrfRb->setChecked(false);
        recordFormatAbfRb->setChecked(true);
        break;

    default:
//        recordFormatDatRb->setChecked(false);
//        recordFormatEdrfRb->setChecked(false);
        recordFormatAbfRb->setChecked(true);
        break;
    }
    recordDurationEdit->setValue(settings.value(GLB_PROTOCOL_RECORD_DURATION_TAG, PSD_DEFAULT_RECORD_DURATION).toDouble());
    chunkDurationEdit->setValue(settings.value(GLB_PROTOCOL_CHUNK_DURATION_TAG, PSD_DEFAULT_CHUNK_DURATION).toDouble());
}

void RecordSettingsDialog::onSaveSettings() {
    QSettings settings;

    settings.setValue(GLB_PROTOCOL_RECORD_PATH_TAG, recordPathEdit->text());
    settings.setValue(GLB_PROTOCOL_RECORD_NAME_TAG, recordNameEdit->text());
    settings.setValue(GLB_PROTOCOL_ADD_DATE_TAG, addDateChx->isChecked());
    settings.setValue(GLB_PROTOCOL_RECORD_FORMAT_TAG, (int)(this->getRecordFileFormat()));
    settings.setValue(GLB_PROTOCOL_RECORD_DURATION_TAG, recordDurationEdit->value());
    settings.setValue(GLB_PROTOCOL_CHUNK_DURATION_TAG, chunkDurationEdit->value());
}

void RecordSettingsDialog::onAccept() {
    this->onSaveSettings();

    RecordSettings_t settings;
    settings.recordPath = recordPathEdit->text();
    settings.filename = recordNameEdit->text();
    settings.appendDate = addDateChx->isChecked();
    settings.fileFormat = this->getRecordFileFormat();
    settings.recordDurationS = recordDurationEdit->value();
    settings.chunkDurationS = chunkDurationEdit->value();
    sigSettingsSet(settings);

    this->accept();
}

void RecordSettingsDialog::onReject() {
    this->onLoadSettings();
    this->reject();
}
