#include <Arduino.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#include <WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <Preferences.h>
#define DHTTYPE DHT11

const int LEDPIN = 16;
const int DHTPIN = 22;
const int SOIL_PIN = 32;
int wifiStatus = WL_IDLE_STATUS;

char deviceid[21];
String wifiSsid;
String wifiPwd;
String ntpServer;

DHT dht(DHTPIN, DHTTYPE);
Preferences preferences;
WiFiClient net;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

void getConfigurationPreferences() {
  Serial.print("Reading Configuration");
  preferences.begin("configuration", true);
  wifiSsid = preferences.getString("wifi_ssid");
  wifiPwd = preferences.getString("wifi_pwd");
  ntpServer = preferences.getString("ntp_server");
  preferences.end();
  Serial.println("...Done");
}



void printWifiData() {
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);
}

void printCurrentNet() {
  Serial.print("SSID: ");
  Serial.print(WiFi.SSID());
  long rssi = WiFi.RSSI();
  Serial.print(" Signal strength (RSSI): ");
  Serial.println(rssi);
}

void setupWifi() {

  while (wifiStatus != WL_CONNECTED) 
  {
    Serial.println("Attempting to connect to SSID: " + wifiSsid) ;
    wifiStatus = WiFi.begin(wifiSsid.c_str(), wifiPwd.c_str());
    delay(10000);
  }

  Serial.println("Connected:");
  printWifiData();
  printCurrentNet();
}

void setupNTPClient() {
  NTPClient timeClient(ntpUDP, ntpServer.c_str(), 3600, 15 * 60 * 1000);
  timeClient.begin();
}

void setup() {
  Serial.begin(9600); 
  sprintf(deviceid, "%" PRIu64, ESP.getEfuseMac());

  dht.begin();

  setConfigurationPreferences();
  getConfigurationPreferences();
  setupWifi();
  setupNTPClient();
}

void loop() {
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();

  int waterlevel = analogRead(SOIL_PIN);
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
