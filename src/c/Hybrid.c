#include <pebble.h>
#include "Hybrid.h"
#include "utils/weekday.h"
#include "utils/month.h"
#include "utils/MathUtils.h"
#include <pebble-fctx/fctx.h>
#include <pebble-fctx/fpath.h>
#include <pebble-fctx/ffont.h>

// Constants for improved readability
#define SECONDS_TICK_INTERVAL_MS 1000

// Main window and layers
static Window *s_window;
static Layer *s_canvas_layer;
static Layer *s_bg_layer;
static Layer *s_canvas_second_hand;
static Layer *s_canvas_bt_icon;
static Layer *s_canvas_qt_icon;
static Layer *s_fg_layer;
static GRect bounds;
static GRect bounds_seconds;
// Fonts
static GFont FontBTQTIcons;

FFont* FCTX_Font;

// Time and date variables
static struct tm *prv_tick_time;
static int s_month;
static int s_day;
static int s_weekday;
static int s_hours;
static int minutes;
static int hours;
static int seconds;
static ClaySettings settings;
static bool showSeconds;

// Position/rendering config struct for different platforms
typedef struct {
  int BTQTxOffset;  //used for BT and QT icon offset from left of inside of foreground circle
  int BTQTRectWidth;  //width of the BT & QT GRects for the icons
  int BTIconYOffset;  //used to put BT icon above centre and above QT icon
  int QTIconYOffset;  //used to put QT icon below centre and below BT icon
  int xOffset;  //use for text offset (date and logo and battery)
  int yOffset;  //use for text offset (date and logo and battery above or below lines)
  int yOffsetBattery;
  int yOffsetDate;
  int yOffsetPercent;  //offset below lines for percent and custom/logo text
  int second_hand_a;  //bounds.size.w/2 - second_hand_a is where second hand line starts
  int second_hand_b;  //where second hand line finishes
  int hour_hand_a; //offset of the hour hand inside the bezel edge
  int min_hand_a;  //where min hand starts & ends -> bounds.size.w / 2 - config.min_hand_a, bounds.size.w / 2 - config.min_hand_b
  int min_hand_b;
  int fg_radius;  //radius of the foreground circle showing the text & digital time
  int tick_inset_inner;
  int tick_inset_outer;
  int minor_tick_inset;
  int time_font_size;
  int info_font_size;
  int other_text_font_size;
  int Line45yOffset;
  int Line3yOffset;
} UIConfig;

#ifdef PBL_PLATFORM_EMERY
static const UIConfig config = {
.BTQTxOffset = 8,
.BTQTRectWidth = 30,
.BTIconYOffset = -15,
.QTIconYOffset = 5,
.xOffset = 4,
.yOffset = 4,
.yOffsetBattery = 10,
.yOffsetDate = 2,
.yOffsetPercent = 11,
.second_hand_a = 0,  
.second_hand_b = 28,  
.hour_hand_a = 12,  
.min_hand_a = 2,  
.min_hand_b = 17,  
.fg_radius = 74,
.tick_inset_inner = 18,
.tick_inset_outer = 2,
.minor_tick_inset = 15,
.time_font_size = 54,
.info_font_size = 30,
.other_text_font_size = 20,
.Line45yOffset = 2,
.Line3yOffset = 6
};
#elif defined(PBL_PLATFORM_GABBRO)
static const UIConfig config = {
.BTQTxOffset = 10,
.BTQTRectWidth = 30,
.BTIconYOffset = -14,
.QTIconYOffset = 4,
.xOffset = 4,
.yOffset = 5,
.yOffsetBattery = 12,
.yOffsetDate = 3,
.yOffsetPercent = 13,
.second_hand_a = 6,  
.second_hand_b = 40,  
.hour_hand_a = 24,  
.min_hand_a = 8,  
.min_hand_b = 32,  
.fg_radius = 90,
.tick_inset_inner = 15,
.tick_inset_outer = 6,
.minor_tick_inset = 7,
.time_font_size = 66,
.info_font_size = 36,
.other_text_font_size = 24,
.Line45yOffset = 2,
.Line3yOffset = 6
};
#elif defined(PBL_BW)
static const UIConfig config = {
.BTQTxOffset = 5,
.BTQTRectWidth = 30,
.BTIconYOffset = -14,
.QTIconYOffset = 4,
.xOffset = 4,
.yOffset = 2,
.yOffsetBattery = 8,
.yOffsetDate = 1,
.yOffsetPercent = 9,
.second_hand_a = 0,
.second_hand_b = 20,
.hour_hand_a = 8,
.min_hand_a = 2,
.min_hand_b = 12,
.fg_radius = 54,
.tick_inset_inner = 14,
.tick_inset_outer = 2,
.minor_tick_inset = 11,
.time_font_size = 38,
.info_font_size = 20,
.other_text_font_size = 14,
.Line45yOffset = 1,
.Line3yOffset = 5
};
#elif defined(PBL_ROUND)
static const UIConfig config = {
.BTQTxOffset = 5,
.BTQTRectWidth = 30,
.BTIconYOffset = -14,
.QTIconYOffset = 4,
.xOffset = 4,
.yOffset = 3,
.yOffsetBattery = 9,
.yOffsetDate = 1,
.yOffsetPercent = 10,
.second_hand_a = 3,
.second_hand_b = 28,
.hour_hand_a = 18,
.min_hand_a = 5,
.min_hand_b = 21,
.fg_radius = 62,
.tick_inset_inner = 10,
.tick_inset_outer = 3,
.minor_tick_inset = 4,
.time_font_size = 46,
.info_font_size = 24,
.other_text_font_size = 16,
.Line45yOffset = 0,
.Line3yOffset = 4
};
#else // Default for other platforms
static const UIConfig config = {
.BTQTxOffset = 5,
.BTQTRectWidth = 30,
.BTIconYOffset = -14,
.QTIconYOffset = 4,
.xOffset = 4,
.yOffset = 2,
.yOffsetBattery = 8,
.yOffsetDate = 1,
.yOffsetPercent = 9,
.second_hand_a = 0,
.second_hand_b = 20,
.hour_hand_a = 8,
.min_hand_a = 2,
.min_hand_b = 12,
.fg_radius = 54,
.tick_inset_inner = 14,
.tick_inset_outer = 2,
.minor_tick_inset = 11,
.time_font_size = 38,
.info_font_size = 20,
.other_text_font_size = 14,
.Line45yOffset = 1,
.Line3yOffset = 5
};
#endif

bool connected = true;
bool ignore_next_tap = false;

//function prototypes

static void prv_save_settings(void);
static void prv_default_settings(void);
static void prv_load_settings(void);
static void prv_inbox_received_handler(DictionaryIterator *iter, void *context);
static void tick_handler(struct tm *tick_time, TimeUnits units_changed);
static void bg_update_proc(Layer *layer, GContext *ctx);
static void fg_update_proc(Layer *layer, GContext *ctx);
static void layer_update_proc_seconds_hand(Layer *layer, GContext * ctx);
static void hour_min_hands_canvas_update_proc(Layer *layer, GContext *ctx);
static void layer_update_proc_qt(Layer *layer, GContext *ctx);
static void layer_update_proc_bt(Layer *layer, GContext *ctx);
static void draw_fancy_hand_hour(GContext *ctx, int angle, int length, GColor fill_color, GColor border_color);
static void draw_fancy_hand_min(GContext *ctx, int angle, int length, int back_length, GColor fill_color, GColor border_color);
static void draw_seconds_line_hand(GContext *ctx, int angle, int length, int back_length, GColor color);
#if PBL_COLOR
static void draw_center_shadow(GContext *ctx, GColor shadow_color);
#endif
static void draw_center(GContext *ctx, GColor seconds_color, GColor fg_color);
static void prv_window_load(Window *window);
static void prv_window_unload(Window *window);
static void prv_init(void);
static void prv_deinit(void);

