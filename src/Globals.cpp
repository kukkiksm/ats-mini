#include <Arduino.h>
#include <time.h>

const char *clockGetDate()
{
  static char dateStr[30];
  struct tm timeinfo;

  if (!getLocalTime(&timeinfo))
  {
    return "wait for update";
  }

  if (timeinfo.tm_year <= 70)
  {
    return "wait for update";
  }

  strftime(dateStr, sizeof(dateStr), "%A %d %B %Y", &timeinfo);
  return dateStr;
}

String validLatitude = "14.10";
String validLongitude = "99.87";
String validUnit = "C";
String myUTC = "Asia/Bangkok";

int aqiValue = 0;
int pm10Value = 0;
int pm25Value = 0;
int updateWeather = 0;

String tempStr;
String rainStr;
String weatherStr;
String windStr;

String forecastText[3] = {"", "", ""};

int doText = 1;
bool textStop = false;

int roundLoop = 0;
int roundDisplay = 0;
int maxround = 4; // ✅ จำนวนรอบทั้งหมด

int visualStyle = 0;
int weatherType = 0;

String dateNow = ""; // ✅ สำหรับแสดงวันที่ปัจจุบัน

unsigned long millisUpdateWeather = 0;
unsigned long lastConnectMillis2 = 0;
bool shouldCycle = false;