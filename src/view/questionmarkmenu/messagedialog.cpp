#include "messagedialog.h"

#include <QLabel>
#include <QDialogButtonBox>
#include <QScrollArea>

MessageDialog::MessageDialog(QString title, bool includeLogo, QWidget* parent) :
    QDialog(parent) {

    /*! hide the help button on the window bar */
    Qt::WindowFlags flags = this->windowFlags();
    Qt::WindowFlags helpFlag = Qt::WindowContextHelpButtonHint;
    flags = flags & (~helpFlag);
    this->setWindowFlags(flags);

    this->setWindowTitle(title);

    mainVl = new QVBoxLayout;
    mainVl->setContentsMargins(6, 6, 6, 6);
    mainVl->setSpacing(6);

    this->setLayout(mainVl);

    if (includeLogo) {
        QLabel* logoLbl = new QLabel;
        QPixmap logoPxm(":/imgs/logo_with_name.png");
        logoLbl->setPixmap(logoPxm.scaledToWidth(381, Qt::SmoothTransformation));
        mainVl->addWidget(logoLbl);

        mainVl->addItem(new QSpacerItem(0, 30, QSizePolicy::Fixed, QSizePolicy::Fixed));
    }
}

void MessageDialog::addMainText(QString text) {
    QLabel* mainTextLbl = new QLabel(text);
    QScrollArea * area = new QScrollArea;
    area->setFrameShape(QFrame::NoFrame);
    area->setWidgetResizable(true);
    area->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    area->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    area->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);
    mainVl->addWidget(area);
    area->setWidget(mainTextLbl);
}

void MessageDialog::addDefaultButtonBox() {
    mainVl->addItem(new QSpacerItem(0, 20, QSizePolicy::Fixed, QSizePolicy::Fixed));

    QDialogButtonBox* okBtn = new QDialogButtonBox(QDialogButtonBox::Ok);
    okBtn->setCenterButtons(true);
    connect(okBtn, &QDialogButtonBox::accepted, this, &MessageDialog::onAccepted);
    mainVl->addWidget(okBtn);
}

void MessageDialog::centerOnParent(QWidget* parent) {
    if (parent != nullptr) {
        QRect pRect = parent->geometry();
        QSize wSize = this->sizeHint();
        QRect wRect;
        wRect.setX(pRect.x() + (pRect.width() - wSize.width()) / 2);
        wRect.setY(pRect.y() + (pRect.height() - wSize.height()) / 2);
        wRect.setWidth(wSize.width());
        wRect.setHeight(wSize.height());
        this->setGeometry(wRect);
    }
}

void MessageDialog::onAccepted() {
    this->close();
}

void MessageDialog::onRejected() {
    this->close();
}
