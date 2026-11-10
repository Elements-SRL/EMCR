#ifndef NOTIFICATIONPOPUP_H
#define NOTIFICATIONPOPUP_H

#include <QFrame>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QVBoxLayout>

#include "notificationcenter.h"

/*
 * Small popup listing the notifications of the NotificationCenter.
 * Closes automatically when clicking outside (Qt::Popup).
 */
class NotificationPopup : public QFrame {
    Q_OBJECT

public:
    explicit NotificationPopup(QWidget * parent = nullptr);

    void showBelow(QWidget * anchor);
    qint64 getLastHideTime();

protected:
    void hideEvent(QHideEvent * event) override;
    bool eventFilter(QObject * obj, QEvent * event) override;

private slots:
    void refresh();

private:
    QFrame * createItem(const NotificationItem &item);
    static QString formatTime(const QDateTime &time);

    static constexpr int popupWidth = 340;
    static constexpr int maxListHeight = 360;

    QPushButton * unreadTabBtn = nullptr;
    QPushButton * allTabBtn = nullptr;
    QPushButton * markAllReadBtn = nullptr;

    QScrollArea * scrollArea = nullptr;
    QWidget * listContainer = nullptr;
    QVBoxLayout * listLayout = nullptr;

    bool showOnlyUnread = true;
    qint64 lastHideTime = 0;
};

#endif // NOTIFICATIONPOPUP_H
