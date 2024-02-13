#include <WiFi.h>
#include <WebServer.h>
#include <Preferences.h>

const char* ssid = "Garduino";
const char* password = "123456789"; // Secure password

const int motorPin = 26; // Assuming this pin is connected to the motor driver's PWM input
const int motorChannel = 0;
const int freq = 5000; // Frequency for PWM signal
const int resolution = 8; // Resolution for PWM signal

WebServer server(80);
Preferences preferences;

// Global variable to hold motor speed percentage
int speedPercentage;

void setup() {
  Serial.begin(115200);
  
  // Initialize NVS
  preferences.begin("motor", false);
  
  // Retrieve the saved motor speed level, default to 50% if not previously saved
  speedPercentage = preferences.getInt("speed", 50);
  
  // Setup PWM for motor control
  ledcSetup(motorChannel, freq, resolution);
  ledcAttachPin(motorPin, motorChannel);
  ledcWrite(motorChannel, map(speedPercentage, 0, 100, 0, 255));

  // Setup WiFi and server
  WiFi.softAP(ssid, password);
  Serial.print("Access Point started, IP address: ");
  Serial.println(WiFi.softAPIP());

  // Handle root URL by sending HTML response
  server.on("/", HTTP_GET, []() {
    if (server.hasArg("speed")) {
      speedPercentage = server.arg("speed").toInt();
      preferences.putInt("speed", speedPercentage);
      ledcWrite(motorChannel, map(speedPercentage, 0, 100, 0, 255));
    }

    String html = "<!DOCTYPE html><html>"
                  "<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\"></head>"
                  "<body><h1>Motor Speed Control</h1>"
                  "<form action='/' method='GET'>"
                  "Speed (%): <input type='range' name='speed' min='0' max='100' value='" + String(speedPercentage) + "' onchange='this.form.submit()' />"
                  "</form></body></html>";

    server.send(200, "text/html", html);
  });

  server.begin();
}

void loop() {
  server.handleClient();
}
