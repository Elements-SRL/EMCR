#ifndef GAPFREEWIDGET_H
#define GAPFREEWIDGET_H

#include <QWidget>
#include <QBoxLayout>
#include <QPushButton>
#include <QLineEdit>
#include <qcheckbox.h>
#include "bigplot.h"
#include "autotoggle.h"
#include "lcddisplay.h"


class GapFreeWidget : public QWidget {
    Q_OBJECT

private:
    QPushButton* autoZoom = nullptr;
    QPushButton* btnZoomIn = nullptr;
    QPushButton* btnZoomOut = nullptr;
    QLineEdit* fileNameLineEdit;
    QLineEdit* recordPathLineEdit;
    QPushButton* browseBtn;
    QPushButton* startStopBtn;
    TimerDisplay* protocolTimer;
    void emitFileName();
    void emitFilePath();

public:
    GapFreeWidget(BigPlot* plot, QWidget* parent = nullptr);
    void setRecording(bool flag);
    void lockUnlockSettings(bool isRecording);

signals:
    void sigFileNameChanged(QString);
    void sigRecordPathChanged(QString);
    void sigStartRecording();
    void sigStopRecording();
    void sigAutoZoom();
};

#endif // GAPFREEWIDGET_H
