//removed delay code 


const int motorPin = 9;         // Motor control pin
const int statusLedPin = 13;    // Motor status LED pin
const int potPin = A0;          // Potentiometer input pin
const int thresholdPin = A1;    // Threshold input pin
const int moisturePin = A2;     // Capacitive soil moisture sensor pin

int motorSpeed = 0;             // Motor speed variable
int moistureValue = 0;          // Moisture value variable
int thresholdValue = 0;         // Threshold value variable
bool motorRunning = false;      // Flag to indicate if motor is running

unsigned long lastMoistureReadTime = 0;
const long moistureReadInterval = 30000;  // 30 seconds

unsigned long lastSerialUpdateTime = 0;
const long serialUpdateInterval = 300;    // 300 milliseconds

void setup() {
  pinMode(motorPin, OUTPUT);
  pinMode(statusLedPin, OUTPUT);
  digitalWrite(motorPin, LOW);        // Turn off motor
  digitalWrite(statusLedPin, LOW);    // Turn off status LED
  Serial.begin(9600);                 // Initialize serial communication
}

void loop() {
  readPotentiometer();    // Read potentiometer value
  readMoistureSensor();   // Read moisture sensor value
  readThresholdValue();   // Read threshold value
  setMotorSpeed();        // Set motor speed using PWM
  checkMoisture();        // Check moisture level and trigger motor
  updateSerialCommunication(); // Update serial communication
}

void readPotentiometer() {
  int potValue = analogRead(potPin);
  motorSpeed = map(potValue, 0, 1023, 0, 255);
}

void readMoistureSensor() {
  if (millis() - lastMoistureReadTime >= moistureReadInterval) {
    moistureValue = analogRead(moisturePin);
    lastMoistureReadTime = millis();
  }
}

void readThresholdValue() {
  thresholdValue = analogRead(thresholdPin);
}

void setMotorSpeed() {
  if (motorRunning) {
    analogWrite(motorPin, motorSpeed);
  } else {
    analogWrite(motorPin, 0);  // Turn off PWM controller
  }
}

void checkMoisture() {
  int moisturePercentage = map(moistureValue, 20, 850, 100, 0);
  int thresholdPercentage = map(thresholdValue, 0, 1023, 0, 100);

  if (!motorRunning && moisturePercentage < thresholdPercentage) {
    startMotor();
  } else if (motorRunning && moisturePercentage >= 85) {
    stopMotor();
  }
}

void startMotor() {
  digitalWrite(motorPin, HIGH);
  digitalWrite(statusLedPin, HIGH);
  motorRunning = true;
}

void stopMotor() {
  digitalWrite(motorPin, LOW);
  digitalWrite(statusLedPin, LOW);
  motorRunning = false;
}

void updateSerialCommunication() {
  if (millis() - lastSerialUpdateTime >= serialUpdateInterval) {
    Serial.print("Motor Speed: ");
    Serial.print(map(motorSpeed, 0, 255, 0, 100));
    Serial.print("% - Moisture: ");
    Serial.print(map(moistureValue, 0, 1023, 100, 30));
    Serial.println("%");
    lastSerialUpdateTime = millis();
  }
}