// Save settings to persistent storage
static void prv_save_settings(void) {
  persist_write_data(SETTINGS_KEY, &settings, sizeof(settings));
}


// Set default settings
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
  settings.TickColor = GColorBlack;
  settings.HoursHandColor = PBL_IF_BW_ELSE(GColorWhite,GColorJaegerGreen);
  settings.HoursHandBorderColor = PBL_IF_BW_ELSE(GColorBlack,GColorJaegerGreen);
  settings.MinutesHandColor = PBL_IF_BW_ELSE(GColorWhite,GColorRed);
  settings.MinutesHandBorderColor = PBL_IF_BW_ELSE(GColorBlack,GColorRed);
  settings.SecondsHandColor = PBL_IF_BW_ELSE(GColorBlack,GColorJaegerGreen);
  settings.BTQTColor = GColorWhite;
  settings.VibeOn = false;
  settings.FGColor = GColorBlack;
  settings.RemoveZero24h = false;
  settings.AddZero12h = false;
  settings.LineColor = GColorLightGray;
  settings.showlocalAMPM = true;
  settings.ShowTime = true;
  settings.EnableLines = true;
}

// Quiet time icon handler
static void quiet_time_icon () {
    layer_set_hidden(s_canvas_qt_icon, !quiet_time_is_active());
}


static AppTimer *s_timeout_timer;


static void timeout_handler(void *context) {
  showSeconds = false;

  // Unsubscribe from second ticks to save power
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);

  layer_mark_dirty(s_canvas_second_hand);
  s_timeout_timer = NULL; // Set the handle to NULL after the timer expires

  //APP_LOG(APP_LOG_LEVEL_DEBUG, "timeout event");

}

static void accel_tap_handler(AccelAxisType axis, int32_t direction) {
   if (ignore_next_tap) {
       ignore_next_tap = false; // Reset the flag for the next tap
       return;
     }

  // Only handle if the seconds hand setting is enabled and not already always on
  if (settings.EnableSecondsHand && settings.SecondsVisibleTime < 135) {
      // If a timer is already running, cancel it
      if (s_timeout_timer) {
        app_timer_cancel(s_timeout_timer);
        s_timeout_timer = NULL;
      }

      // Only subscribe to second ticks if not already subscribed
      if (!showSeconds) {
         tick_timer_service_subscribe(SECOND_UNIT, tick_handler);
      }
      
      showSeconds = true;

      // Register a new timer to hide the seconds hand
      s_timeout_timer = app_timer_register(SECONDS_TICK_INTERVAL_MS * settings.SecondsVisibleTime, timeout_handler, NULL);
      layer_mark_dirty(s_canvas_second_hand);
  }
}

static void bluetooth_vibe_icon (bool connected) {


   layer_set_hidden(s_canvas_bt_icon, connected);

  if((!connected && !quiet_time_is_active()) ||(!connected && quiet_time_is_active() && settings.VibeOn)) {
    if (settings.SecondsVisibleTime > 0 && settings.SecondsVisibleTime < 135) {
      // Unsubscribe from accel_tap before the vibe
      accel_tap_service_unsubscribe();
      showSeconds = false;
    }

    // Issue a vibrating alert
    #ifdef PBL_PLATFORM_DIORITE
    vibes_short_pulse();
    ignore_next_tap = true;

    #else
    vibes_double_pulse();
    // Set the flag to ignore the next tap
    ignore_next_tap = false;
    #endif

    if (settings.SecondsVisibleTime > 0 && settings.SecondsVisibleTime < 135) {
      accel_tap_service_subscribe(accel_tap_handler);
    }
  }
}

// Load settings from persistent storage
static void prv_load_settings(void) {
  prv_default_settings();
  persist_read_data(SETTINGS_KEY, &settings, sizeof(settings));
}

