#ifndef PROTOCOLSSETTINGSDIALOG_H
#define PROTOCOLSSETTINGSDIALOG_H

#include <QDialog>
#include <QBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QButtonGroup>
#include <QDir>
#include <QRadioButton>
#include <QCheckBox>

#define PSD_DEFAULT_RECORD_PATH QString(QDir::homePath() + "/EZ Patch/Recordings/")
#define PSD_DEFAULT_RECORD_NAME QString("file")
#define PSD_DEFAULT_ADD_DATE false
#define PSD_DEFAULT_RECORD_FORMAT 0

class ProtocolsSettingsDialog : public QDialog {
    Q_OBJECT

public:
    typedef enum {
//        RecordFileEdrf,
        RecordFileAbf,
        RecordFileNone
    } RecordFileFormat_t;

    ProtocolsSettingsDialog();

    QString getRecordPath();
    QString getRecordName();
    bool getAddDate();
    RecordFileFormat_t getRecordFileFormat();
    void synchronizeSettings();

private:
    QLineEdit * recordPathEdit;
    QLineEdit * recordNameEdit;
    QCheckBox * addDateChx;
//    QRadioButton * recordFormatEdrfRb;
    QRadioButton * recordFormatAbfRb;

    QButtonGroup * recordFormatBg;

private slots:
    void onRecordPathBrowseBtnClicked();
    void onLoadSettings();
    void onSaveSettings();
    void onAccept();
    void onReject();

signals:
    void newRecordPath();
};

#endif // PROTOCOLSSETTINGSDIALOG_H
