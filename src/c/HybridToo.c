#include <pebble.h>
#include "HybridToo.h"
#include "utils/weekday.h"
#include "utils/month.h"
#include "utils/MathUtils.h"
#include "utils/Weather.h"
#include "utils/UIConfig.h"
#ifndef PBL_BW
#include <pebble-fctx/fctx.h>
#include <pebble-fctx/fpath.h>
#include <pebble-fctx/ffont.h>
#endif

#define SECONDS_TICK_INTERVAL_MS 1000

// Layers
static Window *s_window;
static Layer *s_canvas_layer;
static Layer *s_bg_layer;
static Layer *s_canvas_second_hand;
static Layer *s_fg_layer;
static Layer *s_weather_layer_1;
//#if defined (PBL_PLATFORM_EMERY) || defined (PBL_PLATFORM_GABBRO) || defined (PBL_PLATFORM_DIORITE) || defined (PBL_PLATFORM_FLINT)
#ifndef PBL_PLATFORM_APLITE
static Layer *s_weather_layer_2;
#endif

 static GRect bounds;

// Fonts
static GFont FontBTQTIcons;
static GFont FontWeatherIcons;
static GFont FontWeatherIconsSmall;
static GFont small_font;
static GFont medium_font;
static GFont small_medium_font;

#if defined PBL_BW || defined PBL_PLATFORM_BASALT || defined PBL_PLATFORM_CHALK
static GFont aplite_font;

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
static ClaySettings settings;
static bool showSeconds;
static AppTimer *s_timeout_timer;
static AppTimer *s_weather_timeout_timer;

static int s_countdown = 30;
static int s_loop = 0;
static int showWeather = 0;
static char  citistring[15];
static int s_uvmax_level __attribute__((unused)), s_uvnow_level __attribute__((unused));

#ifdef PBL_PLATFORM_EMERY
const UIConfig __attribute__((section(".rodata"))) config = {
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
  .Line6yOffset = 17,
  .Line7yOffset = 22,
//  .corner_radius_handsrect = 20,
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
  // .hourhandrect_w = 72,
  // .hourhandrect_h = 86,
  .hourhandwidth = 9,
  .hour_hand_a_round = 12,
  .min_hand_a_round = 2,
  .min_hand_b_round = 17,
  .second_hand_a_round = 0,
  .second_hand_b_round = 28,
  .tick_inset_inner_round = 18,
  .tick_inset_outer_round = 2,
  .minor_tick_inset_round = 15,
 // .RainRect = {{{25,106},{50,50}}}, //ok
  // .SunsetRect = {{{133,200-4+2},{200/3,36}}}, //ok
  // .SunriseRect = {{{200/6,200-4+2},{200/3,36}}}, //ok
  // .MoonRect = {{{100,105+2+2},{100,76}}}, //ok
  // .StepsRect = {{{100,156+5},{100,36}}}, //ok
  // .SunsetIconRect = {{{100,200},{200/6,52}}}, //ok
  // .SunriseIconRect = {{{0,200},{200/6,52}}}, //ok
//   .IconNowRect = {{{0+22+4,105+2+2-43+23+12},{100,76}}}, //ok
//   .IconForeRect = {{{100-17-7,105+2+2-43+23+12},{100,76}}}, //ok
//   .WindKtsRect = {{{25+36-45+14,200-4+2-126+6-11},{75,36}}}, //ok
//   .WindForeKtsRect = {{{125-34+5,200-4+2-129-2},{75,36}}}, //ok
//   .TempRect = {{{-5+50-23,156-1-109+75-2+22},{111,36}}}, //ok
//  // .PrecipRect = {{{0,161},{100,36}}}, //ok
//   .TempForeRect = {{{100-50+27,161-102+61-2+22},{100,36}}}, //ok
//   .WindDirNowRect = {{{0+60-9,185+8-143-4},{200/6,52}}}, //ok
//   .WindDirForeRect = {{{100+8+8,185+8-143-4},{200/6,52}}}, //ok
 // .RainRateRect = {{{100,198},{100,36}}}, //ok

  .UVDayValueRect = {{{57,97},{25,25}}},     //UVI value daily forecast max
  .uv_arc_bounds = {{{51,98},{37,37}}},        //UV arc, right of centre, middle row
  .uv_arc_bounds_max = {{{49,96},{41,41}}},    //UVI daily forecast maximum
  .uv_arc_bounds_now = {{{46,93},{47,47}}},    //UVI Current
  .uv_icon = {{{39+ 35,71-3+56},{45,20}}}, 

  .RainDayValueRect = {{{119-1,97+7+6},{25,25}}},     //UVI value daily forecast max
  .Rain1hValueRect = {{{119-1,97},{25,25}}},     //UVI value daily forecast max
  .Rain_arc_bounds = {{{112,98},{37,37}}},        //UV arc, right of centre, middle row
  .Rain_arc_bounds_max = {{{110,96},{41,41}}},    //UVI daily forecast maximum
  .Rain_arc_bounds_now = {{{107,93},{47,47}}},    //UVI Current
  .Rain_icon = {{{116-35,71-3+27},{45,20}}}, 

  .TopTopLeftRect = {{{39,50-5},{45,21}}},            //Wind direction current
  .TopTopRightRect =  {{{116,50-5},{45,21}}},         //Wind direction forecast


  .TopLeftRect ={{{39,71-3},{45,20}}},                //Wind Speed current
  .TopMiddleRect =  {{{85,50+4},{30,41}}},            //Moonphase
  .TopRightRect = {{{116,71-3},{45,20}}},             //Wind speed forecast
  .MiddleLeftRect ={{{39+3,92+8-2},{61-2,49}}},         //Current Weather Condition Icon
  .MiddleRightRect ={{{100,92+8-2},{61,49}}},           //Forecast Weather Condition Icon

  .BottomLeftRect1 = {{{50-2,142-2-3},{50+2,20}}},    //current temperature
  .BottomLeftRect = {{{50,142-2},{50,20}}},           //sunrise time
  .BottomRightRect1 ={{{100+1,142-2},{50+5,20}}},     //forecast high/low temperature
  .BottomRightRect ={{{100+1,142-2},{50,20}}},        //sunset time
  .BottomBottomLeftRect1 ={{{50,162},{50,21}}},       //sunrise icon
  .BottomBottomRightRect ={{{100,162},{50,21}}}       //sunset icon
};
#elif defined(PBL_PLATFORM_GABBRO)
const UIConfig __attribute__((section(".rodata"))) config = {
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
  .Line6yOffset = 23,
  .Line7yOffset = 23,
  .hourhandwidth = 9,
 // .RainRect = {{{18,78},{36,36}}},
  // .SunsetRect = {{{96,147},{144/3,27}}},
  // .SunriseRect = {{{144/6,144+3},{144/3,27}}},
  // .MoonRect = {{{72,77},{72,56}}},
 // .StepsRect = {{{72,115},{72,27}}},
//   .SunsetIconRect = {{{72,148},{144/6,38}}},
//   .SunriseIconRect = {{{0,148},{144/6,38}}},
//   .IconNowRect = {{{0,77},{72,56}}},
//   .IconForeRect = {{{72,77},{72,56}}},
//   .WindKtsRect = {{{18,146},{54,27}}},
//   .WindForeKtsRect = {{{90,146},{54,27}}},
//   .TempRect = {{{-4,115},{80,27}}},
//  // .PrecipRect = {{{0,119},{72,27}}},
//   .TempForeRect = {{{72,119},{72,27}}},
//   .WindDirNowRect = {{{0,136},{144/6,38}}},
//   .WindDirForeRect = {{{72,136},{144/6,38}}},
 // .RainRateRect = {{{72,146},{72,27}}},

  .UVDayValueRect = {{{57+29,98+13},{25,25}}},     //UVI value daily forecast max
  .uv_arc_bounds = {{{51+29,99+13},{37,37}}},        //UV arc, right of centre, middle row
  .uv_arc_bounds_max = {{{49+29,97+13},{41,41}}},    //UVI daily forecast maximum
  .uv_arc_bounds_now = {{{46+29,94+13},{47,47}}},    //UVI Current
  .uv_icon = {{{39+ 8+29,71-3+58+13+6},{45,20}}}, 

  .RainDayValueRect = {{{119-1+29+4,97+7+6+13},{25,25}}},     //UVI value daily forecast max
  .Rain1hValueRect = {{{119-1+29+4,97+13},{25,25}}},     //UVI value daily forecast max
  .Rain_arc_bounds = {{{112+29+4,99+13},{37,37}}},        //UV arc, right of centre, middle row
  .Rain_arc_bounds_max = {{{110+29+4,97+13},{41,41}}},    //UVI daily forecast maximum
  .Rain_arc_bounds_now = {{{107+29+4,94+13},{47,47}}},    //UVI Current
  .Rain_icon = {{{116-8+29+4,71-3+58+13+6},{45,20}}}, 

  .TopTopLeftRect = {{{50+23,50-5+10},{34,21}}},      //ok      //Wind direction current
  .TopTopRightRect =  {{{116+38,50-5+10},{34,21}}},   //ok      //Wind direction forecast
 
  .TopLeftRect ={{{39+23,71-3+10},{45,20}}},           //ok     //Wind Speed current
  .TopMiddleRect =  {{{85+30,50+4+11},{30,41}}},       //ok     //Moonphase
  .TopRightRect = {{{116+38,71-3+10},{45,20}}},        //ok     //Wind speed forecast
  .MiddleLeftRect ={{{39+3+23,92+8+15},{61-2,49}}},   //ok      //Current Weather Condition Icon
  .MiddleRightRect ={{{100+35,92+8+15},{61,49}}},     //ok      //Forecast Weather Condition Icon

  .BottomLeftRect1 = {{{50-2+23,142-2-3+23},{50+2,20}}},  //ok  //current temperature
  .BottomLeftRect = {{{50+23,142-2+22},{50,20}}},          //ok //sunrise time
  .BottomRightRect1 ={{{100+1+36,142-2+23},{50+5,20}}},   //ok  //forecast high/low temperature
  .BottomRightRect ={{{100+1+34+4,142-2+22},{50,20}}},        //sunset time
  .BottomBottomLeftRect1 ={{{50+23,162+22},{50,21}}},     //ok  //sunrise icon
  .BottomBottomRightRect ={{{100+34+4,162+22},{50,21}}}    //ok   //sunset icon
};