// AppMessage inbox handler
static void prv_inbox_received_handler(DictionaryIterator *iter, void *context) {
#ifdef LOG
  APP_LOG(APP_LOG_LEVEL_INFO, "Received message");
#endif

  bool settings_changed = false;
  bool theme_settings_changed = false;

  Tuple *vibe_t = dict_find(iter, MESSAGE_KEY_VibeOn);
  Tuple *enable_seconds_t = dict_find(iter, MESSAGE_KEY_EnableSecondsHand);
  Tuple *enable_secondsvisible_t = dict_find(iter, MESSAGE_KEY_SecondsVisibleTime);
  Tuple *enable_date_t = dict_find(iter, MESSAGE_KEY_EnableDate);
  Tuple *enable_battery_t = dict_find(iter, MESSAGE_KEY_EnableBattery);
  Tuple *enable_logo_t = dict_find(iter, MESSAGE_KEY_EnableLogo);
  Tuple *logotext_t = dict_find(iter, MESSAGE_KEY_LogoText);
  Tuple *bwthemeselect_t = dict_find(iter, MESSAGE_KEY_BWThemeSelect);
  Tuple *themeselect_t = dict_find(iter, MESSAGE_KEY_ThemeSelect);
  Tuple *bg_color1_t = dict_find(iter, MESSAGE_KEY_BackgroundColor1);
  Tuple *bg_color2_t = dict_find(iter, MESSAGE_KEY_ShadowColor);
  Tuple *text_color1_t = dict_find(iter, MESSAGE_KEY_TextColor1);
  Tuple *text_color2_t = dict_find(iter, MESSAGE_KEY_TickColor);
  //Tuple *date_color_t = dict_find(iter, MESSAGE_KEY_DateColor);
  Tuple *hours_color_t = dict_find(iter, MESSAGE_KEY_HoursHandColor);
  Tuple *hours_border_t = dict_find(iter, MESSAGE_KEY_HoursHandBorderColor);
  Tuple *minutes_color_t = dict_find(iter, MESSAGE_KEY_MinutesHandColor);
  Tuple *minutes_border_t = dict_find(iter, MESSAGE_KEY_MinutesHandBorderColor);
  Tuple *seconds_color_t = dict_find(iter, MESSAGE_KEY_SecondsHandColor);
  Tuple *btqt_color_t = dict_find(iter, MESSAGE_KEY_BTQTColor);
  Tuple *shadowon_t = dict_find(iter, MESSAGE_KEY_ShadowOn);
  Tuple *fg_color_t = dict_find(iter, MESSAGE_KEY_FGColor);
  Tuple *addzero12_t = dict_find(iter, MESSAGE_KEY_AddZero12h);
  Tuple *remzero24_t = dict_find(iter, MESSAGE_KEY_RemoveZero24h);
  Tuple *line_color_t = dict_find(iter, MESSAGE_KEY_LineColor);
  Tuple *localampm_t = dict_find(iter, MESSAGE_KEY_showlocalAMPM);
  Tuple *enable_time_t = dict_find(iter, MESSAGE_KEY_ShowTime);
  Tuple *enable_lines_t = dict_find(iter, MESSAGE_KEY_EnableLines);
  
  

  if (addzero12_t){
    if (addzero12_t -> value -> int32 == 0){
      settings.AddZero12h = false;
      APP_LOG(APP_LOG_LEVEL_DEBUG, "Add Zero 12h off");
    } else {
    settings.AddZero12h = true;
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Add Zero 12h on");
      }
      layer_mark_dirty(s_fg_layer);
    }

  if (remzero24_t){
    if (remzero24_t -> value -> int32 == 0){
      settings.RemoveZero24h = false;
      APP_LOG(APP_LOG_LEVEL_DEBUG, "Remove Zero 24h off");
    } else {
    settings.RemoveZero24h = true;
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Remove Zero 24h off");
      }
      layer_mark_dirty(s_fg_layer);
    }

  if (localampm_t) {
    settings.showlocalAMPM = localampm_t->value->int32 == 1;
    layer_mark_dirty(s_fg_layer);
  }

  if (vibe_t){
    if (vibe_t -> value -> int32 == 0){
      settings.VibeOn = false;
      //APP_LOG(APP_LOG_LEVEL_DEBUG, "Vibe off");
    } else {
      settings.VibeOn = true;
      //APP_LOG(APP_LOG_LEVEL_DEBUG, "Vibe on");
    }
    layer_mark_dirty(s_canvas_bt_icon);
  }

  if (enable_seconds_t) {
    settings.EnableSecondsHand = enable_seconds_t->value->int32 == 1;
    // Unsubscribe from any existing tick services
    tick_timer_service_unsubscribe();
    accel_tap_service_unsubscribe();
    // Always subscribe to MINUTE_UNIT by default for efficiency
    tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
    layer_mark_dirty(s_canvas_second_hand);
  }

  if (enable_date_t) {
    settings.EnableDate = enable_date_t->value->int32 == 1;
    layer_mark_dirty(s_canvas_layer);
    layer_mark_dirty(s_fg_layer);
  }

  if (enable_lines_t) {
    settings.EnableLines = enable_lines_t->value->int32 == 1;
    layer_mark_dirty(s_fg_layer);
  }

  if (enable_time_t) {
    settings.ShowTime = enable_time_t->value->int32 == 1;
    layer_mark_dirty(s_fg_layer);
  }

  if (enable_logo_t) {
    settings.EnableLogo = enable_logo_t->value->int32 == 1;

    // Check if the logo is enabled and the custom text string is not empty
    if (settings.EnableLogo && logotext_t && strlen(logotext_t->value->cstring) > 0) {
      // If the custom text field is not blank, use the user's text
      snprintf(settings.LogoText, sizeof(settings.LogoText), "%s", logotext_t->value->cstring);
    } else if (settings.EnableLogo && strlen(logotext_t->value->cstring) == 0) {
      // If the custom text field is blank but the logo is enabled, use the default text
      snprintf(settings.LogoText, sizeof(settings.LogoText), "%s", "HYBRID");
    }
    else {
      snprintf(settings.LogoText, sizeof(settings.LogoText), "%s", "");
    }

    layer_mark_dirty(s_fg_layer);

  }

  if (enable_battery_t) {
    settings.EnableBattery = enable_battery_t->value->int32 == 1;
    layer_mark_dirty(s_fg_layer);
    }

  if (enable_secondsvisible_t) {
    settings.SecondsVisibleTime = (int) enable_secondsvisible_t->value->int32;
    // Cancel and re-register timer if it was running
    if (s_timeout_timer) {
      app_timer_cancel(s_timeout_timer);
      s_timeout_timer = NULL;
    }

    // Handle "Always On" vs. "Timeout" behavior for the seconds hand
    if (settings.SecondsVisibleTime == 135) {
      // "Always On" logic: show seconds, and don't register a timer
      showSeconds = true;
      if (settings.EnableSecondsHand) {
        tick_timer_service_unsubscribe();
        tick_timer_service_subscribe(SECOND_UNIT, tick_handler);
        // Unsubscribe from accel_tap_service as it's not needed
        accel_tap_service_unsubscribe();
      }
    } else if (settings.SecondsVisibleTime > 0) {
      // "Timeout" logic: start with seconds shown, register a timer
      showSeconds = true;
      if (settings.EnableSecondsHand) {
        tick_timer_service_unsubscribe();
        tick_timer_service_subscribe(SECOND_UNIT, tick_handler);
        s_timeout_timer = app_timer_register(SECONDS_TICK_INTERVAL_MS * settings.SecondsVisibleTime, timeout_handler, NULL);
        // Subscribe to accel_tap_service to reset the timer
        accel_tap_service_subscribe(accel_tap_handler);
      }
    } else {
      // "Disabled" logic: don't show seconds, ensure on minute ticks
      showSeconds = false;
      tick_timer_service_unsubscribe();
      tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
      // Unsubscribe from accel_tap_service
      accel_tap_service_unsubscribe();
    }
    layer_mark_dirty(s_canvas_second_hand);
  }

  if (bwthemeselect_t) {
          // Compare the string value received from the phone
          if (strcmp(bwthemeselect_t->value->cstring, "wh") == 0) {
              // Set the theme and other settings for "wh"
                    settings.TextColor1 = GColorWhite;
                    settings.BackgroundColor1 = GColorWhite;
                    settings.HoursHandColor = GColorWhite;
                    settings.HoursHandBorderColor = GColorBlack;
                    settings.MinutesHandColor = GColorWhite;
                    settings.MinutesHandBorderColor = GColorBlack;
                    settings.SecondsHandColor = GColorBlack;
                    settings.TickColor = GColorBlack;
                    settings.LineColor = GColorLightGray;
                    settings.BTQTColor = GColorWhite;
                    settings.FGColor = GColorBlack;
                      theme_settings_changed = true;
                    //    APP_LOG(APP_LOG_LEVEL_DEBUG, "Theme white selected");
          } else if (strcmp(bwthemeselect_t->value->cstring, "bl") == 0) {
              // Set the theme and other settings for "bl"
                    settings.TextColor1 = GColorBlack;
                    settings.BackgroundColor1 = GColorBlack;
                    settings.HoursHandColor = GColorBlack;
                    settings.HoursHandBorderColor = GColorWhite;
                    settings.MinutesHandColor = GColorBlack;
                    settings.MinutesHandBorderColor = GColorWhite;
                    settings.SecondsHandColor = GColorWhite;
                    settings.TickColor = GColorWhite;
                    settings.LineColor = GColorDarkGray;
                    settings.BTQTColor = GColorBlack;
                    settings.FGColor = GColorWhite;
                      theme_settings_changed = true;
                    //    APP_LOG(APP_LOG_LEVEL_DEBUG, "Theme black selected");
          } else if (strcmp(bwthemeselect_t->value->cstring, "gy") == 0) {
              // Set the theme and other settings for "gy"
                    settings.TextColor1 = GColorBlack;
                    settings.BackgroundColor1 = GColorLightGray;
                    settings.HoursHandColor = GColorWhite;
                    settings.HoursHandBorderColor = GColorBlack;
                    settings.MinutesHandColor = GColorWhite;
                    settings.MinutesHandBorderColor = GColorBlack;
                    settings.SecondsHandColor = GColorBlack;
                    settings.TickColor = GColorWhite;
                    settings.LineColor = GColorBlack;
                    settings.BTQTColor = GColorBlack;
                    settings.FGColor = GColorWhite;
                      theme_settings_changed = true;
                    //    APP_LOG(APP_LOG_LEVEL_DEBUG, "Theme black selected");
          } else if (strcmp(bwthemeselect_t->value->cstring, "cu") == 0) {
              // Set the theme for "cu" and handle custom colors
                  //settings.TextColor1 = GColorFromHEX(bwdate_color_t->value->int32);
                  if (bg_color1_t) {
                    settings.BackgroundColor1 = GColorFromHEX(bg_color1_t->value->int32);
                    settings_changed = true;
                  }
                  if (text_color1_t) {
                    settings.TextColor1 = GColorFromHEX(text_color1_t->value->int32);
                  }
                  if (text_color2_t) {
                    settings.TickColor = GColorFromHEX(text_color2_t->value->int32);
                  }              
                  if (hours_color_t) {
                    settings.HoursHandColor = GColorFromHEX(hours_color_t->value->int32);
                  }
                  if (hours_border_t) {
                    settings.HoursHandBorderColor = GColorFromHEX(hours_border_t->value->int32);
                  }
                  if (minutes_color_t) {
                    settings.MinutesHandColor = GColorFromHEX(minutes_color_t->value->int32);
                  }
                  if (minutes_border_t) {
                    settings.MinutesHandBorderColor = GColorFromHEX(minutes_border_t->value->int32);
                  }
                  if (seconds_color_t) {
                    settings.SecondsHandColor = GColorFromHEX(seconds_color_t->value->int32);
                  }
                  if (line_color_t) {
                    settings.LineColor = GColorFromHEX(line_color_t->value->int32);
                  }
                  if (btqt_color_t) {
                    settings.BTQTColor = GColorFromHEX(btqt_color_t->value->int32);
                  }
                   if (fg_color_t) {
                    settings.FGColor = GColorFromHEX(fg_color_t->value->int32);
                  }
                  theme_settings_changed = true;
                  //  APP_LOG(APP_LOG_LEVEL_DEBUG, "Theme custom selected");
                }
          }
/////////////////////////////////////
  if (themeselect_t) {
          // Compare the string value received from the phone
          if (strcmp(themeselect_t->value->cstring, "wh") == 0) {
              // Set the theme and other settings for "wh"
                    settings.BackgroundColor1 = GColorWhite;      
                    if (shadowon_t) {
                      settings.ShadowOn = shadowon_t->value->int32 == 1;
                    }
                        if(settings.ShadowOn){
                          settings.ShadowColor = GColorLightGray;
                        }
                        else {
                        settings.ShadowColor = GColorWhite;
                        }
                   
                    settings.TextColor1 = GColorWhite;
                    settings.TickColor = GColorBlack;
//                    settings.DateColor = GColorWhite;
                    settings.HoursHandColor = GColorJaegerGreen;
                    settings.HoursHandBorderColor = GColorJaegerGreen;
                    settings.MinutesHandColor = GColorRed;
                    settings.MinutesHandBorderColor = GColorRed;
                    settings.SecondsHandColor = GColorJaegerGreen;
                    settings.LineColor = GColorDarkGray;
                    settings.BTQTColor = GColorWhite;
                    settings.FGColor = GColorBlack;
                      theme_settings_changed = true;
                    //    APP_LOG(APP_LOG_LEVEL_DEBUG, "Theme white selected");
          } else if (strcmp(themeselect_t->value->cstring, "bl") == 0) {
              // Set the theme and other settings for "bl"

                    settings.BackgroundColor1 = GColorBlack;
                    if (shadowon_t) {
                      settings.ShadowOn = shadowon_t->value->int32 == 1;
                    }
                        if(settings.ShadowOn){
                          settings.ShadowColor = GColorDarkGray;
                        }
                        else {
                        settings.ShadowColor = GColorBlack;
                        }
                    settings.TextColor1 = GColorWhite;
                    settings.TickColor = GColorWhite;
//                    settings.DateColor = GColorWhite;
                    settings.HoursHandColor = GColorRed;
                    settings.HoursHandBorderColor = GColorRed;
                    settings.MinutesHandColor = GColorGreen;
                    settings.MinutesHandBorderColor = GColorGreen;
                    settings.SecondsHandColor = GColorRed;
                    settings.LineColor = GColorDarkGray;
                    settings.BTQTColor = GColorWhite;
                    settings.FGColor = GColorBlack;
                      theme_settings_changed = true;
                      //  APP_LOG(APP_LOG_LEVEL_DEBUG, "Theme black selected");
          } else if (strcmp(themeselect_t->value->cstring, "bu") == 0) {
              // Set the theme and other settings for "bl"

                    settings.BackgroundColor1 = GColorDukeBlue;
                    if (shadowon_t) {
                      settings.ShadowOn = shadowon_t->value->int32 == 1;
                    }
                        if(settings.ShadowOn){
                          settings.ShadowColor = GColorOxfordBlue;
                        }
                        else {
                        settings.ShadowColor = GColorDukeBlue;
                        }
                    settings.TextColor1 = GColorIcterine;
                    settings.TickColor = GColorIcterine;
//                    settings.DateColor = GColorIcterine;
                    settings.HoursHandColor = GColorChromeYellow;
                    settings.HoursHandBorderColor = GColorChromeYellow;
                    settings.MinutesHandColor = GColorWhite;
                    settings.MinutesHandBorderColor = GColorWhite;
                    settings.SecondsHandColor = GColorChromeYellow;
                    settings.LineColor = GColorIcterine;
                    settings.BTQTColor = GColorWhite;
                    settings.FGColor = GColorDukeBlue;
                      theme_settings_changed = true;
                      //  APP_LOG(APP_LOG_LEVEL_DEBUG, "Theme blue selected");
          } else if (strcmp(themeselect_t->value->cstring, "pl") == 0) {
              // Set the theme and other settings for "bl"

                    settings.BackgroundColor1 = GColorPurple;
                    if (shadowon_t) {
                      settings.ShadowOn = shadowon_t->value->int32 == 1;
                    }
                        if(settings.ShadowOn){
                          settings.ShadowColor = GColorImperialPurple;
                        }
                        else {
                        settings.ShadowColor = GColorPurple;
                        }
                    settings.TextColor1 = GColorWhite;
                    settings.TickColor = GColorBlack;
 //                   settings.DateColor = GColorWhite;
                    settings.HoursHandColor = GColorBlack;
                    settings.HoursHandBorderColor = GColorBlack;
                    settings.MinutesHandColor = GColorBlack;
                    settings.MinutesHandBorderColor = GColorBlack;
                    settings.SecondsHandColor = GColorBlack;
                    settings.LineColor = GColorWhite;
                    settings.BTQTColor = GColorWhite;
                    settings.FGColor = GColorBlack;
                      theme_settings_changed = true;
                      //  APP_LOG(APP_LOG_LEVEL_DEBUG, "Theme purple selected");
          } else if (strcmp(themeselect_t->value->cstring, "gr") == 0) {
              // Set the theme and other settings for "gr"

                    settings.BackgroundColor1 = GColorBlack;
                    if (shadowon_t) {
                      settings.ShadowOn = shadowon_t->value->int32 == 1;
                    }
                        if(settings.ShadowOn){
                          settings.ShadowColor = GColorDarkGreen;
                        }
                        else {
                        settings.ShadowColor = GColorBlack;
                        }
                    settings.TextColor1 = GColorBlack;
                    settings.TickColor = GColorBrightGreen;
//                    settings.DateColor = GColorBrightGreen;
                    settings.HoursHandColor = GColorPastelYellow;
                    settings.HoursHandBorderColor = GColorPastelYellow;
                    settings.MinutesHandColor = GColorBrightGreen;
                    settings.MinutesHandBorderColor = GColorBrightGreen;
                    settings.SecondsHandColor = GColorPastelYellow;
                    settings.LineColor = GColorBlack;
                    settings.BTQTColor = GColorBrightGreen;
                    settings.FGColor = GColorBrightGreen;
                      theme_settings_changed = true;
                      //  APP_LOG(APP_LOG_LEVEL_DEBUG, "Theme black & green selected");
          } else if (strcmp(themeselect_t->value->cstring, "cu") == 0) {
              // Set the theme for "cu" and handle custom colors

                  if (bg_color1_t) {
                    settings.BackgroundColor1 = GColorFromHEX(bg_color1_t->value->int32);
                    settings_changed = true;
                  }
                  if (shadowon_t) {
                    settings.ShadowOn = shadowon_t->value->int32 == 1;

                      if(settings.ShadowOn){
                        if (bg_color2_t) {
                          settings.ShadowColor = GColorFromHEX(bg_color2_t->value->int32);
                          settings_changed = true;
                        }
                      }
                      else {
                      settings.ShadowColor = settings.BackgroundColor1;
                      }
                  }
                  if (text_color1_t) {
                    settings.TextColor1 = GColorFromHEX(text_color1_t->value->int32);
                    //layer_mark_dirty(s_date_battery_logo_layer);
                    layer_mark_dirty(s_fg_layer);
                  }
                  if (text_color2_t) {
                    settings.TickColor = GColorFromHEX(text_color2_t->value->int32);
                    layer_mark_dirty(s_bg_layer);
                    layer_mark_dirty(s_fg_layer);
                  }
                  // if (date_color_t) {
                  //   settings.DateColor = GColorFromHEX(date_color_t->value->int32);
                  //   layer_mark_dirty(s_canvas_layer);
                  //   layer_mark_dirty(s_fg_layer);
                  // }
                  if (hours_color_t) {
                    settings.HoursHandColor = GColorFromHEX(hours_color_t->value->int32);
                    layer_mark_dirty(s_canvas_layer);
                    layer_mark_dirty(s_canvas_second_hand);
                  }
                  if (hours_border_t) {
                    settings.HoursHandBorderColor = GColorFromHEX(hours_border_t->value->int32);
                    layer_mark_dirty(s_canvas_layer);
                  }
                  if (minutes_color_t) {
                    settings.MinutesHandColor = GColorFromHEX(minutes_color_t->value->int32);
                    layer_mark_dirty(s_canvas_layer);
                    layer_mark_dirty(s_canvas_second_hand);
                  }
                  if (minutes_border_t) {
                    settings.MinutesHandBorderColor = GColorFromHEX(minutes_border_t->value->int32);
                    layer_mark_dirty(s_canvas_layer);
                  }
                  if (seconds_color_t) {
                    settings.SecondsHandColor = GColorFromHEX(seconds_color_t->value->int32);
                    layer_mark_dirty(s_canvas_second_hand);
                  }
                  if (line_color_t) {
                    settings.LineColor = GColorFromHEX(line_color_t->value->int32);
                    layer_mark_dirty(s_fg_layer);
                  }
                  if (btqt_color_t) {
                    settings.BTQTColor = GColorFromHEX(btqt_color_t->value->int32);
                    layer_mark_dirty(s_canvas_bt_icon);
                    layer_mark_dirty(s_canvas_qt_icon);
                  }
                   if (fg_color_t) {
                    settings.FGColor = GColorFromHEX(fg_color_t->value->int32);
                  }
                  theme_settings_changed = true;
                //    APP_LOG(APP_LOG_LEVEL_DEBUG, "Theme custom selected");
                }
          }

  if (settings_changed) {
    layer_mark_dirty(s_bg_layer);
    layer_mark_dirty(s_canvas_layer);
    layer_mark_dirty(s_canvas_second_hand);
    layer_mark_dirty(s_fg_layer);
  }

  if (theme_settings_changed) {
    layer_mark_dirty(s_bg_layer);
    layer_mark_dirty(s_canvas_layer);
    layer_mark_dirty(s_canvas_second_hand);
    layer_mark_dirty(s_canvas_qt_icon);
    layer_mark_dirty(s_canvas_bt_icon);
    layer_mark_dirty(s_fg_layer);
  }

  prv_save_settings();

}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  time_t temp = time(NULL);
  prv_tick_time = localtime(&temp);

  // Update hour and minute hands and the date on minute change
  if (units_changed & MINUTE_UNIT) {
    minutes = tick_time->tm_min;
    hours = tick_time->tm_hour % 12;
    s_hours = tick_time->tm_hour;
    //s_day = tick_time->tm_mday;
    
    layer_mark_dirty(s_canvas_layer);
    layer_mark_dirty(s_fg_layer);
    //layer_mark_dirty(s_date_battery_logo_layer);
    if (settings.EnableDate && tick_time->tm_mday != s_day) {
      s_day = tick_time->tm_mday;
      s_weekday = tick_time->tm_wday;
      s_month = tick_time->tm_mon;
    }
  }

  // Update seconds hand on second change, but only if it's visible
  if (showSeconds && (units_changed & SECOND_UNIT)) {
    seconds = tick_time->tm_sec;
    layer_mark_dirty(s_canvas_second_hand);
  }

  // hide or show the seconds hand layer
  layer_set_hidden(s_canvas_second_hand, !(showSeconds && settings.EnableSecondsHand));
}

