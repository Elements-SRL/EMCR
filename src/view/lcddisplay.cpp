#include "lcddisplay.h"

LcdDisplay::LcdDisplay(QWidget * parent) :
    QLCDNumber(parent) {

    this->setStyleSheet("QWidget{color: yellow; background-color: black;} QWidget:disabled{color: silver;}");
    this->setFrameShadow(QFrame::Plain);
    this->setSegmentStyle(QLCDNumber::Flat);
    this->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
    this->setSmallDecimalPoint(true);
}

LcdDisplay::~LcdDisplay() {

}

TimerDisplay::TimerDisplay(QWidget * parent, QString timeFormat) :
    LcdDisplay(parent),
    timeFormat(timeFormat) {

    int digitCount = timeFormat.size()-timeFormat.count('.');
    this->setFixedHeight(LCD_DISPLAY_DIGIT_HEIGHT);
    this->setFixedWidth(LCD_DISPLAY_DIGIT_WIDTH*digitCount);

    this->setDigitCount(digitCount);
    this->displayTime(QTime(0, 0, 0, 0));

    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &TimerDisplay::onUpdateTimer);
}

TimerDisplay::~TimerDisplay() {
    if (timer->isActive()) {
        timer->stop();
    }
    delete timer;
    timer = nullptr;
}

void TimerDisplay::onStartTimer(bool ascending, QTime startTime) {
    ascendingTimer = ascending;
    currentTime = startTime;
    paused = false;

    this->displayTime(currentTime);

    timer->setSingleShot(true);
    elapsedEvents = 0;
    remainingTime = TDP_INTERVAL_MS;
    elapsedTimer.start();
    timer->start(remainingTime);
}

void TimerDisplay::onRestartTimer() {
    if (!(timer->isActive())) {
        if (paused) {
            paused = false;
            elapsedEvents = 0;
            elapsedTimer.start();
            timer->start(remainingTime);

        } else {
            this->onStartTimer();
        }
    }
}

void TimerDisplay::onStopTimer(bool pauseFlag) {
    if (timer->isActive()) {
        paused = pauseFlag;
        remainingTime = timer->remainingTime();
        timer->stop();

    } else {
        /*! If the timer is active it can be paused or stopped
         *  Otherwise it can only pass from paused to stopped */
        paused &= pauseFlag;
    }
}

void TimerDisplay::displayTime(QTime time) {
    QLCDNumber::display(time.toString(timeFormat));
}

void TimerDisplay::onUpdateTimer() {
    if (ascendingTimer) {
        currentTime = currentTime.addSecs(1);

    } else {
        currentTime = currentTime.addSecs(-1);
    }
    this->displayTime(currentTime);

    elapsedEvents++;
    qint64 nextInterval = (elapsedEvents * TDP_INTERVAL_MS) - elapsedTimer.elapsed() + remainingTime;

    timer->start(nextInterval);
}
