#include "zoom.h"

Zoom::Zoom(Rect4 r) {
    zoomStack.push_back(r);
}

Rect4 Zoom::reset() {
    const auto r = zoomStack.front();
    zoomStack.clear();
    push(r);
    return r;
}

Rect4 Zoom::push(Rect4 r) {
    zoomStack.push_back(r);
    emit sigZoomChanged();
    return r;
}

Rect4 Zoom::pop() {
    const auto r = zoomStack.back();
    if (zoomStack.size() > 1) {
        zoomStack.pop_back();
    }
    emit sigZoomChanged();
    return r;
}

Rect4 Zoom::peek(){
    return zoomStack.back();
}
