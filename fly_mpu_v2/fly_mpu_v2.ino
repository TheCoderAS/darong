#include <ESP32Servo.h>
#include <WiFi.h>
#include <WebServer.h>
#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include "root_ui.h"
#include "fly_setup.h"
#include "fly_calibration.h"

// Create servo objects for each ESC
Servo escFL;  // Motor FL
Servo escFR;  // Motor FR
Servo escBL;  // Motor BL
Servo escBR;  // Motor BR

// ESC pins
const int escFLPin = 13;  // Motor FL -> purple
const int escFRPin = 14;  // Motor FR -> blue
const int escBLPin = 15;  // Motor BL -> gray
const int escBRPin = 16;  // Motor BR -> green

bool ESC_CALIBRATION = false;
bool MPU6050_CALIBRATION = false;
bool mpuInitialized = false;  // Add flag to track MPU initialization
bool calibrationInProgress = false;
bool armed = false;
bool watchdogTriggered = false;
unsigned long lastCommandMillis = 0;
unsigned long lastLoopMicros = 0;
bool attitudeInitialized = false;

const unsigned long COMMAND_TIMEOUT_MS = 1500;
const float COMPLEMENTARY_ALPHA = 0.98;
const float RAD_TO_DEG = 180.0 / PI;

// MPU6050 object
Adafruit_MPU6050 mpu;

// PID variables
float Kp = 2.0;  // Proportional gain
float Ki = 0.1;  // Integral gain
float Kd = 1.0;  // Derivative gain

// PID limits
const float MAX_INTEGRAL = 100.0;  // Maximum integral value to prevent windup
const float MAX_OUTPUT = 50.0;     // Maximum PID output in microseconds

// PID variables for roll and pitch
float rollError = 0, pitchError = 0;
float rollPrevError = 0, pitchPrevError = 0;
float rollIntegral = 0, pitchIntegral = 0;
float rollDerivative = 0, pitchDerivative = 0;

// Target angles (in degrees)
float targetRoll = 0;
float targetPitch = 0;

// Current angles
float currentRoll = 0;
float currentPitch = 0;

// Motor adjustments
int motorFLAdjust = 0;
int motorFRAdjust = 0;
int motorBLAdjust = 0;
int motorBRAdjust = 0;

// Calibration variables
float accelX_offset = 0;
float accelY_offset = 0;
float accelZ_offset = 0;
float gyroX_offset = 0;
float gyroY_offset = 0;
float gyroZ_offset = 0;

// WiFi access point credentials
const char* AP_SSID = "ESP32-Quad-CTRL";
const char* AP_PASSWORD = "flysafe123";
WebServer server(80);

int throttle = 0;

void setup() {
  Serial.begin(115200);  // Initialize serial communication
  Serial.println("Quad Motor Control Program");

  // Reset watchdog
  lastCommandMillis = millis();

  setupAndConnectWifi();
  configureControllerRoutes();

  // Always initialize MPU6050
  initializeMPU6050();

  // Calibrate MPU6050 if requested
  if (MPU6050_CALIBRATION && mpuInitialized) {
    calibrationInProgress = true;
    calibrateMPU6050();
    calibrationInProgress = false;
  }

  // Setup ESCs
  setupESCs();

  // ESC calibration
  if (ESC_CALIBRATION) {
    calibrationInProgress = true;
    caliberateESCs();
    calibrationInProgress = false;
  }
  Serial.println("All ready. \nEntering flight mode in 3 seconds.");
  delay(1000);
  Serial.println("Entering flight mode in 2 seconds.");
  delay(1000);
  Serial.println("Entering flight mode in 1 seconds.");
  delay(1000);
  Serial.println("============================== FLIGHT READY ===================================");
}

void loop() {
  // Handle web server requests
  server.handleClient();

  //handleFlight
  handleFlight();

  delay(10);  // Small delay to prevent overwhelming the system
}

// Converts throttle (0-100%) to microseconds
int throttleToMicroseconds(int throttlePercent) {
  return map(throttlePercent, 0, 100, 1000, 2000);
}

// Calculate PID adjustments
void calculatePID() {
  // Calculate errors
  rollError = targetRoll - currentRoll;
  pitchError = targetPitch - currentPitch;

  // Calculate integral with anti-windup
  rollIntegral = constrain(rollIntegral + rollError, -MAX_INTEGRAL, MAX_INTEGRAL);
  pitchIntegral = constrain(pitchIntegral + pitchError, -MAX_INTEGRAL, MAX_INTEGRAL);

  // Calculate derivative with noise reduction
  rollDerivative = (rollError - rollPrevError) * 0.8;  // Apply smoothing factor
  pitchDerivative = (pitchError - pitchPrevError) * 0.8;

  // Calculate PID outputs with proper scaling
  float rollOutput = Kp * rollError + Ki * rollIntegral + Kd * rollDerivative;
  float pitchOutput = Kp * pitchError + Ki * pitchIntegral + Kd * pitchDerivative;

  // Constrain outputs to prevent extreme adjustments
  rollOutput = constrain(rollOutput, -MAX_OUTPUT, MAX_OUTPUT);
  pitchOutput = constrain(pitchOutput, -MAX_OUTPUT, MAX_OUTPUT);

  // Update previous errors
  rollPrevError = rollError;
  pitchPrevError = pitchError;

  // Apply adjustments to motors with proper scaling
  // Front Left (FL) - escFL
  motorFLAdjust = -rollOutput + pitchOutput;
  // Front Right (FR) - escFR
  motorFRAdjust = rollOutput + pitchOutput;
  // Back Left (BL) - escBL
  motorBLAdjust = -rollOutput - pitchOutput;
  // Back Right (BR) - escBR
  motorBRAdjust = rollOutput - pitchOutput;
}

