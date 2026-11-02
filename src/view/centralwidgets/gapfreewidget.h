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
#include "recordingwidget.h"


class GapFreeWidget : public QWidget {
    Q_OBJECT

private:
    QPushButton* autoZoom = nullptr;
    QPushButton* btnZoomIn = nullptr;
    QPushButton* btnZoomOut = nullptr;
    RecordingWidget* recordingWg = nullptr;

public:
    GapFreeWidget(BigPlot* plot, QWidget* parent = nullptr);
    void setRecording(bool flag);

signals:
    void sigFileNameChanged(QString);
    void sigRecordPathChanged(QString);
    void sigStartRecording();
    void sigStopRecording();
    void sigAutoZoom();
};

#endif // GAPFREEWIDGET_H
