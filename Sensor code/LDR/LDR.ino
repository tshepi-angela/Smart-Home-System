#include <WiFi.h>
#include <HTTPClient.h>

const char* ssid = "Horisani";
const char* password = "11111111";
const char* serverUrl = "http://172.20.10.2:5000/update";

const int ldrPin = 34;
const int ledPin = 25;
const int darknessThreshold = 500;

void setup() {
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);
  analogReadResolution(12); // standard 12-bit depth configuration
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
  Serial.println("\nLDR Ambient Node Active!");
}

void loop() {
  int ldrValue = analogRead(ldrPin);
  
  // Keep local control loop intact for immediate safety operations
  if (ldrValue < darknessThreshold) {
    digitalWrite(ledPin, HIGH); 
  } else {
    digitalWrite(ledPin, LOW);
  }

  if(WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(serverUrl);
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");

    String postData = "light=" + String(ldrValue);
    int httpResponseCode = http.POST(postData);
    
    Serial.print("LDR Value Sent. Response: ");
    Serial.println(httpResponseCode);
    http.end();
  }
  delay(8000);
}