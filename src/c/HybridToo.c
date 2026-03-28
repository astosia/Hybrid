#include <pebble.h>
#include "HybridToo.h"
#include "utils/weekday.h"
#include "utils/month.h"
#include "utils/MathUtils.h"
#ifndef PBL_BW
#include <pebble-fctx/fctx.h>
#include <pebble-fctx/fpath.h>
#include <pebble-fctx/ffont.h>
#endif

#define SECONDS_TICK_INTERVAL_MS 1000
//#define CORNER_RADIUS 24

// Layers
static Window *s_window;
static Layer *s_canvas_layer;
static Layer *s_bg_layer;
static Layer *s_canvas_second_hand;
static Layer *s_fg_layer;

 static GRect bounds;
// static GRect bounds_seconds;

// Fonts
static GFont FontBTQTIcons;
//#ifdef PBL_PLATFORM_APLITE
#ifdef PBL_BW
static GFont aplite_font;
static GFont small_font;
static GFont medium_font;
#else
static FFont *FCTX_Font;
#endif

// Time and date
static struct tm prv_tick_time;
static int s_battery_level;
static int s_connected;
static char s_time_text[12], s_ampm_text[6], s_date_text[20], s_batt_text[8], s_logo_text[16];
static int s_month;
static int s_day;
static int s_weekday;
static int s_hours;
static int minutes;
static int hours;
static int seconds;
static ClaySettings settings;
static bool showSeconds;
static AppTimer *s_timeout_timer;
// static bool ignore_next_tap = false;

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
  int corner_radius_handsrect; // Corner radius for the hands/ticks reference rectangle
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
  int hourhandrect_w;
  int hourhandrect_h;
  int hourhandwidth;
  int hour_hand_a_round;
  int min_hand_a_round;
  int min_hand_b_round;
  int second_hand_a_round;
  int second_hand_b_round;
  int tick_inset_inner_round;
  int tick_inset_outer_round;
  int minor_tick_inset_round;
} UIConfig;

#ifdef PBL_PLATFORM_EMERY
static const UIConfig config = {
  .BTxOffset = 8 + 25,
  .QTxOffset = 8 + 93,
  .BTQTRectWidth = 30 ,
  .BTIconYOffset = -15 -49 + 3,
  .QTIconYOffset = 5 - 69 + 3,
  .xOffset = 4,
  .yOffset = 4,
  .yOffsetBattery = 10,
  .yOffsetDate = 2,
  .yOffsetPercent = 11,
  .second_hand_a = 94,  //not really used
  .second_hand_b = 6, //offset of second hand end
  .hour_hand_a = 22, //was 28
  .min_hand_a = 9,
  .min_hand_b = 28, //used
  .fg_radius = 74,
  .tick_inset_outer = -4, // USED
  .time_font_size = 54,
  .info_font_size = 30,
  .other_text_font_size = 20,
  .Line45yOffset = 2,
  .Line3yOffset = 6,
  .corner_radius_handsrect = 20,
  .corner_radius_majortickrect = 20,
  .corner_radius_minortickrect = 28,
  .majortickrect_w = 86,
  .majortickrect_h = 100,
  .minortickrect_w = 90,
  .minortickrect_h = 104,
  .corner_radius_foreground = 16,
  .foregroundrect_x = 34+2,
  .foregroundrect_y = 34+2,
  .foregroundrect_w = 200-34-34-4,
  .foregroundrect_h = 228-34-34-4,
  .corner_radius_secondshand = 24,
  .hourhandrect_w = 72,
  .hourhandrect_h = 86,
  .hourhandwidth = 9,
  .hour_hand_a_round = 12,
  .min_hand_a_round = 2,
  .min_hand_b_round = 17,
  .second_hand_a_round = 0,
  .second_hand_b_round = 28,
  .tick_inset_inner_round = 18,
  .tick_inset_outer_round = 2,
  .minor_tick_inset_round = 15
};
#elif defined(PBL_PLATFORM_GABBRO)
static const UIConfig config = {
  .BTxOffset = 10 + 41,
  .QTxOffset = 10 + 106,
  .BTQTRectWidth = 30,
  .BTIconYOffset = -14 - 59,
  .QTIconYOffset = 4 - 77,
  .xOffset = 4,
  .yOffset = 5,
  .yOffsetBattery = 12,
  .yOffsetDate = 3,
  .yOffsetPercent = 13,
  .second_hand_a = 6,
  .second_hand_b = 40,
  .hour_hand_a = 106,
  .min_hand_a = 122,
  .min_hand_b = 98,
  .fg_radius = 90,
  .tick_inset_inner = 15,
  .tick_inset_outer = 6,
  .minor_tick_inset = 7,
  .time_font_size = 66,
  .info_font_size = 36,
  .other_text_font_size = 24,
  .Line45yOffset = 2,
  .Line3yOffset = 6,
  .hourhandwidth = 9
};

#elif defined(PBL_ROUND)
static const UIConfig config = {
  .BTxOffset = 5 + 26 + 3,
  .QTxOffset = 5 + 75 + 1,
  .BTQTRectWidth = 30,
  .BTIconYOffset = -14 - 38 + 4,
  .QTIconYOffset = 4 - 56 + 4,
  .xOffset = 4,
  .yOffset = 3,
  .yOffsetBattery = 9,
  .yOffsetDate = 1,
  .yOffsetPercent = 10,
  .second_hand_a = 3,
  .second_hand_b = 28,
  .hour_hand_a = 72,
  .min_hand_a = 85,
  .min_hand_b = 69,
  .fg_radius = 62,
  .tick_inset_inner = 10,
  .tick_inset_outer = 3,
  .minor_tick_inset = 4,
  .time_font_size = 46,
  .info_font_size = 24,
  .other_text_font_size = 16,
  .Line45yOffset = 0,
  .Line3yOffset = 4,
  .hourhandwidth = 7
};

#else // Basalt, Aplite, Diorite, Flint
static const UIConfig config = {
  .BTxOffset = 5 + 16 + 2,
  .QTxOffset = 5 + 66 + 2,
  .BTQTRectWidth = 30,
  .BTIconYOffset = -14 - 36 + 3 + 4,
  .QTIconYOffset = 4 - 54 + 3 + 4,
  .xOffset = 4,
  .yOffset = 2,
  .yOffsetBattery = 8,
  .yOffsetDate = 1,
  .yOffsetPercent = 9,
  .second_hand_a = 0,
  .second_hand_b = 6,
  .hour_hand_a = 16,
  .min_hand_a = 4,  //was6
  .min_hand_b = 18,  //was 18
  .fg_radius = 54,
  .tick_inset_outer = -4,
  .time_font_size = 39, //38
  .info_font_size = 20,
  .other_text_font_size = 14,
  .Line45yOffset = 1,
  .Line3yOffset = 5,
  .corner_radius_handsrect = 15,
  .corner_radius_majortickrect = 15,
  .corner_radius_minortickrect = 21,
  .majortickrect_w = 62,
  .majortickrect_h = 72,
  .minortickrect_w = 66,
  .minortickrect_h = 76,
  .corner_radius_foreground = 12,
  .foregroundrect_x = 24+2,
  .foregroundrect_y = 24+2,
  .foregroundrect_w = 144-48-4,
  .foregroundrect_h = 168-48-4,
  .corner_radius_secondshand = 18,
  .hourhandrect_w = 52,  //72
  .hourhandrect_h = 64,   //86
  .hourhandwidth = 7,
  .hour_hand_a_round = 8,
  .min_hand_a_round = 2,
  .min_hand_b_round = 12,
  .second_hand_a_round = 0,
  .second_hand_b_round = 20,
  .tick_inset_inner_round = 14,
  .tick_inset_outer_round = 2,
  .minor_tick_inset_round = 11
};
#endif



// ---------------------------------------------------------------------------
// Geometry calcs for rectangular screens
// ---------------------------------------------------------------------------
#ifndef PBL_ROUND
static GPoint angle_to_rect_edge(GPoint center, int angle_deg, GRect r) {
  int32_t angle = DEG_TO_TRIGANGLE(angle_deg);
  int32_t dx = cos_lookup(angle);
  int32_t dy = sin_lookup(angle);
  int32_t t = INT32_MAX;
  if (dx > 0) { int32_t v = ((r.origin.x + r.size.w - 1 - center.x) * TRIG_MAX_RATIO) / dx; if (v < t) t = v; }
  else if (dx < 0) { int32_t v = ((r.origin.x - center.x) * TRIG_MAX_RATIO) / dx; if (v < t) t = v; }
  if (dy > 0) { int32_t v = ((r.origin.y + r.size.h - 1 - center.y) * TRIG_MAX_RATIO) / dy; if (v < t) t = v; }
  else if (dy < 0) { int32_t v = ((r.origin.y - center.y) * TRIG_MAX_RATIO) / dy; if (v < t) t = v; }
  return GPoint(center.x + (dx * t / TRIG_MAX_RATIO),
                center.y + (dy * t / TRIG_MAX_RATIO));
}

