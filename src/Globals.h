#ifndef GLOBALS_H
#define GLOBALS_H

#include <Arduino.h>

// Weather-related global values
extern String validLatitude;
extern String validLongitude;
extern String validUnit;
extern String myUTC;

extern int aqiValue;
extern int pm10Value;
extern int pm25Value;
extern int updateWeather;

extern String tempStr;
extern String rainStr;
extern String weatherStr;
extern String windStr;

extern String forecastText[3];

extern String dateNow;
extern String lastScrollText;
extern String scrollText;

extern int visualStyle; 
extern int weatherType;

// Display loop control (แก้ชนิดให้ถูกต้อง)
extern int doText;
extern bool textStop;
extern int roundLoop;
extern int roundDisplay;
extern int maxround;

// Date for display
extern String dateNow;

const char *clockGetDate();


extern unsigned long millisUpdateWeather;
extern unsigned long lastConnectMillis2;
extern bool shouldCycle;

#endif