#elif defined(PBL_ROUND)
const UIConfig __attribute__((section(".rodata"))) config = {
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
  .time_font_size = 46, //was 46
  .info_font_size = 24,
  .other_text_font_size = 16,
  .Line45yOffset = 0,
  .Line3yOffset = 4,
  .Line6yOffset = 23,
  .Line7yOffset = 23,
  .hourhandwidth = 7,
 // .RainRect = {{{49,106}, {32,32}}}, // The single GRect at index 0
//   .SunsetRect = {{{122,88},{61,14}}},
//   .SunriseRect = {{{-3,88},{61,14}}},
//   .MoonRect = {{{79,101},{72,56}}},
//  // .StepsRect = {{{82,138},{74,30}}},
//   .SunsetIconRect = {{{143,111},{24,24}}},
//   .SunriseIconRect = {{{12,111},{24,24}}},
//   .IconNowRect = {{{-28, 99},{180,32}}},
//   .IconForeRect = {{{29,99},{180,32}}},
//   .WindKtsRect = {{{-19,90},{86,30}}},
//   .WindForeKtsRect = {{{114,90},{90-4,30}}},
//   .TempRect = {{{19,135},{90,30}}},
 // .PrecipRect = {{{28,138},{74,30}}},
  // .TempForeRect = {{{77,138},{90,30}}},
  // .WindDirNowRect = {{{-21,103},{90,32}}},
  // .WindDirForeRect = {{{112,103},{90,32}}},
 // .RainRateRect = {{{122,88},{61,14}}},

  .UVDayValueRect = {{{45-7+18+1,79-20+8+3+4+4},{20,14}}},     //UVI value daily forecast max
  .uv_arc_bounds = {{{37+18,87-20+8+4},{24,24}}},        //UV arc, right of centre, middle row
  .uv_arc_bounds_max = {{{37-2+18,87-2-20+8+4},{24+4,24+4}}},    //UVI daily forecast maximum
  .uv_arc_bounds_now = {{{37-5+18,87-5-20+8+4},{24+10,24+10}}},    //UVI Current
  .uv_icon = {{{39+ 35-26-1+17,71-3+56-34-1+8},{45,20}}}, 

  .RainDayValueRect = {{{85+2+18,79-2+4},{20,14}}},     //UVI value daily forecast max
  .Rain1hValueRect = {{{180,180},{25,25}}},     //UVI value daily forecast max
  .Rain_arc_bounds = {{{93-14+5+18,87-20+8+4},{24,24}}},        //UV arc, right of centre, middle row
  .Rain_arc_bounds_max = {{{93-2-14+5+18,87-2-20+8+4},{24+4,24+4}}},    //UVI daily forecast maximum
  .Rain_arc_bounds_now = {{{93-5-14+5+18,87-5-20+8+4},{24+10,24+10}}},    //UVI Current
  .Rain_icon = {{{116-35-39+9+18,71-3+27-24+3},{45,20}}}, 

  .TopTopLeftRect = {{{50-36+21+9,50-5-10+2+3},{34,21}}},        //ok    //Wind direction current
  .TopTopRightRect =  {{{116-18-21+27,50-5-10+2+3},{34,21}}},     //ok    //Wind direction forecast

  .TopLeftRect ={{{39-14+13+1,71-3-17+3},{45,20}}},           //ok     //Wind Speed current
  .TopMiddleRect =  {{{85-27+17+1,50+4-9+3},{30,41}}},        //ok    //Moonphase
  .TopRightRect = {{{116-42+23,71-3-17+3},{45,20}}},        //ok     //Wind speed forecast
  .MiddleLeftRect ={{{39+3-18+14,92+8-27+1+5},{61-2,49}}},    //ok     //Current Weather Condition Icon
  .MiddleRightRect ={{{100-38+22,92+8-27+1+5},{61,49}}},      //ok     //Forecast Weather Condition Icon

  .BottomLeftRect1 = {{{50-2-20+14+3,142-2-3-40+2+9+1},{50+2,20}}}, //ok   //current temperature
  .BottomLeftRect = {{{50-20+13,142-2-38+1+10},{50,20}}},           //sunrise time
  .BottomRightRect1 ={{{100+1-37+21,142-2-38+2+9+1},{50+5,20}}},  //ok   //forecast high/low temperature
  .BottomRightRect ={{{100+1-35+23,142-2-38+1+10},{50,20}}},        //sunset time
  .BottomBottomLeftRect1 ={{{50-20+13,162-44+1+10},{50,21}}},       //sunrise icon
  .BottomBottomRightRect ={{{100-35+23,162-44+1+10},{50,21}}}       //sunset icon
};


#else // Basalt, Aplite, Diorite, Flint
const UIConfig __attribute__((section(".rodata"))) config = {
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
  .Line6yOffset = 23,
  .Line7yOffset = 23,
 // .corner_radius_handsrect = 15,
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
  // .hourhandrect_w = 52,  //72
  // .hourhandrect_h = 64,   //86
  .hourhandwidth = 7,
  .hour_hand_a_round = 8,
  .min_hand_a_round = 2,
  .min_hand_b_round = 12,
  .second_hand_a_round = 0,
  .second_hand_b_round = 20,
  .tick_inset_inner_round = 14,
  .tick_inset_outer_round = 2,
  .minor_tick_inset_round = 11,
 // .RainRect = {{{18,78},{36,36}}},
//   .SunsetRect = {{{96,147},{144/3,27}}},
//   .SunriseRect = {{{144/6,144+3},{144/3,27}}},
//   .MoonRect = {{{72,77},{72,56}}},
//  // .StepsRect = {{{72,115},{72,27}}},
//   .SunsetIconRect = {{{72,148},{144/6,38}}},
//   .SunriseIconRect = {{{0,148},{144/6,38}}},
//   .IconNowRect = {{{0,77},{72,56}}},
//   .IconForeRect = {{{72,77},{72,56}}},
//   .WindKtsRect = {{{18,146},{54,27}}},
//   .WindForeKtsRect = {{{90,146},{54,27}}},
//   .TempRect = {{{-4,115+1},{80,27}}},
//  .PrecipRect = {{{0,119},{72,27}}},
  // .TempForeRect = {{{72,119+1},{72,27}}},
  // .WindDirNowRect = {{{0,136},{144/6,38}}},
  // .WindDirForeRect = {{{72,136},{144/6,38}}},
//  .RainRateRect = {{{72,146},{72,27}}},

  .UVDayValueRect = {{{45-7,79-20+8+3+4},{20,14}}},     //UVI value daily forecast max
  .uv_arc_bounds = {{{37,87-20+8},{24,24}}},        //UV arc, right of centre, middle row
  .uv_arc_bounds_max = {{{37-2,87-2-20+8},{24+4,24+4}}},    //UVI daily forecast maximum
  .uv_arc_bounds_now = {{{37-5,87-5-20+8},{24+10,24+10}}},    //UVI Current
  .uv_icon = {{{39+ 35-26-1,71-3+56-34-1},{45,20}}}, 

  .RainDayValueRect = {{{85+2,79-2},{20,14}}},     //UVI value daily forecast max
  .Rain1hValueRect = {{{144,168},{25,25}}},     //UVI value daily forecast max
  .Rain_arc_bounds = {{{93-14+5,87-20+8},{24,24}}},        //UV arc, right of centre, middle row
  .Rain_arc_bounds_max = {{{93-2-14+5,87-2-20+8},{24+4,24+4}}},    //UVI daily forecast maximum
  .Rain_arc_bounds_now = {{{93-5-14+5,87-5-20+8},{24+10,24+10}}},    //UVI Current
  .Rain_icon = {{{116-35-39+9,71-3+27-24},{45,20}}}, 

  .TopTopLeftRect = {{{50-36+21-4,50-5-10+2},{34,21}}},        //ok    //Wind direction current
  .TopTopRightRect =  {{{116-18-21+4,50-5-10+2},{34,21}}},     //ok    //Wind direction forecast

  .TopLeftRect ={{{39-14,71-3-17},{45,20}}},           //ok     //Wind Speed current
  .TopMiddleRect =  {{{85-27,50+4-9},{30,41}}},        //ok    //Moonphase
  .TopRightRect = {{{116-42,71-3-17},{45,20}}},        //ok     //Wind speed forecast
  .MiddleLeftRect ={{{39+3-18,92+8-27+1},{61-2,49}}},    //ok     //Current Weather Condition Icon
  .MiddleRightRect ={{{100-38,92+8-27+1},{61,49}}},      //ok     //Forecast Weather Condition Icon

  .BottomLeftRect1 = {{{50-2-20,142-2-3-40+2},{50+2,20}}}, //ok   //current temperature
  .BottomLeftRect = {{{50-20,142-2-38+1},{50,20}}},           //sunrise time
  .BottomRightRect1 ={{{100+1-37,142-2-38+2},{50+5,20}}},  //ok   //forecast high/low temperature
  .BottomRightRect ={{{100+1-35,142-2-38+1},{50,20}}},        //sunset time
  .BottomBottomLeftRect1 ={{{50-20,162-44+1},{50,21}}},       //sunrise icon
  .BottomBottomRightRect ={{{100-35,162-44+1},{50,21}}}       //sunset icon
};
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
    }
    layer_mark_dirty(s_fg_layer);
    layer_mark_dirty(s_weather_layer_1);
    //#if defined (PBL_PLATFORM_EMERY) || defined (PBL_PLATFORM_GABBRO) || defined (PBL_PLATFORM_DIORITE) || defined (PBL_PLATFORM_FLINT)
    #ifndef PBL_PLATFORM_APLITE
    layer_mark_dirty(s_weather_layer_2);
    #endif
}

