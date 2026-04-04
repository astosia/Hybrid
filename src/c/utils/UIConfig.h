#pragma once
#include <pebble.h>

// Per-platform rendering config
typedef struct {
  int BTxOffset;          // BT/QT icon offset from left of foreground circle
  int QTxOffset;
  int BTQTRectWidth;        // Width of BT & QT icon GRects
  int BTIconYOffset;        // BT icon offset above centre
  int QTIconYOffset;        // QT icon offset below centre
  int xOffset;              // Text offset (date, logo, battery)
  int yOffset;              // Text offset (above/below lines)
  int yOffsetBattery;
  int yOffsetDate;
  int yOffsetPercent;       // Offset below lines for percent and logo text
  int second_hand_a;        // Second hand tail: starts at w/2 - second_hand_a
  int second_hand_b;        // Second hand tip: ends at w/2 - second_hand_b
  int hour_hand_a;          // Hour hand tip length from centre
  int min_hand_a;           // Minute hand tip length from centre
  int min_hand_b;           // Minute hand tail length from centre
  int fg_radius;            // Radius of foreground circle (digital display area)
  int tick_inset_inner;     // Major tick inner end inset from screen edge (round only)
  int tick_inset_outer;     // Major tick outer end inset from screen edge
  int minor_tick_inset;     // Minor tick inset from screen edge (round only)
  int time_font_size;
  int info_font_size;
  int other_text_font_size;
  int Line45yOffset;
  int Line3yOffset;
  int Line6yOffset;
  int Line7yOffset;
  //int corner_radius_handsrect; // Corner radius for the hands/ticks reference rectangle
  int corner_radius_majortickrect;
  int corner_radius_minortickrect;
  int majortickrect_w;
  int majortickrect_h;
  int minortickrect_w;
  int minortickrect_h;
  int corner_radius_foreground;
  int foregroundrect_x;
  int foregroundrect_y;
  int foregroundrect_w;
  int foregroundrect_h;
  int corner_radius_secondshand;
  //int hourhandrect_w;
  //int hourhandrect_h;
  int hourhandwidth;
  int hour_hand_a_round;
  int min_hand_a_round;
  int min_hand_b_round;
  int second_hand_a_round;
  int second_hand_b_round;
  int tick_inset_inner_round;
  int tick_inset_outer_round;
  int minor_tick_inset_round;
  //GRect RainRect[1];
  GRect SunsetRect[1];
  GRect SunriseRect[1];
  GRect MoonRect[1];
  //GRect StepsRect[1];
  GRect SunsetIconRect[1];
  GRect SunriseIconRect[1];
  // GRect IconNowRect[1];
  // GRect IconForeRect[1];
  // GRect WindKtsRect[1];
  // GRect WindForeKtsRect[1];
  // GRect TempRect[1];
  //GRect PrecipRect[1];
  //GRect TempForeRect[1];
  // GRect WindDirNowRect[1];
  // GRect WindDirForeRect[1];
  //GRect RainRateRect[1];
  GRect UVDayValueRect[1];
  GRect uv_arc_bounds[1];
  GRect uv_arc_bounds_max[1];
  GRect uv_arc_bounds_now[1];
  GRect uv_icon[1];
  GRect RainDayValueRect[1];
  GRect Rain1hValueRect[1];
  GRect Rain_arc_bounds[1];
  GRect Rain_arc_bounds_max[1];
  GRect Rain_arc_bounds_now[1];
  GRect Rain_icon[1];
  GRect TopTopLeftRect[1];
  GRect TopTopRightRect[1];
  GRect TopTopLeftRect2[1];
  GRect TopTopRightRect2[1];
  GRect TopLeftRect[1];
  GRect TopMiddleRect[1];
  GRect TopRightRect[1];
  GRect MiddleLeftRect[1];
  GRect MiddleRightRect[1];
  GRect MiddleFullRect[1];
  GRect BottomLeftRect1[1];
  GRect BottomLeftRect[1];
  GRect BottomRightRect1[1];
  GRect BottomRightRect[1];
  GRect BottomBottomLeftRect1[1];
  GRect BottomBottomRightRect[1];
} UIConfig;

extern const UIConfig config;