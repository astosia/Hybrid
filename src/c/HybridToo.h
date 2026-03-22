#pragma once
#include <pebble.h>

#define SETTINGS_KEY 44

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
  GColor MajorTickColor;
  GColor MinorTickColor;
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
  bool EnableMinorTick;
  bool EnableMajorTick;
  bool ForegroundShape;
} __attribute__((__packed__)) ClaySettings;