static void battery_callback(BatteryChargeState state) {
    s_battery_level = state.charge_percent;
    snprintf(s_batt_text, sizeof(s_batt_text), "%d%%", s_battery_level);
    layer_mark_dirty(s_weather_layer_1);
    //#if defined (PBL_PLATFORM_EMERY) || defined (PBL_PLATFORM_GABBRO) || defined (PBL_PLATFORM_DIORITE) || defined (PBL_PLATFORM_FLINT)
    #ifndef PBL_PLATFORM_APLITE
    layer_mark_dirty(s_weather_layer_2);
    #endif
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
        graphics_context_set_text_color(ctx, settings.TextColor1);
        graphics_draw_text(ctx, "z", FontBTQTIcons, GRect(x, y, config.BTQTRectWidth, 20), GTextOverflowModeFill, GTextAlignmentLeft, NULL);
    }
    if (quiet_time_is_active()) {
        int x = bounds.size.w / 2 - config.fg_radius + config.QTxOffset;
        int y = bounds.size.h / 2 + config.QTIconYOffset;
        graphics_context_set_text_color(ctx, settings.TextColor1);
        graphics_draw_text(ctx, "\U0000E061", FontBTQTIcons, GRect(x, y, config.BTQTRectWidth, 20), GTextOverflowModeFill, GTextAlignmentLeft, NULL);
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
// Centre drawing (round face style only)
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
  settings.UVMaxColor = GColorWhite;
  settings.UVNowColor = PBL_IF_BW_ELSE(GColorWhite,GColorRed);
  settings.UVArcColor = GColorLightGray;
  settings.WeatherUnit = 0;
  // memset(settings.WeatherProv, 0, sizeof(settings.WeatherProv));
  // snprintf(settings.WeatherProv, sizeof(settings.WeatherProv), "%s", "ds");
  memset(settings.WindUnit, 0, sizeof(settings.WindUnit));
  //#if defined (PBL_PLATFORM_EMERY) || defined (PBL_PLATFORM_GABBRO) || defined (PBL_PLATFORM_DIORITE) || defined (PBL_PLATFORM_FLINT)
  #ifndef PBL_PLATFORM_APLITE
  memset(settings.RainUnit, 0, sizeof(settings.RainUnit));
  snprintf(settings.RainUnit, sizeof(settings.RainUnit), "%s", "mm");
  memset(settings.PressureUnit, 0, sizeof(settings.PressureUnit));
  snprintf(settings.PressureUnit, sizeof(settings.PressureUnit), "%s", "mb");
  #endif
  snprintf(settings.WindUnit, sizeof(settings.WindUnit), "%s", "kts");
  settings.UpSlider = 30;
  settings.HealthOn = false;
  settings.UseWeather = false;
}

static void prv_load_settings(void) {
  prv_default_settings();
  int stored_size = persist_get_size(SETTINGS_KEY);
  if (stored_size == (int)sizeof(settings)) {
    persist_read_data(SETTINGS_KEY, &settings, sizeof(settings));
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Settings loaded: size=%d HealthOn=%d", stored_size, (int)settings.HealthOn);
  } else {
    APP_LOG(APP_LOG_LEVEL_WARNING, "Settings size mismatch (stored=%d expected=%d), using defaults",
            stored_size, (int)sizeof(settings));
    prv_save_settings();
  }
     APP_LOG(APP_LOG_LEVEL_DEBUG, "sizeof(ClaySettings)=%d SETTINGS_KEY=%d", 
           (int)sizeof(ClaySettings), (int)SETTINGS_KEY);
}

// ---------------------------------------------------------------------------
// Event handlers
// ---------------------------------------------------------------------------

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  prv_tick_time = *tick_time;

    update_cached_strings();
    
  if (units_changed & MINUTE_UNIT) {
    if (s_countdown == 0){
      //Reset weather update countdown
      s_countdown = settings.UpSlider;
    } else{
      s_countdown = s_countdown - 1;
    }
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Countdown to update %d loop is %d", s_countdown, s_loop);
    minutes = tick_time->tm_min;
    hours = tick_time->tm_hour % 12;
    s_hours = tick_time->tm_hour;
    layer_mark_dirty(s_canvas_layer);
    layer_mark_dirty(s_fg_layer);
    layer_mark_dirty(s_weather_layer_1);
    //#if defined (PBL_PLATFORM_EMERY) || defined (PBL_PLATFORM_GABBRO) || defined (PBL_PLATFORM_DIORITE) || defined (PBL_PLATFORM_FLINT)
    #ifndef PBL_PLATFORM_APLITE
    layer_mark_dirty(s_weather_layer_2);
    #endif
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

  if (s_countdown == 0 || s_countdown == 5){
      APP_LOG(APP_LOG_LEVEL_DEBUG, "countdown is 0, updated weather at %d", tick_time -> tm_min);
      s_loop = 0;
      // Begin dictionary
      DictionaryIterator * iter;
      app_message_outbox_begin( & iter);
      // Add a key-value pair
      dict_write_uint8(iter, 0, 0);
      // Send the message!
      app_message_outbox_send();
  }

}

static void update_weather_view_visibility() {
 // If UseWeather was just turned off, force the view back to main (0)
  if (!settings.UseWeather) {
    showWeather = 0;
  }
  //#if defined (PBL_PLATFORM_EMERY) || defined (PBL_PLATFORM_GABBRO) || defined (PBL_PLATFORM_DIORITE) || defined (PBL_PLATFORM_FLINT)
  #ifndef PBL_PLATFORM_APLITE
  if (showWeather == 0) {
    layer_set_hidden(s_fg_layer, false);
    layer_set_hidden(s_weather_layer_1, true);
    layer_set_hidden(s_weather_layer_2, true);
  } else if (showWeather == 1) {
    layer_set_hidden(s_fg_layer, true);
    layer_set_hidden(s_weather_layer_1, false);
    layer_set_hidden(s_weather_layer_2, true);
  } else { // showWeather == 2
    layer_set_hidden(s_fg_layer, true);
    layer_set_hidden(s_weather_layer_1, true);
    layer_set_hidden(s_weather_layer_2, false);
  }
  #else
  if (showWeather == 0) {
    layer_set_hidden(s_fg_layer, false);
    layer_set_hidden(s_weather_layer_1, true);
  } else {
    layer_set_hidden(s_fg_layer, true);
    layer_set_hidden(s_weather_layer_1, false);
  } 
  #endif
}

static void timeout_handler(void *context) {
  showSeconds = false;
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  layer_mark_dirty(s_canvas_second_hand);
  s_timeout_timer = NULL;
  
}

static void weather_timeout_handler(void *context) {
  s_weather_timeout_timer = NULL;
  showWeather = 0;
  update_weather_view_visibility();
}

// static void accel_tap_handler(AccelAxisType axis, int32_t direction) {

//   APP_LOG(APP_LOG_LEVEL_DEBUG, "accel tap registered");

//   if (settings.EnableSecondsHand && settings.SecondsVisibleTime < 135) {
//     if (s_timeout_timer) {
//       app_timer_cancel(s_timeout_timer);
//       s_timeout_timer = NULL;
//     }
//     if (!showSeconds) {
//       tick_timer_service_subscribe(SECOND_UNIT, tick_handler);
//     }
//     showSeconds = true;
//     s_timeout_timer = app_timer_register(SECONDS_TICK_INTERVAL_MS * settings.SecondsVisibleTime, timeout_handler, NULL);
//     layer_mark_dirty(s_canvas_second_hand);
//   }

// //#if defined (PBL_PLATFORM_EMERY) || defined (PBL_PLATFORM_GABBRO) || defined (PBL_PLATFORM_DIORITE) || defined (PBL_PLATFORM_FLINT)
// #ifndef PBL_PLATFORM_APLITE  
//   if(settings.UseWeather){
//     APP_LOG(APP_LOG_LEVEL_DEBUG, "showWeather v1 = %d", showWeather);
//     if(showWeather >= 2) {
//         showWeather = 0;
//         APP_LOG(APP_LOG_LEVEL_DEBUG, "showWeather v2= %d", showWeather);
//        }
//     else{
//       showWeather = showWeather + 1;
//       APP_LOG(APP_LOG_LEVEL_DEBUG, "showWeather v3= %d", showWeather);
//     }
//   }
//   else{
//     showWeather = 0;
//     APP_LOG(APP_LOG_LEVEL_DEBUG, "showWeather v4= %d", showWeather);
//   } 
// #else
//   if(settings.UseWeather){
//     APP_LOG(APP_LOG_LEVEL_DEBUG, "showWeather v1 = %d", showWeather);
//     if(showWeather >= 1) {
//         showWeather = 0;
//         APP_LOG(APP_LOG_LEVEL_DEBUG, "showWeather v2= %d", showWeather);
//        }
//     else{
//       showWeather = showWeather + 1;
//       APP_LOG(APP_LOG_LEVEL_DEBUG, "showWeather v3= %d", showWeather);
//     }
//   }
//   else{
//     showWeather = 0;
//     APP_LOG(APP_LOG_LEVEL_DEBUG, "showWeather v4= %d", showWeather);
//   } 
// #endif

//   update_weather_view_visibility();

  
// }

static void accel_tap_handler(AccelAxisType axis, int32_t direction) {

  APP_LOG(APP_LOG_LEVEL_DEBUG, "accel tap registered");

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

//#if defined (PBL_PLATFORM_EMERY) || defined (PBL_PLATFORM_GABBRO) || defined (PBL_PLATFORM_DIORITE) || defined (PBL_PLATFORM_FLINT)
#ifndef PBL_PLATFORM_APLITE  
  if (settings.UseWeather) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "showWeather v1 = %d", showWeather);
    if (showWeather >= 2) {
      showWeather = 0;
      APP_LOG(APP_LOG_LEVEL_DEBUG, "showWeather v2= %d", showWeather);
    } else {
      showWeather = showWeather + 1;
      APP_LOG(APP_LOG_LEVEL_DEBUG, "showWeather v3= %d", showWeather);
    }
  } else {
    showWeather = 0;
    APP_LOG(APP_LOG_LEVEL_DEBUG, "showWeather v4= %d", showWeather);
  }
