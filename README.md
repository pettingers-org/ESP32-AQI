# ESP32-AQI
Use an ESP32 to read air quality from the Internet and activate a relay accordingly  

Using AirNow API current as of September 2026  

  *Details*  
  
ESP32-AQI uses an ESP32 microcontroller to poll air quality index (AQI) data from the US Government website https://www.airnow.gov . When the AQI number is above a set point (bad air quality) the ESP32 will activate a relay via GPIO pin.  When the air quality improves, (AQI falls below another set point) the relay is switched off.  AQI data is retrieved for a specific location set via postal zip code.
  
Wi-Fi parameters are set via captive portal when the ESP32 first boots.  If the ESP32 cannot connect to Wi-Fi (or at first boot) it will present itself as a captive portal AP and can be configured with a mobile phone or other Wi-Fi device.  Simply connect to the access point, and follow the prompts.  Once set, the Wi-Fi settings are non-volatile.  If the ESP32 cannot connect to the saved Wi-Fi AP/router in the future, it will revert to AP mode and allow the user to configure it again.  

The user needs to obtain an API key from https://docs.airnowapi.org/ and enter that into the code, along with the desired zip code.  The API data is currently limited (by AirNow) to 500 data pulls per hour, however, data on the site is usually updated every hour so more frequent pulls are not necessarily going to return better data.  The (configurable) poll frequency defaults to once every five minutes.  It is unlikely you would ever exceed the 500 polls per hour, even if you were running several devices.  

  *Use Cases*

So why would anyone want this?  Well, if you have an HVAC system that ducts fresh air to increase efficiency (e.g., pulling in cold outside air to reduce cooling load), you may wish to disable this during periods of high pollution such as wildfire or smog.  Or if you have electronic windows or vents controlled by something like Home Assistant, you may want to close those sources of outside air if pollutants are high.  

At a higher level of abstraction, the code is an exercise in parsing JSON data, and activating real-world devices based on the results.  

  *Prerequisites and Libraries*  

Assuming use of the Arduino IDE, configure your board to be an ESP32 variant before attempting a compile.  The only non-embedded libraries needed are ArduinoJson and WiFiManager.  Both are readily available under > Tools > Library Manager.
