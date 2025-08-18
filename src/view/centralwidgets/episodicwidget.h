#ifndef EPISODICWIDGET_H
#define EPISODICWIDGET_H

#include <QWidget>
#include <QBoxLayout>
#include <QPushButton>
#include <QLineEdit>

class EpisodicWidget : public QWidget {
    Q_OBJECT

private:
    QLineEdit* fileNameLineEdit;
    QLineEdit* recordPathLineEdit;
    QPushButton* recordingStopBtn;
    QPushButton* recordingStartBtn;
    void emitFileName();
    void emitFilePath();

public:
    EpisodicWidget(QWidget* plot, QWidget* parent = nullptr);
    void setRecording(bool flag);

signals:
    void sigFileNameChanged(QString);
    void sigRecordPathChanged(QString);
    void sigStartRecording();
    void sigStopRecording();
    void sigAutoZoom();
};

#endif // EPISODICWIDGET_H
