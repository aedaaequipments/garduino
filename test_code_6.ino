#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <Preferences.h>
#include <WiFi.h>

const int motorPin = 26;
const int valvePin = 22;
const int moisturePin = 32;

const char *ssid = "Vishnu";
const char *password = "12345678";

AsyncWebServer server(80);
const int pwmChannel = 0;
const int freq = 5000;
const int resolution = 8;

Preferences preferences;

enum Mode {
 MODE_NONE,
 MODE_GARDENING_AUTO,
 MODE_TIMELY_WATER,
 MODE_HYDROPONIC
};

Mode currentMode = MODE_NONE;

const char *htmlPage = R"(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>Farm Assistant Control Panel</title>
<link href="https://fonts.googleapis.com/css2?family=Roboto:wght@400;700&display=swap" rel="stylesheet">
<style>
body {
 font-family: 'Roboto', sans-serif;
 background-color: #f4f4f4;
 color: #333;
}
.container {
 padding: 20px;
 background-color: #fff;
 box-shadow: 0 0 10px rgba(0, 0, 0, 0.1);
 border-radius: 5px;
 max-width: 600px;
 margin: 40px auto;
 text-align: center;
}
.logo {
 width: 100px;
 height: auto;
 margin-bottom: 20px;
}
h1 {
 color: #4CAF50;
}
.mode {
 display: none;
 margin-top: 20px;
}
.active {
 display: block;
}
#currentMode {
 font-weight: bold;
 color: #4CAF50;
 margin-bottom: 20px;
}
button {
 background-color: #4CAF50;
 color: white;
 padding: 10px 20px;
 border: none;
 border-radius: 5px;
 cursor: pointer;
 transition: background-color 0.3s;
}
button:hover {
 background-color: #367c39;
}
input[type=range] {
 width: 100%;
}
.value-display {
 display: inline-block;
 width: 50px;
 text-align: left;
}
</style>
</head>
<body>
<div class="container">
 <img src="path_to_your_logo.png" alt="Farm Assistant Logo" class="logo">
 <h1>Farm Assistant Control Panel</h1>
 <p>Current Mode: <span id="currentMode">None</span></p>
 <form action="/setMode" method="post">
 <select name="mode" id="mode">
    <option value="MODE_NONE">None</option>
    <option value="MODE_GARDENING_AUTO">Gardening Auto Mode</option>
    <option value="MODE_TIMELY_WATER">Timely Water Mode</option>
    <option value="MODE_HYDROPONIC">Hydroponic Mode</option>
 </select>
 <button type="submit">Set Mode</button>
 </form>
</div>
<script>
function switchMode(mode) {
 document.querySelectorAll('.mode').forEach(function(div) {
 div.style.display = 'none';
 });
 document.getElementById(mode).style.display = 'block';
 currentMode = mode;
 let modeText = mode.replace(/([A-Z])/g, ' $1').trim();
 document.getElementById('currentMode').textContent = modeText;
}
function activateMode(mode) {
 alert('Activating ' + mode);
}
</script>
</body>
</html>
)";

void setup() {
 Serial.begin(115200);
 pinMode(motorPin, OUTPUT);
 pinMode(valvePin, OUTPUT);
 ledcSetup(pwmChannel, freq, resolution);
 ledcAttachPin(motorPin, pwmChannel);
 digitalWrite(valvePin, LOW);

 preferences.begin("gardeningSettings", false);
 currentMode = static_cast<Mode>(preferences.getUInt("currentMode", MODE_NONE));
 preferences.end();

 WiFi.mode(WIFI_AP);
 WiFi.softAP(ssid, password);
 Serial.println(WiFi.softAPIP());

 server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/html", htmlPage);
 });

server.on("/setMode", HTTP_POST, [](AsyncWebServerRequest *request) {
    if (request->hasParam("mode", true)) {
      String mode = request->getParam("mode", true)->value();
      Serial.print("Received mode: ");
      Serial.println(mode); // Debugging line
      if (mode == "MODE_NONE") {
        currentMode = MODE_NONE;
      } else if (mode == "MODE_GARDENING_AUTO") {
        currentMode = MODE_GARDENING_AUTO;
      } else if (mode == "MODE_TIMELY_WATER") {
        currentMode = MODE_TIMELY_WATER;
      } else if (mode == "MODE_HYDROPONIC") {
        currentMode = MODE_HYDROPONIC;
      }
      preferences.begin("gardeningSettings", false);
      preferences.putUInt("currentMode", static_cast<unsigned int>(currentMode));
      preferences.end();
      Serial.print("Current mode set to: ");
      Serial.println(currentMode); // Debugging line
    }
    request->send(200, "text/plain", "Mode set");
});


 server.begin();
 preferences.begin("gardeningSettings", false);
currentMode = static_cast<Mode>(preferences.getUInt("currentMode", MODE_NONE));
preferences.end();

}
void loop() {
 // Placeholder for mode-specific logic
 switch (currentMode) {
    case MODE_NONE:
      Serial.println("Mode: None");
      break;
    case MODE_GARDENING_AUTO:
      Serial.println("Mode: Gardening Auto");
      // Implement logic for Gardening Auto Mode
      break;
    case MODE_TIMELY_WATER:
      Serial.println("Mode: Timely Water");
      // Implement logic for Timely Water Mode
      break;
    case MODE_HYDROPONIC:
      Serial.println("Mode: Hydroponic");
      // Implement logic for Hydroponic Mode
      break;
    default:
      Serial.println("Mode: Default");
      break;
 }
}
