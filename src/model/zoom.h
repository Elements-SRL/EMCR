#ifndef ZOOM_H
#define ZOOM_H

#include "rect4.h"
#include <deque>
#include <QObject>

class Zoom: public QObject {
    Q_OBJECT

private:
    std::deque<Rect4> zoomStack;
public:
    explicit Zoom(Rect4 = Rect4());
    Rect4 reset();
    Rect4 push(Rect4);
    Rect4 pop();
    Rect4 peek();
signals:
    void sigZoomChanged();
};

#endif // ZOOM_H