#else
  if (settings.UseWeather) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "showWeather v1 = %d", showWeather);
    if (showWeather >= 1) {
      showWeather = 0;
      APP_LOG(APP_LOG_LEVEL_DEBUG, "showWeather v2= %d", showWeather);
    } else {
      showWeather = showWeather + 1;
      APP_LOG(APP_LOG_LEVEL_DEBUG, "showWeather v3= %d", showWeather);
    }
  } else {
    showWeather = 0;
    APP_LOG(APP_LOG_LEVEL_DEBUG, "showWeather v4= %d", showWeather);
  }
#endif

  // Manage weather screen timeout
  if (s_weather_timeout_timer) {
    app_timer_cancel(s_weather_timeout_timer);
    s_weather_timeout_timer = NULL;
  }
  if (showWeather != 0) {
    s_weather_timeout_timer = app_timer_register(31000, weather_timeout_handler, NULL);
  }

  update_weather_view_visibility();
}

// ---------------------------------------------------------------------------
// Health handlers
// ---------------------------------------------------------------------------

static char s_current_steps_buffer[12];
static int s_step_count = 0;

// Is step data available?
bool step_data_is_available(){
    return HealthServiceAccessibilityMaskAvailable &
      health_service_metric_accessible(HealthMetricStepCount,
        time_start_of_today(), time(NULL));
      }

char get_thousands_separator() {
  const char* sys_locale = i18n_get_system_locale();
  
  // UK/US use a comma
  if (strncmp(sys_locale, "en", 2) == 0) {
      return ',';
  } else if (strncmp(sys_locale, "fr", 2) == 0 || strncmp(sys_locale, "ru", 2) == 0) {
      return ' '; //French or Russian, show a space
  } else {
      return '.';// Default (Dot)
  }
}

// Todays current step count
  static void get_step_count() {
      s_step_count = (int)health_service_sum_today(HealthMetricStepCount);
  }

static void display_step_count() {

    int thousands = s_step_count / 1000;
    //int hundreds = (s_step_count % 1000)/100;
    int hundreds2 = (s_step_count % 1000);
    char sep = get_thousands_separator();

    snprintf(s_current_steps_buffer, sizeof(s_current_steps_buffer),
    "%s", "");

    if(thousands > 0) {
        snprintf(s_current_steps_buffer, sizeof(s_current_steps_buffer),
    //   "%d.%d%s", thousands, hundreds, "k");
          "%d%c%03d", thousands, sep, hundreds2);
    }
    else {
      snprintf(s_current_steps_buffer, sizeof(s_current_steps_buffer),
        "%d", hundreds2);
    }
    APP_LOG(APP_LOG_LEVEL_DEBUG, "step buffer is %s",s_current_steps_buffer);

  }


static void health_handler(HealthEventType event, void *context) {
    if(event != HealthEventSleepUpdate) {
      get_step_count();
      display_step_count();
      if (s_fg_layer) {  // guard against firing before layer is created
      layer_mark_dirty(s_fg_layer);
      } 
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
  //(void)settings_changed;
  //(void)theme_settings_changed;

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

  ///weather and health data
  Tuple * uvarccol_t = dict_find(iter,MESSAGE_KEY_UVArcColor);
  Tuple * uvmaxcol_t = dict_find(iter,MESSAGE_KEY_UVMaxColor);
  Tuple * uvnowcol_t = dict_find(iter,MESSAGE_KEY_UVNowColor);
  //Tuple * uvi_t = dict_find(iter, MESSAGE_KEY_UseUVI);

  Tuple * wtemp_t = dict_find(iter, MESSAGE_KEY_WeatherTemp);
  Tuple * wwind_t = dict_find(iter, MESSAGE_KEY_WeatherWind);
  Tuple * iconwinddirnow_tuple = dict_find(iter, MESSAGE_KEY_WindIconNow);
  Tuple * iconnow_tuple = dict_find(iter, MESSAGE_KEY_IconNow);
  Tuple * wwindave_t = dict_find(iter, MESSAGE_KEY_WindFore);
  Tuple * sunset_dt = dict_find(iter, MESSAGE_KEY_WEATHER_SUNSET_KEY);
  Tuple * sunrise_dt = dict_find(iter, MESSAGE_KEY_WEATHER_SUNRISE_KEY);
  Tuple * sunset12_dt = dict_find(iter, MESSAGE_KEY_WEATHER_SUNSET_KEY_12H);
  Tuple * sunrise12_dt = dict_find(iter, MESSAGE_KEY_WEATHER_SUNRISE_KEY_12H);
  Tuple * iconfore_tuple = dict_find(iter, MESSAGE_KEY_IconFore);
  Tuple * iconwinddirave_tuple = dict_find(iter, MESSAGE_KEY_WindIconAve);
  Tuple * wforetemp_t = dict_find(iter, MESSAGE_KEY_TempFore);
  //Tuple * weatherprov_t =dict_find(iter, MESSAGE_KEY_WeatherProv);
  //#if defined (PBL_PLATFORM_EMERY) || defined (PBL_PLATFORM_GABBRO) || defined (PBL_PLATFORM_DIORITE) || defined (PBL_PLATFORM_FLINT)
  #ifndef PBL_PLATFORM_APLITE
  Tuple * rain1h_t = dict_find(iter, MESSAGE_KEY_rain1h);
  Tuple * pop1h_t = dict_find(iter, MESSAGE_KEY_pop1h);
  Tuple * rainfore_t = dict_find(iter, MESSAGE_KEY_rainfore);
  Tuple * popmax_t = dict_find(iter, MESSAGE_KEY_popmax);
  Tuple * popmin_t = dict_find(iter, MESSAGE_KEY_popmin);
  Tuple * uvmax_tuple = dict_find(iter, MESSAGE_KEY_UVIndexMax);
  Tuple * uvday_tuple = dict_find(iter, MESSAGE_KEY_UVIndexDay);
  Tuple * uvnow_tuple = dict_find(iter, MESSAGE_KEY_UVIndexNow);
  Tuple * pressurenow_t = dict_find(iter, MESSAGE_KEY_pressurenow);
  Tuple * pressure1h_t = dict_find(iter, MESSAGE_KEY_pressure1h);
  Tuple * barotrend_t = dict_find(iter, MESSAGE_KEY_barotrend);
  //#endif
  //#ifndef PBL_PLATFORM_APLITE
  Tuple * health_t = dict_find(iter, MESSAGE_KEY_HealthOn);
  #endif
  //Tuple * wforetemplow_t = dict_find(iter, MESSAGE_KEY_TempForeLow);
  Tuple * moon_tuple = dict_find(iter, MESSAGE_KEY_MoonPhase);
  Tuple * weather_t = dict_find(iter, MESSAGE_KEY_Weathertime);
  Tuple * neigh_t = dict_find(iter, MESSAGE_KEY_NameLocation);
  Tuple * frequpdate = dict_find(iter, MESSAGE_KEY_UpSlider);
  Tuple * useweather_t = dict_find(iter, MESSAGE_KEY_UseWeather);


  if (useweather_t) {
    settings.UseWeather = useweather_t->value->int32 != 0;
    if(settings.UseWeather){
      accel_tap_service_subscribe(accel_tap_handler); 
      APP_LOG(APP_LOG_LEVEL_DEBUG, "accel subscribed weather on");
    }
    settings_changed = true;
  }

  if (addzero12_t) {
    settings.AddZero12h = addzero12_t->value->int32 != 0;
    settings_changed = true;
  }

  if (remzero24_t) {
    settings.RemoveZero24h = remzero24_t->value->int32 != 0;
    settings_changed = true;
  }

  if (localampm_t) {
    settings.showlocalAMPM = localampm_t->value->int32 == 1;
    settings_changed = true;
  }

  if (vibe_t) {
    settings.VibeOn = vibe_t->value->int32 != 0;
    settings_changed = true;
  }

  if (enable_seconds_t) {
    settings.EnableSecondsHand = enable_seconds_t->value->int32 == 1;
  }

  if (enable_date_t) {
    settings.EnableDate = enable_date_t->value->int32 == 1;
    settings_changed = true;
  }

  if (enable_lines_t) {
    settings.EnableLines = enable_lines_t->value->int32 == 1;
    settings_changed = true;
  }

  if (enable_minor_t) {
    settings.EnableMinorTick = enable_minor_t->value->int32 == 1;
    settings_changed = true;
  }
  
  if (enable_major_t) {
    settings.EnableMajorTick = enable_major_t->value->int32 == 1;
    settings_changed = true;
  }

  if (enable_time_t) {
    settings.ShowTime = enable_time_t->value->int32 == 1;
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
     settings_changed = true;
  }

  if (fg_shape_t) {
    settings.ForegroundShape = fg_shape_t->value->int32 == 1;
    settings_changed = true;
  }

  if (enable_battery_t) {
    settings.EnableBattery = enable_battery_t->value->int32 == 1;
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

        tick_timer_service_subscribe(SECOND_UNIT, tick_handler);
        if(!settings.UseWeather){
          accel_tap_service_unsubscribe();
          APP_LOG(APP_LOG_LEVEL_DEBUG, "accel unsubscribed weather off, seconds always on");
        }
      }
    } else if (settings.SecondsVisibleTime > 0) {
      showSeconds = true;
      if (settings.EnableSecondsHand) {
        tick_timer_service_subscribe(SECOND_UNIT, tick_handler);
        s_timeout_timer = app_timer_register(SECONDS_TICK_INTERVAL_MS * settings.SecondsVisibleTime, timeout_handler, NULL);
        accel_tap_service_subscribe(accel_tap_handler);
        APP_LOG(APP_LOG_LEVEL_DEBUG, "accel subscribed seconds on");
      }
    } else {
      showSeconds = false;
      tick_timer_service_unsubscribe();
      tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
        if(!settings.UseWeather){
          accel_tap_service_unsubscribe();
          APP_LOG(APP_LOG_LEVEL_DEBUG, "accel unsubscribed weather off, seconds off");
        }
    }
    settings_changed = true;
  }