//draw hour hand
static void draw_fancy_hand_hour(GContext *ctx, int angle, int length, GColor fill_color, GColor border_color) {
    GPoint origin = GPoint(bounds.size.w / 2, bounds.size.h / 2);
    int p1l = 18;
    int p2l = length;
    GPoint p1 = polar_to_point_offset(origin, angle, p1l);
    GPoint p2 = polar_to_point_offset(origin, angle, p2l);
    graphics_context_set_antialiased(ctx, true);

    // Define shadow color
    #if PBL_COLOR
    GColor shadow_color = settings.ShadowColor;
    // Draw the shadow first, with an offset
    GPoint shadow_offset = GPoint(2, 2);
    graphics_context_set_stroke_color(ctx, shadow_color);
    graphics_context_set_stroke_width(ctx, 9);
    graphics_draw_line(ctx, GPoint(origin.x + shadow_offset.x, origin.y + shadow_offset.y), GPoint(p2.x + shadow_offset.x, p2.y + shadow_offset.y));
    #endif

    // Now draw the main hand on top
    graphics_context_set_stroke_color(ctx, border_color);
    graphics_context_set_stroke_width(ctx, 9);
    graphics_draw_line(ctx, origin, p1);
    graphics_draw_line(ctx, p1, p2);
    graphics_context_set_stroke_color(ctx, fill_color);
    graphics_context_set_stroke_width(ctx, 5);
    graphics_draw_line(ctx, p1, p2);

}

