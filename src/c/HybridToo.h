#pragma once
#include <pebble.h>

#define SETTINGS_KEY 869

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
  bool UseWeather;
  bool UseUVI;
  bool HealthOn;
  int UpSlider;
 // char WeatherTemp [8];
 // char TempFore [10];
  char moonstring[4];
  char sunsetstring[8];
  char sunrisestring[8];
  char sunsetstring12[8];
  char sunrisestring12[8];
  char tempstring[8];
  char windstring[10];
  char windavestring[10];
  char iconnowstring[4];
  char iconforestring[4];
  char icon1hstring[4];
  char windiconnowstring[4];
  char windiconavestring[4];
  char temphistring[10];
  int WeatherUnit;
  char WindUnit [8];
  //char WeatherProv [4];
  #ifndef PBL_PLATFORM_APLITE
  int rainstring;
  int popstring;
  int rainfore;
  int popmax;
  int popmin;
  int UVIndexMax;
  int UVIndexNow;
  int UVIndexDay;
  char RainUnit [8];
  char PressureUnit [8];
  int pressurenow;
  int pressure1h;
  int barotrend;
  #endif
  int Weathertimecapture;
  GColor UVMaxColor;
  GColor UVNowColor;
  GColor UVArcColor;
  GColor WeatherColor;
} __attribute__((__packed__)) ClaySettings;

_Static_assert(sizeof(ClaySettings) <= 256, "ClaySettings exceeds Pebble 256-byte persist limit!");


