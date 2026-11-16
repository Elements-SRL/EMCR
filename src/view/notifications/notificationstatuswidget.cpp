#include "notificationstatuswidget.h"

#include <QHBoxLayout>
#include <QIcon>
#include <QMouseEvent>
#include <QPainter>
#include <QStyle>

#include "themecontroller.h"

NotificationStatusWidget::NotificationStatusWidget(QWidget * parent) :
    QFrame(parent) {

    this->setObjectName("notificationStatus");
    this->setCursor(Qt::PointingHandCursor);

    QHBoxLayout * layout = new QHBoxLayout(this);
    layout->setContentsMargins(8, 2, 10, 2);
    layout->setSpacing(4);

    bellLbl = new QLabel();
    bellLbl->setObjectName("notificationBell");
    bellLbl->setFixedSize(bellBox, bellBox);

    textLbl = new QLabel();
    textLbl->setObjectName("notificationStatusText");

    // Content aligned to the right side of the menu bar
    layout->addStretch();
    layout->addWidget(textLbl);
    layout->addWidget(bellLbl);

    popup = new NotificationPopup(this);

    // Bell shake: damped oscillation around the top of the bell
    shakeAnim = new QVariantAnimation(this);
    shakeAnim->setDuration(700);
    shakeAnim->setStartValue(0.0);
    shakeAnim->setKeyValueAt(0.15, 16.0);
    shakeAnim->setKeyValueAt(0.35, -12.0);
    shakeAnim->setKeyValueAt(0.55, 8.0);
    shakeAnim->setKeyValueAt(0.75, -4.0);
    shakeAnim->setEndValue(0.0);

    connect(shakeAnim, &QVariantAnimation::valueChanged, this, [this](const QVariant &value) {
        this->updateBellPixmap(value.toDouble());
    });

    NotificationCenter &center = NotificationCenter::getInstance();
    connect(&center, &NotificationCenter::sigNotificationsChanged, this, &NotificationStatusWidget::updateStatus);
    connect(&center, &NotificationCenter::sigNotificationAdded, this, &NotificationStatusWidget::onNotificationAdded);

    // Bell color comes from the QSS, refresh it when the theme changes
    connect(&ThemeController::getInstance(), &ThemeController::sigThemeUpdated, this, &NotificationStatusWidget::updateStatus);

    this->updateStatus();
}

/*
 * The menu bar places the corner widget using its sizeHint,
 * so the reserved minimum width must be part of it.
 */
QSize NotificationStatusWidget::sizeHint() const {
    return QFrame::sizeHint().expandedTo(this->minimumSize());
}

void NotificationStatusWidget::mousePressEvent(QMouseEvent * event) {
    if (event->button() != Qt::LeftButton) {
        return;
    }

    // The same click that closed the popup must not reopen it
    if (QDateTime::currentMSecsSinceEpoch() - popup->getLastHideTime() < 200) {
        return;
    }

    popup->showBelow(this);
}

/*
 * Updates text, color and tooltip based on the unread notifications.
 */
void NotificationStatusWidget::updateStatus() {
    NotificationCenter &center = NotificationCenter::getInstance();
    int unreadCount = center.getUnreadCount();

    QString levelStr = "OK";
    if (unreadCount == 0) {
        textLbl->setText("All good");
        this->setToolTip("No new notifications");

    } else {
        levelStr = NotificationCenter::getLevelStr(center.getHighestUnreadLevel());
        textLbl->setText(unreadCount == 1 ? "1 notification" : QString("%1 notifications").arg(unreadCount));
        this->setToolTip(QString("%1 errors, %2 warnings, %3 info")
                             .arg(center.getUnreadCount(NOTIFY_ERROR))
                             .arg(center.getUnreadCount(NOTIFY_WARNING))
                             .arg(center.getUnreadCount(NOTIFY_INFO)));
    }

    // Dynamic QSS refresh
    this->setProperty("level", levelStr);
    textLbl->setProperty("level", levelStr);
    this->style()->unpolish(this);
    this->style()->polish(this);
    textLbl->style()->unpolish(textLbl);
    textLbl->style()->polish(textLbl);

    // Bell tinted with the same color of the text (set by QSS)
    QColor color = textLbl->palette().color(QPalette::WindowText);
    qreal dpr = this->devicePixelRatioF();

    bellPix = QIcon(ThemeController::iconPath() + "/bell.svg").pixmap(QSize(bellSize, bellSize), dpr);
    QPainter painter(&bellPix);
    painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
    painter.fillRect(QRectF(0, 0, bellSize, bellSize), color);
    painter.end();

    double angle = (shakeAnim->state() == QAbstractAnimation::Running) ? shakeAnim->currentValue().toDouble() : 0.0;
    this->updateBellPixmap(angle);

    // The menu bar does not resize the corner widget when the text
    // changes, so the space for the longest text is reserved
    int maxTextWidth = textLbl->fontMetrics().horizontalAdvance("99 notifications");
    this->setMinimumWidth(maxTextWidth + bellBox + 30);
}

void NotificationStatusWidget::onNotificationAdded(QString, Notification level) {
    // No animation while the user is looking at the list
    if (popup->isVisible() || shakeAnim->state() == QAbstractAnimation::Running) {
        return;
    }

    // Errors shake twice
    shakeAnim->setLoopCount(level == NOTIFY_ERROR ? 2 : 1);
    shakeAnim->start();
}

/*
 * Draws the bell rotated by the given angle (degrees),
 * pivoting on the top center as if it was hanging.
 */
void NotificationStatusWidget::updateBellPixmap(double angle) {
    qreal dpr = this->devicePixelRatioF();

    QPixmap canvas(QSize(bellBox, bellBox) * dpr);
    canvas.setDevicePixelRatio(dpr);
    canvas.fill(Qt::transparent);

    QPainter painter(&canvas);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);
    painter.translate(bellBox / 2.0, bellPadding);
    painter.rotate(angle);
    painter.translate(-bellBox / 2.0, -bellPadding);
    painter.drawPixmap(QPointF(bellPadding, bellPadding), bellPix);
    painter.end();

    bellLbl->setPixmap(canvas);
}
