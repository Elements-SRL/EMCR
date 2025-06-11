#ifndef ZOOM_H
#define ZOOM_H

#include "rect4.h"
#include <deque>

class Zoom {

private:
    std::deque<Rect4> zoomStack;
public:
    explicit Zoom(Rect4 = Rect4());
    Rect4 reset();
    Rect4 push(Rect4);
    Rect4 pop();
    Rect4 peek();
};

#endif // ZOOM_H
