#include "messagedialog.h"

#include <QLabel>
#include <QDialogButtonBox>
#include <QScrollArea>
#include "qtextbrowser.h"
#include "themecontroller.h"

MessageDialog::MessageDialog(QString title, bool includeLogo, QWidget* parent) :
    QDialog(parent) {

    /*! hide the help button on the window bar */
    Qt::WindowFlags flags = this->windowFlags();
    Qt::WindowFlags helpFlag = Qt::WindowContextHelpButtonHint;
    flags = flags & (~helpFlag);
    this->setWindowFlags(flags);

    this->setWindowTitle(title);

    mainVl = new QVBoxLayout;
    mainVl->setContentsMargins(10, 10, 10, 10);
    mainVl->setSpacing(8);

    mainVl->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

    this->setLayout(mainVl);

    if (includeLogo) {
        QLabel* logoLbl = new QLabel;
        QPixmap logoPxm(ThemeController::imgsPath() + "/logo_with_name.png");
        logoLbl->setPixmap(logoPxm.scaledToWidth(381, Qt::SmoothTransformation));
        mainVl->addWidget(logoLbl);

        mainVl->addItem(new QSpacerItem(0, 30, QSizePolicy::Fixed, QSizePolicy::Fixed));

    }
}

void MessageDialog::addMainText(QString text) {
    QTextBrowser* browser = new QTextBrowser();
    browser->setObjectName("msgDialogText");
    browser->setHtml(text);
    browser->setFrameShape(QFrame::NoFrame);
    browser->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    mainVl->addWidget(browser);
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
