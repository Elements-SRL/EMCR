#include "elementslogowidget.h"

#include <QGridLayout>
#include <QSpacerItem>

ElementsLogoWidget::ElementsLogoWidget(QWidget * parent) :
    QWidget(parent) {

    QGridLayout * mainLo = new QGridLayout;
    mainLo->setContentsMargins(6, 6, 6, 6);
    mainLo->setSpacing(6);

    this->setLayout(mainLo);

    mainLo->addItem(new QSpacerItem(0, 0, QSizePolicy::MinimumExpanding, QSizePolicy::Fixed), 1, 0);
    mainLo->addItem(new QSpacerItem(0, 0, QSizePolicy::MinimumExpanding, QSizePolicy::Fixed), 1, 2);
    mainLo->addItem(new QSpacerItem(0, 0, QSizePolicy::Fixed, QSizePolicy::MinimumExpanding), 0, 1);
    mainLo->addItem(new QSpacerItem(0, 0, QSizePolicy::Fixed, QSizePolicy::MinimumExpanding), 2, 1);

    pixmapLbl = new QLabel;
    pixmap = QPixmap(":/imgs/logo_with_name.png");

    mainLo->addWidget(pixmapLbl, 1, 1);
}

ElementsLogoWidget::~ElementsLogoWidget() {

}

void ElementsLogoWidget::resizeEvent(QResizeEvent *) {
    QPixmap p = pixmap.scaledToWidth(this->width()-12, Qt::SmoothTransformation);
    pixmapLbl->setPixmap(p);
}
