#include <ESP8266WiFi.h>
#include <WiFiClient.h>

const char* ssid = "Bb";             // Nama WiFi
const char* password = "14012004";     // Kata sandi WiFi

const char* server = "api.thingspeak.com";
const char* apiKey = "1YAZT75TA4WFVIPK";       // Kunci API Write dari Thingspeak
const unsigned long postingInterval = 5L * 1000L;  // Interval pengiriman data (5 detik)

const int analogInPin = A0;                 // Pin analog yang terhubung ke sensor kelembaban tanah

WiFiClient client;
unsigned long lastDataSentTime = 0;

void setup() {
  Serial.begin(115200);
  delay(10);
  pinMode(analogInPin, INPUT);
  connectToWiFi();
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    float soilMoisture = readSoilMoisture();
    Serial.print("Soil Moisture: ");
    Serial.println(soilMoisture);
    
    unsigned long currentMillis = millis();
    if (currentMillis - lastDataSentTime >= postingInterval) {
      sendToThingspeak(soilMoisture);
      lastDataSentTime = currentMillis;
    }
  }
}

void connectToWiFi() {
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

float readSoilMoisture() {
  int sensorValue = analogRead(analogInPin);
  float soilMoisture = map(sensorValue, 0, 1023, 0, 100);
  return soilMoisture;
}

void sendToThingspeak(float value) {
  if (!client.connected()) {
    if (!client.connect(server, 80)) {
      Serial.println("Failed to connect to Thingspeak server");
      return;
    }
  }
  
  String postStr = "api_key=";
  postStr += apiKey;
  postStr += "&field1=";
  postStr += String(value);
  postStr += "\r\n";

  client.print("POST /update HTTP/1.1\n");
  client.print("Host: api.thingspeak.com\n");
  client.print("Connection: close\n");
  client.print("X-THINGSPEAKAPIKEY: ");
  client.print(apiKey);
  client.print("\n");
  client.print("Content-Type: application/x-www-form-urlencoded\n");
  client.print("Content-Length: ");
  client.print(postStr.length());
  client.print("\n\n");
  client.print(postStr);

  // Wait for the server to respond
  unsigned long timeout = millis();
  while (client.available() == 0) {
    if (millis() - timeout > 5000) {
      Serial.println("Error: Timeout occurred while waiting for Thingspeak response");
      client.stop();
      return;
    }
  }

  // Print response from server
  while (client.available()) {
    Serial.write(client.read());
  }
}