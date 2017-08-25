#ifndef MAPPING_CONSTANTS_H
#define MAPPING_CONSTANTS_H
#include "graphics.h"

// Colors
const t_color HIGHWAY(242, 117, 120);
const t_color MOTORWAY(199, 153, 232);
const t_color BLENDEDROAD(255, 206, 147);
const t_color GRASS(202, 223, 170);
const t_color SANDS(236, 194, 149);
const t_color HIGHLIGHTED(44, 161, 248);
const t_color PATH(119,221,119);
const t_color PATH_INTR_START(115,163,255);
const t_color PATH_INTR_END(150,111,214);
const t_color PROMPT(74,84,89);

// Zoom Levels (km)
const float CITY            = 35.0;
const float AREA            = 9.0;
const float STREET          = 5.0;
const float LOCAL           = 3.0;
const float HOUSE           = 0.75;
const float TOP_POI_SYMBOL  = 0.75;
const float MED_POI_SYMBOL  = 0.5;
const float LOW_POI_SYMBOL  = 0.25;
const float TOP_POI_TEXT    = 0.5;
const float MED_POI_TEXT    = 0.25;
const float LOW_POI_TEXT    = 0.25;

// Street Widths (line width)
const float STREETWIDTH_CORE = 3;
const float STREETWIDTH_HIGHLIGHTED = 4;
const float STREETWIDTH_BLENDED = 1;

// Symbol width scale factor
const float SYMBOL_SCALE = 7.5;

// Mouse Clicks
// copied from t_button_pressed struct from easygl_constants.h
const unsigned int  CLICK_LEFT      = 1;
const unsigned int  CLICK_WHEEL     = 2;
const unsigned int  CLICK_RIGHT     = 3;
const unsigned int  CLICK_WHEEL_FWD = 4;
const unsigned int  CLICK_WHEEL_BWD = 5;

// Click Threshold (m)
const float CLICK_THRESHOLD = 40;

#endif /* MAPPING_CONSTANTS_H */

