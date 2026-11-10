# Smart Home System

A smart home monitoring and control system built with ESP32 sensors and a Flask web dashboard.

Monitor temperature, light, motion, distance and camera feed from your phone or laptop, all from one dashboard.

## Features

- **Live Sensor Monitoring** - Temperature & Humidity, Light, Motion, Distance
- **Camera Feed** - ESP32-CAM live stream in the browser
- **Web Dashboard** - Built with Flask, HTML, CSS
- **Database Logging** - Sensor data saved to SQL database
- **Automated Control** - Logic for lights / curtains based on sensor readings

## Tech Stack

- **Hardware:** ESP32, ESP32-CAM, DHT22, LDR, PIR, HC-SR04 Ultrasonic
- **Backend:** Python, Flask
- **Frontend:** HTML, CSS, Bootstrap, JavaScript
- **Database:** MySQL
- **Tools:** VS Code, Arduino IDE, GitHub

## Project Structure
Smart-Home-System/
├── Sensor code/
│   ├── CameraWebServer/   # ESP32-CAM live stream
│   ├── DHT22/             # Temperature & Humidity sensor
│   ├── LDR/               # Light sensor
│   ├── PIR/               # Motion sensor
│   └── Ultra-sonic/       # Distance sensor
│
└── Web/
    ├── app.py             # Flask web app
    ├── templates/         # HTML pages
    ├── static/            # CSS, JS, images
    └── database.sql       # Database schema

## How To Run

### 1. Sensor Code (ESP32)
1. Open any folder in `Sensor code/` in Arduino IDE
2. Select Board: ESP32 Dev Module
3. Select your COM port
4. Upload to your ESP32

### 2. Web Dashboard (Flask)
```bash
cd Web
python -m venv venv
venv\Scripts\activate
pip install flask
python app.py
```

Then open in your browser:
http://127.0.0.1:5000

Author
Tshepi Angela Mokgohloa
Computer Systems Engineering Student 
