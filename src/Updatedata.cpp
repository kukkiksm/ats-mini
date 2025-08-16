#include <Arduino.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "updateData.h"
//#include "Preferences.h"
#include "Globals.h"
#include "Storage.h"

// External declarations (from main project)
extern Preferences preferences;
extern int aqiValue, pm10Value, pm25Value;
extern String tempStr, rainStr, weatherStr, windStr;
extern int updateWeather;
extern String forecastText[3];

const char *GEOAPIFY_API_KEY = "cba79e2cf3754766a051074cba6cc793";

void loadWeatherPreferences()
{
  prefs.begin("settings", true, STORAGE_PARTITION);
  validLatitude  = prefs.getString("latitude",  "14.10");
  validLongitude = prefs.getString("longitude", "99.87");
  validUnit      = prefs.getString("unit",      "C");
  prefs.end();
}

bool ensureFullTimeSync()
{
  struct tm timeinfo;

  if (getLocalTime(&timeinfo) && timeinfo.tm_year > 70) {
    return true;
  }

  configTime(0, 0, "pool.ntp.org");

  for (int i = 0; i < 10; i++) {
    if (getLocalTime(&timeinfo) && timeinfo.tm_year > 70) {
      return true;
    }
    delay(500);
  }

  return false;
}

String windDirectionText(float deg)
{
  if (deg < 22.5 || deg >= 337.5)
    return "North";
  if (deg < 67.5)
    return "Northeast";
  if (deg < 112.5)
    return "East";
  if (deg < 157.5)
    return "Southeast";
  if (deg < 202.5)
    return "South";
  if (deg < 247.5)
    return "Southwest";
  if (deg < 292.5)
    return "West";
  return "Northwest";
}

String windDirectionToText(float deg)
{
  String direction = windDirectionText(deg);
  return direction;
}

// Function to interpret weather codes
String describeWeatherCode(int code)
{
  if (code == 0)
    return "Clear";
  if (code == 1)
    return "Mainly clear";
  if (code == 2)
    return "Partly cloudy";
  if (code == 3)
    return "Overcast Cloud";
  if (code == 45)
    return "Fog";
  if (code >= 51 && code <= 55)
    return "Drizzle";
  if (code >= 61 && code <= 65)
    return "Rain";
  if (code >= 66 && code <= 67)
    return "Freezing rain";
  if (code >= 71 && code <= 75)
    return "Snow fall";
  if (code == 77)
    return "Snow grains";
  if (code >= 80 && code <= 82)
    return "Rain showers";
  if (code == 85 || code == 86)
    return "Snow showers";
  if (code == 95)
    return "Thunderstorm";
  if (code == 96 || code == 99)
    return "Thunderstorm with hail";
  return "Unknown";
}

// Function to convert wind speed from m/s to km/h
String formatWindSpeed(float windSpeed)
{
  // return String(windSpeed * 3.6, 1) + " kph";
  if (validUnit == "F")
  {
    return String(windSpeed, 1) + " mph";
  }
  else
  {
    return String(windSpeed, 1) + " kph";
  }
}

bool fetchTimezoneFromGeoapify(float lat, float lon, char *tzBuffer, size_t bufferSize)
{
  HTTPClient http;
  String url = "https://api.geoapify.com/v1/geocode/reverse?lat=" + String(lat, 6) +
               "&lon=" + String(lon, 6) +
               "&format=json&apiKey=" + GEOAPIFY_API_KEY;

  http.begin(url);
  int httpCode = http.GET();

  if (httpCode == 200)
  {
    String payload = http.getString();
    DynamicJsonDocument doc(2048);
    if (deserializeJson(doc, payload) == DeserializationError::Ok)
    {
      if (doc["results"].size() > 0)
      {
        const char *tz = doc["results"][0]["timezone"]["name"];
        if (tz)
        {
          strlcpy(tzBuffer, tz, bufferSize);
          http.end();
          return true;
        }
      }
    }
  }

  http.end();
  return false;
}

String getCurrentHourString()
{
  time_t now = time(nullptr);
  struct tm *t = localtime(&now);
  char buf[25];
  strftime(buf, sizeof(buf), "%Y-%m-%dT%H:00", t); // Format: 2025-05-05T15:00
  return String(buf);
}

void getmyutc()
{
  char tzBuffer[64];
  if (fetchTimezoneFromGeoapify(validLatitude.toFloat(), validLongitude.toFloat(), tzBuffer, sizeof(tzBuffer)))
  {
    myUTC = String(tzBuffer); // เช่น "Asia/Bangkok"
  }
  else
  {
    myUTC = "auto"; // fallback
  }
}

