#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <Preferences.h>
#include <NTPClient.h>
#include <HTTPClient.h>
#include <BH1750.h>
#include "DHT.h"
#include "configuration.h"

#define DHTTYPE DHT11

const int _powerCtrl = GPIO_NUM_4;
const int _i2cSDA = GPIO_NUM_25;
const int _i2cSCL = GPIO_NUM_26;
const int _dhtPin = GPIO_NUM_16;
const int _soilPin = GPIO_NUM_32;
const int _voltageAdc = GPIO_NUM_33;
const int _salinityPin = GPIO_NUM_34;
const int _lightPin = GPIO_NUM_35;

int wifiStatus = WL_IDLE_STATUS;

char deviceid[21];
int _updateDataInSeconds;
String _plantID;
String _wifiSSID;
String _wifiPassword;
String _ntpServer;
String _influxDBHost;
int _influxDBPort;
String _influxDBDatabase;
String _InfluxURI;

Preferences preferences;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);
HTTPClient http;
BH1750 lightMeter(_lightPin);
DHT dht(_dhtPin, DHTTYPE);

void getApplicationConfiguration() {
  Serial.print("Reading Configuration");
  preferences.begin("configuration", true);
 
  _updateDataInSeconds = preferences.getInt("UpdateInSeconds");
  _plantID = preferences.getString("PlantID");
  _wifiSSID = preferences.getString("WifiSSID");
  _wifiPassword = preferences.getString("WifiPassword");
  _ntpServer = preferences.getString("NtpServer");
  _influxDBHost = preferences.getString("InfluxDBHost");
  _influxDBPort = preferences.getInt("InfluxDBPort");
  _influxDBDatabase = preferences.getString("InfluxDBDB");
  _InfluxURI = "/write?db=" + _influxDBDatabase + "&precision=s";

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
  long rssiStrength = WiFi.RSSI();
  Serial.print(" Signal strength (RSSI): ");
  Serial.println(rssiStrength);
}

void setupWifi() {
  while (wifiStatus != WL_CONNECTED) 
  {
    Serial.println("Attempting to connect to SSID: " + _wifiSSID) ;
    wifiStatus = WiFi.begin(_wifiSSID.c_str(), _wifiPassword.c_str());
    delay(2000);
  }

  Serial.println("Connected");
  printWifiStatus();
  printNetworkStatus();
}

void setupNTPClient() {
  NTPClient timeClient(ntpUDP, _ntpServer.c_str(), 3600, 15 * 60 * 1000);
  timeClient.begin();
}

void printToSerial(int waterlevel, float humidity, float temperature, float lux, uint32_t salinity, float voltage) {
  Serial.print(timeClient.getEpochTime());
  Serial.print(" " + timeClient.getFormattedTime());
  Serial.print(" DeviceId: ");
  Serial.print(deviceid);
  Serial.print(" waterlevel: ");
  Serial.print(waterlevel);
  Serial.print(" humidity: ");
  Serial.print(humidity);
  Serial.print(" temperature: "); 
  Serial.print(temperature);
  Serial.print(" lux: "); 
  Serial.print(lux);
  Serial.print(" salinity: ");
  Serial.print(salinity);
  Serial.print(" voltage: ");
  Serial.print(voltage);
}

void sendToInfluxDB(int waterlevel, float humidity, float temperature, float lux, uint32_t salinity, float voltage) {  
  String Data = "plant_measurement";
  Data.concat(",device=");
  Data.concat(deviceid);
  Data.concat(",plant=" + _plantID);
  Data.concat(" ");
  Data.concat("waterlevel=");
  Data.concat(waterlevel);
  Data.concat(",temperature=");
  Data.concat(temperature);
  Data.concat(",humidity=");
  Data.concat(humidity);
  Data.concat(",lux=");
  Data.concat(lux);
  Data.concat(",salinity=");
  Data.concat(salinity);
  Data.concat(",voltage=");
  Data.concat(voltage);
  Data.concat(" ");
  Data.concat(timeClient.getEpochTime());
  
  http.setReuse(true);
  http.begin("10.0.0.93",8086,_InfluxURI);
  http.addHeader("Content-Type", "text/plain");  
  int httpCode = http.POST(Data);      
  http.end();

  if (httpCode == 204 ) {
    Serial.print(" *SENT* ");
  }  
} 

int getWaterLevel() {
  int waterlevel = analogRead(_soilPin);
  waterlevel = map(waterlevel, 0, 4095, 1000, 0);
  waterlevel = constrain(waterlevel, 0, 1000);
  return waterlevel;
}

uint32_t readSalinity()
{
    uint8_t samples = 120;
    uint32_t humi = 0;
    uint16_t array[120];

    for (int i = 0; i < samples; i++) {
        array[i] = analogRead(_salinityPin);
        delay(2);
    }
    std::sort(array, array + samples);
    for (int i = 0; i < samples; i++) {
        if (i == 0 || i == samples - 1)continue;
        humi += array[i];
    }
    humi /= samples - 2;
    return humi;
}

uint16_t readSoil()
{
    uint16_t soil = analogRead(_soilPin);
    return map(soil, 0, 4095, 100, 0);
}

float readVoltage()
{
    int vref = 1100;
    uint16_t volt = analogRead(_voltageAdc);
    float voltageMV = ((float)volt / 4095.0) * 2.0 * 3.3 * (vref);
    return voltageMV;
}

void setup() {
  Serial.begin(115200);

  pinMode(_powerCtrl, OUTPUT);
  digitalWrite(_powerCtrl, 1);
  delay(200);
  Wire.begin(_i2cSDA, _i2cSCL);
	delay(200);

  dht.begin();
  lightMeter.begin();
  sprintf(deviceid, "%" PRIu64, ESP.getEfuseMac());
  
  //setConfigurationPreferences();
  getApplicationConfiguration();
  setupWifi();
  setupNTPClient();
}

void loop() {
  int waterlevel = getWaterLevel();
  float lux = lightMeter.readLightLevel();
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();
  uint32_t salinity = readSalinity();
  float voltageMV = readVoltage();

  timeClient.update();

  printToSerial(waterlevel,humidity,temperature,lux,salinity,voltageMV);
  sendToInfluxDB(waterlevel,humidity,temperature,lux,salinity,voltageMV);
  Serial.println("");

  delay(_updateDataInSeconds * 1000);
} 