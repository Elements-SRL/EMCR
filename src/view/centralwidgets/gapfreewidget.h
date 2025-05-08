#ifndef GAPFREEWIDGET_H
#define GAPFREEWIDGET_H

#include <QWidget>
#include <QBoxLayout>
#include <QPushButton>
#include <QLineEdit>
#include "bigplot.h"


class GapFreeWidget : public QWidget {
    Q_OBJECT

private:
    QLineEdit* fileNameLineEdit;
    QLineEdit* recordPathLineEdit;
    QPushButton* recordingStopBtn;
    QPushButton* recordingStartBtn;
    void emitFileName();
    void emitFilePath();

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
