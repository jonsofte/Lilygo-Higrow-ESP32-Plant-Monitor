#include <Arduino.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#include <WiFi.h>

#define DHTTYPE DHT11

const int LEDPIN = 16;
const int DHTPIN = 22;
const int SOIL_PIN = 32;

uint64_t chipid;
char deviceid[21];

char ssid[] = "Telenor8820jag";

int status = WL_IDLE_STATUS;

DHT dht(DHTPIN, DHTTYPE);

WiFiClient net;
//MQTTClient client;

void printWifiData() {
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);
}

void printCurrentNet() {
  Serial.print("SSID: ");
  Serial.print(WiFi.SSID());

  long rssi = WiFi.RSSI();
  Serial.print(" Signal strength (RSSI):");
  Serial.println(rssi);
}

void setup() {
  Serial.begin(9600);
  dht.begin();

  chipid = ESP.getEfuseMac();
  sprintf(deviceid, "%" PRIu64, chipid);

  Serial.println(WiFi.status()); 

  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to WPA SSID: ");
    Serial.println(ssid);
    status = WiFi.begin(ssid, pwd);
    delay(10000);
  }

  Serial.print("Connected to the network");
  printCurrentNet();
  printWifiData();

//  client.begin("10.0.0.10",net);

//  while (!client.connect("chilli", "", "")) {
//    Serial.print(".");
 //   delay(1000);
 // }

//  Serial.println("\nconnected!");
//  client.subscribe("/hello");
}

void loop() {
  int waterlevel = analogRead(SOIL_PIN);
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();

  waterlevel = map(waterlevel, 1500, 3200, 1000, 0);
  waterlevel = constrain(waterlevel, 0, 1000);

  Serial.print("DeviceId: ");
  Serial.print(deviceid);
  Serial.print(" waterlevel: ");
  Serial.print( waterlevel);
  Serial.print(" humidity: ");
  Serial.print(humidity);
  Serial.print(" temperature: "); 
  Serial.println(temperature);
 // client.publish("/temperature", String(temperature));
  delay(5000);
} 