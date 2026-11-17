#include "scrollshadow.h"

#include <QAbstractScrollArea>
#include <QEvent>
#include <QScrollBar>

namespace {

// Height of each shadow, in logical pixels
constexpr int shadowHeight = 8;

// Owned by the scroll area: it lives as long as the area
class EdgeShadows : public QObject {
public:
    explicit EdgeShadows(QAbstractScrollArea *scrollArea) :
        QObject(scrollArea),
        area(scrollArea),
        top(createShadow("scrollShadowTop")),
        bottom(createShadow("scrollShadowBottom")) {

        const QScrollBar *bar = area->verticalScrollBar();
        connect(bar, &QScrollBar::valueChanged, this, &EdgeShadows::updateVisibility);
        connect(bar, &QScrollBar::rangeChanged, this, &EdgeShadows::updateVisibility);

        // The viewport changes size with the dock and when the scroll bar appears
        area->viewport()->installEventFilter(this);

        updateGeometry();
        updateVisibility();
    }

protected:
    bool eventFilter(QObject *watched, QEvent *event) override {
        if (event->type() == QEvent::Resize || event->type() == QEvent::Move) {
            updateGeometry();
        }
        return QObject::eventFilter(watched, event);
    }

private:
    QWidget *createShadow(const char *name) {
        QWidget *shadow = new QWidget(area);
        shadow->setObjectName(name);
        shadow->setAttribute(Qt::WA_StyledBackground);
        shadow->setAttribute(Qt::WA_TransparentForMouseEvents);
        shadow->hide();
        return shadow;
    }

    // Over the viewport edges, 1 px in from the sides so that the side borders
    // of the content stay clean
    void updateGeometry() {
        const QRect viewport = area->viewport()->geometry();
        const int width = qMax(0, viewport.width() - 2);
        top->setGeometry(viewport.left() + 1, viewport.top(), width, shadowHeight);
        bottom->setGeometry(viewport.left() + 1, viewport.bottom() - shadowHeight + 1, width, shadowHeight);
        top->raise();
        bottom->raise();
    }

    // setVisible() does nothing when the state does not change
    void updateVisibility() {
        const QScrollBar *bar = area->verticalScrollBar();
        top->setVisible(bar->value() > bar->minimum());
        bottom->setVisible(bar->value() < bar->maximum());
    }

    QAbstractScrollArea *area;
    QWidget *top;
    QWidget *bottom;
};

} // namespace

void ScrollShadow::install(QAbstractScrollArea *area) {
    if (area != nullptr) {
        new EdgeShadows(area);
    }
}