///Weather data


  if (wtemp_t){
  snprintf(settings.tempstring, sizeof(settings.tempstring), "%s", wtemp_t -> value -> cstring);
   settings_changed = true;
  }

  if (wwind_t){
  snprintf(settings.windstring, sizeof(settings.windstring), "%s", wwind_t -> value -> cstring);
   settings_changed = true;
  }


  if (iconwinddirnow_tuple){
  snprintf(settings.windiconnowstring,sizeof(settings.windiconnowstring),"%s",safe_wind_direction((int)iconwinddirnow_tuple->value->int32));
  settings_changed = true;  
  }

  if (iconnow_tuple){
      snprintf(settings.iconnowstring,sizeof(settings.iconnowstring),"%s",safe_weather_condition((int)iconnow_tuple->value->int32));
     settings_changed = true;
  }

  if (wwindave_t){
    snprintf(settings.windavestring, sizeof(settings.windavestring), "%s", wwindave_t -> value -> cstring);
     settings_changed = true;
  }

    //Hour Sunrise and Sunset

  if (sunset_dt){
     snprintf(settings.sunsetstring, sizeof(settings.sunsetstring), "%s", sunset_dt -> value -> cstring);
      settings_changed = true;
  }

  if (sunrise_dt){
     snprintf(settings.sunrisestring, sizeof(settings.sunrisestring), "%s", sunrise_dt -> value -> cstring);
      settings_changed = true;
  }

  //12hr version of sunset & sunrise
  if (sunset12_dt){
     snprintf(settings.sunsetstring12, sizeof(settings.sunsetstring12), "%s", sunset12_dt -> value -> cstring);
      settings_changed = true;
  }

  if (sunrise12_dt){
     snprintf(settings.sunrisestring12, sizeof(settings.sunrisestring12), "%s", sunrise12_dt -> value -> cstring);
      settings_changed = true;
  }

  if (iconfore_tuple){
    snprintf(settings.iconforestring,sizeof(settings.iconforestring),"%s",safe_weather_condition((int)iconfore_tuple->value->int32));
     settings_changed = true;
  }

  if (iconwinddirave_tuple){
    snprintf(settings.windiconavestring,sizeof(settings.windiconavestring),"%s",safe_wind_direction((int)iconwinddirave_tuple->value->int32));
     settings_changed = true;
  }

  if (wforetemp_t){
    snprintf(settings.temphistring, sizeof(settings.temphistring), "%s", wforetemp_t -> value -> cstring);
     settings_changed = true;
  }

//#if defined (PBL_PLATFORM_EMERY) || defined (PBL_PLATFORM_GABBRO) || defined (PBL_PLATFORM_DIORITE) || defined (PBL_PLATFORM_FLINT)
#ifndef PBL_PLATFORM_APLITE  
  if (pressurenow_t){
     settings.pressurenow = (int) pressurenow_t -> value -> int32;
     settings_changed = true;
  }

  if (pressure1h_t){
     settings.pressure1h = (int) pressure1h_t -> value -> int32;
     settings_changed = true;
  }

  if (barotrend_t){
     settings.barotrend = (int) barotrend_t -> value -> int32;
     settings_changed = true;
  }

  if (rain1h_t){
     settings.rainstring = (int) rain1h_t -> value -> int32;
     settings_changed = true;
  }

  if (pop1h_t){
     settings.popstring = (int) pop1h_t -> value -> int32;
     settings_changed = true;
  }

  if (popmax_t){
     settings.popmax = (int) popmax_t -> value -> int32;
     settings_changed = true;
  }

 if (popmin_t){
     settings.popmin = (int) popmin_t -> value -> int32;
     settings_changed = true;
  }

  if (rainfore_t){
     settings.rainfore = (int) rainfore_t -> value -> int32;
     settings_changed = true;
  }

  if (uvmax_tuple){
  settings.UVIndexMax = (int) uvmax_tuple -> value -> int32;
  APP_LOG(APP_LOG_LEVEL_DEBUG, "UVIndexMax is %d",settings.UVIndexMax);
  settings_changed = true;
  }


  if (uvday_tuple){
    settings.UVIndexDay = (int) uvday_tuple -> value -> int32;
  APP_LOG(APP_LOG_LEVEL_DEBUG, "UVIndexDay is %d",settings.UVIndexDay);
  settings_changed = true;
  }


  if (uvnow_tuple){
    settings.UVIndexNow = (int) uvnow_tuple -> value -> int32;
    s_uvnow_level = settings.UVIndexNow;
    settings_changed = true;
  }

#endif

  // if (wforetemplow_t){
  //   snprintf(settings.templowstring, sizeof(settings.templowstring), "%s", wforetemplow_t -> value -> cstring);
  //    settings_changed = true;
  // }

  if (moon_tuple){
    snprintf(settings.moonstring,sizeof(settings.moonstring),"%s",safe_moon_phase((int)moon_tuple->value->int32));
     settings_changed = true;
  }

  if (weather_t){
    settings.Weathertimecapture = (int) weather_t -> value -> int32;
  //   snprintf(settings.weathertimecapture, sizeof(settings.weathertimecapture), "%s", weather_t -> value -> cstring);
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Weather data captured at time %d",settings.Weathertimecapture);
     settings_changed = true;
  }

  if (neigh_t){
    snprintf(citistring, sizeof(citistring), "%s", neigh_t -> value -> cstring);
     settings_changed = true;
  }
  //Control of data gathered for http
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Location Timezone is %s", citistring);
  if (strcmp(citistring, "") == 0 || strcmp(citistring,"NotSet") == 0 ){
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Missing info at loop %d, GPS false, citistring is %s, 1 is %d, 2 is %d", s_loop, citistring, strcmp(citistring, ""),strcmp(citistring,"NotSet"));
   
     settings_changed = true;
  } else{
    APP_LOG(APP_LOG_LEVEL_DEBUG, "GPS on, loop %d, citistring is %s, 1 is %d, 2 is %d", s_loop, citistring, strcmp(citistring, ""),strcmp(citistring,"NotSet"));
   
     settings_changed = true;
  }

  if (frequpdate){
    settings.UpSlider = (int) frequpdate -> value -> int32;
    //Restart the counter
    s_countdown = settings.UpSlider;
     settings_changed = true;
  }
#ifndef PBL_PLATFORM_APLITE
 if (health_t) {
    settings.HealthOn = health_t->value->int32 == 1;
    if (settings.HealthOn) {
      APP_LOG(APP_LOG_LEVEL_DEBUG, "Health on");
      health_service_events_unsubscribe();              // avoid double-subscribing
      health_service_events_subscribe(health_handler, NULL);  // ← ADD THIS
      get_step_count();
      display_step_count();
      settings_changed = true;
    } else {
      APP_LOG(APP_LOG_LEVEL_DEBUG, "Health off");
      health_service_events_unsubscribe();              // ← ADD THIS
      snprintf(s_current_steps_buffer, sizeof(s_current_steps_buffer), "%s", "");
      settings_changed = true;
    }
  }
#endif
////////////


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
      settings.FGColor = GColorBlack;
      settings.UVArcColor = GColorLightGray;
      settings.UVMaxColor = GColorWhite;
      settings.UVNowColor = GColorWhite;
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
      settings.FGColor = GColorWhite;
      settings.UVArcColor = GColorDarkGray;
      settings.UVMaxColor = GColorBlack;
      settings.UVNowColor = GColorBlack;
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
      settings.FGColor = GColorWhite;
      settings.UVArcColor = GColorDarkGray;
      settings.UVMaxColor = GColorBlack;
      settings.UVNowColor = GColorBlack;
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
      if (fg_color_t)     { settings.FGColor = GColorFromHEX(fg_color_t->value->int32); }
      if (uvarccol_t){ settings.UVArcColor = GColorFromHEX(uvarccol_t-> value -> int32);}
      if (uvmaxcol_t){settings.UVMaxColor = GColorFromHEX(uvmaxcol_t-> value -> int32);}
      if (uvnowcol_t){settings.UVNowColor = GColorFromHEX(uvnowcol_t-> value -> int32);}
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
      settings.FGColor = GColorBlack;
      settings.UVArcColor = GColorLightGray;
      settings.UVMaxColor = GColorWhite;
      settings.UVNowColor = GColorRed;
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
      settings.FGColor = GColorBlack;
      settings.UVArcColor = GColorLightGray;
      settings.UVMaxColor = GColorWhite;
      settings.UVNowColor = GColorRed;
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
      settings.FGColor = GColorDukeBlue;
      settings.UVArcColor = GColorLightGray;
      settings.UVMaxColor = GColorWhite;
      settings.UVNowColor = GColorRed;
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
      settings.FGColor = GColorBlack;
      settings.UVArcColor = GColorLightGray;
      settings.UVMaxColor = GColorWhite;
      settings.UVNowColor = GColorRed;
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
      settings.FGColor = GColorBrightGreen;
      settings.UVArcColor = GColorDarkGray;
      settings.UVMaxColor = GColorBlack;
      settings.UVNowColor = GColorWhite;
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
      if (fg_color_t)     { settings.FGColor = GColorFromHEX(fg_color_t->value->int32); }
      if (uvarccol_t){ settings.UVArcColor = GColorFromHEX(uvarccol_t-> value -> int32);}
      if (uvmaxcol_t){settings.UVMaxColor = GColorFromHEX(uvmaxcol_t-> value -> int32);}
      if (uvnowcol_t){settings.UVNowColor = GColorFromHEX(uvnowcol_t-> value -> int32);}
      theme_settings_changed = true;
    }
  }

  if (settings_changed || theme_settings_changed) {
    layer_mark_dirty(s_bg_layer);
    layer_mark_dirty(s_canvas_layer);
    layer_mark_dirty(s_canvas_second_hand);
    layer_mark_dirty(s_fg_layer);
    layer_mark_dirty(s_weather_layer_1);
 //   #if defined (PBL_PLATFORM_EMERY) || defined (PBL_PLATFORM_GABBRO) || defined (PBL_PLATFORM_DIORITE) || defined (PBL_PLATFORM_FLINT)
    #ifndef PBL_PLATFORM_APLITE   
      layer_mark_dirty(s_weather_layer_2);
    #endif
  }

  prv_save_settings();
  update_weather_view_visibility();
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
  (void)bounds;

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

