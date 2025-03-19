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
    this->setWindowTitle("Gap free recording properties");

    QVBoxLayout * mainVl = new QVBoxLayout();
    this->setLayout(mainVl);

    QGridLayout * pathGl = new QGridLayout();
    mainVl->addLayout(pathGl);

    addDateChx = new QCheckBox("Append date to file name");
    pathGl->addWidget(addDateChx, 3, 0, 1, -1);

//    QRegularExpression re("^[a-zA-Z0-9_ ]*$");
//    QRegularExpressionValidator * validator = new QRegularExpressionValidator(re, this);
//    recordNameEdit->setValidator(validator);

    /*! Record format settings */
    QGroupBox * recordFormatGb = new QGroupBox();
    recordFormatGb->setTitle("File format");
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

    /*! Voltage settings */
    QGroupBox * voltageGb = new QGroupBox();
    voltageGb->setTitle("Data options");
    QVBoxLayout * voltageVl = new QVBoxLayout();
    voltageGb->setLayout(voltageVl);
    mainVl->addWidget(voltageGb);

    voltageBg = new QButtonGroup();
    voltageBg->setExclusive(true);

    voltageSaveAllRb = new QRadioButton("Save full voltage with current");
    voltageVl->addWidget(voltageSaveAllRb);
    voltageBg->addButton(voltageSaveAllRb);

    voltageSaveSeparatelyRb = new QRadioButton("Save voltage at reduced rate in a separate file");
    voltageVl->addWidget(voltageSaveSeparatelyRb);
    voltageBg->addButton(voltageSaveSeparatelyRb);

    voltageDontSaveRb = new QRadioButton("Do not save voltage");
    voltageVl->addWidget(voltageDontSaveRb);
    voltageBg->addButton(voltageDontSaveRb);

    connect(voltageSaveAllRb, &QRadioButton::clicked, this, &RecordSettingsDialog::onVoltageFormatChanged);
    connect(voltageSaveSeparatelyRb, &QRadioButton::clicked, this, &RecordSettingsDialog::onVoltageFormatChanged);
    connect(voltageDontSaveRb, &QRadioButton::clicked, this, &RecordSettingsDialog::onVoltageFormatChanged);

    QHBoxLayout * voltageDecimatorHl = new QHBoxLayout;
    voltageVl->addLayout(voltageDecimatorHl);
    voltageDecimatorHl->addWidget(new QLabel("Voltage decimation factor"));
    voltageDecimatorSbx = new QSpinBox;
    voltageDecimatorSbx->setRange(2, 10000);
    voltageDecimatorSbx->setValue(1000);
    voltageDecimatorHl->addWidget(voltageDecimatorSbx);

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
    /*! \todo FCON al momento non la facciamo vedere */
    recordSizeLbl ->setVisible(false);
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

void RecordSettingsDialog::forceSettingsEmit() {
    this->onAccept();
}

void RecordSettingsDialog::onFileSizeComputed(QString message) {
    recordSizeLbl->setText(message);
}

void RecordSettingsDialog::onVoltageFormatChanged() {
    voltageDecimatorSbx->setEnabled(voltageSaveSeparatelyRb->isChecked());
}

RecordSettingsDialog::RecordFileFormat_t RecordSettingsDialog::getRecordFileFormat() {
    RecordFileFormat_t format = (RecordFileFormat_t)PSD_DEFAULT_RECORD_FORMAT;

    if (recordFormatAbfRb->isChecked()) {
        format = RecordFileAbf;
    }

    return format;
}

RecordSettingsDialog::VoltageFormat_t RecordSettingsDialog::getVoltageRecordFormat() {
    VoltageFormat_t format = (VoltageFormat_t)PSD_DEFAULT_VOLTAGE_FORMAT;

    if (voltageSaveAllRb->isChecked()) {
        format = VoltageAll;

    } else if (voltageSaveSeparatelyRb->isChecked()) {
        format = VoltageSeparate;

    } else if (voltageDontSaveRb->isChecked()) {
        format = VoltageNone;
    }

    return format;
}

void RecordSettingsDialog::onLoadSettings() {
    QSettings settings;

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

    switch ((VoltageFormat_t)(settings.value(GLB_PROTOCOL_VOLTAGE_FORMAT_TAG, PSD_DEFAULT_VOLTAGE_FORMAT).toInt())) {
    case VoltageAll:
        voltageSaveAllRb->setChecked(true);
        voltageSaveSeparatelyRb->setChecked(false);
        voltageDontSaveRb->setChecked(false);
        break;

    case VoltageSeparate:
        voltageSaveAllRb->setChecked(false);
        voltageSaveSeparatelyRb->setChecked(true);
        voltageDontSaveRb->setChecked(false);
        break;

    case VoltageNone:
        voltageSaveAllRb->setChecked(false);
        voltageSaveSeparatelyRb->setChecked(false);
        voltageDontSaveRb->setChecked(true);
        break;

    default:
        voltageSaveAllRb->setChecked(true);
        voltageSaveSeparatelyRb->setChecked(false);
        voltageDontSaveRb->setChecked(false);
        break;
    }
    voltageDecimatorSbx->setValue(settings.value(GLB_PROTOCOL_VOLTAGE_DECIMATOR_FACTOR_TAG, PSD_DEFAULT_VOLTAGE_DECIMATION_FACTOR).toInt());
    recordDurationEdit->setValue(settings.value(GLB_PROTOCOL_RECORD_DURATION_TAG, PSD_DEFAULT_RECORD_DURATION).toDouble());
    chunkDurationEdit->setValue(settings.value(GLB_PROTOCOL_CHUNK_DURATION_TAG, PSD_DEFAULT_CHUNK_DURATION).toDouble());
}



void RecordSettingsDialog::onSaveSettings() {
    QSettings settings;

//    settings.setValue(GLB_PROTOCOL_RECORD_PATH_TAG, recordPathEdit->text());
//    settings.setValue(GLB_PROTOCOL_RECORD_NAME_TAG, recordNameEdit->text());
    settings.setValue(GLB_PROTOCOL_ADD_DATE_TAG, addDateChx->isChecked());
    settings.setValue(GLB_PROTOCOL_RECORD_FORMAT_TAG, (int)(this->getRecordFileFormat()));
    settings.setValue(GLB_PROTOCOL_VOLTAGE_FORMAT_TAG, (int)(this->getVoltageRecordFormat()));
    settings.setValue(GLB_PROTOCOL_VOLTAGE_DECIMATOR_FACTOR_TAG, voltageDecimatorSbx->value());
    settings.setValue(GLB_PROTOCOL_RECORD_DURATION_TAG, recordDurationEdit->value());
    settings.setValue(GLB_PROTOCOL_CHUNK_DURATION_TAG, chunkDurationEdit->value());
}

void RecordSettingsDialog::onAccept() {
    this->onSaveSettings();

    RecordSettings_t settings;
    settings.appendDate = addDateChx->isChecked();
    settings.fileFormat = this->getRecordFileFormat();
    settings.voltageFormat = this->getVoltageRecordFormat();
    settings.voltageDecimationFactor = voltageDecimatorSbx->value();
    settings.recordDurationS = recordDurationEdit->value();
    settings.chunkDurationS = chunkDurationEdit->value();
    emit sigSettingsSet(settings);

    this->accept();
}

void RecordSettingsDialog::onReject() {
    this->onLoadSettings();
    this->reject();
}
