// Using API current as of September 2026
// ESP32-AQI uses an ESP32 microcontroller to poll air quality index (AQI) data from the 
// US Government website www.airnow.gov. When the AQI number is above a set point (bad air quality)
// the ESP32 will activate a relay via GPIO pin.  When the air quality improves, (AQI falls below
// another set point) the relay is switched off.  AQI data is retrieved for a specific location
// set via postal zip code.
//
// WiFi parameters are set via captive portal when the ESP32 first boots.  If the ESP32 cannot connect
// to WiFi (or at first boot) it will present itself as a captive portal AP and can be configured
// with a mobile phone or other wifi device.  Simply connect to the access point, and follow the prompts.
// Once set, the WiFi settings are non-volatile.  If the ESP32 cannot connect to the saved WiFi AP/router
// in the future, it will revert to AP mode and allow the user to configure it again.

#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include <WiFiManager.h>


const char* AIRNOW_API_KEY = "INSERT-YOUR-API-KEY-HERE";    // API key available at docs.airnowapi.org
const char* ZIP_CODE       = "90210";                       // Your zip code

const int RELAY_PIN = 16;   // GPIO pin for connection to relay.  Logic HIGH = Active
const int AQI_ON    = 45;   // Turn relay ON above this AQI number
const int AQI_OFF   = 40;   // Turn relay OFF below this AQI number (for hysteresis)
const unsigned long POLL_MS = 5 * 60 * 1000;  // poll every 5 min.  Requests are limited to 500 per hour, and updates usually occur hourly

bool relayOn = false;

void setup() {
  Serial.begin(115200);
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);
  
  // Initialize WiFiManager
  WiFiManager wm;
  
  // Attempt to connect using saved credentials
  // If it fails, it starts a captive portal named "ESP32-AQI_AP" and will remember the supplied credentials
  if (!wm.autoConnect("ESP32-AQI_AP")) {
    Serial.println("Failed to connect");
    ESP.restart();
  }
  
  Serial.println("Connected to Wi-Fi!");
  Serial.println(WiFi.localIP());
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) { WiFi.reconnect(); delay(5000); return; }    // Make sure WiFi is up

  int aqi = fetchAQI();
  if (aqi >= 0) {
    Serial.printf("Current AQI = %d\n", aqi);

    if (!relayOn && aqi > AQI_ON) {
      digitalWrite(RELAY_PIN, HIGH); relayOn = true;
      Serial.println("AQI high -> relay ON");
    } else if (relayOn && aqi < AQI_OFF) {
      digitalWrite(RELAY_PIN, LOW); relayOn = false;
      Serial.println("AQI good -> relay OFF");
    }
  }
  delay(POLL_MS);
}

int fetchAQI() {
  String url = "https://www.airnowapi.org/aq/observation/current/ziplatlong/?format=application/json&zipcode="
               + String(ZIP_CODE) + "&API_KEY=" + String(AIRNOW_API_KEY);

  WiFiClientSecure client;
  client.setInsecure();  // Skip cert verification for maintainability
  HTTPClient http;

  http.useHTTP10(true);   // Convert to http 1.0 for de-serialize

  if (!http.begin(client, url)) { Serial.println("http.begin failed"); return -1; }

  int code = http.GET();
  if (code != HTTP_CODE_OK) {
    Serial.printf("HTTP error: %d\n", code);
    http.end(); return -1;
  }

  int maxAQI = -1;   // AQI lockdown in case we get bad data from the website
  JsonDocument doc;

DeserializationError error = deserializeJson(doc, http.getStream());

if (error) {
  Serial.print("deserializeJson() failed: ");
  Serial.println(error.c_str());
  return maxAQI;
}

for (JsonObject item : doc.as<JsonArray>()) {

  int nowcastAQI = item["nowcastAQI"]; // Extract AQI from website JSON data

  if (nowcastAQI > maxAQI) maxAQI = nowcastAQI;
}

  http.end();
  return maxAQI;
}