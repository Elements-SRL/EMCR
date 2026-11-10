#ifndef NOTIFICATIONSTATUSWIDGET_H
#define NOTIFICATIONSTATUSWIDGET_H

#include <QFrame>
#include <QLabel>
#include <QVariantAnimation>

#include "notificationcenter.h"
#include "notificationpopup.h"

/*
 * Bell + status text shown in the top-right corner of the menu bar.
 * Text and color follow the unread notifications:
 *   none     -> "All good"
 *   1 or more -> "N notifications", colored by the most severe level
 * The bell shakes when a new notification arrives.
 * Clicking the widget opens the NotificationPopup.
 */
class NotificationStatusWidget : public QFrame {
    Q_OBJECT

public:
    explicit NotificationStatusWidget(QWidget * parent = nullptr);

    QSize sizeHint() const override;

protected:
    void mousePressEvent(QMouseEvent * event) override;

private slots:
    void updateStatus();
    void onNotificationAdded(QString text, Notification level);

private:
    void updateBellPixmap(double angle);

    static constexpr int bellSize = 16;
    static constexpr int bellBox = 22;
    static constexpr int bellPadding = 3;

    QLabel * bellLbl = nullptr;
    QLabel * textLbl = nullptr;
    NotificationPopup * popup = nullptr;
    QVariantAnimation * shakeAnim = nullptr;

    QPixmap bellPix; // Tinted bell, not rotated
};

#endif // NOTIFICATIONSTATUSWIDGET_H
