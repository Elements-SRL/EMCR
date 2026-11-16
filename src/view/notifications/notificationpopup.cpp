#include "notificationpopup.h"

#include <QButtonGroup>
#include <QHBoxLayout>
#include <QMouseEvent>
#include <QStyle>

NotificationPopup::NotificationPopup(QWidget * parent) :
    QFrame(parent, Qt::Popup) {

    this->setObjectName("notificationPopup");
    this->setFixedWidth(popupWidth);

    QVBoxLayout * mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->setSpacing(8);

    // --- HEADER: tabs + mark all as read ---
    QHBoxLayout * headerLayout = new QHBoxLayout();
    headerLayout->setSpacing(4);

    unreadTabBtn = new QPushButton("Unread");
    unreadTabBtn->setObjectName("notificationTabBtn");
    unreadTabBtn->setCheckable(true);
    unreadTabBtn->setChecked(true);
    unreadTabBtn->setCursor(Qt::PointingHandCursor);

    allTabBtn = new QPushButton("All");
    allTabBtn->setObjectName("notificationTabBtn");
    allTabBtn->setCheckable(true);
    allTabBtn->setCursor(Qt::PointingHandCursor);

    QButtonGroup * tabGroup = new QButtonGroup(this);
    tabGroup->setExclusive(true);
    tabGroup->addButton(unreadTabBtn);
    tabGroup->addButton(allTabBtn);

    markAllReadBtn = new QPushButton("Mark all as read");
    markAllReadBtn->setObjectName("notificationLinkBtn");
    markAllReadBtn->setCursor(Qt::PointingHandCursor);

    headerLayout->addWidget(unreadTabBtn);
    headerLayout->addWidget(allTabBtn);
    headerLayout->addStretch();
    headerLayout->addWidget(markAllReadBtn);

    connect(unreadTabBtn, &QPushButton::clicked, this, [this]() {
        showOnlyUnread = true;
        this->refresh();
    });

    connect(allTabBtn, &QPushButton::clicked, this, [this]() {
        showOnlyUnread = false;
        this->refresh();
    });

    connect(markAllReadBtn, &QPushButton::clicked, this, []() {
        NotificationCenter::getInstance().markAllAsRead();
    });

    // --- NOTIFICATIONS LIST ---
    scrollArea = new QScrollArea();
    scrollArea->setObjectName("notificationScrollArea");
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    listContainer = new QWidget();
    listContainer->setObjectName("notificationContainer");

    listLayout = new QVBoxLayout(listContainer);
    listLayout->setContentsMargins(0, 0, 0, 0);
    listLayout->setSpacing(6);

    scrollArea->setWidget(listContainer);

    mainLayout->addLayout(headerLayout);
    mainLayout->addWidget(scrollArea);

    // Keeps the list updated while the popup is open
    connect(&NotificationCenter::getInstance(), &NotificationCenter::sigNotificationsChanged, this, [this]() {
        if (this->isVisible()) {
            this->refresh();
        }
    });
}

/*
 * Shows the popup under the anchor widget, right aligned.
 */
void NotificationPopup::showBelow(QWidget * anchor) {
    this->refresh();

    QPoint pos = anchor->mapToGlobal(QPoint(anchor->width() - this->width(), anchor->height() + 4));
    this->move(pos);
    this->show();
}

qint64 NotificationPopup::getLastHideTime() {
    return lastHideTime;
}

void NotificationPopup::hideEvent(QHideEvent * event) {
    lastHideTime = QDateTime::currentMSecsSinceEpoch();
    QFrame::hideEvent(event);
}

/*
 * Clicking on a notification marks it as read.
 */
bool NotificationPopup::eventFilter(QObject * obj, QEvent * event) {
    if (event->type() == QEvent::MouseButtonPress) {
        QFrame * row = qobject_cast<QFrame *>(obj);
        if (row && row->objectName() == "notificationItem") {
            NotificationCenter::getInstance().markAsRead(row->property("notificationId").toInt());
            return true;
        }
    }
    return QFrame::eventFilter(obj, event);
}

/*
 * Rebuilds the list of notifications.
 */
