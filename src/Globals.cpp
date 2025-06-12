#include <Arduino.h>
#include <time.h>

static time_t lastSyncedTime = 0;
static uint32_t lastSyncedMicros = 0;
unsigned long millisUpdateWeather = 0;
unsigned long lastConnectMillis2 = 0;

bool shouldCycle = false;
bool textStop = false;

int doText = 1;
int roundLoop = 0;
int roundDisplay = 0;
int maxround = 4;
int visualStyle = 0;
int weatherType = 0;
int aqiValue = 0;
int pm10Value = 0;
int pm25Value = 0;
int updateWeather = 0;

String validLatitude = "14.10";
String validLongitude = "99.87";
String validUnit = "C";
String myUTC = "Asia/Bangkok";

String tempStr;
String rainStr;
String weatherStr;
String windStr;
String dateNow = "";
String forecastText[3] = {"", "", ""};

void clockStoreLastSynced() {
  lastSyncedTime = time(NULL);
  lastSyncedMicros = micros();
}



