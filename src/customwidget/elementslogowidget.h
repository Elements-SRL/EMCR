#ifndef ELEMENTSLOGOWIDGET_H
#define ELEMENTSLOGOWIDGET_H

#include <QWidget>
#include <QLabel>

class ElementsLogoWidget : public QWidget {
    Q_OBJECT

public:
    explicit ElementsLogoWidget(QWidget * parent = nullptr);
    ~ElementsLogoWidget();

protected:
    /*! QWidget interface */
    virtual void resizeEvent(QResizeEvent *) override;

private:
    QPixmap pixmap;
    QLabel * pixmapLbl;
};

#endif // ELEMENTSLOGOWIDGET_H
