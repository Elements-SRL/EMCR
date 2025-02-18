#include "collapsiblesection.h"

CollapsibleSection::CollapsibleSection(QString title, Qt::Orientation orientation) :
    QWidget(),
    title(title),
    orientation(orientation) {

    this->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

    QVBoxLayout * mainLo = new QVBoxLayout();
    this->setLayout(mainLo);
    mainLo->setContentsMargins(0, 0, 0, 0);
    mainLo->setSpacing(0);

    QHBoxLayout * titleLo = new QHBoxLayout();
    mainLo->addLayout(titleLo);

    hideShowBtn = new QToolButton();
    hideShowBtn->setStyleSheet("QToolButton {border: none;}");
    hideShowBtn->setArrowType(Qt::ArrowType::RightArrow);
    hideShowBtn->setCheckable(true);
    hideShowBtn->setChecked(false);

    titleEdit = new QLabel(title);
    titleEdit->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

    QWidget * spacer = new QWidget();
    spacer->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Minimum);

    titleLo->addWidget(hideShowBtn);
    titleLo->addWidget(titleEdit);
    titleLo->addWidget(spacer);

    groupBox = new QGroupBox();
    mainLo->addWidget(groupBox);
    groupBox->setVisible(false);

    if (orientation == Qt::Vertical) {
        gbLo = new QVBoxLayout();

    } else {
        gbLo = new QHBoxLayout();
    }
    groupBox->setLayout(gbLo);
    gbLo->setContentsMargins(3, 3, 3, 3);
    gbLo->setSpacing(3);

    buttonGroup = new QButtonGroup();

    connect(hideShowBtn, &QPushButton::clicked, this, &CollapsibleSection::onHideShowBtnClick);
    connect(buttonGroup, &QButtonGroup::buttonToggled, this, &CollapsibleSection::onButtonToggled);
}

void CollapsibleSection::setSingleOption(QString name) {
    titleEdit->setText(title + ": " + name);
    hideShowBtn->setEnabled(false);
    singleOption = true;
    emit buttonToggled(0, true);
}

bool CollapsibleSection::getSingleOption() {
    return singleOption;
}

void CollapsibleSection::addLayout(QBoxLayout * lo) {
    gbLo->addLayout(lo);
}

void CollapsibleSection::addLayout(QGridLayout * lo) {
    gbLo->addLayout(lo);
}

void CollapsibleSection::addPushButton(QPushButton * btn, bool checked) {
    buttonGroup->addButton(btn, btnCount++);
    gbLo->addWidget(btn);
    if (checked == btn->isChecked()) {
        this->onButtonToggled(btn, checked);
        /*! The button emits the toggled signal only if it toggles, so to force the signal emission */
    }
    btn->setChecked(checked);
    buttons.push_back(btn);
}

void CollapsibleSection::addRadioButton(QString name, bool checked) {
    QRadioButton * btn = new QRadioButton(name);
    buttonGroup->addButton(btn, btnCount++);
    gbLo->addWidget(btn);
    btn->setChecked(checked);
    buttons.push_back(btn);
}

void CollapsibleSection::setParametricTitle(bool flag) {
    parametricTitleFlag = flag;
    if (flag) {
        /*! If title can contain selected control, controls must be exclusive */
        /*! \todo FCON c'era un motivo per il quale si può solo settare true il controllo esclusivo ma non tornare indietro, ma non me lo ricordo */
        this->setExclusiveControls(flag);
    }
}

void CollapsibleSection::setExclusiveControls(bool flag) {
    buttonGroup->setExclusive(flag);
    if (!flag) {
        /*! Title cannot contain selected control if controls are not exclusive */
        this->setParametricTitle(flag);
    }
}

void CollapsibleSection::onClickButton(int idx) {
    if (idx < btnCount) {
        buttonGroup->button(idx)->click();
        if (buttonGroup->button(idx)->isChecked()) {
            currentSelectedButton = idx;
        }
    }
}

void CollapsibleSection::onCheckButton(int idx, bool flag) {
    if (idx < btnCount) {
        if (flag) {
            currentSelectedButton = idx;
        }
        buttonGroup->button(idx)->setChecked(flag);
    }
}

void CollapsibleSection::onSetEnabled(bool flag) {
    if (!singleOption) {
        this->setEnabled(flag);
    }
}

void CollapsibleSection::addClosingSpacer() {
    QSpacerItem * w;
    if (orientation == Qt::Vertical) {
        w = new QSpacerItem(1, 1, QSizePolicy::Fixed, QSizePolicy::Expanding);

    } else {
        w = new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Fixed);
    }
    gbLo->addSpacerItem(w);
}

void CollapsibleSection::setAvailable(bool flag, QString toolTip) {
    if (flag) {
        this->setStyleSheet(CLS_AVAILABLE_STYLESHEET);
        this->setToolTip("");

    } else {
        this->setStyleSheet(CLS_UNAVAILABLE_STYLESHEET);
        this->setToolTip(toolTip);
    }
    this->setVisible(flag);
}

int CollapsibleSection::getCurrentSelectedButton() {
    return currentSelectedButton;
}

std::vector <QAbstractButton *> CollapsibleSection::getButtons() {
    return buttons;
}

void CollapsibleSection::onHideShowBtnClick(bool show) {
    groupBox->setVisible(show);
    if (show) {
        hideShowBtn->setArrowType(Qt::ArrowType::DownArrow);

    } else {
        hideShowBtn->setArrowType(Qt::ArrowType::RightArrow);
    }
}

void CollapsibleSection::onButtonToggled(QAbstractButton * btn, bool checked) {
    int idx = buttonGroup->id(btn);
    if (checked) {
        currentSelectedButton = idx;
    }
    emit buttonToggled(idx, checked);
    if (checked && parametricTitleFlag) {
        titleEdit->setText(title + ": " + btn->text());
    }
}
