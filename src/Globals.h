#ifndef GLOBALS_H
#define GLOBALS_H

#include <Arduino.h>

const char *clockGetDate();
void clockStoreLastSynced();

extern unsigned long millisUpdateWeather;
extern unsigned long lastConnectMillis2;

extern bool shouldCycle;
extern bool textStop;

extern int visualStyle; 
extern int weatherType;
extern int aqiValue;
extern int pm10Value;
extern int pm25Value;
extern int updateWeather;
extern int doText;
extern int roundLoop;
extern int roundDisplay;
extern int maxround;

extern String tempStr;
extern String rainStr;
extern String weatherStr;
extern String windStr;
extern String dateNow;
extern String lastScrollText;
extern String scrollText;
extern String dateNow;
extern String validLatitude;
extern String validLongitude;
extern String validUnit;
extern String myUTC;
extern String forecastText[3];

#endif
