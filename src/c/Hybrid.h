#pragma once
#include <pebble.h>

#define MODEL_COUNT 9
#define SETTINGS_KEY 42

typedef struct DialSpec {
  GPoint markers[12];
//  GPoint logo;
//  GPoint model;
  GPoint date_box;
  GPoint date1;
  GPoint date2;
  GPoint date_single;

  GSize marker_size;
  GSize digit_size;
//  GSize logo_size;
//  GSize model_size;
  GSize date_box_size;

  uint32_t marker_res;
  uint32_t digit_res;
//  uint32_t logo_res;
//  uint32_t models_res;
  uint32_t date_box_res;
} __attribute__((__packed__)) DialSpec;

enum DialType {
  FONT1,
  FONT2,
  FONT3,
  FONT1_ROUND,
  FONT2_ROUND,
  FONT3_ROUND
};

typedef struct ClaySettings {
  bool EnableSecondsHand;
  int SecondsVisibleTime;
  bool EnableDate;
  bool EnableBattery;
  bool EnableBatteryLine;
  bool EnableLogo;
  char LogoText[7];
  bool VibeOn;
  int Font;
  char BWThemeSelect[4];
  char ThemeSelect[4];
  GColor BackgroundColor1;
  GColor ShadowColor;
  GColor TextColor1;
  GColor TickColor;
  GColor HoursHandColor;
  GColor HoursHandBorderColor;
  GColor MinutesHandColor;
  GColor MinutesHandBorderColor;
  GColor SecondsHandColor;
  GColor BatteryLineColor;
  GColor BTQTColor;
  GColor FGColor;
  bool ShadowOn;
  bool RemoveZero24h;
  bool AddZero12h;
  GColor LineColor;
  bool showlocalAMPM;
  bool ShowTime;
  bool EnableLines;
} __attribute__((__packed__)) ClaySettings;
