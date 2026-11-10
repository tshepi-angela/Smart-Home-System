#include <WiFi.h>
#include <HTTPClient.h>

const char* ssid = "Horisani";
const char* password = "11111111";
const char* serverUrl = "http://172.20.10.2:5000/update";

const int pirPin = 14;

void setup() {
  Serial.begin(115200);
  pinMode(pirPin, INPUT);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
  Serial.println("\nPIR Security Node Active!");
}

void loop() {
  int motion = digitalRead(pirPin);

  if(WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(serverUrl);
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");

    String postData = "motion=" + String(motion);
    int httpResponseCode = http.POST(postData);
    
    Serial.print("PIR Status Sent. Response: ");
    Serial.println(httpResponseCode);
    http.end();
  }
  delay(3000); // Shorter interval for active area monitoring
}