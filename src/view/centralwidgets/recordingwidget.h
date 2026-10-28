#ifndef RECORDINGWIDGET_H
#define RECORDINGWIDGET_H

#include "lcddisplay.h"
#include <QObject>
#include <qlabel.h>
#include <qlineedit.h>
#include <qpushbutton.h>

class RecordingWidget : public QWidget {
    Q_OBJECT

public:
    RecordingWidget(QWidget * parent = nullptr,
                    const char * defaultFileNameTag = nullptr,
                    const char * defaultFilePathTag = nullptr,
                    QString defaultFileName = nullptr,
                    QString defaultFilePath = nullptr);
    void lockUnlockSettings(bool isRecording);      // Enable/disable UI componets based on recording stataus
    void setRecording(bool flag);                   // Manages the timer and start button

private:
    QVBoxLayout * mainLayout = nullptr;
    QLabel * recordingTitle = nullptr;
    QLabel * fileIco = nullptr;
    QLabel * fileNameLbl = nullptr;
    QLineEdit * fileNameLineEdit = nullptr;
    QLabel * recordIco = nullptr;
    QLabel * recordPathLbl = nullptr;
    QLineEdit * recordPathLineEdit = nullptr;
    TimerDisplay * protocolTimer = nullptr;
    QPushButton * startStopBtn = nullptr;
    QPushButton * btnSettings = nullptr;
    QPushButton * browseBtn = nullptr;
    QPushButton * goToDirBtn = nullptr;

    const char * defaultFileNameTag = nullptr;
    const char * defaultFilePathTag = nullptr;

    void emitFilePath();
    void emitFileName();

signals:
    void sigRecordingStarted();
    void sigRecordingStopped();
    void sigRecordPathChanged(QString);
    void sigFileNameChanged(QString);

};

#endif // RECORDINGWIDGET_H