static GPoint angle_to_rounded_rect_edge(GPoint center, int angle_deg, int half_w, int half_h, int r) {
  int32_t angle = DEG_TO_TRIGANGLE(angle_deg);
  int32_t dx = cos_lookup(angle);
  int32_t dy = sin_lookup(angle);

  int32_t dx16 = (dx + 32) >> 6;
  int32_t dy16 = (dy + 32) >> 6;
  int32_t ratio = TRIG_MAX_RATIO >> 6;

  int32_t t = INT32_MAX;
  if (dx16 > 0) { int32_t v = ( half_w - 1) * ratio / dx16; if (v < t) t = v; }
  else if (dx16 < 0) { int32_t v = (-half_w    ) * ratio / dx16; if (v < t) t = v; }
  if (dy16 > 0) { int32_t v = ( half_h - 1) * ratio / dy16; if (v < t) t = v; }
  else if (dy16 < 0) { int32_t v = (-half_h    ) * ratio / dy16; if (v < t) t = v; }

  // px = dx16 * t / ratio, with rounding
  int32_t px = (dx16 * t + (ratio / 2)) / ratio;
  int32_t py = (dy16 * t + (ratio / 2)) / ratio;

  // Corner projection
  int32_t inner_w = half_w - r;
  int32_t inner_h = half_h - r;
  if ((px > inner_w || px < -inner_w) && (py > inner_h || py < -inner_h)) {
    int32_t cx = (px > 0) ? inner_w : -inner_w;
    int32_t cy = (py > 0) ? inner_h : -inner_h;
    int32_t ex = px - cx;
    int32_t ey = py - cy;

    // Scale ex/ey up by 16 for sub-pixel precision in isqrt, stays in 32-bit.
    int32_t ex16s = ex << 4;
    int32_t ey16s = ey << 4;
    uint32_t len16 = isqrt((uint32_t)(ex16s * ex16s + ey16s * ey16s));

    if (len16 > 0) {
      // ex * r * 16 / len16 = ex * r / len, with rounding
      px = cx + (ex * r * 16 + (int32_t)(len16 / 2)) / (int32_t)len16;
      py = cy + (ey * r * 16 + (int32_t)(len16 / 2)) / (int32_t)len16;
    }
  }

  return GPoint(center.x + px, center.y + py);
}

static GPoint point_from_edge(GPoint origin, int angle_deg, GRect r, int inset) {
    GPoint edge = angle_to_rect_edge(origin, angle_deg, r);
    int32_t angle = DEG_TO_TRIGANGLE(angle_deg);
    return GPoint(edge.x - (int)((cos_lookup(angle) * inset) / TRIG_MAX_RATIO), 
                  edge.y - (int)((sin_lookup(angle) * inset) / TRIG_MAX_RATIO));
}


#endif

// ---------------------------------------------------------------------------
// Efficiency: Subscribe & Cache 
// ---------------------------------------------------------------------------
static void update_cached_strings() {

    minutes   = prv_tick_time.tm_min;
    hours     = prv_tick_time.tm_hour % 12;
    s_hours   = prv_tick_time.tm_hour;
    s_day     = prv_tick_time.tm_mday;
    s_weekday = prv_tick_time.tm_wday;
    s_month   = prv_tick_time.tm_mon;

    // 1. Time string
    int hourdraw = prv_tick_time.tm_hour;
    if (!clock_is_24h_style()) {
        hourdraw = (hourdraw == 0 || hourdraw == 12) ? 12 : hourdraw % 12;
        snprintf(s_time_text, sizeof(s_time_text), settings.AddZero12h ? "%02d:%02d" : "%d:%02d", hourdraw, prv_tick_time.tm_min);
        strftime(s_ampm_text, sizeof(s_ampm_text), "%p", &prv_tick_time);
    } else {
        snprintf(s_time_text, sizeof(s_time_text), settings.RemoveZero24h ? "%d:%02d" : "%02d:%02d", hourdraw, prv_tick_time.tm_min);
        s_ampm_text[0] = '\0';
    }
    // 2. Date string
    if (settings.EnableDate) {
        char weekdaydraw[10];
        fetchwday(prv_tick_time.tm_wday, i18n_get_system_locale(), weekdaydraw);
        snprintf(s_date_text, sizeof(s_date_text), "%s %d", weekdaydraw, prv_tick_time.tm_mday);
    }
    // 3. Logo text
    snprintf(s_logo_text, sizeof(s_logo_text), "%s", settings.LogoText);
}



static void bluetooth_callback(bool connected) {
    bool was_connected = s_connected;
    s_connected = connected;
    if (was_connected && !connected && (!quiet_time_is_active() || settings.VibeOn)) {
        vibes_double_pulse();
        //ignore_next_tap = true;
    }
    layer_mark_dirty(s_fg_layer);
}

static void battery_callback(BatteryChargeState state) {
    s_battery_level = state.charge_percent;
    snprintf(s_batt_text, sizeof(s_batt_text), "%d%%", s_battery_level);
    layer_mark_dirty(s_fg_layer);
}

// ---------------------------------------------------------------------------
// Hand drawing
// ---------------------------------------------------------------------------

static void draw_fancy_hand_hour(GContext *ctx, int angle, int length, GColor fill_color, GColor border_color) {
  GPoint origin = GPoint(bounds.size.w / 2, bounds.size.h / 2);
  //GPoint p1;
  GPoint p2;

  #ifdef PBL_ROUND
    p2 = polar_to_point_offset(origin, angle, length);
  #else
    if(settings.ForegroundShape){
      p2 = polar_to_point_offset(origin, angle, length);
    }
    else{
      GRect r = GRect(0, 0, bounds.size.w, bounds.size.h);
      p2 = point_from_edge(origin, angle, r, length);
    }
  #endif

  graphics_context_set_antialiased(ctx, true);

  #if PBL_COLOR && PBL_ROUND
  graphics_context_set_stroke_color(ctx, settings.ShadowColor);
  graphics_context_set_stroke_width(ctx, 9);
  graphics_draw_line(ctx, GPoint(origin.x + 2, origin.y + 2), GPoint(p2.x + 2, p2.y + 2));
  #elif PBL_COLOR
      if(settings.ForegroundShape){
        graphics_context_set_stroke_color(ctx, settings.ShadowColor);
        graphics_context_set_stroke_width(ctx, 9);
        graphics_draw_line(ctx, GPoint(origin.x + 2, origin.y + 2), GPoint(p2.x + 2, p2.y + 2));
      }
  #endif

  graphics_context_set_stroke_color(ctx, border_color);
  graphics_context_set_stroke_width(ctx, config.hourhandwidth);
  graphics_draw_line(ctx, origin, p2);
  graphics_context_set_stroke_color(ctx, fill_color);
  graphics_context_set_stroke_width(ctx, config.hourhandwidth - 6);
  graphics_draw_line(ctx, origin, p2);
}

static void draw_fancy_hand_min(GContext *ctx, int angle, int length, int back_length, GColor fill_color, GColor border_color) {
  GPoint origin = GPoint(bounds.size.w / 2, bounds.size.h / 2);
  GPoint p1;
  GPoint p2;

  #ifdef PBL_ROUND
   p1 = polar_to_point_offset(origin, angle, back_length);
   p2 = polar_to_point_offset(origin, angle, length);
  #else
    if(settings.ForegroundShape){
      p1 = polar_to_point_offset(origin, angle, back_length);
      p2 = polar_to_point_offset(origin, angle, length);
    }
    else{
      GRect r = GRect(0, 0, bounds.size.w, bounds.size.h);
      p1 = angle_to_rounded_rect_edge(origin, angle, bounds.size.w/2 - config.min_hand_b, bounds.size.h/2 - config.min_hand_b, config.corner_radius_foreground);
      p2 = point_from_edge(origin, angle, r, length);
    }
  #endif

  graphics_context_set_antialiased(ctx, true);

  #if PBL_COLOR && PBL_ROUND
  GPoint shadow_offset = PBL_IF_BW_ELSE(GPoint(1, 1), GPoint(2, 2));
  graphics_context_set_stroke_color(ctx, settings.ShadowColor);
  graphics_context_set_stroke_width(ctx, config.hourhandwidth-2);
  graphics_draw_line(ctx, GPoint(p1.x + shadow_offset.x, p1.y + shadow_offset.y),
                          GPoint(p2.x + shadow_offset.x, p2.y + shadow_offset.y));
  #elif PBL_COLOR
      if(settings.ForegroundShape){
        GPoint shadow_offset = PBL_IF_BW_ELSE(GPoint(1, 1), GPoint(2, 2));
        graphics_context_set_stroke_color(ctx, settings.ShadowColor);
        graphics_context_set_stroke_width(ctx, config.hourhandwidth-2);
        graphics_draw_line(ctx, GPoint(p1.x + shadow_offset.x, p1.y + shadow_offset.y),
                                GPoint(p2.x + shadow_offset.x, p2.y + shadow_offset.y));
      }
  #endif

  graphics_context_set_stroke_color(ctx, border_color);
  graphics_context_set_stroke_width(ctx, config.hourhandwidth-2);
  graphics_draw_line(ctx, p1, p2);
  graphics_context_set_stroke_color(ctx, fill_color);
  graphics_context_set_stroke_width(ctx, config.hourhandwidth-6);
  graphics_draw_line(ctx, p1, p2);
}

static void draw_seconds_line_hand(GContext *ctx, int angle, int length, int back_length, GColor color) {
  GPoint origin = GPoint(bounds.size.w / 2, bounds.size.h / 2);
  GPoint p1;
  GPoint p2;

  #ifdef PBL_ROUND
   p1 = polar_to_point_offset(origin, angle, back_length);
   p2 = polar_to_point_offset(origin, angle, length);
  #else
    if(settings.ForegroundShape){
      p1 = polar_to_point_offset(origin, angle, back_length);
      p2 = polar_to_point_offset(origin, angle, length);
    }
    else{
      GRect r = GRect(0, 0, bounds.size.w, bounds.size.h);
      p1 = angle_to_rounded_rect_edge(origin, angle, bounds.size.w/2 - config.second_hand_b, bounds.size.h/2 - config.second_hand_b, config.corner_radius_secondshand);
      p2 = point_from_edge(origin, angle, r, back_length);
    }
  #endif

  graphics_context_set_antialiased(ctx, true);

  #if PBL_COLOR && PBL_ROUND
  graphics_context_set_stroke_color(ctx, settings.ShadowColor);
  graphics_context_set_stroke_width(ctx, 2);
  graphics_draw_line(ctx, GPoint(p1.x + 2, p1.y + 2), GPoint(p2.x + 2, p2.y + 2));
  #elif PBL_COLOR
      if(settings.ForegroundShape){
        graphics_context_set_stroke_color(ctx, settings.ShadowColor);
        graphics_context_set_stroke_width(ctx, 2);
        graphics_draw_line(ctx, GPoint(p1.x + 2, p1.y + 2), GPoint(p2.x + 2, p2.y + 2));
      }
  #endif

  graphics_context_set_stroke_color(ctx, color);
  graphics_context_set_stroke_width(ctx, 2);
  graphics_draw_line(ctx, p1, p2);
}