static void weather_update_proc(Layer *layer, GContext *ctx) {
  // if(!settings.UseWeather){
  //   return;
  // }

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

  //Vertical line in centre section
  // graphics_draw_line(ctx, GPoint(cx, cy - config.Line6yOffset ),
  //                         GPoint(cx,  cy + config.Line7yOffset ));

  // Vertical lines above centre (left and right of 12 o'clock)
  graphics_draw_line(ctx, GPoint(bounds.size.w * 0.42, cy - half_time - config.Line45yOffset),
                          GPoint(bounds.size.w * 0.42, cy - (config.fg_radius - 10)));
  graphics_draw_line(ctx, GPoint(bounds.size.w * 0.58, cy - half_time - config.Line45yOffset),
                          GPoint(bounds.size.w * 0.58, cy - (config.fg_radius - 10)));
  }


GRect IconNowRect = config.MiddleLeftRect[0]; 
GRect IconForeRect = config.MiddleRightRect[0];
GRect WindKtsRect = config.TopLeftRect[0];
GRect WindForeKtsRect = config.TopRightRect[0];
GRect TempRect = config.BottomLeftRect1[0];
GRect TempForeRect = config.BottomRightRect1[0];
GRect WindDirNowRect = config.TopTopLeftRect[0];
GRect WindDirForeRect = config.TopTopRightRect[0];
GRect MoonRect = config.TopMiddleRect[0];


            char TempForeToDraw[10];
            snprintf(TempForeToDraw, sizeof(TempForeToDraw), "%s",settings.temphistring);

 
            char CondToDraw[4];
            char CondForeToDraw[4];
            char TempToDraw[8];

            char SpeedToDraw[10];
            char SpeedForeToDraw[10];
            char DirectionToDraw[4];
            char DirectionForeToDraw[4];

            snprintf(SpeedToDraw,sizeof(SpeedToDraw),"%s",settings.windstring);
            snprintf(SpeedForeToDraw,sizeof(SpeedForeToDraw),"%s",settings.windavestring);
            snprintf(DirectionToDraw,sizeof(DirectionToDraw),"%s",settings.windiconnowstring);
            snprintf(DirectionForeToDraw,sizeof(DirectionForeToDraw),"%s", settings.windiconavestring);
            snprintf(CondToDraw, sizeof(CondToDraw), "%s",settings.iconnowstring);
            snprintf(TempToDraw, sizeof(TempToDraw), "%s",settings.tempstring);
            snprintf(CondForeToDraw, sizeof(CondForeToDraw), "%s",settings.iconforestring);

            char MoonToDraw[4];
            snprintf(MoonToDraw, sizeof(MoonToDraw), "%s",settings.moonstring); 

            //Wind speed
            graphics_context_set_text_color(ctx,settings.TextColor1);
            graphics_draw_text(ctx, SpeedToDraw, small_font, WindKtsRect, GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
            graphics_draw_text(ctx, SpeedForeToDraw, small_font, WindForeKtsRect, GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
            
            //Wind Direction
            graphics_draw_text(ctx, DirectionToDraw, FontWeatherIcons, WindDirNowRect, GTextOverflowModeFill, GTextAlignmentCenter, NULL);
            graphics_draw_text(ctx, DirectionForeToDraw, FontWeatherIcons, WindDirForeRect, GTextOverflowModeFill,GTextAlignmentCenter, NULL);

            //Weathericons
            graphics_draw_text(ctx, CondToDraw, FontWeatherIcons, IconNowRect, GTextOverflowModeFill, GTextAlignmentCenter, NULL);
            graphics_draw_text(ctx, CondForeToDraw, FontWeatherIcons, IconForeRect, GTextOverflowModeFill, GTextAlignmentCenter, NULL);

            //weather temps
            graphics_draw_text(ctx, TempToDraw, medium_font, TempRect, GTextOverflowModeFill, GTextAlignmentCenter, NULL);
            graphics_draw_text(ctx, TempForeToDraw, small_font, TempForeRect, GTextOverflowModeFill, GTextAlignmentCenter, NULL);
            
            //Moonphase
            graphics_draw_text(ctx, MoonToDraw, FontWeatherIcons, MoonRect, GTextOverflowModeFill, GTextAlignmentCenter, NULL);

}

//#if defined (PBL_PLATFORM_EMERY) || defined (PBL_PLATFORM_GABBRO) || defined (PBL_PLATFORM_DIORITE) || defined (PBL_PLATFORM_FLINT)
#ifndef PBL_PLATFORM_APLITE
static void weather2_update_proc(Layer *layer, GContext *ctx) {
  
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


  GRect SunsetIconRect = config.BottomBottomRightRect[0];
  GRect SunriseIconRect = config.BottomBottomLeftRect1[0];
  GRect SunsetRect = config.BottomRightRect[0];
  GRect SunriseRect = config.BottomLeftRect[0];
  GRect UVDayValueRect = config.UVDayValueRect[0];
  GRect uv_arc_bounds = config.uv_arc_bounds[0];
  GRect uv_arc_bounds_max = config.uv_arc_bounds_max[0];
  GRect uv_arc_bounds_now = config.uv_arc_bounds_now[0];
  GRect uv_icon = config.uv_icon[0];

  GRect RainDayValueRect = config.RainDayValueRect[0];
  GRect Rain1hValueRect = config.Rain1hValueRect[0];
  GRect Rain_arc_bounds = config.Rain_arc_bounds[0];
  GRect Rain_arc_bounds_max = config.Rain_arc_bounds_max[0];
  GRect Rain_arc_bounds_now = config.Rain_arc_bounds_now[0];
  GRect Rain_icon = config.Rain_icon[0];

  GRect BarotrendRect = config.TopMiddleRect[0];
  GRect PressureNowRect = config.TopLeftRect[0];
  GRect Pressure1hRect = config.TopRightRect[0];

  graphics_context_set_text_color(ctx,settings.TextColor1);


            if(settings.barotrend == 2){  //falling
              graphics_draw_text(ctx, "\U0000F088", FontWeatherIcons, BarotrendRect, GTextOverflowModeFill, GTextAlignmentCenter, NULL);
            } else if(settings.barotrend == 1){  //rising
              graphics_draw_text(ctx, "\U0000F057", FontWeatherIcons, BarotrendRect, GTextOverflowModeFill, GTextAlignmentCenter, NULL);
            } else { //default or steady
              graphics_draw_text(ctx, "\U0000F04D", FontWeatherIcons, BarotrendRect, GTextOverflowModeFill, GTextAlignmentCenter, NULL);
            }

            //"%d.%d", s_rainday_level / 10, s_rainday_level % 10


            int s_pressurenow_level = settings.pressurenow;
            char PressureNowToDraw[7];
            if (settings.pressurenow<650000){
              snprintf(PressureNowToDraw, sizeof(PressureNowToDraw), "%d.%02d", s_pressurenow_level /1000, (s_pressurenow_level % 1000 / 10));
            }
            else{
              snprintf(PressureNowToDraw, sizeof(PressureNowToDraw), "%d", s_pressurenow_level /1000);
            }
            //APP_LOG(APP_LOG_LEVEL_DEBUG, "s_pressurenow_level is %d, settings.pressurenow is %d, PressureNowToDraw is %s", s_pressurenow_level, settings.pressurenow, PressureNowToDraw);

            graphics_draw_text(ctx, PressureNowToDraw, small_font, PressureNowRect, GTextOverflowModeFill, GTextAlignmentCenter, NULL);
           
           
            int s_pressure1h_level = settings.pressure1h;
            char Pressure1hToDraw[7];
            if (settings.pressure1h<650000){
              snprintf(Pressure1hToDraw, sizeof(Pressure1hToDraw), "%d.%02d", s_pressure1h_level /1000, (s_pressure1h_level % 1000 / 10));
            } else {
              snprintf(Pressure1hToDraw, sizeof(Pressure1hToDraw), "%d", s_pressure1h_level / 1000);
            }
            //APP_LOG(APP_LOG_LEVEL_DEBUG, "s_pressure1h_level is %d, settings.pressuren1h is %d, Pressure1hToDraw is %s", s_pressure1h_level, settings.pressure1h, Pressure1hToDraw);

            graphics_draw_text(ctx, Pressure1hToDraw, small_font, Pressure1hRect, GTextOverflowModeFill, GTextAlignmentCenter, NULL);

            char SunsetIconToShow[4];

            snprintf(SunsetIconToShow, sizeof(SunsetIconToShow),  "%s", "\U0000F052");

            char SunriseIconToShow[4];

            snprintf(SunriseIconToShow, sizeof(SunriseIconToShow),  "%s",  "\U0000F051");

            //sunsettime variable by clock setting
            char SunsetToDraw[8];
            if (clock_is_24h_style()){
              snprintf(SunsetToDraw, sizeof(SunsetToDraw), "%s",settings.sunsetstring);
            }
            else {
              snprintf(SunsetToDraw, sizeof(SunsetToDraw), "%s",settings.sunsetstring12);
            }

            char SunriseToDraw[8];
            if (clock_is_24h_style()){
               snprintf(SunriseToDraw, sizeof(SunriseToDraw), "%s",settings.sunrisestring);
             }
            else {
               snprintf(SunriseToDraw, sizeof(SunriseToDraw), "%s",settings.sunrisestring12);
             }

             graphics_draw_text(ctx, SunriseIconToShow, FontWeatherIconsSmall, SunriseIconRect, GTextOverflowModeFill, GTextAlignmentCenter, NULL);
             graphics_draw_text(ctx, SunsetIconToShow, FontWeatherIconsSmall, SunsetIconRect, GTextOverflowModeFill, GTextAlignmentCenter, NULL);
             graphics_draw_text(ctx, SunriseToDraw, small_font, SunriseRect, GTextOverflowModeFill, GTextAlignmentCenter, NULL);
             graphics_draw_text(ctx, SunsetToDraw, small_font, SunsetRect, GTextOverflowModeFill, GTextAlignmentCenter, NULL);

             graphics_draw_text(ctx, "\U0000F00D", FontWeatherIconsSmall, uv_icon, GTextOverflowModeFill, GTextAlignmentCenter, NULL);
             graphics_draw_text(ctx, "\U0000F084", FontWeatherIconsSmall, Rain_icon, GTextOverflowModeFill, GTextAlignmentCenter, NULL);

             int s_uvmax_level = settings.UVIndexMax;  //forecast max uv for the day, limited to max of 10 for dial
             int s_uvnow_level = settings.UVIndexNow;  //mark on the circle, also limited to nmax of 10
             int s_uvday_level = settings.UVIndexDay;  //forecast max uv for the day - value in the circle, not limited to a max of 10
            //  int s_uvmax_level = 8;
            //  int s_uvnow_level = 5;
            //  int s_uvday_level = 15;
            //(void)s_uvmax_level; (void)s_uvnow_level; (void)s_uvday_level;
                    
                    graphics_context_set_fill_color(ctx, settings.UVArcColor);
                    int32_t angle_start = DEG_TO_TRIGANGLE(180+30);
                    int32_t angle_end = DEG_TO_TRIGANGLE(360+180-30);
                    uint16_t inset_thickness = 2;
                    graphics_fill_radial(ctx,uv_arc_bounds,GOvalScaleModeFitCircle,inset_thickness,angle_start,angle_end);

                    graphics_context_set_fill_color(ctx, settings.UVMaxColor);// GColorBlack);
                    //graphics_fill_rect(ctx, UVMaxRect, 0, GCornerNone);
                                    
                    int32_t angle_start_max = DEG_TO_TRIGANGLE(180+30);
                    int32_t angle_end_max = DEG_TO_TRIGANGLE((180+30)+ ((360-60)*s_uvmax_level/10));
                    uint16_t inset_thickness_max = 4;
                    graphics_fill_radial(ctx,uv_arc_bounds_max,GOvalScaleModeFitCircle,inset_thickness_max,angle_start_max,angle_end_max);

                    graphics_context_set_fill_color(ctx, settings.UVNowColor);
                    //graphics_fill_rect(ctx,UVNowRect, 3, GCornersAll);
                                    
                        int32_t angle_start_now = DEG_TO_TRIGANGLE((180+30)+((360-60)*s_uvnow_level/10)-3);
                        int32_t angle_end_now = DEG_TO_TRIGANGLE((180+30)+((360-60)*s_uvnow_level/10)+3);

                    uint16_t inset_thickness_now = 8;
                    graphics_fill_radial(ctx,uv_arc_bounds_now,GOvalScaleModeFitCircle,inset_thickness_now,angle_start_now,angle_end_now);

                    char UVValueToDraw[4];
                    snprintf(UVValueToDraw, sizeof(UVValueToDraw), "%d",s_uvday_level);
                    //snprintf(UVValueToDraw, sizeof(UVValueToDraw), "%d", 10 );
                    graphics_context_set_text_color(ctx,settings.TextColor1);
                    if (s_uvday_level<20){
                      graphics_draw_text(ctx, UVValueToDraw, small_medium_font, UVDayValueRect, GTextOverflowModeFill, GTextAlignmentCenter, NULL);
                    }
                    else{
                      graphics_draw_text(ctx, UVValueToDraw, small_medium_font, UVDayValueRect, GTextOverflowModeFill, GTextAlignmentCenter, NULL);
                    }

             int s_rainmax_level = settings.popmax; //change to be forecast probability or max prob of rain
             int s_rainmin_level = settings.popmin;
             int s_rainnow_level = settings.popstring;  //prob rain in next hour
             int s_rainday_level = settings.rainfore;  //raistring is rain in the next hour, rainfore is the amount in next day, value in the circle 
             int s_rain1h_level = settings.rainstring;

                    
                    graphics_context_set_fill_color(ctx, settings.UVArcColor);
                    int32_t angle_start_r = DEG_TO_TRIGANGLE(180+30);
                    int32_t angle_end_r = DEG_TO_TRIGANGLE(360+180-30);
                    uint16_t inset_thickness_r = 2;
                    graphics_fill_radial(ctx,Rain_arc_bounds,GOvalScaleModeFitCircle,inset_thickness_r,angle_start_r,angle_end_r);

                    graphics_context_set_fill_color(ctx, settings.UVMaxColor);// GColorBlack);
                    //graphics_fill_rect(ctx, UVMaxRect, 0, GCornerNone);
                                    
                    int32_t angle_start_max_r = DEG_TO_TRIGANGLE((180+30)+ ((360-60)*s_rainmin_level/100));
                    int32_t angle_end_max_r = DEG_TO_TRIGANGLE((180+30)+ ((360-60)*s_rainmax_level/100));
                    uint16_t inset_thickness_max_r = 4;
                    graphics_fill_radial(ctx,Rain_arc_bounds_max,GOvalScaleModeFitCircle,inset_thickness_max_r,angle_start_max_r,angle_end_max_r);

                    graphics_context_set_fill_color(ctx, settings.UVNowColor);
                                    
                        int32_t angle_start_now_r = DEG_TO_TRIGANGLE((180+30)+((360-60)*s_rainnow_level/100)-3);
                        int32_t angle_end_now_r = DEG_TO_TRIGANGLE((180+30)+((360-60)*s_rainnow_level/100)+3);

                    uint16_t inset_thickness_now_r = 8;
                    graphics_fill_radial(ctx,Rain_arc_bounds_now,GOvalScaleModeFitCircle,inset_thickness_now_r,angle_start_now_r,angle_end_now_r);

                    char RainValueToDraw [9];
                    if (s_rainday_level<10){
                    snprintf(RainValueToDraw, sizeof(RainValueToDraw), "%d.%d", s_rainday_level / 10, (s_rainday_level % 10/10));
                    }
                    else{
                     snprintf(RainValueToDraw, sizeof(RainValueToDraw), "%d", s_rainday_level / 10);  
                    }

                    char Rain1hToDraw [9];
                    if (s_rain1h_level<10){
                    snprintf(Rain1hToDraw, sizeof(Rain1hToDraw), "%d.%d", s_rain1h_level / 10, (s_rainday_level % 10/10));
                    }
                    else{
                     snprintf(Rain1hToDraw, sizeof(Rain1hToDraw), "%d", s_rain1h_level / 10);  
                    }

                      graphics_context_set_text_color(ctx,settings.TextColor1);

                      graphics_draw_text(ctx, RainValueToDraw, small_font, RainDayValueRect, GTextOverflowModeFill, GTextAlignmentCenter, NULL);
                      graphics_draw_text(ctx, Rain1hToDraw, small_font, Rain1hValueRect, GTextOverflowModeFill, GTextAlignmentCenter, NULL);             

}
#endif

//#ifndef PBL_BW
#if defined (PBL_PLATFORM_GABBRO) || defined (PBL_PLATFORM_EMERY) 
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
  if (FCTX_Font == NULL) {
      APP_LOG(APP_LOG_LEVEL_ERROR, "Font not loaded yet, skipping draw!");
      return; 
  }
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
 


  if (settings.HealthOn) {
    //display_step_count();
    fctx_begin_fill(&fctx);
    fctx_set_fill_color(&fctx, settings.TextColor1);
    fctx_set_text_em_height(&fctx, FCTX_Font, config.other_text_font_size);
    FPoint word_pos = {
      INT_TO_FIXED(bounds.size.w / 2 - config.xOffset),
      INT_TO_FIXED(bounds.size.h / 2 + config.other_text_font_size + config.yOffsetPercent)
    };
    fctx_set_offset(&fctx, word_pos);
    fctx_draw_string(&fctx, s_current_steps_buffer, FCTX_Font, GTextAlignmentRight, FTextAnchorTop);
    fctx_end_fill(&fctx);
  }
  else if (settings.EnableLogo) {
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

  draw_btqt_icons(ctx, bounds);

}
#else
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

  // GRect bounds = layer_get_bounds(layer);
  // if (settings.ForegroundShape) {
  //   draw_center(ctx, settings.SecondsHandColor, settings.FGColor);
  // }
  // else{
  //   GRect fg_rect = GRect(config.foregroundrect_x,config.foregroundrect_y, config.foregroundrect_w, config.foregroundrect_h);
  //   graphics_context_set_fill_color(ctx, settings.FGColor);
  //   graphics_fill_rect(ctx, fg_rect, config.corner_radius_foreground, GCornersAll);
  //   graphics_context_set_stroke_color(ctx, settings.SecondsHandColor);
  //   graphics_context_set_stroke_width(ctx, 2);
  //   graphics_draw_round_rect(ctx, fg_rect, config.corner_radius_foreground);
  // }

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
    int time_y = cy - time_size.h / 2;

    graphics_draw_text(ctx, timedraw, aplite_font,
      GRect(left_edge, PBL_IF_ROUND_ELSE(time_y - 4, time_y - 3), time_size.w, time_size.h),
      GTextOverflowModeWordWrap, GTextAlignmentLeft, NULL);

    if (settings.showlocalAMPM && !clock_is_24h_style() && local_ampm_string[0] != '\0') {
      GRect ampm_rect = GRect(left_edge + time_size.w + 2, time_y + time_size.h - PBL_IF_ROUND_ELSE(19+12,17),
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

    GRect weekday_rect = GRect(0 - 11, date_y, cx - PBL_IF_ROUND_ELSE(config.xOffset+3, config.xOffset), 16);
    graphics_draw_text(ctx, weekdaydraw, small_font, weekday_rect,
                       GTextOverflowModeWordWrap, GTextAlignmentRight, NULL);

    GRect month_rect = GRect(cx + PBL_IF_ROUND_ELSE(config.xOffset+3, config.xOffset) + 11, date_y, bounds.size.w / 2, 16);
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
    int batt_left = cx - total_batt_w / 2 + PBL_IF_ROUND_ELSE(22,19);
    int batt_y = cy + config.other_text_font_size + config.yOffsetBattery - 8;

    graphics_draw_text(ctx, battperc, medium_font,
      GRect(batt_left, batt_y, batt_size.w, batt_size.h),
      GTextOverflowModeWordWrap, GTextAlignmentLeft, NULL);

    graphics_draw_text(ctx, "%", small_font,
      GRect(batt_left + batt_size.w + 2, batt_y + batt_size.h - percent_size.h - 5,
            percent_size.w, percent_size.h),
      GTextOverflowModeWordWrap, GTextAlignmentLeft, NULL);
  }

  #ifndef PBL_PLATFORM_APLITE
  if (settings.HealthOn) {
    int logo_y = cy + config.other_text_font_size + config.yOffsetPercent - 4;
    GRect logo_rect = GRect(0, logo_y, cx - config.xOffset + 1, 16);
    graphics_draw_text(ctx, s_current_steps_buffer, small_font, logo_rect,
                       GTextOverflowModeWordWrap, GTextAlignmentRight, NULL);
  }
  else if (settings.EnableLogo) {
    char textdraw[15];
    snprintf(textdraw, sizeof(textdraw), "%s", settings.LogoText);
    int logo_y = cy + config.other_text_font_size + config.yOffsetPercent - 4;
    GRect logo_rect = GRect(0, logo_y, cx - config.xOffset + 1, 16);
    graphics_draw_text(ctx, textdraw, small_font, logo_rect,
                       GTextOverflowModeWordWrap, GTextAlignmentRight, NULL);
  }
  #endif
  #ifdef PBL_PLATFORM_APLITE
   if (settings.EnableLogo) {
        char textdraw[15];
        snprintf(textdraw, sizeof(textdraw), "%s", settings.LogoText);
        int logo_y = cy + config.other_text_font_size + config.yOffsetPercent - 4;
        GRect logo_rect = GRect(0, logo_y, cx - config.xOffset + 1, 16);
        graphics_draw_text(ctx, textdraw, small_font, logo_rect,
                          GTextOverflowModeWordWrap, GTextAlignmentRight, NULL);
  }
  #endif

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

  draw_btqt_icons(ctx, bounds);

}
#endif



// ---------------------------------------------------------------------------
// Window lifecycle
// ---------------------------------------------------------------------------

static void prv_window_load(Window *window) {
  
    Layer *window_layer = window_get_root_layer(s_window);
    bounds = layer_get_bounds(window_layer);

  // Initial State Fetch
    time_t now = time(NULL);
    prv_tick_time = *localtime(&now);
    s_connected = connection_service_peek_pebble_app_connection();
    s_battery_level = battery_state_service_peek().charge_percent;
    update_cached_strings();

  #if defined PBL_BW || defined PBL_PLATFORM_BASALT || defined PBL_PLATFORM_CHALK
   aplite_font = fonts_load_custom_font(resource_get_handle(PBL_IF_ROUND_ELSE(RESOURCE_ID_FONT_DIN_CON_APLITE_46, RESOURCE_ID_FONT_DIN_CON_APLITE_38)));
   small_font = fonts_get_system_font(FONT_KEY_GOTHIC_14);
   small_medium_font = fonts_get_system_font(FONT_KEY_GOTHIC_18);
   medium_font = fonts_get_system_font(FONT_KEY_GOTHIC_24);
   FontWeatherIcons = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_WEATHERICONS_20));
   FontWeatherIconsSmall = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_WEATHERICONS_10));
  #elif defined (PBL_PLATFORM_EMERY) || defined (PBL_PLATFORM_GABBRO)
   FCTX_Font = ffont_create_from_resource(RESOURCE_ID_DIN_CONDENSED_FFONT);
   small_font = fonts_get_system_font(FONT_KEY_GOTHIC_18);
   small_medium_font = fonts_get_system_font(FONT_KEY_GOTHIC_28);
   medium_font = fonts_get_system_font(FONT_KEY_GOTHIC_28);
   FontWeatherIcons = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_WEATHERICONS_31));
   FontWeatherIconsSmall = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_WEATHERICONS_12));
  // #else //basalt and chalk
  //  aplite_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_DIN_CON_APLITE_38));
  //  //FCTX_Font = ffont_create_from_resource(RESOURCE_ID_DIN_CONDENSED_FFONT);
  //  small_font = fonts_get_system_font(FONT_KEY_GOTHIC_14);
  //  medium_font = fonts_get_system_font(FONT_KEY_GOTHIC_24);
  //  FontWeatherIcons = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_WEATHERICONS_20));
  //  FontWeatherIconsSmall = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_WEATHERICONS_10));
  #endif
 
  #if defined (PBL_PLATFORM_EMERY) || defined (PBL_PLATFORM_GABBRO)
  FontBTQTIcons = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_DRIPICONS_16));
  #else
  FontBTQTIcons = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_DRIPICONS_12));
  #endif

  APP_LOG(APP_LOG_LEVEL_DEBUG, "prv_window_load: HealthOn=%d", (int)settings.HealthOn);
  if (settings.HealthOn) {
      health_service_events_subscribe(health_handler, NULL);
        get_step_count();
      display_step_count();
  }
    

    tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
    battery_state_service_subscribe(battery_callback);
    connection_service_subscribe((ConnectionHandlers){ .pebble_app_connection_handler = bluetooth_callback });

  if (settings.UseWeather) {
    s_timeout_timer = app_timer_register(1000, timeout_handler, NULL);
    accel_tap_service_subscribe(accel_tap_handler);
    APP_LOG(APP_LOG_LEVEL_DEBUG, "accel subscribed weather on, prv_window_load");
  }
  
  if (settings.EnableSecondsHand) {
    tick_timer_service_subscribe(SECOND_UNIT, tick_handler);
    if (settings.SecondsVisibleTime != 135) {
      s_timeout_timer = app_timer_register(1000 * settings.SecondsVisibleTime, timeout_handler, NULL);
      accel_tap_service_subscribe(accel_tap_handler);
      APP_LOG(APP_LOG_LEVEL_DEBUG, "accel subscribed seconds on <135 seconds, prv_window_load");
    }
  } else {
    tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  }
  showSeconds = settings.EnableSecondsHand;

  

  if (settings.Weathertimecapture == 0) {
      strcpy(settings.iconnowstring, "\U0000F03D");
      s_loop = 0;
      APP_LOG(APP_LOG_LEVEL_DEBUG, "Weather loaded at startup (First Run)");
      DictionaryIterator * iter;
      app_message_outbox_begin( & iter);
      dict_write_uint8(iter, 0, 0);
      app_message_outbox_send();
    }

  s_bg_layer = layer_create(bounds);
  s_canvas_second_hand = layer_create(bounds);
  s_canvas_layer = layer_create(bounds);
  s_fg_layer = layer_create(bounds);
  s_weather_layer_1 = layer_create(bounds);
  //#if defined (PBL_PLATFORM_EMERY) || defined (PBL_PLATFORM_GABBRO) || defined (PBL_PLATFORM_DIORITE) || defined (PBL_PLATFORM_FLINT)
  #ifndef PBL_PLATFORM_APLITE
  s_weather_layer_2 = layer_create(bounds);
  #endif

  layer_add_child(window_layer, s_bg_layer);
  layer_add_child(window_layer, s_canvas_layer);
  layer_add_child(window_layer, s_canvas_second_hand);
  layer_add_child(window_layer, s_fg_layer);
  layer_add_child(window_layer, s_weather_layer_1);
  //#if defined (PBL_PLATFORM_EMERY) || defined (PBL_PLATFORM_GABBRO) || defined (PBL_PLATFORM_DIORITE) || defined (PBL_PLATFORM_FLINT)
  #ifndef PBL_PLATFORM_APLITE
  layer_add_child(window_layer, s_weather_layer_2);
  #endif
  
  //bluetooth_vibe_icon(connection_service_peek_pebble_app_connection());

  layer_set_update_proc(s_bg_layer, bg_update_proc);
  layer_set_update_proc(s_canvas_second_hand, layer_update_proc_seconds_hand);
  layer_set_update_proc(s_canvas_layer, hour_min_hands_canvas_update_proc);
  layer_set_update_proc(s_fg_layer, fg_update_proc);
  layer_set_update_proc(s_weather_layer_1, weather_update_proc);
  //#if defined (PBL_PLATFORM_EMERY) || defined (PBL_PLATFORM_GABBRO) || defined (PBL_PLATFORM_DIORITE) || defined (PBL_PLATFORM_FLINT)
  #ifndef PBL_PLATFORM_APLITE
  layer_set_update_proc(s_weather_layer_2, weather2_update_proc);
  #endif

  showWeather = 0; 
  update_weather_view_visibility();
        
}