void NotificationPopup::refresh() {
    NotificationCenter &center = NotificationCenter::getInstance();
    int unreadCount = center.getUnreadCount();

    unreadTabBtn->setText(QString("Unread (%1)").arg(unreadCount));
    markAllReadBtn->setEnabled(unreadCount > 0);

    // Removing old items. The refresh may be triggered by a button
    // inside an item, so the items are hidden and deleted later
    QLayoutItem * layoutItem;
    while ((layoutItem = listLayout->takeAt(0)) != nullptr) {
        if (layoutItem->widget()) {
            layoutItem->widget()->hide();
            layoutItem->widget()->deleteLater();
        }
        delete layoutItem;
    }

    // Newest notifications on top
    QVector<NotificationItem> items = center.getNotifications();
    int shownCount = 0;
    for (int i = items.size() - 1; i >= 0; i--) {
        if (showOnlyUnread && items[i].read) {
            continue;
        }
        QFrame * row = this->createItem(items[i]);
        listLayout->addWidget(row);
        row->show(); // Needed to compute the height while the popup is open
        shownCount++;
    }

    if (shownCount == 0) {
        QLabel * emptyLbl = new QLabel(showOnlyUnread ? "You're all caught up" : "No notifications");
        emptyLbl->setObjectName("notificationEmptyLbl");
        emptyLbl->setAlignment(Qt::AlignCenter);
        listLayout->addWidget(emptyLbl);
        emptyLbl->show();
    }

    listLayout->addStretch();

    // The popup height follows the content, up to a maximum
    int listWidth = popupWidth - 20;
    int listHeight = listLayout->hasHeightForWidth() ? listLayout->heightForWidth(listWidth) : listLayout->sizeHint().height();
    scrollArea->setFixedHeight(qMin(listHeight, maxListHeight));
    this->adjustSize();
}

QFrame * NotificationPopup::createItem(const NotificationItem &item) {
    QString levelStr = NotificationCenter::getLevelStr(item.level);

    QFrame * row = new QFrame();
    row->setObjectName("notificationItem");
    row->setProperty("level", levelStr);
    row->setProperty("read", item.read);
    row->setProperty("notificationId", item.id);
    row->setCursor(Qt::PointingHandCursor);
    row->installEventFilter(this);

    QHBoxLayout * rowLayout = new QHBoxLayout(row);
    rowLayout->setContentsMargins(8, 8, 8, 8);
    rowLayout->setSpacing(8);

    QLabel * dotLbl = new QLabel("●");
    dotLbl->setObjectName("notificationDot");
    dotLbl->setProperty("level", levelStr);

    QLabel * textLbl = new QLabel(item.text);
    textLbl->setObjectName("notificationText");
    textLbl->setProperty("level", levelStr);
    textLbl->setProperty("read", item.read);
    textLbl->setWordWrap(true);

    QLabel * timeLbl = new QLabel(formatTime(item.time));
    timeLbl->setObjectName("notificationTime");

    QPushButton * closeBtn = new QPushButton("✕");
    closeBtn->setObjectName("closeNotificationBtn");
    closeBtn->setCursor(Qt::PointingHandCursor);
    closeBtn->setFixedWidth(20);
    closeBtn->setToolTip("Dismiss");

    int id = item.id;
    connect(closeBtn, &QPushButton::clicked, this, [id]() {
        NotificationCenter::getInstance().removeNotification(id);
    });

    QVBoxLayout * rightLayout = new QVBoxLayout();
    rightLayout->setSpacing(2);
    rightLayout->addWidget(timeLbl, 0, Qt::AlignRight);
    rightLayout->addWidget(closeBtn, 0, Qt::AlignRight);
    rightLayout->addStretch();

    rowLayout->addWidget(dotLbl, 0, Qt::AlignTop);
    rowLayout->addWidget(textLbl, 1, Qt::AlignTop);
    rowLayout->addLayout(rightLayout);

    return row;
}

/*
 * Today: hh:mm, yesterday: "Yesterday", otherwise dd/MM hh:mm.
 */
QString NotificationPopup::formatTime(const QDateTime &time) {
    QDate today = QDate::currentDate();
    if (time.date() == today) {
        return time.toString("hh:mm");
    } else if (time.date() == today.addDays(-1)) {
        return "Yesterday";
    }
    return time.toString("dd/MM hh:mm");
}