static void draw_fancy_hand_min(GContext *ctx, int angle, int length, int back_length, GColor fill_color, GColor border_color) {
    GPoint origin = GPoint(bounds.size.w / 2, bounds.size.h / 2);
    int p1l = back_length;
    int p2l = length;
    GPoint p1 = polar_to_point_offset(origin, angle, p1l);
    GPoint p2 = polar_to_point_offset(origin, angle, p2l);
    graphics_context_set_antialiased(ctx, true);

    #if PBL_COLOR
    // Define shadow color
    GColor shadow_color = settings.ShadowColor;

    // Draw the shadow first, with an offset
    GPoint shadow_offset = PBL_IF_BW_ELSE(GPoint(1,1),GPoint(2, 2));
    graphics_context_set_stroke_color(ctx, shadow_color);
    graphics_context_set_stroke_width(ctx, 7);
    graphics_draw_line(ctx, GPoint(p1.x + shadow_offset.x, p1.y + shadow_offset.y), GPoint(p2.x + shadow_offset.x, p2.y + shadow_offset.y));
    #endif

    // Now draw the main hand on top
    graphics_context_set_stroke_color(ctx, border_color);
    graphics_context_set_stroke_width(ctx, 7);
    //graphics_draw_line(ctx, origin, p1);
    graphics_draw_line(ctx, p1, p2);
    graphics_context_set_stroke_color(ctx, fill_color);
    graphics_context_set_stroke_width(ctx, 3); //was 3
    graphics_draw_line(ctx, p1, p2);
}

