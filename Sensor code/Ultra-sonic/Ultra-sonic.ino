#include <WiFi.h>
#include <HTTPClient.h>

const char* ssid = "Horisani";
const char* password = "11111111";
const char* serverUrl = "http://172.20.10.2:5000/update";

const int trigPin = 19;
const int echoPin = 13;

void setup() {
  Serial.begin(115200);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
  Serial.println("\nUltrasonic Node Active!");
}

void loop() {
  digitalWrite(trigPin, LOW); delayMicroseconds(2);
  digitalWrite(trigPin, HIGH); delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  long duration = pulseIn(echoPin, HIGH);
  float distance = duration * 0.034 / 2;

  if(WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(serverUrl);
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");

    String postData = "dist=" + String(distance);
    int httpResponseCode = http.POST(postData);
    
    Serial.print("Distance Data Sent. Response: ");
    Serial.println(httpResponseCode);
    http.end();
  }
  delay(5000);
}