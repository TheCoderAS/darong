#ifndef FLY_CALIBRATION_H
#define FLY_CALIBRATION_H

#include <ESP32Servo.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>

// Function declarations
void calibrateMPU6050();
void caliberateESCs();

// External variables that need to be declared
extern Servo escFL;
extern Servo escFR;
extern Servo escBL;
extern Servo escBR;
extern Adafruit_MPU6050 mpu;
extern float accelX_offset;
extern float accelY_offset;
extern float accelZ_offset;
extern float gyroX_offset;
extern float gyroY_offset;
extern float gyroZ_offset;

#endif 