///second hand
static void draw_seconds_line_hand(GContext *ctx, int angle, int length, int back_length, GColor color) {
  GPoint origin = GPoint(bounds.size.w / 2, bounds.size.h / 2);
  GPoint p1 = polar_to_point_offset(origin, angle, back_length);
  GPoint p2 = polar_to_point_offset(origin, angle, length);
  graphics_context_set_antialiased(ctx, true);

  #if PBL_COLOR
  // Define shadow color
  GColor shadow_color = settings.ShadowColor;
  // Draw the shadow first, with a small offset
  graphics_context_set_stroke_color(ctx, shadow_color);
  graphics_context_set_stroke_width(ctx, 2); // Same width as the hand
  graphics_draw_line(ctx, GPoint(p1.x + 2, p1.y + 2 ), GPoint(p2.x + 2, p2.y + 2 ));
  #endif

  // Now draw the main second hand on top
  graphics_context_set_stroke_color(ctx, color);
  graphics_context_set_stroke_width(ctx, 2);
  graphics_draw_line(ctx, p1, p2);

}

#if PBL_COLOR
static void draw_center_shadow(GContext *ctx, GColor shadow_color) {
  GPoint origin = GPoint(bounds.size.w / 2, bounds.size.h / 2);
  graphics_context_set_antialiased(ctx, true);

  graphics_context_set_fill_color(ctx, shadow_color);
  graphics_fill_circle(ctx, GPoint(origin.x + 2 , origin.y + 2 ), config.fg_radius);
}
#endif


static void draw_center(GContext *ctx, GColor seconds_color, GColor fg_color) {
  GPoint origin = GPoint(bounds.size.w / 2, bounds.size.h / 2);
  graphics_context_set_antialiased(ctx, true);

  graphics_context_set_fill_color(ctx, seconds_color);
  graphics_fill_circle(ctx, origin, config.fg_radius); //started as 3

  graphics_context_set_fill_color(ctx, fg_color);
  graphics_fill_circle(ctx, origin, config.fg_radius - 2); //started as 3
}


static void draw_major_tick (GContext *ctx, int angle, int length, GColor fill_color, GColor border_color) {
    GPoint origin = GPoint(bounds.size.w / 2, bounds.size.h / 2);

    // The tick starts away from the center
    GPoint p1 = polar_to_point_offset(origin, angle, bounds.size.h / 2 - config.tick_inset_inner);  // 15 was -20
    // The tick ends at a fixed length from p1
    GPoint p2 = polar_to_point_offset(origin, angle, bounds.size.h / 2 - config.tick_inset_outer);  //+was -15 + length

    // Now, draw the main tick on top. It covers the full length.
    graphics_context_set_antialiased(ctx, true);
    graphics_context_set_stroke_color(ctx, border_color);
    graphics_context_set_stroke_width(ctx, 2);  // was 7
    graphics_draw_line(ctx, p1, p2);


}


static void draw_minor_tick(GContext *ctx, GPoint center, GColor border_color) {
 
  graphics_context_set_antialiased(ctx, true);
  graphics_context_set_fill_color(ctx, border_color);
  graphics_fill_circle(ctx, center, 1);  // radius 2 = small dot
}

// Update procedure for the seconds hand layer
static void layer_update_proc_seconds_hand(Layer *layer, GContext *ctx) {
  if (!showSeconds || !prv_tick_time) {
      // Do not draw the second hand if it should be hidden or if time data is not yet available
      return;
    }

  GRect bounds = layer_get_bounds(layer);
  seconds = prv_tick_time->tm_sec;
  //for test & screenshots
  //int
  // seconds = 8;

  if (!settings.EnableSecondsHand || !showSeconds) {
    seconds = 0;
  }

  int seconds_angle = ((double)seconds / 60 * 360) - 90;

  draw_seconds_line_hand(ctx, seconds_angle, bounds.size.w/2 - config.second_hand_a, bounds.size.w/2 - config.second_hand_b,  settings.SecondsHandColor);
 
}

//Update procedure for the Bluetooth Icon (shows when disconnected) layer
static void layer_update_proc_bt(Layer * layer, GContext * ctx){
  GRect bounds = layer_get_bounds(layer);
   minutes = prv_tick_time->tm_min;
   hours = prv_tick_time->tm_hour % 12;

//use this for testing
   // minutes = 30;
   // hours = 9;

      int xPosition;
      int yPosition;
      int textboxwidth;

        xPosition = bounds.size.w/2 - config.fg_radius + config.BTQTxOffset ;
        yPosition = bounds.size.h/2 + config.BTIconYOffset;
        textboxwidth = config.BTQTRectWidth;
      
  GRect BTIconRect =
    GRect(xPosition, yPosition, textboxwidth, 20);

 graphics_context_set_text_color(ctx, settings.BTQTColor);
 graphics_context_set_antialiased(ctx, true);
 graphics_draw_text(ctx, "z", FontBTQTIcons, BTIconRect, GTextOverflowModeFill,GTextAlignmentLeft, NULL);


}

//Update procedure for the QT Icon layer (shows when Quiet time is active)
static void layer_update_proc_qt(Layer * layer, GContext * ctx){

  GRect bounds = layer_get_bounds(layer);
   minutes = prv_tick_time->tm_min;
   hours = prv_tick_time->tm_hour % 12;

//use this for testing
   // minutes = 30;
   // hours = 9;

      int xPosition;
      int yPosition;
      int textboxwidth;

        // Left position
        xPosition = bounds.size.w/2 - config.fg_radius + config.BTQTxOffset;
        yPosition = bounds.size.h/2 + config.QTIconYOffset;
        textboxwidth = config.BTQTRectWidth;
  
     
  GRect QTIconRect =
    GRect(xPosition , yPosition , textboxwidth, 20);

  quiet_time_icon(); //checks whether quiet time is active

  graphics_context_set_text_color(ctx, settings.BTQTColor);
  graphics_context_set_antialiased(ctx, true);
  graphics_draw_text(ctx, "\U0000E061", FontBTQTIcons, QTIconRect, GTextOverflowModeFill,GTextAlignmentLeft, NULL);

}

// Update procedure for the main canvas layer (hour & minute hands)
static void hour_min_hands_canvas_update_proc(Layer *layer, GContext *ctx) {

 GRect bounds = layer_get_bounds(layer);

      #ifdef PBL_COLOR
        draw_center_shadow(ctx, settings.ShadowColor);
      #endif

//use these for live version

   minutes = prv_tick_time->tm_min;
   hours = prv_tick_time->tm_hour % 12;

  #ifdef HOUR
    hours = HOUR;
  #endif

  #ifdef MINUTE
    minutes = MINUTE;
  #endif

   int hours_angle = ((double)hours / 12 * 360) + ((double)minutes / 60 * 360 / 12) + /*((double)seconds / 60 * 360 / 60 / 12)*/  - 90;

  draw_fancy_hand_hour(ctx, hours_angle, bounds.size.w / 2 - config.hour_hand_a, settings.HoursHandColor, settings.HoursHandBorderColor);

   int minutes_angle = ((double)minutes / 60 * 360) + /*((double)seconds / 60 * 360 / 60)*/ - 90;

   draw_fancy_hand_min(ctx, minutes_angle, bounds.size.w / 2 - config.min_hand_a, bounds.size.w / 2 - config.min_hand_b, settings.MinutesHandColor, settings.MinutesHandBorderColor);

  
}

