
# Plant Monitor

Simple plant monitor based on the [LILYGO® T-Higrow ESP32 board](https://www.banggood.com/LILYGO-T-Higrow-ESP32-WiFi-+-bluetooth-+-DHT11-Soil-Temperature-And-Humidity-Sensor-Module-p-1196250.html). For monitoring of my chilli plants with InfluxDB and Grafana.

Logs the following data:

* Air temperature and humidity
* Light amount
* Soil moisture and salinity
* Device voltage  
  
## Configuration  

Rename file ***configuration_template.h*** to ***configuration.h***. Enable configuration by uncommenting *setConfigurationPreferences()* in the *setup()* method  

Add the following values:

**UpdateInSeconds:** Interval in seconds for sending data to InfluxDB  
**WifiSSID:** SSID for wifi network  
**WifiPassword:** Wifi password  
**NtpServer:** Time and date server (europe.pool.ntp.org)  
**InfluxDBHost:** Hostname or IP address to InfluxDB server  
**InfluxDBPort:** InfluxDB port (Default 8086)  
**InfluxDBDB:** Database name on InfluxDB server  
**PlantID:** ID for plant when logging to InfluxDB  
