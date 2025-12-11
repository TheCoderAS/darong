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
extern const char* ssid;
extern const char* password;
extern WebServer server;
extern int throttle;
extern bool mpuInitialized;

#endif 