static void draw_btqt_icons(GContext *ctx, GRect bounds) {
    if (!s_connected) {
        int x = bounds.size.w / 2 - config.fg_radius + config.BTxOffset;
        int y = bounds.size.h / 2 + config.BTIconYOffset;
        graphics_context_set_text_color(ctx, settings.BTQTColor);
        graphics_draw_text(ctx, "z", FontBTQTIcons, GRect(x, y, config.BTQTRectWidth, 20), GTextOverflowModeFill, GTextAlignmentLeft, NULL);
    }
    if (quiet_time_is_active()) {
        int x = bounds.size.w / 2 - config.fg_radius + config.QTxOffset;
        int y = bounds.size.h / 2 + config.QTIconYOffset;
        graphics_context_set_text_color(ctx, settings.BTQTColor);
        graphics_draw_text(ctx, "y", FontBTQTIcons, GRect(x, y, config.BTQTRectWidth, 20), GTextOverflowModeFill, GTextAlignmentLeft, NULL);
    }
}

// ---------------------------------------------------------------------------
// Tick drawing
// ---------------------------------------------------------------------------

static void draw_major_tick(GContext *ctx, int angle, int length, GColor fill_color, GColor border_color) {
  GPoint origin = GPoint(bounds.size.w / 2, bounds.size.h / 2);
  GPoint p1;
  GPoint p2;

  #ifdef PBL_ROUND
   p1 = polar_to_point_offset(origin, angle, bounds.size.h / 2 - config.tick_inset_inner);
   p2 = polar_to_point_offset(origin, angle, bounds.size.h / 2 - config.tick_inset_outer);
  #else
    if(settings.ForegroundShape){
      p1 = polar_to_point_offset(origin, angle, bounds.size.h / 2 - config.tick_inset_inner_round);
      p2 = polar_to_point_offset(origin, angle, bounds.size.h / 2 - config.tick_inset_outer_round);
    }
    else{
      GRect r = GRect(0, 0, bounds.size.w, bounds.size.h);
      GPoint edge = angle_to_rect_edge(origin, angle, r);
      int32_t dx = cos_lookup(DEG_TO_TRIGANGLE(angle));
      int32_t dy = sin_lookup(DEG_TO_TRIGANGLE(angle));
      p2 = GPoint(edge.x - (int)((dx * config.tick_inset_outer) / TRIG_MAX_ANGLE),
                        edge.y - (int)((dy * config.tick_inset_outer) / TRIG_MAX_ANGLE));
      p1 = angle_to_rounded_rect_edge(origin, angle, config.majortickrect_w, config.majortickrect_h, config.corner_radius_majortickrect);
    }
  #endif

  graphics_context_set_antialiased(ctx, true);
  graphics_context_set_stroke_color(ctx, border_color);
  graphics_context_set_stroke_width(ctx, 3);
  graphics_draw_line(ctx, p1, p2);
}

#ifdef PBL_ROUND
static void draw_minor_tick(GContext *ctx, GPoint center, GColor border_color) {
  graphics_context_set_antialiased(ctx, true);
  graphics_context_set_fill_color(ctx, border_color);
  graphics_fill_circle(ctx, center, 1);
}
#else
static void draw_minor_tick(GContext *ctx, int angle, GColor fill_color, GColor border_color) {
  if (settings.ForegroundShape) {
    GPoint origin = GPoint(bounds.size.w / 2, bounds.size.h / 2);
    GPoint center = polar_to_point_offset(origin, angle, bounds.size.h / 2 - config.minor_tick_inset_round);
    graphics_context_set_antialiased(ctx, true);
    graphics_context_set_fill_color(ctx, border_color);
    graphics_fill_circle(ctx, center, 1);
  } else {
    GPoint origin = GPoint(bounds.size.w / 2, bounds.size.h / 2);
    GRect r = GRect(0, 0, bounds.size.w, bounds.size.h);
    GPoint edge = angle_to_rect_edge(origin, angle, r);
    int32_t dx = cos_lookup(DEG_TO_TRIGANGLE(angle));
    int32_t dy = sin_lookup(DEG_TO_TRIGANGLE(angle));
    GPoint p2 = GPoint(edge.x - (int)((dx * config.tick_inset_outer) / TRIG_MAX_ANGLE),
                       edge.y - (int)((dy * config.tick_inset_outer) / TRIG_MAX_ANGLE));
    GPoint p1 = angle_to_rounded_rect_edge(origin, angle, config.minortickrect_w, config.minortickrect_h, config.corner_radius_minortickrect);
    graphics_context_set_antialiased(ctx, true);
    graphics_context_set_stroke_color(ctx, border_color);
    graphics_context_set_stroke_width(ctx, 1);
    graphics_draw_line(ctx, p1, p2);
  }
}
#endif

// ---------------------------------------------------------------------------
// Centre drawing helpers (round platforms only)
// ---------------------------------------------------------------------------
 
#ifndef PBL_BW
static void draw_center_shadow(GContext *ctx, GColor shadow_color) {
  GPoint origin = GPoint(bounds.size.w / 2, bounds.size.h / 2);
  graphics_context_set_antialiased(ctx, true);
  graphics_context_set_fill_color(ctx, shadow_color);
  graphics_fill_circle(ctx, GPoint(origin.x + 2, origin.y + 2), config.fg_radius);
}
#endif

static void draw_center(GContext *ctx, GColor seconds_color, GColor fg_color) {
  GPoint origin = GPoint(bounds.size.w / 2, bounds.size.h / 2);
  graphics_context_set_antialiased(ctx, true);
  graphics_context_set_fill_color(ctx, fg_color);
  graphics_fill_circle(ctx, origin, config.fg_radius);
  graphics_context_set_stroke_color(ctx, seconds_color);
  graphics_context_set_stroke_width(ctx, 2);
  graphics_draw_circle(ctx, origin, config.fg_radius);
}

// ---------------------------------------------------------------------------
// Settings
// ---------------------------------------------------------------------------

static void prv_save_settings(void) {
  persist_write_data(SETTINGS_KEY, &settings, sizeof(settings));
}

static void prv_default_settings(void) {
  settings.EnableSecondsHand = true;
  settings.SecondsVisibleTime = 135;
  settings.EnableDate = true;
  settings.EnableBattery = true;
  settings.EnableBatteryLine = true;
  settings.EnableLogo = true;
  snprintf(settings.LogoText, sizeof(settings.LogoText), "%s", "HYBRID");
  settings.BackgroundColor1 = GColorWhite;
  #if PBL_COLOR
  settings.ShadowColor = GColorLightGray;
  settings.ShadowOn = true;
  snprintf(settings.ThemeSelect, sizeof(settings.ThemeSelect), "%s", "wh");
  #else
  snprintf(settings.BWThemeSelect, sizeof(settings.BWThemeSelect), "%s", "wh");
  #endif
  settings.TextColor1 = GColorWhite;
  settings.MajorTickColor = GColorBlack;
  settings.MinorTickColor = GColorDarkGray;
  settings.HoursHandColor = PBL_IF_BW_ELSE(GColorWhite, GColorWhite);
  settings.HoursHandBorderColor = PBL_IF_BW_ELSE(GColorBlack, GColorJaegerGreen);
  settings.MinutesHandColor = PBL_IF_BW_ELSE(GColorWhite, GColorWhite);
  settings.MinutesHandBorderColor = PBL_IF_BW_ELSE(GColorBlack, GColorRed);
  settings.SecondsHandColor = PBL_IF_BW_ELSE(GColorBlack, GColorJaegerGreen);
  settings.BTQTColor = GColorWhite;
  settings.VibeOn = false;
  settings.FGColor = GColorBlack;
  settings.RemoveZero24h = false;
  settings.AddZero12h = false;
  settings.LineColor = GColorLightGray;
  settings.showlocalAMPM = true;
  settings.ShowTime = true;
  settings.EnableLines = true;
  settings.EnableMinorTick = true;
  settings.EnableMajorTick = true;
  settings.ForegroundShape = false;
}

static void prv_load_settings(void) {
  prv_default_settings();
  persist_read_data(SETTINGS_KEY, &settings, sizeof(settings));
}

// ---------------------------------------------------------------------------
// Event handlers
// ---------------------------------------------------------------------------

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  prv_tick_time = *tick_time;

    update_cached_strings();
    layer_mark_dirty(s_canvas_layer);
    layer_mark_dirty(s_fg_layer);

  if (units_changed & MINUTE_UNIT) {
    minutes = tick_time->tm_min;
    hours = tick_time->tm_hour % 12;
    s_hours = tick_time->tm_hour;
    layer_mark_dirty(s_canvas_layer);
    layer_mark_dirty(s_fg_layer);
    if (settings.EnableDate && tick_time->tm_mday != s_day) {
      s_day = tick_time->tm_mday;
      s_weekday = tick_time->tm_wday;
      s_month = tick_time->tm_mon;
    }
  }

  if (showSeconds && (units_changed & SECOND_UNIT)) {
    layer_mark_dirty(s_canvas_second_hand);
  }

  layer_set_hidden(s_canvas_second_hand, !(showSeconds && settings.EnableSecondsHand));
}

static void timeout_handler(void *context) {
  showSeconds = false;
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  layer_mark_dirty(s_canvas_second_hand);
  s_timeout_timer = NULL;
}

static void accel_tap_handler(AccelAxisType axis, int32_t direction) {
 
  if (settings.EnableSecondsHand && settings.SecondsVisibleTime < 135) {
    if (s_timeout_timer) {
      app_timer_cancel(s_timeout_timer);
      s_timeout_timer = NULL;
    }
    if (!showSeconds) {
      tick_timer_service_subscribe(SECOND_UNIT, tick_handler);
    }
    showSeconds = true;
    s_timeout_timer = app_timer_register(SECONDS_TICK_INTERVAL_MS * settings.SecondsVisibleTime, timeout_handler, NULL);
    layer_mark_dirty(s_canvas_second_hand);
  }
}

