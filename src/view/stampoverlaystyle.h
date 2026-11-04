#ifndef STAMPOVERLAYSTYLE_H
#define STAMPOVERLAYSTYLE_H

#include "stampplot.h"

/*! Look of the chessboard mini-plots (StampPlot).
 *
 *  Plot info like - channel name, color and badges - are 
 *  painted with QPainter for drawing performance.
 *  Tune the params below to change their look:
 *  - sizes and fonts are in logical pixels (as in the QSS);
 *  - colors are the ones specified in the controller
 *    (themecontroller.cpp), so theme aware (dark/light)
 * 
 *  To add a badge: add row in badges[] plus two vars
 *  @BDG_<key>_BG_CH / @BDG_<key>_FG_CH.
 */
struct StampOverlayStyle {
    /*! Canvas inset with respect to the card, so that the
     *  rectangular canvas does not cover the rounded corners */
    static constexpr int canvasInset = 3;

    /*! Distance of overlay with respect to canvas */
    static constexpr int margin = 0;

    /*! Min/max size of the boxes: status badges and name box (height) */
    static constexpr int minBoxSize = 10;
    static constexpr int maxBoxSize = 15;

    /*! Upper left - Channel name. The box is as wide as the text,
     *  so the channel number is always readable */
    static constexpr int nameFontPx = 12;
    static constexpr int nameHPadding = 1;
    static constexpr qreal nameRadius = 2.0;
    static constexpr qreal nameBorderWidth = 1.0;
    static constexpr const char * nameBackground = "@TEXT_LIGHT";
    static constexpr const char * nameText = "@BG_APP";
    static constexpr const char * nameBorder = "@BORDER_PANEL";

    /*! Upper right - Channel color */
    static constexpr int colorSize = 10;
    static constexpr qreal colorRadius = 4.0;
    static constexpr qreal colorBorderWidth = 0.5;
    static constexpr const char * colorBorder = "@BORDER_PANEL";

    /*! Lower left: status badges*/
    static constexpr int badgeFontPx = 10;
    static constexpr int badgeHPadding = 2;
    static constexpr int badgeSpacing = 4;
    static constexpr qreal badgeRadius = 2.0;

    /*! Minimum card size (StampPlot::minimumSizeHint): fits the
     *  worst case without overlaps, i.e. a 3 digit channel name with the
     *  color next to it and, below, minVisibleBadges badges */
    static constexpr const char * widestName = "888";
    static constexpr int minVisibleBadges = 3;

    struct Badge {
        StampPlot::States_t state;
        char letter;
        const char * paletteKey; /*!< colors: @BDG_<paletteKey>_BG_CH and @BDG_<paletteKey>_FG_CH */
    };

    static constexpr Badge badges[] = {
        {StampPlot::StateSwitchedOff,                'O', "O"},
        {StampPlot::StateCalibrationResistorsOn,     'R', "R"},
        {StampPlot::StateStimuliDisabled,            'X', "X"},
        {StampPlot::StateOffsetRecalibrationOn,      'C', "C"},
        {StampPlot::StateLiquidJunctionCompensation, 'J', "C"}, /*!< same colors as C */
        {StampPlot::StateTraceExpanded,              'E', "E"},
        {StampPlot::StatePlotDetailOn,               'P', "P"},
    };
};

#endif // STAMPOVERLAYSTYLE_H
