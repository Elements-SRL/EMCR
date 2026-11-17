#ifndef SCROLLSHADOW_H
#define SCROLLSHADOW_H

class QAbstractScrollArea;

/*! Shadows at the top and bottom edges of a scroll area, telling the user that
 *  there is more content in that direction:
 *  - scrolled to the top    -> bottom shadow only;
 *  - in the middle          -> both;
 *  - scrolled to the bottom -> top shadow only;
 *  - nothing to scroll      -> none.
 *
 *  The shadows are two thin widgets laid over the viewport (they let the mouse
 *  through) and drawn by the QSS (#scrollShadowTop, #scrollShadowBottom), so
 *  they follow the theme. They are shown/hidden only when the scroll state
 *  changes: no cost while scrolling.
 */
namespace ScrollShadow {
void install(QAbstractScrollArea *area);
}

#endif // SCROLLSHADOW_H