// ---------------------------------------------------------------------------
// AppMessage inbox handler
// ---------------------------------------------------------------------------

static void prv_inbox_received_handler(DictionaryIterator *iter, void *context) {
#ifdef LOG
  APP_LOG(APP_LOG_LEVEL_INFO, "Received message");
#endif

  bool settings_changed = false;
  bool theme_settings_changed = false;

  Tuple *vibe_t               = dict_find(iter, MESSAGE_KEY_VibeOn);
  Tuple *enable_seconds_t     = dict_find(iter, MESSAGE_KEY_EnableSecondsHand);
  Tuple *enable_secondsvisible_t = dict_find(iter, MESSAGE_KEY_SecondsVisibleTime);
  Tuple *enable_date_t        = dict_find(iter, MESSAGE_KEY_EnableDate);
  Tuple *enable_battery_t     = dict_find(iter, MESSAGE_KEY_EnableBattery);
  Tuple *enable_logo_t        = dict_find(iter, MESSAGE_KEY_EnableLogo);
  Tuple *logotext_t           = dict_find(iter, MESSAGE_KEY_LogoText);
  Tuple *bwthemeselect_t      = dict_find(iter, MESSAGE_KEY_BWThemeSelect);
  Tuple *themeselect_t        = dict_find(iter, MESSAGE_KEY_ThemeSelect);
  Tuple *bg_color1_t          = dict_find(iter, MESSAGE_KEY_BackgroundColor1);
  Tuple *bg_color2_t          = dict_find(iter, MESSAGE_KEY_ShadowColor);
  Tuple *text_color1_t        = dict_find(iter, MESSAGE_KEY_TextColor1);
  Tuple *major_tick_color_t   = dict_find(iter, MESSAGE_KEY_MajorTickColor);
  Tuple *minor_tick_color_t   = dict_find(iter, MESSAGE_KEY_MinorTickColor);
  Tuple *hours_color_t        = dict_find(iter, MESSAGE_KEY_HoursHandColor);
  Tuple *hours_border_t       = dict_find(iter, MESSAGE_KEY_HoursHandBorderColor);
  Tuple *minutes_color_t      = dict_find(iter, MESSAGE_KEY_MinutesHandColor);
  Tuple *minutes_border_t     = dict_find(iter, MESSAGE_KEY_MinutesHandBorderColor);
  Tuple *seconds_color_t      = dict_find(iter, MESSAGE_KEY_SecondsHandColor);
  Tuple *btqt_color_t         = dict_find(iter, MESSAGE_KEY_BTQTColor);
  Tuple *shadowon_t           = dict_find(iter, MESSAGE_KEY_ShadowOn);
  Tuple *fg_color_t           = dict_find(iter, MESSAGE_KEY_FGColor);
  Tuple *addzero12_t          = dict_find(iter, MESSAGE_KEY_AddZero12h);
  Tuple *remzero24_t          = dict_find(iter, MESSAGE_KEY_RemoveZero24h);
  Tuple *line_color_t         = dict_find(iter, MESSAGE_KEY_LineColor);
  Tuple *localampm_t          = dict_find(iter, MESSAGE_KEY_showlocalAMPM);
  Tuple *enable_time_t        = dict_find(iter, MESSAGE_KEY_ShowTime);
  Tuple *enable_lines_t       = dict_find(iter, MESSAGE_KEY_EnableLines);
  Tuple *enable_minor_t       = dict_find(iter, MESSAGE_KEY_EnableMinorTick);
  Tuple *enable_major_t       = dict_find(iter, MESSAGE_KEY_EnableMajorTick);
  Tuple *fg_shape_t           = dict_find(iter, MESSAGE_KEY_ForegroundShape);

  if (addzero12_t) {
    settings.AddZero12h = addzero12_t->value->int32 != 0;
    //layer_mark_dirty(s_fg_layer);
    settings_changed = true;
  }

  if (remzero24_t) {
    settings.RemoveZero24h = remzero24_t->value->int32 != 0;
    //layer_mark_dirty(s_fg_layer);
    settings_changed = true;
  }

  if (localampm_t) {
    settings.showlocalAMPM = localampm_t->value->int32 == 1;
    //layer_mark_dirty(s_fg_layer);
    settings_changed = true;
  }

  if (vibe_t) {
    settings.VibeOn = vibe_t->value->int32 != 0;
  //  layer_mark_dirty(s_canvas_bt_icon);
    settings_changed = true;
  }

  if (enable_seconds_t) {
    settings.EnableSecondsHand = enable_seconds_t->value->int32 == 1;
  }

  if (enable_date_t) {
    settings.EnableDate = enable_date_t->value->int32 == 1;
    // layer_mark_dirty(s_canvas_layer);
    // layer_mark_dirty(s_fg_layer);
    settings_changed = true;
  }

  if (enable_lines_t) {
    settings.EnableLines = enable_lines_t->value->int32 == 1;
    // layer_mark_dirty(s_fg_layer);
    settings_changed = true;
  }

  if (enable_minor_t) {
    settings.EnableMinorTick = enable_minor_t->value->int32 == 1;
    // layer_mark_dirty(s_bg_layer);
    settings_changed = true;
  }
  
  if (enable_major_t) {
    settings.EnableMajorTick = enable_major_t->value->int32 == 1;
    // layer_mark_dirty(s_bg_layer);
    settings_changed = true;
  }

  if (enable_time_t) {
    settings.ShowTime = enable_time_t->value->int32 == 1;
    // layer_mark_dirty(s_fg_layer);
    settings_changed = true;
  }

  if (enable_logo_t) {
    settings.EnableLogo = enable_logo_t->value->int32 == 1;
    if (settings.EnableLogo && logotext_t && strlen(logotext_t->value->cstring) > 0) {
      snprintf(settings.LogoText, sizeof(settings.LogoText), "%s", logotext_t->value->cstring);
    } else if (settings.EnableLogo) {
      snprintf(settings.LogoText, sizeof(settings.LogoText), "%s", "HYBRID");
    } else {
      snprintf(settings.LogoText, sizeof(settings.LogoText), "%s", "");
    }
    // layer_mark_dirty(s_fg_layer);
    settings_changed = true;
  }

  if (fg_shape_t) {
    settings.ForegroundShape = fg_shape_t->value->int32 == 1;
    // layer_mark_dirty(s_fg_layer);
    // layer_mark_dirty(s_canvas_layer);
    settings_changed = true;
  }

  if (enable_battery_t) {
    settings.EnableBattery = enable_battery_t->value->int32 == 1;
    // layer_mark_dirty(s_fg_layer);
    settings_changed = true;
  }

  if (enable_secondsvisible_t) {
    settings.SecondsVisibleTime = (int)enable_secondsvisible_t->value->int32;
    if (s_timeout_timer) {
      app_timer_cancel(s_timeout_timer);
      s_timeout_timer = NULL;
    }
    if (settings.SecondsVisibleTime == 135) {
      showSeconds = true;
      if (settings.EnableSecondsHand) {
        tick_timer_service_unsubscribe();
        tick_timer_service_subscribe(SECOND_UNIT, tick_handler);
        accel_tap_service_unsubscribe();
      }
    } else if (settings.SecondsVisibleTime > 0) {
      showSeconds = true;
      if (settings.EnableSecondsHand) {
        tick_timer_service_unsubscribe();
        tick_timer_service_subscribe(SECOND_UNIT, tick_handler);
        s_timeout_timer = app_timer_register(SECONDS_TICK_INTERVAL_MS * settings.SecondsVisibleTime, timeout_handler, NULL);
        accel_tap_service_subscribe(accel_tap_handler);
      }
    } else {
      showSeconds = false;
      tick_timer_service_unsubscribe();
      tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
      accel_tap_service_unsubscribe();
    }
    //layer_mark_dirty(s_canvas_second_hand);
  }

  if (bwthemeselect_t) {
    if (strcmp(bwthemeselect_t->value->cstring, "wh") == 0) {
      settings.TextColor1 = GColorWhite;
      settings.BackgroundColor1 = GColorWhite;
      settings.HoursHandColor = GColorWhite;
      settings.HoursHandBorderColor = GColorBlack;
      settings.MinutesHandColor = GColorWhite;
      settings.MinutesHandBorderColor = GColorBlack;
      settings.SecondsHandColor = GColorBlack;
      settings.MajorTickColor = GColorBlack;
      settings.MinorTickColor = GColorBlack;
      settings.LineColor = GColorLightGray;
      settings.BTQTColor = GColorWhite;
      settings.FGColor = GColorBlack;
      theme_settings_changed = true;
    } else if (strcmp(bwthemeselect_t->value->cstring, "bl") == 0) {
      settings.TextColor1 = GColorBlack;
      settings.BackgroundColor1 = GColorBlack;
      settings.HoursHandColor = GColorBlack;
      settings.HoursHandBorderColor = GColorWhite;
      settings.MinutesHandColor = GColorBlack;
      settings.MinutesHandBorderColor = GColorWhite;
      settings.SecondsHandColor = GColorWhite;
      settings.MajorTickColor = GColorWhite;
      settings.MinorTickColor = GColorWhite;
      settings.LineColor = GColorDarkGray;
      settings.BTQTColor = GColorBlack;
      settings.FGColor = GColorWhite;
      theme_settings_changed = true;
    } else if (strcmp(bwthemeselect_t->value->cstring, "gy") == 0) {
      settings.TextColor1 = GColorBlack;
      settings.BackgroundColor1 = GColorLightGray;
      settings.HoursHandColor = GColorWhite;
      settings.HoursHandBorderColor = GColorBlack;
      settings.MinutesHandColor = GColorWhite;
      settings.MinutesHandBorderColor = GColorBlack;
      settings.SecondsHandColor = GColorBlack;
      settings.MajorTickColor = GColorWhite;
      settings.MinorTickColor = GColorWhite;
      settings.LineColor = GColorBlack;
      settings.BTQTColor = GColorBlack;
      settings.FGColor = GColorWhite;
      theme_settings_changed = true;
    } else if (strcmp(bwthemeselect_t->value->cstring, "cu") == 0) {
      if (bg_color1_t)    { settings.BackgroundColor1 = GColorFromHEX(bg_color1_t->value->int32); settings_changed = true; }
      if (text_color1_t)  { settings.TextColor1 = GColorFromHEX(text_color1_t->value->int32); }
      if (major_tick_color_t)  { settings.MajorTickColor = GColorFromHEX(major_tick_color_t->value->int32); }
      if (minor_tick_color_t)  { settings.MinorTickColor = GColorFromHEX(minor_tick_color_t->value->int32); }
      if (hours_color_t)  { settings.HoursHandColor = GColorFromHEX(hours_color_t->value->int32); }
      if (hours_border_t) { settings.HoursHandBorderColor = GColorFromHEX(hours_border_t->value->int32); }
      if (minutes_color_t){ settings.MinutesHandColor = GColorFromHEX(minutes_color_t->value->int32); }
      if (minutes_border_t){ settings.MinutesHandBorderColor = GColorFromHEX(minutes_border_t->value->int32); }
      if (seconds_color_t){ settings.SecondsHandColor = GColorFromHEX(seconds_color_t->value->int32); }
      if (line_color_t)   { settings.LineColor = GColorFromHEX(line_color_t->value->int32); }
      if (btqt_color_t)   { settings.BTQTColor = GColorFromHEX(btqt_color_t->value->int32); }
      if (fg_color_t)     { settings.FGColor = GColorFromHEX(fg_color_t->value->int32); }
      theme_settings_changed = true;
    }
  }

  if (themeselect_t) {
    if (strcmp(themeselect_t->value->cstring, "wh") == 0) {
      settings.BackgroundColor1 = GColorWhite;
      if (shadowon_t) { settings.ShadowOn = shadowon_t->value->int32 == 1; }
      settings.ShadowColor = settings.ShadowOn ? GColorLightGray : GColorWhite;
      settings.TextColor1 = GColorWhite;
      settings.MajorTickColor = GColorBlack;
      settings.MinorTickColor = GColorDarkGray;
      settings.HoursHandColor = GColorJaegerGreen;
      settings.HoursHandBorderColor = GColorJaegerGreen;
      settings.MinutesHandColor = GColorRed;
      settings.MinutesHandBorderColor = GColorRed;
      settings.SecondsHandColor = GColorJaegerGreen;
      settings.LineColor = GColorDarkGray;
      settings.BTQTColor = GColorWhite;
      settings.FGColor = GColorBlack;
      theme_settings_changed = true;
    } else if (strcmp(themeselect_t->value->cstring, "bl") == 0) {
      settings.BackgroundColor1 = GColorBlack;
      if (shadowon_t) { settings.ShadowOn = shadowon_t->value->int32 == 1; }
      settings.ShadowColor = settings.ShadowOn ? GColorDarkGray : GColorBlack;
      settings.TextColor1 = GColorWhite;
      settings.MajorTickColor = GColorWhite;
      settings.MinorTickColor = GColorLightGray;
      settings.HoursHandColor = GColorRed;
      settings.HoursHandBorderColor = GColorRed;
      settings.MinutesHandColor = GColorGreen;
      settings.MinutesHandBorderColor = GColorGreen;
      settings.SecondsHandColor = GColorRed;
      settings.LineColor = GColorDarkGray;
      settings.BTQTColor = GColorWhite;
      settings.FGColor = GColorBlack;
      theme_settings_changed = true;
    } else if (strcmp(themeselect_t->value->cstring, "bu") == 0) {
      settings.BackgroundColor1 = GColorDukeBlue;
      if (shadowon_t) { settings.ShadowOn = shadowon_t->value->int32 == 1; }
      settings.ShadowColor = settings.ShadowOn ? GColorOxfordBlue : GColorDukeBlue;
      settings.TextColor1 = GColorWhite;
      settings.MajorTickColor = GColorIcterine;
      settings.MinorTickColor = GColorIcterine;
      settings.HoursHandColor = GColorChromeYellow;
      settings.HoursHandBorderColor = GColorChromeYellow;
      settings.MinutesHandColor = GColorWhite;
      settings.MinutesHandBorderColor = GColorWhite;
      settings.SecondsHandColor = GColorChromeYellow;
      settings.LineColor = GColorIcterine;
      settings.BTQTColor = GColorWhite;
      settings.FGColor = GColorDukeBlue;
      theme_settings_changed = true;
    } else if (strcmp(themeselect_t->value->cstring, "pl") == 0) {
      settings.BackgroundColor1 = GColorPurple;
      if (shadowon_t) { settings.ShadowOn = shadowon_t->value->int32 == 1; }
      settings.ShadowColor = settings.ShadowOn ? GColorImperialPurple : GColorPurple;
      settings.TextColor1 = GColorWhite;
      settings.MajorTickColor = GColorBlack;
      settings.MinorTickColor = GColorBlack;
      settings.HoursHandColor = GColorBlack;
      settings.HoursHandBorderColor = GColorBlack;
      settings.MinutesHandColor = GColorBlack;
      settings.MinutesHandBorderColor = GColorBlack;
      settings.SecondsHandColor = GColorBlack;
      settings.LineColor = GColorWhite;
      settings.BTQTColor = GColorWhite;
      settings.FGColor = GColorBlack;
      theme_settings_changed = true;
    } else if (strcmp(themeselect_t->value->cstring, "gr") == 0) {
      settings.BackgroundColor1 = GColorBlack;
      if (shadowon_t) { settings.ShadowOn = shadowon_t->value->int32 == 1; }
      settings.ShadowColor = settings.ShadowOn ? GColorDarkGreen : GColorBlack;
      settings.TextColor1 = GColorBlack;
      settings.MajorTickColor = GColorBrightGreen;
      settings.MinorTickColor = GColorBrightGreen;
      settings.HoursHandColor = GColorPastelYellow;
      settings.HoursHandBorderColor = GColorPastelYellow;
      settings.MinutesHandColor = GColorBrightGreen;
      settings.MinutesHandBorderColor = GColorBrightGreen;
      settings.SecondsHandColor = GColorPastelYellow;
      settings.LineColor = GColorBlack;
      settings.BTQTColor = GColorBrightGreen;
      settings.FGColor = GColorBrightGreen;
      theme_settings_changed = true;
    } else if (strcmp(themeselect_t->value->cstring, "cu") == 0) {
      if (bg_color1_t) { settings.BackgroundColor1 = GColorFromHEX(bg_color1_t->value->int32); settings_changed = true; }
      if (shadowon_t) {
        settings.ShadowOn = shadowon_t->value->int32 == 1;
        if (settings.ShadowOn) {
          if (bg_color2_t) { settings.ShadowColor = GColorFromHEX(bg_color2_t->value->int32); settings_changed = true; }
        } else {
          settings.ShadowColor = settings.BackgroundColor1;
        }
      }
      if (text_color1_t)  { settings.TextColor1 = GColorFromHEX(text_color1_t->value->int32); layer_mark_dirty(s_fg_layer); }
      if (major_tick_color_t)  { settings.MajorTickColor = GColorFromHEX(major_tick_color_t->value->int32); layer_mark_dirty(s_bg_layer); layer_mark_dirty(s_fg_layer); }
      if (minor_tick_color_t)  { settings.MinorTickColor = GColorFromHEX(minor_tick_color_t->value->int32); layer_mark_dirty(s_bg_layer); layer_mark_dirty(s_fg_layer); }
      if (hours_color_t)  { settings.HoursHandColor = GColorFromHEX(hours_color_t->value->int32); layer_mark_dirty(s_canvas_layer); layer_mark_dirty(s_canvas_second_hand); }
      if (hours_border_t) { settings.HoursHandBorderColor = GColorFromHEX(hours_border_t->value->int32); layer_mark_dirty(s_canvas_layer); }
      if (minutes_color_t){ settings.MinutesHandColor = GColorFromHEX(minutes_color_t->value->int32); layer_mark_dirty(s_canvas_layer); layer_mark_dirty(s_canvas_second_hand); }
      if (minutes_border_t){ settings.MinutesHandBorderColor = GColorFromHEX(minutes_border_t->value->int32); layer_mark_dirty(s_canvas_layer); }
      if (seconds_color_t){ settings.SecondsHandColor = GColorFromHEX(seconds_color_t->value->int32); layer_mark_dirty(s_canvas_second_hand); }
      if (line_color_t)   { settings.LineColor = GColorFromHEX(line_color_t->value->int32); layer_mark_dirty(s_fg_layer); }
      if (btqt_color_t)   { settings.BTQTColor = GColorFromHEX(btqt_color_t->value->int32); } //layer_mark_dirty(s_canvas_bt_icon); layer_mark_dirty(s_canvas_qt_icon); }
      if (fg_color_t)     { settings.FGColor = GColorFromHEX(fg_color_t->value->int32); }
      theme_settings_changed = true;
    }
  }

  if (settings_changed || theme_settings_changed) {
    layer_mark_dirty(s_bg_layer);
    layer_mark_dirty(s_canvas_layer);
    layer_mark_dirty(s_canvas_second_hand);
    layer_mark_dirty(s_fg_layer);
  }

  prv_save_settings();
}