static void prv_window_unload(Window *window) {
  if (s_timeout_timer) {
    app_timer_cancel(s_timeout_timer);
  }
  accel_tap_service_unsubscribe();
  app_message_deregister_callbacks();
  connection_service_unsubscribe();
  health_service_events_unsubscribe();
  battery_state_service_unsubscribe();
  tick_timer_service_unsubscribe();
  layer_destroy(s_canvas_layer);
  layer_destroy(s_bg_layer);
  layer_destroy(s_fg_layer);
  layer_destroy(s_weather_layer_1);
  //#if defined (PBL_PLATFORM_EMERY) || defined (PBL_PLATFORM_GABBRO) || defined (PBL_PLATFORM_DIORITE) || defined (PBL_PLATFORM_FLINT)
  #ifndef PBL_PLATFORM_APLITE
  layer_destroy(s_weather_layer_2);
  #endif
  layer_destroy(s_canvas_second_hand);
  fonts_unload_custom_font(FontBTQTIcons);
  fonts_unload_custom_font(FontWeatherIcons);
  fonts_unload_custom_font(FontWeatherIconsSmall);
  #if defined PBL_BW || defined PBL_PLATFORM_BASALT || defined PBL_PLATFORM_CHALK
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

  s_countdown = settings.UpSlider;

  app_message_register_inbox_received(prv_inbox_received_handler);
  #ifdef PBL_PLATFORM_APLITE
    app_message_open(512, 512); // Smaller buffers for older hardware
  #else
    app_message_open(2048, 1024);
  #endif

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