int classifyWeatherCode(int code)
{
  if (code == 0)
    return 0; // Clear
  if (code == 1 || code == 2 || code == 3)
    return 1; // Clouds
  if ((code >= 51 && code <= 67) || (code >= 80 && code <= 82))
    return 2; // Rain
  if (code == 95 || code == 96 || code == 99)
    return 3; // Thunder
  if (code == 45 || code == 48)
    return 4; // Fog
  if ((code >= 71 && code <= 77) || code == 85 || code == 86)
    return 5; // Snow
  return 1;   // Default = Clouds
}

void fetchWeatherNow()
{
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo) || timeinfo.tm_year <= 70)
  {
    Serial.println("⚠️ Time not synced properly. Skipping fetchWeather.");
    return;
  }

  Serial.println("fetchWeatherNow()");
  Serial.print("validLatitude: ");
  Serial.println(validLatitude);
  Serial.print("validLongitude: ");
  Serial.println(validLongitude);

  if (updateWeather == 1)
    return;

  String weatherENUrl = "https://api.open-meteo.com/v1/forecast?latitude=" + validLatitude + "&longitude=" + validLongitude +
                        "&current=weather_code,temperature_2m,precipitation,cloudcover,wind_speed_10m,wind_direction_10m&wind_speed_unit=kmh";
  if (validUnit == "F")
    weatherENUrl += "&temperature_unit=fahrenheit&wind_speed_unit=mph";

  Serial.print("[Weather] Fetching: ");
  Serial.println(weatherENUrl);

  HTTPClient http;
  http.setTimeout(10000);
  http.begin(weatherENUrl);
  int httpCode = http.GET();

  if (httpCode == HTTP_CODE_OK)
  {
    String payload = http.getString();
    DynamicJsonDocument doc(2048);
    DeserializationError error = deserializeJson(doc, payload);

    if (error)
    {
      Serial.printf("[Weather] ❌ JSON parse failed: %s\n", error.c_str());
      http.end();
      return;
    }

    JsonObject current = doc["current"];
    float temp = current["temperature_2m"] | NAN;
    float rain = current["precipitation"] | 0.0f;
    int code = current["weather_code"] | -1;
    float windSpeed = current["wind_speed_10m"] | 0.0f;
    float windDir = current["wind_direction_10m"] | 0.0f;

    if (isnan(temp) || code < 0)
    {
      Serial.println("[Weather] ❌ Invalid or incomplete weather data.");
      http.end();
      return;
    }

    // ✅ จัดประเภท weatherType
    weatherType = classifyWeatherCode(code);
    Serial.printf("☁️ WeatherType = %d (code: %d)\n", weatherType, code);

    String windDesc = windDirectionToText(windDir);
    String desc = describeWeatherCode(code);
    String windSpeedStr = formatWindSpeed(windSpeed);

    tempStr = "Temp " + String(temp, 1) + validUnit;
    rainStr = "Rain " + String(rain, 1) + " mm";
    weatherStr = desc;
    windStr = "Wind: " + windDesc + " >> " + windSpeedStr;

    Serial.println("📍 Current weather (Open-Meteo):");
    Serial.println(weatherStr);
    Serial.println(tempStr);
    Serial.println(rainStr);
    Serial.println(windStr);
    Serial.print("weatherType: ");
    Serial.println(weatherType);
  }
  else
  {
    Serial.printf("[Weather] ❌ HTTP error: %d\n", httpCode);
  }

  http.end();
}