static void fg_update_proc (Layer *layer,GContext *ctx) {

    draw_center(ctx, settings.SecondsHandColor, settings.FGColor);

    GRect bounds = layer_get_bounds(layer);

    FContext fctx;
    fctx_init_context(&fctx, ctx);
    fctx_set_color_bias(&fctx, 0);
    #ifdef PBL_COLOR
    fctx_enable_aa(true);
    #endif

    if(settings.ShowTime){
        fctx_set_fill_color(&fctx, settings.TextColor1);

        int hourdraw;
        char hournow[4];
        if (clock_is_24h_style()) {
            hourdraw = s_hours;
            if (settings.RemoveZero24h) {
                snprintf(hournow, sizeof(hournow), "%d", hourdraw);
            } else {
                snprintf(hournow, sizeof(hournow), "%02d", hourdraw);
            }
        } else {
            if (s_hours == 0 || s_hours == 12) hourdraw = 12;
            else hourdraw = s_hours % 12;
            if (settings.AddZero12h) {
                snprintf(hournow, sizeof(hournow), "%02d", hourdraw);
            } else {
                snprintf(hournow, sizeof(hournow), "%d", hourdraw);
            }
        }

        char minnow[3];
        snprintf(minnow, sizeof(minnow), "%02d", minutes);

        char timedraw[10];
        char local_ampm_string[5];
        char AMPMdraw[5];
        strftime(local_ampm_string, sizeof(local_ampm_string), "%p", prv_tick_time);

        snprintf(timedraw, sizeof(timedraw), "%s:%s", hournow, minnow);
        
        snprintf(AMPMdraw, sizeof(AMPMdraw),"%s", local_ampm_string);  

        FPoint time_pos;
        time_pos.x = INT_TO_FIXED(bounds.size.w / 2);
        time_pos.y = INT_TO_FIXED(bounds.size.h / 2);
        fctx_begin_fill(&fctx);
        fctx_set_text_em_height(&fctx, FCTX_Font, config.time_font_size);
        fctx_set_color_bias(&fctx, 0);
        fctx_set_offset(&fctx, time_pos);
        fctx_draw_string(&fctx, timedraw, FCTX_Font, GTextAlignmentCenter, FTextAnchorMiddle);
        fctx_end_fill(&fctx);

        if(settings.showlocalAMPM && !clock_is_24h_style()){
            fixed_t time_width = fctx_string_width(&fctx, timedraw, FCTX_Font);

            FPoint ampm_pos;
            ampm_pos.x = INT_TO_FIXED(bounds.size.w / 2) + (time_width / 2) + INT_TO_FIXED(2);
            ampm_pos.y = INT_TO_FIXED(bounds.size.h / 2) + INT_TO_FIXED(1);

            fctx_begin_fill(&fctx);
            fctx_set_text_em_height(&fctx, FCTX_Font, config.other_text_font_size);
            fctx_set_color_bias(&fctx, 0);
            fctx_set_offset(&fctx, ampm_pos);
            fctx_draw_string(&fctx, AMPMdraw, FCTX_Font, GTextAlignmentLeft, FTextAnchorMiddle);
            fctx_end_fill(&fctx);
        }
    }

    
      if(settings.EnableDate){
        fctx_begin_fill(&fctx);
        fctx_set_fill_color(&fctx, settings.TextColor1);
        
        const char * sys_locale = i18n_get_system_locale();
        char weekdaydraw[10];
        fetchwday(s_weekday, sys_locale, weekdaydraw);
        
        int datedraw = s_day;
        char datenow[3];
        snprintf(datenow, sizeof(datenow), "%02d", datedraw);

        char monthnow[10];
        fetchmonth(s_month, sys_locale, monthnow);

        fctx_set_text_em_height(&fctx, FCTX_Font, config.other_text_font_size);
        
        FPoint weekday_pos;
        weekday_pos.x = INT_TO_FIXED(bounds.size.w*0.42 - config.xOffset);  //was - 4
        weekday_pos.y = INT_TO_FIXED(bounds.size.h / 2 - config.other_text_font_size - config.yOffset);  //was -5
        fctx_set_offset(&fctx, weekday_pos);  
        fctx_draw_string(&fctx, weekdaydraw, FCTX_Font, GTextAlignmentRight, FTextAnchorBottom);
        //fctx_draw_string(&fctx, "WED", FCTX_Font, GTextAlignmentRight, FTextAnchorBottom); //use for testing

        FPoint month_pos;
        month_pos.x = INT_TO_FIXED(bounds.size.w*0.58 + config.xOffset);  //was +4
        month_pos.y = INT_TO_FIXED(bounds.size.h / 2 - config.other_text_font_size - config.yOffset);  //was -5
        fctx_set_offset(&fctx, month_pos);  
        fctx_draw_string(&fctx, monthnow, FCTX_Font, GTextAlignmentLeft, FTextAnchorBottom);
        //fctx_draw_string(&fctx, "DEC", FCTX_Font, GTextAlignmentLeft, FTextAnchorBottom);  //use for testing

        fctx_set_text_em_height(&fctx, FCTX_Font, config.info_font_size);
        
        FPoint date_pos;
        date_pos.x = INT_TO_FIXED(bounds.size.w / 2);
        date_pos.y = INT_TO_FIXED(bounds.size.h / 2 - config.other_text_font_size - config.yOffsetDate);
        fctx_set_offset(&fctx, date_pos);  
        fctx_draw_string(&fctx, datenow, FCTX_Font, GTextAlignmentCenter, FTextAnchorBottom);  
        //fctx_draw_string(&fctx, "24", FCTX_Font, GTextAlignmentCenter, FTextAnchorBottom);  // use for testing
        fctx_end_fill(&fctx);
      }

      if(settings.EnableBattery){
        int battery_level = battery_state_service_peek().charge_percent;
        char battperc[6];
        snprintf(battperc, sizeof(battperc), "%d", battery_level);
        
        fctx_begin_fill(&fctx);
        fctx_set_fill_color(&fctx, settings.TextColor1);

        fctx_set_text_em_height(&fctx, FCTX_Font, config.info_font_size);
        FPoint battery_pos;
        battery_pos.x = INT_TO_FIXED(bounds.size.w / 2 + config.xOffset);
        battery_pos.y = INT_TO_FIXED(bounds.size.h / 2 + config.other_text_font_size + config.yOffsetBattery);
        fctx_set_offset(&fctx, battery_pos);
        fctx_draw_string(&fctx, battperc, FCTX_Font, GTextAlignmentLeft, FTextAnchorTop);
        //fctx_draw_string(&fctx, "105", FCTX_Font, GTextAlignmentRight, FTextAnchorTop);  //use for testing

        fixed_t battery_width = fctx_string_width(&fctx, battperc, FCTX_Font);

        // Draw % at smaller size, anchored left from a the numbers
        fctx_set_text_em_height(&fctx, FCTX_Font, config.other_text_font_size);

        FPoint percent_pos;
        percent_pos.x = INT_TO_FIXED(bounds.size.w / 2 + config.xOffset) + battery_width;
        percent_pos.y = INT_TO_FIXED(bounds.size.h / 2 + config.other_text_font_size + config.yOffsetPercent);
        fctx_set_offset(&fctx, percent_pos);
        fctx_draw_string(&fctx, "%", FCTX_Font, GTextAlignmentLeft, FTextAnchorTop);     

        fctx_end_fill(&fctx);
      }

    if(settings.EnableLogo){
        FPoint word_pos;
        fctx_begin_fill(&fctx);
        fctx_set_fill_color(&fctx,settings.TextColor1);
        fctx_set_text_em_height(&fctx, FCTX_Font, config.other_text_font_size);

        word_pos.x = INT_TO_FIXED(bounds.size.w / 2 - config.xOffset);
        word_pos.y = INT_TO_FIXED(bounds.size.h / 2 + config.other_text_font_size + config.yOffsetPercent );

        fctx_set_offset(&fctx, word_pos);
        char textdraw [15];
        snprintf(textdraw, sizeof(textdraw), "%s", settings.LogoText);
        fctx_draw_string(&fctx, textdraw, FCTX_Font, GTextAlignmentRight, FTextAnchorTop);
        fctx_end_fill(&fctx);
    }

    fctx_deinit_context(&fctx);

    if(settings.EnableLines){
     graphics_context_set_antialiased(ctx, true);
     graphics_context_set_stroke_width(ctx, 1);
     graphics_context_set_stroke_color(ctx, settings.LineColor);
     GPoint start = GPoint (bounds.size.w / 4, bounds.size.h / 2 + config.time_font_size/2);  //PBL_IF_ROUND_ELSE (GPoint (42,66),GPoint (31,62));
     GPoint end = GPoint (bounds.size.w / 4*3, bounds.size.h / 2 + config.time_font_size/2); //GPoint end = PBL_IF_ROUND_ELSE (GPoint (89,66),GPoint (71,62));
     graphics_draw_line(ctx, start, end);
     GPoint start2 = GPoint (bounds.size.w / 4, bounds.size.h / 2 - config.time_font_size/2 + 4);  //PBL_IF_ROUND_ELSE (GPoint (42,66),GPoint (31,62));
     GPoint end2 = GPoint (bounds.size.w / 4*3, bounds.size.h / 2 - config.time_font_size/2 + 4); //GPoint end = PBL_IF_ROUND_ELSE (GPoint (89,66),GPoint (71,62));
     graphics_draw_line(ctx, start2, end2);

     GPoint start3 = GPoint (bounds.size.w / 2, bounds.size.h / 2 + config.time_font_size/2 + config.Line3yOffset);  //PBL_IF_ROUND_ELSE (GPoint (42,66),GPoint (31,62));
     GPoint end3 = GPoint (bounds.size.w / 2, bounds.size.h/2 + (config.fg_radius - 6)); //GPoint end = PBL_IF_ROUND_ELSE (GPoint (89,66),GPoint (71,62));
     graphics_draw_line(ctx, start3, end3);

     GPoint start4 = GPoint (bounds.size.w *0.42, bounds.size.h / 2 - config.time_font_size/2 - config.Line45yOffset);  //PBL_IF_ROUND_ELSE (GPoint (42,66),GPoint (31,62));
     GPoint end4 = GPoint (bounds.size.w *0.42, bounds.size.h/2 - (config.fg_radius - 10)); //GPoint end = PBL_IF_ROUND_ELSE (GPoint (89,66),GPoint (71,62));
     graphics_draw_line(ctx, start4, end4);

     GPoint start5 = GPoint (bounds.size.w *0.58, bounds.size.h / 2 - config.time_font_size/2 - config.Line45yOffset);  //PBL_IF_ROUND_ELSE (GPoint (42,66),GPoint (31,62));
     GPoint end5 = GPoint (bounds.size.w *0.58, bounds.size.h/2 - (config.fg_radius - 10)); //GPoint end = PBL_IF_ROUND_ELSE (GPoint (89,66),GPoint (71,62));
     graphics_draw_line(ctx, start5, end5);
    }

}