// ---------------------------------------------------------------------------
// Layer update procedures
// ---------------------------------------------------------------------------

static void bg_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);

  graphics_context_set_fill_color(ctx, settings.BackgroundColor1);
  graphics_fill_rect(ctx, GRect(0, 0, bounds.size.w, bounds.size.h), 0, GCornersAll);

  // Minor ticks — every degree except on major-tick angles (multiples of 30)
  if (settings.EnableMinorTick) {
    for (int i = 0; i < 60; i++) {
      if (i % 5 == 0) continue;
      int angle = i * 6;
      #ifdef PBL_ROUND
        GPoint origin = GPoint(bounds.size.w / 2, bounds.size.h / 2);
        GPoint center = polar_to_point_offset(origin, angle, bounds.size.h / 2 - config.minor_tick_inset);
        draw_minor_tick(ctx, center, settings.MinorTickColor);
      #else
        draw_minor_tick(ctx, angle, settings.BackgroundColor1, settings.MinorTickColor);
      #endif
    }
  }
    // Major ticks — 12 hour positions
    if(settings.EnableMajorTick){
      for (int i = 0; i < 12; i++) {
        int angle = i * 30 - 90;
        draw_major_tick(ctx, angle, 16, settings.BackgroundColor1, settings.MajorTickColor);
      }
    }
  }

  static void hour_min_hands_canvas_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);

  #if PBL_COLOR && PBL_ROUND
    draw_center_shadow(ctx, settings.ShadowColor);
  #elif PBL_COLOR
    if (settings.ForegroundShape) {
      draw_center_shadow(ctx, settings.ShadowColor);
    }
  #endif

  minutes = prv_tick_time.tm_min;
  hours = prv_tick_time.tm_hour % 12;

  #ifdef HOUR
  hours = HOUR;
  #endif
  #ifdef MINUTE
  minutes = MINUTE;
  #endif

  int hours_angle = ((double)hours / 12 * 360) + ((double)minutes / 60 * 360 / 12) - 90;
  int minutes_angle = ((double)minutes / 60 * 360) - 90;

  #ifdef PBL_ROUND
      draw_fancy_hand_hour(ctx, hours_angle, config.hour_hand_a, settings.HoursHandColor, settings.HoursHandBorderColor);
      draw_fancy_hand_min(ctx, minutes_angle, config.min_hand_a, config.min_hand_b, settings.MinutesHandColor, settings.MinutesHandBorderColor);
  #else
    if(settings.ForegroundShape){
      draw_fancy_hand_hour(ctx, hours_angle, bounds.size.w / 2 - config.hour_hand_a_round, settings.HoursHandColor, settings.HoursHandBorderColor);
      draw_fancy_hand_min(ctx, minutes_angle, bounds.size.w / 2 - config.min_hand_a_round, bounds.size.w / 2 - config.min_hand_b_round, settings.MinutesHandColor, settings.MinutesHandBorderColor);

    }
    else{
      draw_fancy_hand_hour(ctx, hours_angle, config.hour_hand_a, settings.HoursHandColor, settings.HoursHandBorderColor);
      draw_fancy_hand_min(ctx, minutes_angle, config.min_hand_a, config.min_hand_b, settings.MinutesHandColor, settings.MinutesHandBorderColor);
    }
  #endif
}
  
  static void layer_update_proc_seconds_hand(Layer *layer, GContext *ctx) {
  if (!showSeconds) return;

  GRect bounds = layer_get_bounds(layer);

  time_t now = time(NULL);
  struct tm *t = localtime(&now);
  int seconds = t->tm_sec;

  int seconds_angle = ((double)seconds / 60 * 360) - 90;
  #ifdef PBL_ROUND
    draw_seconds_line_hand(ctx, seconds_angle,
                          bounds.size.w / 2 - config.second_hand_a,
                          bounds.size.w / 2 - config.second_hand_b,
                          settings.SecondsHandColor);
  #else
    if(settings.ForegroundShape){
      draw_seconds_line_hand(ctx, seconds_angle,
                          bounds.size.w / 2 - config.second_hand_a_round,
                          bounds.size.w / 2 - config.second_hand_b_round,
                          settings.SecondsHandColor);
    }
    else{
      draw_seconds_line_hand(ctx, seconds_angle,
                          bounds.size.w / 2 - config.second_hand_a,
                          bounds.size.w / 2 - config.second_hand_b,
                          settings.SecondsHandColor);
    }
  #endif
}


