#include "fly_calibration.h"

void calibrateMPU6050() {
  Serial.println("----------------------------------------------------");
  Serial.println("IMPORTANT: Make sure your MPU 6050 is powered on now!");

  Serial.println("Calibrating MPU6050...");
  Serial.println("Please keep the drone still and level!");
  delay(2000);

  float accelX_sum = 0;
  float accelY_sum = 0;
  float accelZ_sum = 0;
  float gyroX_sum = 0;
  float gyroY_sum = 0;
  float gyroZ_sum = 0;

  // Take 1000 readings for calibration
  for (int i = 0; i < 1000; i++) {
    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);

    accelX_sum += a.acceleration.x;
    accelY_sum += a.acceleration.y;
    accelZ_sum += a.acceleration.z;
    gyroX_sum += g.gyro.x;
    gyroY_sum += g.gyro.y;
    gyroZ_sum += g.gyro.z;

    delay(2);
  }

  // Calculate offsets
  accelX_offset = accelX_sum / 1000;
  accelY_offset = accelY_sum / 1000;
  accelZ_offset = accelZ_sum / 1000 - 9.80665;  // includes gravity component of g value ~= 9.8
  gyroX_offset = gyroX_sum / 1000;
  gyroY_offset = gyroY_sum / 1000;
  gyroZ_offset = gyroZ_sum / 1000;

  Serial.println("MPU6050 Calibration Complete!");
  Serial.println("Offsets:");
  Serial.print("Accel X: ");
  Serial.println(accelX_offset);
  Serial.print("Accel Y: ");
  Serial.println(accelY_offset);
  Serial.print("Accel Z: ");
  Serial.println(accelZ_offset);
  Serial.print("Gyro X: ");
  Serial.println(gyroX_offset);
  Serial.print("Gyro Y: ");
  Serial.println(gyroY_offset);
  Serial.print("Gyro Z: ");
  Serial.println(gyroZ_offset);
  // while (!Serial.available() || Serial.read() != 'c') {
  //   delay(100);
  // }
}

void caliberateESCs() {
  Serial.println("--------------------------------------------------");
  Serial.println("IMPORTANT: Make sure your ESCs are powered on now!");

  delay(2000);
  Serial.println("\nStep 1: Sending maximum signal (2000) to all motors");
  escFL.writeMicroseconds(2000);
  escFR.writeMicroseconds(2000);
  escBL.writeMicroseconds(2000);
  escBR.writeMicroseconds(2000);
  delay(2000);

  Serial.println("\nStep 2: Sending minimum signal (1000) to all motors");
  escFL.writeMicroseconds(1000);
  escFR.writeMicroseconds(1000);
  escBL.writeMicroseconds(1000);
  escBR.writeMicroseconds(1000);
  delay(2000);

  Serial.println("\nESCs Calibration completed!");
}