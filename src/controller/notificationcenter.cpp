#include "notificationcenter.h"
#include <QCoreApplication>
#include <QThread>

NotificationCenter::NotificationCenter() {
    // The center must live in the GUI thread, even when
    // the first call comes from a worker thread
    if (QCoreApplication::instance()) {
        this->moveToThread(QCoreApplication::instance()->thread());
    }
}

NotificationCenter::~NotificationCenter() {}

// Singleton
NotificationCenter& NotificationCenter::getInstance() {
    static NotificationCenter instance;
    return instance;
}

QString NotificationCenter::getLevelStr(Notification level) {
    switch (level) {
    case NOTIFY_INFO:
        return "INFO";
    case NOTIFY_WARNING:
        return "WARNING";
    case NOTIFY_ERROR:
        return "ERROR";
    default:
        return "NONE";
    }
}

/*
 * Adds a new notification.
 * Can be called from any thread: calls coming from a worker
 * thread are forwarded to the GUI thread.
 *
 * Param: text  | The message to be shown.
 * Param: level | Notification type - error, info etc.
 */
void NotificationCenter::addNotification(const QString &text, Notification level) {
    if (level == NOTIFY_NONE || text.isEmpty()) {
        return;
    }

    if (QThread::currentThread() != this->thread()) {
        QMetaObject::invokeMethod(this, [this, text, level]() {
            this->addNotification(text, level);
        }, Qt::QueuedConnection);
        return;
    }

    NotificationItem item;
    item.id = nextId++;
    item.text = text;
    item.level = level;
    item.time = QDateTime::currentDateTime();
    notifications.append(item);

    // Dropping the oldest notifications
    while (notifications.size() > maxNotifications) {
        notifications.removeFirst();
    }

    emit sigNotificationAdded(text, level);
    emit sigNotificationsChanged();
}

void NotificationCenter::markAsRead(int id) {
    for (NotificationItem &item : notifications) {
        if (item.id == id && !item.read) {
            item.read = true;
            emit sigNotificationsChanged();
            return;
        }
    }
}

void NotificationCenter::markAllAsRead() {
    bool changed = false;
    for (NotificationItem &item : notifications) {
        if (!item.read) {
            item.read = true;
            changed = true;
        }
    }

    if (changed) {
        emit sigNotificationsChanged();
    }
}

void NotificationCenter::removeNotification(int id) {
    for (int i = 0; i < notifications.size(); i++) {
        if (notifications[i].id == id) {
            notifications.removeAt(i);
            emit sigNotificationsChanged();
            return;
        }
    }
}

void NotificationCenter::clearNotifications() {
    if (notifications.isEmpty()) {
        return;
    }
    notifications.clear();
    emit sigNotificationsChanged();
}

QVector<NotificationItem> NotificationCenter::getNotifications() const {
    return notifications;
}

/*
 * Returns the number of unread notifications.
 * Param: level | NOTIFY_NONE counts all levels, otherwise only the given one.
 */
int NotificationCenter::getUnreadCount(Notification level) const {
    int count = 0;
    for (const NotificationItem &item : notifications) {
        if (!item.read && (level == NOTIFY_NONE || item.level == level)) {
            count++;
        }
    }
    return count;
}

/*
 * Returns the most severe level among unread notifications,
 * NOTIFY_NONE if everything has been read.
 */
Notification NotificationCenter::getHighestUnreadLevel() const {
    Notification highest = NOTIFY_NONE;
    for (const NotificationItem &item : notifications) {
        if (!item.read && item.level > highest) {
            highest = item.level;
        }
    }
    return highest;
}
