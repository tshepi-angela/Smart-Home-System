#include "esp_camera.h"
#include <WiFi.h>

// --- WIFI CONFIGURATION ---
const char *ssid = "Horisani";     
const char *password = "11111111";
// --- CAMERA PIN CONFIGURATION (AI-THINKER BOARD TYPE) ---
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

WiFiServer streamServer(81); // Dedicated video stream listener port

void startCameraServer() {
  streamServer.begin();
  Serial.print("\nCamera Ready! Use URL string: http://");
  Serial.print(WiFi.localIP());
  Serial.println(":81/stream"); // This is the exact route URL your app.py targets!
}

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();

  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0; 
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM; 
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM; 
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000; 
  config.pixel_format = PIXFORMAT_JPEG; 
  
  // High-performance image buffer setup
  if(psramFound()){ 
    config.frame_size = FRAMESIZE_VGA; // Configured for fluid web dashboard playback
    config.jpeg_quality = 12;  
    config.fb_count = 2; 
    config.grab_mode = CAMERA_GRAB_LATEST; 
  } else {
    config.frame_size = FRAMESIZE_SVGA; 
    config.jpeg_quality = 12;
    config.fb_count = 1; 
    config.fb_location = CAMERA_FB_IN_DRAM; 
  }

  // Initialize the camera engine hardware
  esp_err_t err = esp_camera_init(&config); 
  if (err != ESP_OK) { 
    Serial.printf("Camera init failed with error 0x%x", err); 
    return; 
  }

  // Connect to your mobile hotspot network
  WiFi.begin(ssid, password);
  Serial.print("Connecting to Hotspot");
  while (WiFi.status() != WL_CONNECTED) { 
    delay(500); 
    Serial.print("."); 
  }
  
  Serial.println("\nWiFi Connected!");

  // Start handling stream sockets
  startCameraServer();
}

void loop() {
  WiFiClient client = streamServer.available();
  if (client) {
    // Write out the raw multipart HTTP video header layout stream definitions
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: multipart/x-mixed-replace; boundary=frame");
    client.println();

    while (client.connected()) {
      camera_fb_t * fb = esp_camera_fb_get();
      if (!fb) {
        Serial.println("Camera capture failed");
        break;
      }

      // Format frame payload packages cleanly
      client.printf("--frame\r\nContent-Type: image/jpeg\r\nContent-Length: %d\r\n\r\n", fb->len);
      client.write(fb->buf, fb->len);
      client.print("\r\n");
      
      esp_camera_fb_return(fb); // Clear up the buffer instantly to prevent memory memory crashes
      delay(40); // Standardizes ~25 Frames Per Second (FPS) delivery speed
    }
    client.stop();
  }
  delay(1);
}