///update procedure for background
static void bg_update_proc(Layer *layer, GContext *ctx) {

  GRect bounds = layer_get_bounds(layer);

  GRect Background =
       GRect(0, 0, bounds.size.w, bounds.size.h);

   graphics_context_set_fill_color(ctx,settings.BackgroundColor1);
   graphics_fill_rect(ctx, Background,0,GCornersAll);

      for (int i = 0; i < 12; i++) {
        //if (i == 6 || i == 12) continue;
        int angle = i * 30 - 90;
        draw_major_tick(ctx, angle, 16, settings.BackgroundColor1, settings.TickColor);
      }

      
      for (int i = 0; i < 60; i++) {
        if (i % 5 == 0) continue;
        int angle = i * 6;
        GPoint origin = GPoint(bounds.size.w /2 , bounds.size.h / 2);
        GPoint center = polar_to_point_offset(origin, angle, bounds.size.h / 2 - config.minor_tick_inset); //was 7
        draw_minor_tick(ctx, center, settings.TickColor);
      }



}


static void prv_window_load(Window *window) {
  time_t temp = time(NULL);
  prv_tick_time = localtime(&temp);
  s_day = prv_tick_time->tm_mday;
  s_weekday = prv_tick_time->tm_wday;
  minutes = prv_tick_time->tm_min;
  hours = prv_tick_time->tm_hour % 12;
  s_hours = prv_tick_time->tm_hour;
  s_month = prv_tick_time->tm_mon;
  

  Layer *window_layer = window_get_root_layer(window);
  bounds = layer_get_bounds(window_layer);

  Layer *seconds_root_layer = window_get_root_layer(window);
  bounds_seconds = layer_get_bounds(seconds_root_layer);

  // Load fctx ffonts
    FCTX_Font =  ffont_create_from_resource(RESOURCE_ID_DIN_CONDENSED_FFONT);
    
    FontBTQTIcons = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_DRIPICONS_16));

  connection_service_subscribe((ConnectionHandlers){
    .pebble_app_connection_handler = bluetooth_vibe_icon
  });

  // Subscribe to the correct tick service based on settings
    if (settings.EnableSecondsHand) {
        if (settings.SecondsVisibleTime == 135) {
        tick_timer_service_subscribe(SECOND_UNIT, tick_handler);
        } else {
        tick_timer_service_subscribe(SECOND_UNIT, tick_handler);
        s_timeout_timer = app_timer_register(1000*settings.SecondsVisibleTime, timeout_handler,NULL);
        accel_tap_service_subscribe(accel_tap_handler);
        }
    }
    else {
    tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
    }
    showSeconds = settings.EnableSecondsHand;

  //create layers
  s_bg_layer = layer_create(bounds);
  s_canvas_second_hand = layer_create(bounds_seconds);
  s_canvas_qt_icon = layer_create(bounds);
     quiet_time_icon();
  s_canvas_bt_icon = layer_create(bounds);
    bool is_connected = connection_service_peek_pebble_app_connection();
    layer_set_hidden(s_canvas_bt_icon, is_connected);
  s_canvas_layer = layer_create(bounds);
  s_fg_layer = layer_create(bounds);

  // Change the order here
  layer_add_child(window_layer, s_bg_layer); //backforound, circles, major tick shoadow &tickmask
  layer_add_child(window_layer, s_canvas_layer);  //hour and minute hands
  layer_add_child(window_layer, s_canvas_second_hand);  //second hand
  layer_add_child(window_layer, s_fg_layer); //digital content layer on top
  layer_add_child(window_layer, s_canvas_bt_icon);
  layer_add_child(window_layer, s_canvas_qt_icon);
  

  bluetooth_vibe_icon(connection_service_peek_pebble_app_connection());

  layer_set_update_proc(s_bg_layer, bg_update_proc);
  layer_set_update_proc(s_canvas_second_hand, layer_update_proc_seconds_hand);
  layer_set_update_proc(s_canvas_bt_icon, layer_update_proc_bt);
  layer_set_update_proc(s_canvas_qt_icon, layer_update_proc_qt);
  layer_set_update_proc(s_canvas_layer, hour_min_hands_canvas_update_proc);
  layer_set_update_proc(s_fg_layer, fg_update_proc);
  


}


static void prv_window_unload(Window *window) {
  if (s_timeout_timer) {
    app_timer_cancel(s_timeout_timer);
  }
  accel_tap_service_unsubscribe();
  connection_service_unsubscribe();
  battery_state_service_unsubscribe();
  tick_timer_service_unsubscribe();
  layer_destroy(s_canvas_layer);
  layer_destroy(s_bg_layer);
  layer_destroy(s_fg_layer);
  layer_destroy(s_canvas_second_hand);
  layer_destroy(s_canvas_bt_icon);
  layer_destroy(s_canvas_qt_icon);
  ffont_destroy(FCTX_Font);
  fonts_unload_custom_font(FontBTQTIcons);
}

static void prv_init(void) {
  prv_load_settings();

  // Open AppMessage and set the message handler
  app_message_open(1024, 1024);
  app_message_register_inbox_received(prv_inbox_received_handler);

  s_window = window_create();
  window_set_window_handlers(s_window, (WindowHandlers) {
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