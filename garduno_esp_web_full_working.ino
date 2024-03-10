#include <Arduino.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <Preferences.h> // Include Preferences library for storing data
#include <WiFi.h>
// Pin Definitions
const int motorPin = 26; // PWM capable pin
const int valvePin = 22; // Valve control pin
// Gardening Mode Variables
// Additional Pin Definitions
const int moistureSensorPin = 32; // Example pin for moisture sensor
const int highMoistureThreshold =
    95; // When the moisture level is at or above this value, stop watering
int gardenMotorSpeed = 0;
int motorTriggerThreshold = 0; // Not used directly in simplified example
// Timely Water Mode Variables
int timelyMotorSpeed = 0;
unsigned long waterCycleTime = 0; // In milliseconds
unsigned long waterDuration = 0;  // In milliseconds
// Timing Control Variables
unsigned long previousMillis = 0;
bool motorState = false;
// Mode Settings
bool gardeningModeEnabled = false; // CHANGED: Was previously 'true', now it is set based on HTML page
const char *ssid = "Gardeingauto";
const char *password = "12345678";

AsyncWebServer server(80);
Preferences preferences; // Declare Preferences object for data storage

// HTML content directly within the code (for simplicity)
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>AEDAA EQUIPMENTS GARDENING KIT</title>
<link href="https://fonts.googleapis.com/css2?family=Roboto:wght@300;400&display=swap" rel="stylesheet">
<style>
  body {
    font-family: 'Roboto', sans-serif;
    background-color: #f8f9fa;
    color: #333;
    margin: 0;
    padding: 0;
    box-sizing: border-box;
  }
  .container {
    max-width: 600px;
    margin: 40px auto;
    background-color: #fff;
    padding: 20px;
    border-radius: 8px;
    box-shadow: 0 4px 6px rgba(0,0,0,0.1);
  }
  .logo {
    display: block;
    width: 100px;
    height: auto;
    margin: 0 auto 20px auto;
  }
  h1 {
    color: #5c946e;
    text-align: center;
  }
  .section {
    margin-bottom: 30px;
    background-color: #edf7f3;
    padding: 15px;
    border-radius: 8px;
  }
  .section h3 {
    color: #333;
    margin-top: 0;
  }
  input[type=checkbox] {
    margin-right: 5px;
  }
  input[type=range], input[type=number] {
    width: 100%;
    margin: 10px 0;
    padding: 6px;
    border-radius: 5px;
    border: 1px solid #ced4da;
  }
  label {
    font-weight: 400;
    margin-top: 10px;
    display: inline-block;
  }
  .value-display {
    margin-left: 6px;
    font-weight: 700;
  }
</style>
</head>
<body>

<div class="container">
  <img src="path_to_your_logo.png" alt="Logo" class="logo">
  <h1>AEDAA EQUIPMENTS GARDENING KIT</h1>

  <div class="section" id="gardeningMode">
    <h3>Gardening Mode</h3>
    <label for="gardeningEnable"><input type="checkbox" id="gardeningEnable"> Enable Gardening Mode</label>

    <div>
      Motor Speed (%): <span id="gardenMotorSpeedDisplay" class="value-display">50</span>%
      <input type="range" id="gardenMotorSpeed" name="gardenMotorSpeed" min="0" max="100" value="50" oninput="updateSettings()">
    </div>

    <div>
      Motor Triggering Threshold (%): <span id="triggerThresholdDisplay" class="value-display">30</span>%
      <input type="range" id="motorTriggerThreshold" name="motorTriggerThreshold" min="0" max="100" value="30" oninput="updateSettings()">
    </div>
  </div>

  <div class="section" id="timelyWaterMode">
    <h3>Timely Water Mode</h3>
    <label for="timelyWaterEnable"><input type="checkbox" id="timelyWaterEnable"> Enable Timely Water Mode</label>

    <div>
      Motor Speed2 (%): <span id="timelyMotorSpeedDisplay" class="value-display">50</span>%
      <input type="range" id="timelyMotorSpeed" name="timelyMotorSpeed" min="0" max="100" value="50" oninput="updateSettings()">
    </div>

    <div>
      Water Cycle Time (minutes):
      <input type="number" id="waterCycleTime" name="waterCycleTime" min="1" value="10" oninput="updateSettings()">
    </div>

    <div>
      Water Duration (minutes):
      <input type="number" id="waterDuration" name="waterDuration" min="1" value="5" oninput="updateSettings()">
    </div>
  </div>

</div>

<script>
function updateSettings() {
  let gardeningEnable = document.getElementById('gardeningEnable').checked ? 1 : 0;
  let gardenMotorSpeed = document.getElementById('gardenMotorSpeed').value;
  let motorTriggerThreshold = document.getElementById('motorTriggerThreshold').value;
  let timelyMotorSpeed = document.getElementById('timelyMotorSpeed').value;
  let waterCycleTime = document.getElementById('waterCycleTime').value;
  let waterDuration = document.getElementById('waterDuration').value;

  fetch('/update-settings', {
    method: 'POST',
    headers: {
      'Content-Type': 'application/x-www-form-urlencoded',
    },
    body: `gardeningEnable=${gardeningEnable}&gardenMotorSpeed=${gardenMotorSpeed}&motorTriggerThreshold=${motorTriggerThreshold}&timelyMotorSpeed=${timelyMotorSpeed}&waterCycleTime=${waterCycleTime}&waterDuration=${waterDuration}`
  })
  .then(response => response.text())
  .then(data => {
    console.log('Settings updated');
  })
  .catch((error) => {
    console.error('Error:', error);
  });
}
</script>

