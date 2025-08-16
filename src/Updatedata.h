#ifndef UPDATEDATA_H
#define UPDATEDATA_H

#include <Arduino.h>

String windDirectionToText(float deg);
String describeWeatherCode(int code);
String formatWindSpeed(float windSpeed);
String getCurrentHourString();
bool fetchTimezoneFromGeoapify(float lat, float lon, char *tzBuffer, size_t bufferSize);
bool ensureFullTimeSync();
void getmyutc();
void loadWeatherPreferences();
int classifyWeatherCode(int code);
void fetchWeatherNow();
void fetchWeatherForecast();
void fetchAQI();

#endif
