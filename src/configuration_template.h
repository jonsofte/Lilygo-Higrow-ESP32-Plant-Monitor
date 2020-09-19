#include <Arduino.h>
#include <Preferences.h>

Preferences setupPreferences;

void setApplicationConfiguration() {
  Serial.print("Writing Configuration");
  setupPreferences.begin("configuration", false);
  setupPreferences.clear();
  setupPreferences.putInt("UpdateInSeconds",10);
  setupPreferences.putString("WifiSSID","");
  setupPreferences.putString("WifiPassword","");
  setupPreferences.putString("NtpServer","");
  setupPreferences.putString("InfluxDBHost","");
  setupPreferences.putInt("InfluxDBPort", 8086);
  setupPreferences.putString("InfluxDBDB","");
  setupPreferences.putString("InfluxDBMeasurement","");
  setupPreferences.putString("PlantID","");
  setupPreferences.end();
  Serial.println("...Done");
}