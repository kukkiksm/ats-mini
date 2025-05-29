#ifndef GLOBALS_H
#define GLOBALS_H

#include <Arduino.h>

extern unsigned long lastConnectMillis2;
const char *clockGetDate();

extern bool textStop;

extern int doText;
extern int roundLoop;
extern int roundDisplay;
extern int maxround;
extern int aqiValue;
extern int pm10Value;
extern int pm25Value;
extern int updateWeather;

extern String dateNow;
extern String tempStr;
extern String rainStr;
extern String weatherStr;
extern String windStr;
extern String aqiStr;
extern String validLatitude;
extern String validLongitude;
extern String validUnit;
extern String myUTC;
extern String forecastText[3];

#endif