#ifndef PBL_BW
static void fg_update_proc(Layer *layer, GContext *ctx) {
 GRect bounds = layer_get_bounds(layer);
  #ifndef PBL_ROUND
  if (settings.ForegroundShape) {
      // Round foreground (Hybrid style)
      draw_center(ctx, settings.SecondsHandColor, settings.FGColor);
    } else {
      // Rect foreground (HybridToo style)
      GRect fg_rect = GRect(config.foregroundrect_x, config.foregroundrect_y,
                            config.foregroundrect_w, config.foregroundrect_h);
      graphics_context_set_antialiased(ctx, true);
      graphics_context_set_fill_color(ctx, settings.FGColor);
      graphics_fill_rect(ctx, fg_rect, config.corner_radius_foreground, GCornersAll);
      graphics_context_set_stroke_color(ctx, settings.SecondsHandColor);
      graphics_context_set_stroke_width(ctx, 2);
      graphics_draw_round_rect(ctx, fg_rect, config.corner_radius_foreground);
    }
  #else
    draw_center(ctx, settings.SecondsHandColor, settings.FGColor);
  #endif

  FContext fctx;
  fctx_init_context(&fctx, ctx);
  fctx_set_scale(&fctx, FPointOne, FPointOne);
  fctx_set_offset(&fctx, FPointZero);
  fctx_set_color_bias(&fctx, 0);
  #ifdef PBL_COLOR
  fctx_enable_aa(true);
  #endif
  
  

  if (settings.ShowTime) {
    fctx_set_fill_color(&fctx, settings.TextColor1);

    int hourdraw;
    char hournow[4];
    if (clock_is_24h_style()) {
      hourdraw = s_hours;
      snprintf(hournow, sizeof(hournow), settings.RemoveZero24h ? "%d" : "%02d", hourdraw);
    } else {
      if (s_hours == 0 || s_hours == 12) hourdraw = 12;
      else hourdraw = s_hours % 12;
      snprintf(hournow, sizeof(hournow), settings.AddZero12h ? "%02d" : "%d", hourdraw);
    }

    char minnow[3];
    snprintf(minnow, sizeof(minnow), "%02d", minutes);

    char timedraw[10];
    snprintf(timedraw, sizeof(timedraw), "%s:%s", hournow, minnow);

    char local_ampm_string[5];
    strftime(local_ampm_string, sizeof(local_ampm_string), "%p", &prv_tick_time);

    fctx_set_text_em_height(&fctx, FCTX_Font, config.time_font_size);
    fixed_t time_width = fctx_string_width(&fctx, timedraw, FCTX_Font);

    fixed_t ampm_width = 0;
    if (settings.showlocalAMPM && !clock_is_24h_style() && (hourdraw > 9 || settings.AddZero12h)) {
      fctx_set_text_em_height(&fctx, FCTX_Font, config.other_text_font_size);
      ampm_width = fctx_string_width(&fctx, local_ampm_string, FCTX_Font) + INT_TO_FIXED(2); // 2px gap
    }

    fixed_t total_width = time_width + ampm_width;
    fixed_t left_edge = INT_TO_FIXED(bounds.size.w / 2) - (total_width / 2);

    // Draw time string left-aligned from left_edge
    fctx_set_text_em_height(&fctx, FCTX_Font, config.time_font_size);
    FPoint time_pos = { left_edge, INT_TO_FIXED(bounds.size.h / 2) };
    fctx_begin_fill(&fctx);
    fctx_set_color_bias(&fctx, 0);
    fctx_set_offset(&fctx, time_pos);
    fctx_draw_string(&fctx, timedraw, FCTX_Font, GTextAlignmentLeft, FTextAnchorMiddle);
    fctx_end_fill(&fctx);

    // Draw AM/PM immediately to the right of the time string
    if (settings.showlocalAMPM && !clock_is_24h_style()) {
      FPoint ampm_pos = {
        left_edge + time_width + INT_TO_FIXED(2),
        INT_TO_FIXED(bounds.size.h / 2) + INT_TO_FIXED(1)
      };
      fctx_begin_fill(&fctx);
      fctx_set_text_em_height(&fctx, FCTX_Font, config.other_text_font_size);
      fctx_set_color_bias(&fctx, 0);
      fctx_set_offset(&fctx, ampm_pos);
      fctx_draw_string(&fctx, local_ampm_string, FCTX_Font, GTextAlignmentLeft, FTextAnchorMiddle);
      fctx_end_fill(&fctx);
    }
  }

  if (settings.EnableDate) {
    fctx_begin_fill(&fctx);
    fctx_set_fill_color(&fctx, settings.TextColor1);

    const char *sys_locale = i18n_get_system_locale();
    char weekdaydraw[10];
    fetchwday(s_weekday, sys_locale, weekdaydraw);
    char datenow[3];
    snprintf(datenow, sizeof(datenow), "%02d", s_day);
    char monthnow[10];
    fetchmonth(s_month, sys_locale, monthnow);

    fctx_set_text_em_height(&fctx, FCTX_Font, config.other_text_font_size);

    FPoint weekday_pos = {
      INT_TO_FIXED(bounds.size.w * 0.42 - config.xOffset),
      INT_TO_FIXED(bounds.size.h / 2 - config.other_text_font_size - config.yOffset)
    };
    fctx_set_offset(&fctx, weekday_pos);
    fctx_draw_string(&fctx, weekdaydraw, FCTX_Font, GTextAlignmentRight, FTextAnchorBottom);

    FPoint month_pos = {
      INT_TO_FIXED(bounds.size.w * 0.58 + config.xOffset),
      INT_TO_FIXED(bounds.size.h / 2 - config.other_text_font_size - config.yOffset)
    };
    fctx_set_offset(&fctx, month_pos);
    fctx_draw_string(&fctx, monthnow, FCTX_Font, GTextAlignmentLeft, FTextAnchorBottom);

    fctx_set_text_em_height(&fctx, FCTX_Font, config.info_font_size);

    FPoint date_pos = {
      INT_TO_FIXED(bounds.size.w / 2),
      INT_TO_FIXED(bounds.size.h / 2 - config.other_text_font_size - config.yOffsetDate)
    };
    fctx_set_offset(&fctx, date_pos);
    fctx_draw_string(&fctx, datenow, FCTX_Font, GTextAlignmentCenter, FTextAnchorBottom);
    fctx_end_fill(&fctx);
  }
 
  if (settings.EnableBattery) {
    int battery_level = battery_state_service_peek().charge_percent;
    char battperc[6];
    snprintf(battperc, sizeof(battperc), "%d", battery_level);

    fctx_begin_fill(&fctx);
    fctx_set_fill_color(&fctx, settings.TextColor1);
    fctx_set_text_em_height(&fctx, FCTX_Font, config.info_font_size);

    FPoint battery_pos = {
      INT_TO_FIXED(bounds.size.w / 2 + config.xOffset),
      INT_TO_FIXED(bounds.size.h / 2 + config.other_text_font_size + config.yOffsetBattery)
    };
    fctx_set_offset(&fctx, battery_pos);
    fctx_draw_string(&fctx, battperc, FCTX_Font, GTextAlignmentLeft, FTextAnchorTop);

    fixed_t battery_width = fctx_string_width(&fctx, battperc, FCTX_Font);

    fctx_set_text_em_height(&fctx, FCTX_Font, config.other_text_font_size);
    FPoint percent_pos = {
      INT_TO_FIXED(bounds.size.w / 2 + config.xOffset) + battery_width,
      INT_TO_FIXED(bounds.size.h / 2 + config.other_text_font_size + config.yOffsetPercent)
    };
    fctx_set_offset(&fctx, percent_pos);
    fctx_draw_string(&fctx, "%", FCTX_Font, GTextAlignmentLeft, FTextAnchorTop);
    fctx_end_fill(&fctx);
  }
 
  if (settings.EnableLogo) {
    fctx_begin_fill(&fctx);
    fctx_set_fill_color(&fctx, settings.TextColor1);
    fctx_set_text_em_height(&fctx, FCTX_Font, config.other_text_font_size);
    FPoint word_pos = {
      INT_TO_FIXED(bounds.size.w / 2 - config.xOffset),
      INT_TO_FIXED(bounds.size.h / 2 + config.other_text_font_size + config.yOffsetPercent)
    };
    fctx_set_offset(&fctx, word_pos);
    char textdraw[15];
    snprintf(textdraw, sizeof(textdraw), "%s", settings.LogoText);
    fctx_draw_string(&fctx, textdraw, FCTX_Font, GTextAlignmentRight, FTextAnchorTop);
    fctx_end_fill(&fctx);
  }
  

  fctx_deinit_context(&fctx);

  if (settings.EnableLines) {
    graphics_context_set_antialiased(ctx, true);
    graphics_context_set_stroke_width(ctx, 1);
    graphics_context_set_stroke_color(ctx, settings.LineColor);

    int cx = bounds.size.w / 2;
    int cy = bounds.size.h / 2;
    int half_time = config.time_font_size / 2;

    // Horizontal lines above and below the time display
    graphics_draw_line(ctx, GPoint(bounds.size.w / 4, cy + half_time),
                            GPoint(bounds.size.w / 4 * 3, cy + half_time));
    graphics_draw_line(ctx, GPoint(bounds.size.w / 4, cy - half_time + 4),
                            GPoint(bounds.size.w / 4 * 3, cy - half_time + 4));

    // Vertical line below centre
    graphics_draw_line(ctx, GPoint(cx, cy + half_time + config.Line3yOffset),
                            GPoint(cx, cy + (config.fg_radius - 6)));

    // Vertical lines above centre (left and right of 12 o'clock)
    graphics_draw_line(ctx, GPoint(bounds.size.w * 0.42, cy - half_time - config.Line45yOffset),
                            GPoint(bounds.size.w * 0.42, cy - (config.fg_radius - 10)));
    graphics_draw_line(ctx, GPoint(bounds.size.w * 0.58, cy - half_time - config.Line45yOffset),
                            GPoint(bounds.size.w * 0.58, cy - (config.fg_radius - 10)));
  }

  // if (!connection_service_peek_pebble_app_connection()) {  //turn back to false after testing!
  // // draw BT icon
  // int xPosition = bounds.size.w / 2 - config.fg_radius + config.BTxOffset;
  // int yPosition = bounds.size.h / 2 + config.BTIconYOffset;
  // GRect BTIconRect = GRect(xPosition, yPosition, config.BTQTRectWidth, 20);
  // graphics_context_set_text_color(ctx, settings.BTQTColor);
  // graphics_context_set_antialiased(ctx, true);
  // graphics_draw_text(ctx, "z", FontBTQTIcons, BTIconRect, GTextOverflowModeFill, GTextAlignmentLeft, NULL);

  // }

  // if (quiet_time_is_active()) {  //turn back to true after testing
  // // draw QT icon  
  // //quiet_time_icon();
  // int xPosition = bounds.size.w / 2 - config.fg_radius + config.QTxOffset;
  // int yPosition = bounds.size.h / 2 + config.QTIconYOffset;
  // GRect QTIconRect = GRect(xPosition, yPosition, config.BTQTRectWidth, 20);
  // graphics_context_set_text_color(ctx, settings.BTQTColor);
  // graphics_context_set_antialiased(ctx, true);
  // graphics_draw_text(ctx, "\U0000E061", FontBTQTIcons, QTIconRect, GTextOverflowModeFill, GTextAlignmentLeft, NULL);
  // }

  draw_btqt_icons(ctx, bounds);

}
#else
static void fg_update_proc(Layer *layer, GContext *ctx) {

  GRect bounds = layer_get_bounds(layer);
  if (settings.ForegroundShape) {
    draw_center(ctx, settings.SecondsHandColor, settings.FGColor);
  }
  else{
    GRect fg_rect = GRect(config.foregroundrect_x,config.foregroundrect_y, config.foregroundrect_w, config.foregroundrect_h);
    graphics_context_set_fill_color(ctx, settings.FGColor);
    graphics_fill_rect(ctx, fg_rect, config.corner_radius_foreground, GCornersAll);
    graphics_context_set_stroke_color(ctx, settings.SecondsHandColor);
    graphics_context_set_stroke_width(ctx, 2);
    graphics_draw_round_rect(ctx, fg_rect, config.corner_radius_foreground);
  }

  int cx = bounds.size.w / 2;
  int cy = bounds.size.h / 2;

  // Draw foreground rect
  graphics_context_set_text_color(ctx, settings.TextColor1);

  if (settings.ShowTime) {
    int hourdraw;
    char hournow[4];
    if (clock_is_24h_style()) {
      hourdraw = s_hours;
      snprintf(hournow, sizeof(hournow), settings.RemoveZero24h ? "%d" : "%02d", hourdraw);
    } else {
      if (s_hours == 0 || s_hours == 12) hourdraw = 12;
      else hourdraw = s_hours % 12;
      snprintf(hournow, sizeof(hournow), settings.AddZero12h ? "%02d" : "%d", hourdraw);
    }
    char minnow[3];
    snprintf(minnow, sizeof(minnow), "%02d", minutes);
    char timedraw[10];
    snprintf(timedraw, sizeof(timedraw), "%s:%s", hournow, minnow);

    char local_ampm_string[5] = "";
    int ampm_w = 0;
    GSize ampm_size = {0, 0};

    if (settings.showlocalAMPM && !clock_is_24h_style()) {
      strftime(local_ampm_string, sizeof(local_ampm_string), "%p", &prv_tick_time);
      ampm_size = graphics_text_layout_get_content_size(
        local_ampm_string, small_font,
        GRect(0, 0, bounds.size.w, 16),
        GTextOverflowModeWordWrap, GTextAlignmentLeft
      );
      ampm_w = ampm_size.w + 2; // 2px gap
    }

    GSize time_size = graphics_text_layout_get_content_size(
      timedraw, aplite_font,
      GRect(0, 0, bounds.size.w, config.time_font_size + 4),
      GTextOverflowModeWordWrap, GTextAlignmentLeft
    );

    // Centre time+ampm block together
    bool ampm_affects_centre = settings.showlocalAMPM && !clock_is_24h_style() 
                             && (settings.AddZero12h || hourdraw > 9);

    int total_w = time_size.w + (ampm_affects_centre ? ampm_w : 0);
    int left_edge = cx - total_w / 2;
    //int total_w = time_size.w + ampm_w;
    //int left_edge = cx - total_w / 2;
    int time_y = cy - time_size.h / 2;

    graphics_draw_text(ctx, timedraw, aplite_font,
      GRect(left_edge, time_y - 3, time_size.w, time_size.h),
      GTextOverflowModeWordWrap, GTextAlignmentLeft, NULL);

    if (settings.showlocalAMPM && !clock_is_24h_style() && local_ampm_string[0] != '\0') {
      GRect ampm_rect = GRect(left_edge + time_size.w + 2, time_y + time_size.h - 17,
                              ampm_size.w, 16);
      graphics_draw_text(ctx, local_ampm_string, small_font, ampm_rect,
                        GTextOverflowModeWordWrap, GTextAlignmentLeft, NULL);
    }
  }

  if (settings.EnableDate) {
    const char *sys_locale = i18n_get_system_locale();
    char weekdaydraw[10];
    fetchwday(s_weekday, sys_locale, weekdaydraw);
    char datenow[3];
    snprintf(datenow, sizeof(datenow), "%02d", s_day);
    char monthnow[10];
    fetchmonth(s_month, sys_locale, monthnow);

    int date_y = cy - config.other_text_font_size - config.yOffset - 14 - 3;

    GRect weekday_rect = GRect(0 - 11, date_y, cx - config.xOffset, 16);
    graphics_draw_text(ctx, weekdaydraw, small_font, weekday_rect,
                       GTextOverflowModeWordWrap, GTextAlignmentRight, NULL);

    GRect month_rect = GRect(cx + config.xOffset + 11, date_y, bounds.size.w / 2, 16);
    graphics_draw_text(ctx, monthnow, small_font, month_rect,
                       GTextOverflowModeWordWrap, GTextAlignmentLeft, NULL);

    GRect date_rect = GRect(0, date_y - 10, bounds.size.w, 16);
    graphics_draw_text(ctx, datenow, medium_font, date_rect,
                       GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
  }

  if (settings.EnableBattery) {
    int battery_level = battery_state_service_peek().charge_percent;
    char battperc[6];
    snprintf(battperc, sizeof(battperc), "%d", battery_level);

    GSize batt_size = graphics_text_layout_get_content_size(
      battperc, medium_font,
      GRect(0, 0, bounds.size.w, config.info_font_size + 4),
      GTextOverflowModeWordWrap, GTextAlignmentLeft
    );
    GSize percent_size = graphics_text_layout_get_content_size(
      "%", small_font,
      GRect(0, 0, bounds.size.w, 16),
      GTextOverflowModeWordWrap, GTextAlignmentLeft
    );

    int total_batt_w = batt_size.w + 2 + percent_size.w;
    int batt_left = cx - total_batt_w / 2 + 19;
    int batt_y = cy + config.other_text_font_size + config.yOffsetBattery - 8;

    graphics_draw_text(ctx, battperc, medium_font,
      GRect(batt_left, batt_y, batt_size.w, batt_size.h),
      GTextOverflowModeWordWrap, GTextAlignmentLeft, NULL);

    graphics_draw_text(ctx, "%", small_font,
      GRect(batt_left + batt_size.w + 2, batt_y + batt_size.h - percent_size.h - 5,
            percent_size.w, percent_size.h),
      GTextOverflowModeWordWrap, GTextAlignmentLeft, NULL);
  }

  if (settings.EnableLogo) {
    char textdraw[15];
    snprintf(textdraw, sizeof(textdraw), "%s", settings.LogoText);
    int logo_y = cy + config.other_text_font_size + config.yOffsetPercent - 4;
    GRect logo_rect = GRect(0, logo_y, cx - config.xOffset + 1, 16);
    graphics_draw_text(ctx, textdraw, small_font, logo_rect,
                       GTextOverflowModeWordWrap, GTextAlignmentRight, NULL);
  }

  if (settings.EnableLines) {
    graphics_context_set_antialiased(ctx, true);
    graphics_context_set_stroke_width(ctx, 1);
    graphics_context_set_stroke_color(ctx, settings.LineColor);

    int cx = bounds.size.w / 2;
    int cy = bounds.size.h / 2;
    int half_time = config.time_font_size / 2;

    // Horizontal lines above and below the time display
    graphics_draw_line(ctx, GPoint(bounds.size.w / 4, cy + half_time),
                            GPoint(bounds.size.w / 4 * 3, cy + half_time));
    graphics_draw_line(ctx, GPoint(bounds.size.w / 4, cy - half_time + 4),
                            GPoint(bounds.size.w / 4 * 3, cy - half_time + 4));

    // Vertical line below centre
    graphics_draw_line(ctx, GPoint(cx, cy + half_time + config.Line3yOffset),
                            GPoint(cx, cy + (config.fg_radius - 6)));

    // Vertical lines above centre (left and right of 12 o'clock)
    graphics_draw_line(ctx, GPoint(bounds.size.w * 0.42, cy - half_time - config.Line45yOffset),
                            GPoint(bounds.size.w * 0.42, cy - (config.fg_radius - 10)));
    graphics_draw_line(ctx, GPoint(bounds.size.w * 0.58, cy - half_time - config.Line45yOffset),
                            GPoint(bounds.size.w * 0.58, cy - (config.fg_radius - 10)));
  }

  // if (!connection_service_peek_pebble_app_connection()) { //turn back to false after testing
  // // draw BT icon
  // int xPosition = bounds.size.w / 2 - config.fg_radius + config.BTxOffset;
  // int yPosition = bounds.size.h / 2 + config.BTIconYOffset;
  // GRect BTIconRect = GRect(xPosition, yPosition, config.BTQTRectWidth, 20);
  // graphics_context_set_text_color(ctx, settings.BTQTColor);
  // graphics_context_set_antialiased(ctx, true);
  // graphics_draw_text(ctx, "z", FontBTQTIcons, BTIconRect, GTextOverflowModeFill, GTextAlignmentLeft, NULL);

  // }

  // if (quiet_time_is_active()) {  //turn back to true after testing
  // // draw QT icon  
  // //quiet_time_icon();
  // int xPosition = bounds.size.w / 2 - config.fg_radius + config.QTxOffset;
  // int yPosition = bounds.size.h / 2 + config.QTIconYOffset;
  // GRect QTIconRect = GRect(xPosition, yPosition, config.BTQTRectWidth, 20);
  // graphics_context_set_text_color(ctx, settings.BTQTColor);
  // graphics_context_set_antialiased(ctx, true);
  // graphics_draw_text(ctx, "\U0000E061", FontBTQTIcons, QTIconRect, GTextOverflowModeFill, GTextAlignmentLeft, NULL);
  // }

  draw_btqt_icons(ctx, bounds);

}
#endif



// ---------------------------------------------------------------------------
// Window lifecycle
// ---------------------------------------------------------------------------

static void prv_window_load(Window *window) {
  // time_t temp = time(NULL);
  // prv_tick_time = *localtime(&temp);
  // s_day = prv_tick_time.tm_mday;
  // s_weekday = prv_tick_time.tm_wday;
  // minutes = prv_tick_time.tm_min;
  // hours = prv_tick_time.tm_hour % 12;
  // s_hours = prv_tick_time.tm_hour;
  // s_month = prv_tick_time.tm_mon;
  //seconds = prv_tick_time.tm_sec;
    Layer *window_layer = window_get_root_layer(s_window);
    bounds = layer_get_bounds(window_layer);

  // Initial State Fetch
    time_t now = time(NULL);
    prv_tick_time = *localtime(&now);
    s_connected = connection_service_peek_pebble_app_connection();
    s_battery_level = battery_state_service_peek().charge_percent;
    update_cached_strings();

  // Layer *window_layer = window_get_root_layer(window);
  // bounds = layer_get_bounds(window_layer);
  //APP_LOG(APP_LOG_LEVEL_INFO, "Settings size: %d bytes", (int)sizeof(ClaySettings)); 
  //bounds_seconds = bounds;

  #ifdef PBL_BW
   aplite_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_DIN_CON_APLITE_38));
   small_font = fonts_get_system_font(FONT_KEY_GOTHIC_14);
   medium_font = fonts_get_system_font(FONT_KEY_GOTHIC_24);
  #else
   FCTX_Font = ffont_create_from_resource(RESOURCE_ID_DIN_CONDENSED_FFONT);
  #endif
 
  #if defined (PBL_PLATFORM_EMERY) || defined (PBL_PLATFORM_GABBRO)
  FontBTQTIcons = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_DRIPICONS_16));
  #else
  FontBTQTIcons = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_DRIPICONS_12));
  #endif

  // connection_service_subscribe((ConnectionHandlers){
  //   .pebble_app_connection_handler = bluetooth_vibe_icon
  // });

  // Subscriptions
    tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
    battery_state_service_subscribe(battery_callback);
    connection_service_subscribe((ConnectionHandlers){ .pebble_app_connection_handler = bluetooth_callback });

  if (settings.EnableSecondsHand) {
    tick_timer_service_subscribe(SECOND_UNIT, tick_handler);
    if (settings.SecondsVisibleTime != 135) {
      s_timeout_timer = app_timer_register(1000 * settings.SecondsVisibleTime, timeout_handler, NULL);
      accel_tap_service_subscribe(accel_tap_handler);
    }
  } else {
    tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  }
  showSeconds = settings.EnableSecondsHand;

  s_bg_layer = layer_create(bounds);
  s_canvas_second_hand = layer_create(bounds);
  s_canvas_layer = layer_create(bounds);
  s_fg_layer = layer_create(bounds);

  layer_add_child(window_layer, s_bg_layer);
  layer_add_child(window_layer, s_canvas_layer);
  layer_add_child(window_layer, s_canvas_second_hand);
  layer_add_child(window_layer, s_fg_layer);
  
  //bluetooth_vibe_icon(connection_service_peek_pebble_app_connection());

  layer_set_update_proc(s_bg_layer, bg_update_proc);
  layer_set_update_proc(s_canvas_second_hand, layer_update_proc_seconds_hand);
  layer_set_update_proc(s_canvas_layer, hour_min_hands_canvas_update_proc);
  layer_set_update_proc(s_fg_layer, fg_update_proc);
}

