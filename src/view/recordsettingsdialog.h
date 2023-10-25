#ifndef RECORDSETTINGSDIALOG_H
#define RECORDSETTINGSDIALOG_H

#include <QDialog>
#include <QBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QButtonGroup>
#include <QDir>
#include <QRadioButton>
#include <QCheckBox>
#include <QDoubleSpinBox>
#include <QLabel>
#include <QGroupBox>

#include "globaldefines.h"

#define PSD_DEFAULT_ADD_DATE false
#define PSD_DEFAULT_RECORD_FORMAT 0
#define PSD_DEFAULT_RECORD_DURATION 0.0
#define PSD_DEFAULT_CHUNK_DURATION 0.0
#define PSD_MAX_MB_PER_FILE 1900.0
#define BYTES_PER_MEGA_BYTES 1048576.0

class RecordSettingsDialog : public QDialog {
    Q_OBJECT

public:
    typedef enum {
//        RecordFileDat,
//        RecordFileEdrf,
        RecordFileAbf,
        RecordFileNone
    } RecordFileFormat_t;

    typedef struct RecordSettings {
        bool appendDate = false;
        RecordFileFormat_t fileFormat;
        double recordDurationS = 0.0;
        double chunkDurationS = 0.0;
    } RecordSettings_t;

    RecordSettingsDialog();
    void forceSettingsEmit();

public slots:
    void onFileSizeComputed(QString message);

private:
    RecordFileFormat_t getRecordFileFormat();

    QCheckBox * addDateChx;
//    QRadioButton * recordFormatDatRb;
//    QRadioButton * recordFormatEdrfRb;
    QRadioButton * recordFormatAbfRb;

    QButtonGroup * recordFormatBg;

    QDoubleSpinBox * recordDurationEdit;
    QSpinBox * chunkDurationEdit;
    QLabel * recordSizeLbl;

    RecordFileFormat_t format = RecordFileAbf;

//    QString baseFileName = "";
//    QString filePath = "";
//    QString subFolder = "";
//    int chunkIdx = 0;

private slots:
//    void onRecordPathBrowseBtnClicked();
    void onLoadSettings();
    void onSaveSettings();
    void onAccept();
    void onReject();

signals:
    void sigSettingsSet(RecordSettings_t settings);
//    void newRecordPath();
};

#endif // RECORDSETTINGSDIALOG_H
