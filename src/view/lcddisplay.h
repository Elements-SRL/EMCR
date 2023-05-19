#ifndef LCDDISPLAY_H
#define LCDDISPLAY_H

#include <QLCDNumber>
#include <QTimer>
#include <QTime>

/*! \todo FCON bisogna verificare se questi numeri vanno bene in assoluto (altri monitor, altri OS) */

#define LCD_DISPLAY_DIGIT_HEIGHT 25
#define LCD_DISPLAY_DIGIT_WIDTH 16

#define TDP_INTERVAL_MS 1000

class LcdDisplay : public QLCDNumber {
public:
    LcdDisplay(QWidget * parent);
    virtual ~LcdDisplay();
};

class TimerDisplay : public LcdDisplay {
    Q_OBJECT

public:
    TimerDisplay(QWidget * parent = nullptr, QString timeFormat = "mm.ss");
    ~TimerDisplay();

    void displayTime(QTime time);

public slots:
    void onStartTimer(bool ascending = true, QTime startTime = QTime(0, 0, 0, 0));
    void onRestartTimer();
    void onStopTimer(bool pauseFlag = false);

private:
    QString timeFormat;
    QTimer * timer = nullptr;
    bool ascendingTimer;
    QTime currentTime;
    int reaminingTime = TDP_INTERVAL_MS;
    bool paused = false;

private slots:
    void onUpdateTimer();
};

#endif // LCDDISPLAY_H