static void prv_window_unload(Window *window) {
  if (s_timeout_timer) {
    app_timer_cancel(s_timeout_timer);
  }
  accel_tap_service_unsubscribe();
  connection_service_unsubscribe();
  //battery_state_service_unsubscribe();
  tick_timer_service_unsubscribe();
  layer_destroy(s_canvas_layer);
  layer_destroy(s_bg_layer);
  layer_destroy(s_fg_layer);
  layer_destroy(s_canvas_second_hand);
  // layer_destroy(s_canvas_bt_icon);
  // layer_destroy(s_canvas_qt_icon);
  fonts_unload_custom_font(FontBTQTIcons);
  //#ifdef PBL_PLATFORM_APLITE
  #ifdef PBL_BW
  fonts_unload_custom_font(aplite_font);
  #else
  ffont_destroy(FCTX_Font);
  #endif
}

// ---------------------------------------------------------------------------
// App lifecycle
// ---------------------------------------------------------------------------

static void prv_init(void) {
  prv_load_settings();
  app_message_open(256, 256);
  app_message_register_inbox_received(prv_inbox_received_handler);
  s_window = window_create();
  window_set_window_handlers(s_window, (WindowHandlers){
    .load = prv_window_load,
    .unload = prv_window_unload,
  });
  window_stack_push(s_window, true);
}

static void prv_deinit(void) {
  window_destroy(s_window);
}

int main(void) {
  prv_init();
  app_event_loop();
  prv_deinit();
}