#ifndef NOTIFICATIONCENTER_H
#define NOTIFICATIONCENTER_H

#include <QObject>
#include <QString>
#include <QDateTime>
#include <QVector>

// Notification levels, sorted by severity
enum Notification {
    NOTIFY_NONE,
    NOTIFY_INFO,
    NOTIFY_WARNING,
    NOTIFY_ERROR
};

struct NotificationItem {
    int id = 0;
    QString text;
    Notification level = NOTIFY_NONE;
    QDateTime time;
    bool read = false;
};

/*
 * Collects the notifications of the whole application.
 * Any widget/controller can add a notification with:
 *   NotificationCenter::getInstance().addNotification("text", NOTIFY_WARNING);
 *
 * addNotification can be called from any thread, the other
 * methods must be called from the GUI thread.
 * Notifications are kept in memory only.
 */
class NotificationCenter : public QObject {
    Q_OBJECT

public:

    // Singleton
    static NotificationCenter& getInstance();
    NotificationCenter(const NotificationCenter&) = delete;
    NotificationCenter& operator=(const NotificationCenter&) = delete;

    static QString getLevelStr(Notification level);

    void addNotification(const QString &text, Notification level);
    void markAsRead(int id);
    void markAllAsRead();
    void removeNotification(int id);
    void clearNotifications();

    QVector<NotificationItem> getNotifications() const;
    int getUnreadCount(Notification level = NOTIFY_NONE) const;
    Notification getHighestUnreadLevel() const;

private:
    NotificationCenter();
    ~NotificationCenter();

    static constexpr int maxNotifications = 100;

    QVector<NotificationItem> notifications;
    int nextId = 1;

signals:
    void sigNotificationAdded(QString text, Notification level);
    void sigNotificationsChanged();
};

#endif // NOTIFICATIONCENTER_H
