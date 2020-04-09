#include <Arduino.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#include <WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <Preferences.h>

#define DHTTYPE DHT11
const int _ledPin = 16;
const int _dhtPin = 22;
const int _soilPin = 32;
int wifiStatus = WL_IDLE_STATUS;

char deviceid[21];
String _wifiSSID;
String _wifiPassword;
String _ntpServer;
String _influxDBServerUrl;
String _influxDBDatabase;
String _JWTBearerToken;

DHT dht(_dhtPin, DHTTYPE);
Preferences preferences;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

void getApplicationConfiguration() {
  Serial.print("Reading Configuration");
  preferences.begin("configuration", true);

  _wifiSSID = preferences.getString("_wifiSSID");
  _wifiPassword = preferences.getString("_wifiPassword");
  _ntpServer = preferences.getString("_ntpServer");
  _influxDBServerUrl = preferences.getString("_influxDBUrl");
  _influxDBDatabase = preferences.getString("_influxDBDatabase");
  _JWTBearerToken = preferences.getString("_JWTBearerToken");
  
  preferences.end();
  Serial.println("...Done");
}



void printNetworkStatus() {
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);
}

void printWifiStatus() {
  Serial.print("SSID: ");
  Serial.print(WiFi.SSID());
  long rssi = WiFi.RSSI();
  Serial.print(" Signal strength (RSSI): ");
  Serial.println(rssi);
}

void setupWifi() {

  while (wifiStatus != WL_CONNECTED) 
  {
    Serial.println("Attempting to connect to SSID: " + wifiSSID) ;
    wifiStatus = WiFi.begin(wifiSSID.c_str(), wifiPassword.c_str());
    delay(10000);
  }

  Serial.println("Connected");
  printWifiStatus();
  printNetworkStatus();
}

void setupNTPClient() {
  NTPClient timeClient(ntpUDP, ntpServer.c_str(), 3600, 15 * 60 * 1000);
  timeClient.begin();
}

void setup() {
  Serial.begin(9600);

  // Get ESP32 DeviceID 
  sprintf(deviceid, "%" PRIu64, ESP.getEfuseMac());

  dht.begin();

//  setConfigurationPreferences();
  getApplicationConfiguration();
  setupWifi();
  setupNTPClient();
}

void loop() {
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();

  int waterlevel = analogRead(_soilPin);
  waterlevel = map(waterlevel, 1500, 3200, 1000, 0);
  waterlevel = constrain(waterlevel, 0, 1000);

  timeClient.update();

  Serial.print(timeClient.getEpochTime());
  Serial.print(" " + timeClient.getFormattedTime());
  Serial.print(" DeviceId: ");
  Serial.print(deviceid);
  Serial.print(" waterlevel: ");
  Serial.print( waterlevel);
  Serial.print(" humidity: ");
  Serial.print(humidity);
  Serial.print(" temperature: "); 
  Serial.println(temperature);
  
  delay(2000);
} 