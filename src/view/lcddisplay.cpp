#include "lcddisplay.h"
#include "qpauseanimation.h"
#include <QSvgWidget>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include <QSequentialAnimationGroup>
#include <QEasingCurve>

LcdDisplay::LcdDisplay(QWidget * parent) :
    QLCDNumber(parent) {

    this->setFrameShadow(QFrame::Plain);
    this->setSegmentStyle(QLCDNumber::Flat);
    this->setObjectName("QLCD");
    this->setSmallDecimalPoint(false);
}

LcdDisplay::~LcdDisplay() {}

TimerDisplay::TimerDisplay(QWidget * parent, QString timeFormat) :
    QWidget(parent),
    timeFormat(timeFormat) {

    this->setFixedHeight(25);
    this->setAttribute(Qt::WA_StyledBackground, true);

    // RECORDING icons
    iconLabel = new QLabel(this);
    iconLabel->setObjectName("recordingIcon");

    // RECODING icon - idle
    iconLabelIdle = new QFrame(this);
    iconLabelIdle->setObjectName("recordingIconIdle");
    iconLabelIdle->setFixedSize(10, 10);

    /* Blinking recording animation */
    auto *effect = new QGraphicsOpacityEffect(iconLabel);
    effect->setOpacity(1.0);

    iconLabel->setGraphicsEffect(effect);

    auto *pulse = new QSequentialAnimationGroup(iconLabel);

    auto *fadeOut = new QPropertyAnimation(effect, "opacity");
    fadeOut->setDuration(1500);
    fadeOut->setStartValue(1.0);
    fadeOut->setEndValue(0.4);
    fadeOut->setEasingCurve(QEasingCurve::InOutSine);

    auto *fadeIn = new QPropertyAnimation(effect, "opacity");
    fadeIn->setDuration(1500);
    fadeIn->setStartValue(0.4);
    fadeIn->setEndValue(1.0);
    fadeIn->setEasingCurve(QEasingCurve::InOutSine);

    auto *pause = new QPauseAnimation(800);

    pulse->addAnimation(pause);
    pulse->addAnimation(fadeOut);
    pulse->addAnimation(fadeIn);
    pulse->setLoopCount(-1);
    pulse->start();

    iconLabelIdle->setVisible(true);
    iconLabel->setVisible(false);

    lcdDisplay = new LcdDisplay(this);
    int digitCount = timeFormat.size();
    lcdDisplay->setDigitCount(digitCount);
    lcdDisplay->setFixedHeight(25);

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(4, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(iconLabelIdle, 0, Qt::AlignCenter);
    layout->addWidget(iconLabel, 0, Qt::AlignCenter);
    layout->addWidget(lcdDisplay, 0, Qt::AlignCenter);

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

void TimerDisplay::displayTime(QTime time) {
    lcdDisplay->display(time.toString(timeFormat));
}

void TimerDisplay::onStartTimer(bool ascending, QTime startTime) {
    ascendingTimer = ascending;
    currentTime = startTime;
    paused = false;

    this->displayTime(currentTime);
    iconLabel->setVisible(true);
    iconLabelIdle->setVisible(false);

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
            iconLabel->setVisible(true);
            iconLabelIdle->setVisible(false);
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

        if (!paused) {
            iconLabel->setVisible(false);
            iconLabelIdle->setVisible(true);
        }
    } else {
        paused &= pauseFlag;
        if (!paused) iconLabel->setVisible(false);
        if (paused) iconLabelIdle->setVisible(true);
    }
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