void fetchWeatherForecast()
{
  HTTPClient http;
  String url = "https://api.open-meteo.com/v1/forecast?latitude=" + validLatitude +
               "&longitude=" + validLongitude +
               "&daily=weathercode,temperature_2m_max,temperature_2m_min" +
               "&forecast_days=4&timezone=" + myUTC;

  Serial.println("[Forecast] Requesting: " + url);
  http.begin(url);
  int httpCode = http.GET();

  if (httpCode != HTTP_CODE_OK)
  {
    Serial.printf("[Forecast] ❌ HTTP error: %d\n", httpCode);
    for (int i = 0; i < 3; i++)
      forecastText[i] = "Forecast error";
    http.end();
    return;
  }

  String payload = http.getString();
  DynamicJsonDocument doc(16 * 1024);
  DeserializationError error = deserializeJson(doc, payload);

  if (error)
  {
    Serial.print("[Forecast] ❌ JSON parse failed: ");
    Serial.println(error.c_str());
    for (int i = 0; i < 3; i++)
      forecastText[i] = "Parse error";
    http.end();
    return;
  }

  JsonObject daily = doc["daily"];
  JsonArray dates = daily["time"];
  JsonArray weatherCodes = daily["weathercode"];
  JsonArray tempsMax = daily["temperature_2m_max"];
  JsonArray tempsMin = daily["temperature_2m_min"];

  const char *dayNames[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
  const char *weatherDesc[] = {
      "Clear", "Mainly clear", "Partly cloudy", "Overcast", "Fog",
      "Drizzle", "Rain", "Snow", "Thunder"};

  for (int i = 1; i <= 3; i++)
  {
    String dateStr = dates[i];
    int year = dateStr.substring(0, 4).toInt();
    int month = dateStr.substring(5, 7).toInt();
    int day = dateStr.substring(8, 10).toInt();

    struct tm timeinfo = {};
    timeinfo.tm_year = year - 1900;
    timeinfo.tm_mon = month - 1;
    timeinfo.tm_mday = day;
    mktime(&timeinfo);
    String dayName = dayNames[timeinfo.tm_wday];

    int code = weatherCodes[i];
    String desc = weatherDesc[min(code / 10, 8)]; // safe index

    float tMin = tempsMin[i];
    float tMax = tempsMax[i];

    if (validUnit == "F")
    {
      tMin = tMin * 9.0 / 5.0 + 32;
      tMax = tMax * 9.0 / 5.0 + 32;
    }

    forecastText[i - 1] = dayName + " " + desc + " " +
                          String(tMin, 0) + "°" + validUnit + "/" +
                          String(tMax, 0) + "°" + validUnit;

    Serial.println("[Forecast] " + forecastText[i - 1]);
  }

  http.end();
}

void fetchAQI()
{
  Serial.println("[AQI] Fetching data...");

  HTTPClient http;
  http.setTimeout(10000); // Timeout after 10 seconds

  String url = "https://air-quality-api.open-meteo.com/v1/air-quality?latitude=" + validLatitude +
               "&longitude=" + validLongitude + "&hourly=us_aqi,pm10,pm2_5&timezone=" + myUTC;

  Serial.println(url);

  http.begin(url);           // Start the HTTP request
  int httpCode = http.GET(); // Send GET request

  if (httpCode == HTTP_CODE_OK)
  {
    String payload = http.getString();

    // Dynamically allocate enough memory for parsing based on payload size
    DynamicJsonDocument doc(payload.length() + 1024); // Add a little buffer
    DeserializationError error = deserializeJson(doc, payload);

    if (error)
    {
      Serial.printf("[AQI] ❌ JSON parsing error: %s\n", error.c_str());
      http.end();
      return; // Exit early if JSON parsing fails
    }

    // Ensure that the 'hourly' key exists in the JSON response
    if (doc.containsKey("hourly"))
    {
      JsonObject hourly = doc["hourly"];
      JsonArray times = hourly["time"];
      JsonArray aqi = hourly["us_aqi"];
      JsonArray pm10 = hourly["pm10"];
      JsonArray pm2_5 = hourly["pm2_5"];

      String nowHour = getCurrentHourString(); // Get the current hour
      int foundIndex = -1;

      // Try to find the current hour
      for (int i = 0; i < times.size(); i++)
      {
        String timeStr = times[i].as<String>();
        if (timeStr == nowHour)
        {
          foundIndex = i;
          break;
        }
      }

      // If not found, fallback to "12:00" of the latest date
      if (foundIndex == -1 && times.size() > 0)
      {
        String lastDate = times[times.size() - 1].as<String>().substring(0, 10); // YYYY-MM-DD
        String noonHour = lastDate + "T12:00";

        for (int i = 0; i < times.size(); i++)
        {
          if (times[i].as<String>() == noonHour)
          {
            foundIndex = i;
            Serial.println("[AQI] ⚠️ Fallback to noon data: " + noonHour);
            break;
          }
        }

        // If still not found, fallback to last
        if (foundIndex == -1)
        {
          foundIndex = times.size() - 1;
          Serial.println("[AQI] ⚠️ Fallback to latest available data: " + times[foundIndex].as<String>());
        }
      }

      if (foundIndex != -1)
      {
        aqiValue = aqi[foundIndex];
        pm10Value = pm10[foundIndex];
        pm25Value = pm2_5[foundIndex];

        Serial.printf("[AQI] ✅ Data at %s:\n", times[foundIndex].as<String>().c_str());
        Serial.printf("       - US AQI: %d\n", aqiValue);
        Serial.printf("       - PM10: %.1f µg/m³\n", pm10Value);
        Serial.printf("       - PM2.5: %.1f µg/m³\n", pm25Value);
      }
      else
      {
        Serial.println("[AQI] ❌ No data found at all.");
      }
    }
    else
    {
      Serial.println("[AQI] ❌ Missing 'hourly' data in API response.");
    }
  }
  else
  {
    Serial.printf("[AQI] ❌ HTTP request failed, code: %d\n", httpCode);
  }

  http.end(); // Always end the HTTP request to release resources
}
