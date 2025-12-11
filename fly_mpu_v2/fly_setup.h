#ifndef FLY_SETUP_H
#define FLY_SETUP_H

#include <ESP32Servo.h>
#include <WiFi.h>
#include <WebServer.h>
#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include "root_ui.h"

// Function declarations
void setupESCs();
void setupAndConnectWifi();
void configureControllerRoutes();
void initializeMPU6050();
void handleRoot();
void handleSet();
void handleArm();
void handleStatus();
void handleCalibrateMpu();
void handleCalibrateEsc();
void markCommandReceived();

// External variables that need to be declared
extern Servo escFL;
extern Servo escFR;
extern Servo escBL;
extern Servo escBR;
extern const int escFLPin;
extern const int escFRPin;
extern const int escBLPin;
extern const int escBRPin;
extern Adafruit_MPU6050 mpu;
extern const char* AP_SSID;
extern const char* AP_PASSWORD;
extern WebServer server;
extern int throttle;
extern bool armed;
extern bool mpuInitialized;
extern unsigned long lastCommandMillis;
extern bool watchdogTriggered;
extern bool calibrationInProgress;

#endif 