</body>
</html>
)rawliteral";
void setup() {
  Serial.begin(115200);
  pinMode(motorPin, OUTPUT);
  pinMode(valvePin, OUTPUT);
  ledcSetup(0, 5000, 8); // Channel 0, 5KHz, 8-bit resolution
  ledcAttachPin(motorPin, 0);
  pinMode(moistureSensorPin, INPUT); // Set moisture sensor pin as input

  preferences.begin("gardening", false);
//  setupPWM();
  // Gardening Mode Enabled is now set based on the HTML page value
  gardeningModeEnabled = preferences.getBool("gardeningModeEnabled", false); //Default to false if not set

  gardenMotorSpeed = 50; // Example values, replace with actual logic
  // Load timely water mode example values if gardeningModeEnabled is false
  preferences.begin("gardeningKit",
                    false); // Open Preferences with my-app namespace. RW-mode
                            // (second parameter) is false

  // Connect to Wi-Fi
  WiFi.softAP(ssid, password); // Change the mode to act as an Access Point
  Serial.println("Access Point Started");
  Serial.println(WiFi.softAPIP()); // Print the IP address

  // Route for serving the HTML page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {request->send_P(200, "text/html", index_html);});

  // Handling POST request to update settings
  server.on("/update-settings", HTTP_POST, [](AsyncWebServerRequest *request) {
    if (request->hasParam("gardeningEnable", true) &&
        request->hasParam("gardenMotorSpeed", true) &&
        request->hasParam("motorTriggerThreshold", true) &&
        request->hasParam("timelyMotorSpeed", true) &&
        request->hasParam("waterCycleTime", true) &&
        request->hasParam("waterDuration", true)) {
      // Obtain the values from the request
      gardeningModeEnabled = request->getParam("gardeningEnable", true)->value().toInt();
      preferences.putBool("gardeningModeEnabled", gardeningModeEnabled); // Save the mode state
      gardenMotorSpeed = request->getParam("gardenMotorSpeed", true)->value().toInt();
      motorTriggerThreshold = request->getParam("motorTriggerThreshold", true)->value().toInt();
      timelyMotorSpeed = request->getParam("timelyMotorSpeed", true)->value().toInt();
      waterCycleTime = request->getParam("waterCycleTime", true)->value().toInt() * 60000; // Convert minutes to milliseconds
      waterDuration = request->getParam("waterDuration", true)->value().toInt() * 60000; // Convert minutes to milliseconds

      Serial.println("Settings updated & stored in Preferences");
      request->send(200, "text/html", "<p>Settings updated</p>");
    } else {
      request->send(400, "text/html", "<p>Missing parameters</p>");
    }
  });

  // Start server
  server.begin();
}
void operateGardeningMode() {
  Serial.print(" loop1 trig :");
  Serial.println("3");
  int moisturePercentage = analogRead(moistureSensorPin); // Read moisture value
  moisturePercentage = map(moisturePercentage, 100, 3500, 100,0); // Assuming a 12-bit ADC, adjust if different
  Serial.print(" moisture persentage :");
  Serial.println(moisturePercentage);
  // If soil is drier than the threshold and motor is not already running, start
  // the motor
  if (moisturePercentage < motorTriggerThreshold && motorState == false) {
    Serial.print("Gardening Mode: Low moisture detected (");
    Serial.print(moisturePercentage);
    Serial.println("%), activating motor and valve.");
    ledcWrite(0, map(gardenMotorSpeed, 0, 100, 0, 255)); // Start motor
    Serial.print("Motor Speed set to ");
    Serial.print(gardenMotorSpeed);
    Serial.println("%");
    digitalWrite(valvePin, HIGH);                        // Open valve
    motorState = true;
  }

  // If moisture reaches highMoistureThreshold, stop the motor
  if (moisturePercentage >= highMoistureThreshold && motorState == true) {
    Serial.println("Gardening Mode: High moisture level reached, stopping "
                   "motor and valve.");


    ledcWrite(0, 0);             // Stop motor
    digitalWrite(valvePin, LOW); // Close valve
    motorState = false;
  }
}
void operateTimelyWaterMode() {
   Serial.print(" loop1 trig2 :");
  Serial.println("4");
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >=
      (motorState ? waterDuration : waterCycleTime)) {
    previousMillis = currentMillis;
    if (motorState) {
      Serial.println("Timely Water Mode: Stopping Motor and Closing Valve");
      ledcWrite(0, 0);             // Stop motor
      digitalWrite(valvePin, LOW); // Close valve
      Serial.print("Time left for next cycle: ");
      Serial.print(waterCycleTime - (currentMillis - previousMillis));
       Serial.print("water set time: ");
      Serial.println(waterCycleTime);
    } else {
      Serial.println("Timely Water Mode: Starting Motor and Opening Valve");
      ledcWrite(0, map(timelyMotorSpeed, 0, 100, 0, 255)); // Start motor

      Serial.print("Timely Water Mode: Motor Speed set to ");
      Serial.print(timelyMotorSpeed);
      Serial.println("%");
      digitalWrite(valvePin, HIGH);                        // Open valve
      Serial.print("Time left for duration: ");
      Serial.println(waterDuration - (currentMillis - previousMillis));
      Serial.print("water set time waterDuration: ");
      Serial.println(waterDuration);
    }
    motorState = !motorState;
  }
}
void loop() {
  // Check the operation mode and act accordingly
  Serial.print("gardeningModeEnabled:");

  Serial.println(gardeningModeEnabled);
  if (gardeningModeEnabled) {
    operateGardeningMode();
    Serial.print(" first mode :");
    Serial.println("1");
  } else {
    operateTimelyWaterMode();
    Serial.print("  :");
    Serial.println("2");
  }

  vTaskDelay(pdMS_TO_TICKS(1000)); // Alternative to delay for demonstration, adjust as needed
}