void handleFlight() {
  int commandThrottle = (armed && !watchdogTriggered && mpuInitialized) ? throttle : 0;

  // Only read MPU6050 if it's initialized
  if (mpuInitialized) {
    // Read MPU6050 data
    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);
    // Apply calibration offsets
    float accelX = a.acceleration.x - accelX_offset;
    float accelY = a.acceleration.y - accelY_offset;
    float accelZ = a.acceleration.z - accelZ_offset;

    float accelRoll = atan2(accelY, accelZ) * RAD_TO_DEG;
    float accelPitch = atan2(-accelX, sqrt(accelY * accelY + accelZ * accelZ)) * RAD_TO_DEG;

    // Complementary filter blending gyro integration with accelerometer stability
    unsigned long nowMicros = micros();
    float dt = (lastLoopMicros == 0) ? 0.0 : (nowMicros - lastLoopMicros) / 1e6;
    lastLoopMicros = nowMicros;

    float gyroRollRate = (g.gyro.x - gyroX_offset) * RAD_TO_DEG;
    float gyroPitchRate = (g.gyro.y - gyroY_offset) * RAD_TO_DEG;

    if (!attitudeInitialized) {
      currentRoll = accelRoll;
      currentPitch = accelPitch;
      attitudeInitialized = true;
    } else if (dt > 0) {
      float rollPrediction = currentRoll + gyroRollRate * dt;
      float pitchPrediction = currentPitch + gyroPitchRate * dt;

      currentRoll = COMPLEMENTARY_ALPHA * rollPrediction + (1.0 - COMPLEMENTARY_ALPHA) * accelRoll;
      currentPitch = COMPLEMENTARY_ALPHA * pitchPrediction + (1.0 - COMPLEMENTARY_ALPHA) * accelPitch;
    }
  } else {
    // If MPU is not initialized, keep angles at 0 and disarm
    currentRoll = 0;
    currentPitch = 0;
    armed = false;
    throttle = 0;
    commandThrottle = 0;
  }

  // Watchdog: drop throttle if commands stop coming in
  if (millis() - lastCommandMillis > COMMAND_TIMEOUT_MS) {
    watchdogTriggered = true;
    commandThrottle = 0;
    throttle = 0;
  }

  int basePulse = throttleToMicroseconds(commandThrottle);

  // Reset PID variables if throttle changes from 0
  static int prevThrottle = 0;
  if (prevThrottle == 0 && commandThrottle > 0) {
    rollError = 0;
    pitchError = 0;
    rollPrevError = 0;
    pitchPrevError = 0;
    rollIntegral = 0;
    pitchIntegral = 0;
    rollDerivative = 0;
    pitchDerivative = 0;
  }
  prevThrottle = commandThrottle;

  // Only calculate and apply PID adjustments if throttle is non-zero
  if (commandThrottle > 0) {
    calculatePID();

    // Apply PID adjustments to each motor
    escFL.writeMicroseconds(constrain(basePulse - motorFLAdjust, 1000, 2000));
    escFR.writeMicroseconds(constrain(basePulse - motorFRAdjust, 1000, 2000));
    escBL.writeMicroseconds(constrain(basePulse - motorBLAdjust, 1000, 2000));
    escBR.writeMicroseconds(constrain(basePulse - motorBRAdjust, 1000, 2000));
  } else {
    // At zero throttle, just send minimum pulse width
    escFL.writeMicroseconds(1000);
    escFR.writeMicroseconds(1000);
    escBL.writeMicroseconds(1000);
    escBR.writeMicroseconds(1000);
  }

  // Print debug information
  Serial.print("Roll: ");
  Serial.print(currentRoll);
  Serial.print(" Pitch: ");
  Serial.print(currentPitch);
  Serial.print(" Throttle: ");
  Serial.print(commandThrottle);
  Serial.print("% | Adjustments: ");
  Serial.print(motorFLAdjust);
  Serial.print(" ");
  Serial.print(motorFRAdjust);
  Serial.print(" ");
  Serial.print(motorBLAdjust);
  Serial.print(" ");
  Serial.print(motorBRAdjust);
  Serial.print("% | Pulse us: ");
  Serial.print(basePulse - motorFLAdjust);
  Serial.print(" ");
  Serial.print(basePulse - motorFRAdjust);
  Serial.print(" ");
  Serial.print(basePulse - motorBLAdjust);
  Serial.print(" ");
  Serial.print(basePulse - motorBRAdjust);
  Serial.print(" | WD: ");
  Serial.print(watchdogTriggered ? "1" : "0");
  Serial.print(" | Armed: ");
  Serial.println(armed ? "1" : "0");
}