#include "DHT.h"
#include <WiFi.h>
#include <HTTPClient.h>

const char* ssid = "Horisani";
const char* password = "11111111";
const char* serverUrl = "http://172.20.10.2:5000/update";

#define DHTPIN 4
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(115200);
  dht.begin();
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
  Serial.println("\nDHT22 Node Active!");
}

void loop() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if (!isnan(h) && !isnan(t)) {
    if(WiFi.status() == WL_CONNECTED) {
      HTTPClient http;
      http.begin(serverUrl);
      http.addHeader("Content-Type", "application/x-www-form-urlencoded");

      String postData = "temp=" + String(t) + "&hum=" + String(h);
      int httpResponseCode = http.POST(postData);
      
      Serial.print("DHT22 Data Sent. Response: ");
      Serial.println(httpResponseCode);
      http.end();
    }
  }
  delay(10000); // 10 second environmental tracking interval
}