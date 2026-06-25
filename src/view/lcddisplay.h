#ifndef LCDDISPLAY_H
#define LCDDISPLAY_H

#include <QLCDNumber>
#include <QElapsedTimer>
#include <QTimer>
#include <QTime>
#include <QWidget>
#include <QLabel>
#include <QHBoxLayout>

#define TDP_INTERVAL_MS 1000

class LcdDisplay : public QLCDNumber {
    Q_OBJECT
public:
    LcdDisplay(QWidget * parent);
    virtual ~LcdDisplay();
};

class TimerDisplay : public QWidget {
    Q_OBJECT
public:
    TimerDisplay(QWidget * parent = nullptr, QString timeFormat = "hh:mm:ss");
    ~TimerDisplay();

    void displayTime(QTime time);

public slots:
    void onStartTimer(bool ascending = true, QTime startTime = QTime(0, 0, 0, 0));
    void onRestartTimer();
    void onStopTimer(bool pauseFlag = false);

private:
    QString timeFormat;
    QTimer * timer = nullptr;
    QElapsedTimer elapsedTimer;
    qint64 elapsedEvents = 0;
    bool ascendingTimer;
    QTime currentTime;
    int remainingTime = TDP_INTERVAL_MS;
    bool paused = false;

    QFrame *iconLabelIdle = nullptr;
    QLabel *iconLabel = nullptr;
    LcdDisplay *lcdDisplay = nullptr;

private slots:
    void onUpdateTimer();
};

#endif // LCDDISPLAY_H
