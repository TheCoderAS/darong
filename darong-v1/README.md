# Darong Flight Controller (ESP32)

This project provides a minimal ESP32-based quadcopter controller with a web UI for adjusting throttle and PID gains. It uses an MPU6050 for attitude estimation, drives four ESCs via PWM, and exposes HTTP endpoints for tuning.

## Features
- Complementary-filter attitude estimation using MPU6050 gyro/accelerometer data.
- PID-based roll, pitch, and yaw stabilization with live gain tuning endpoints.
- Simple web dashboard for throttle and attitude commands.
- Safety additions: command timeout disarms, MPU6050 read failures disarm, and watchdog coverage for PID and web tasks.
- MPU6050 calibration offsets are persisted to EEPROM so reboots reuse the last valid calibration.

## Running
1. Configure pins and PID defaults in `config.h` (the board hosts its own access point by default).
2. Flash `darong-v1.ino` to an ESP32 board with an attached MPU6050 and four ESCs.
3. Connect your phone or laptop to the `Darong-AP` Wi-Fi network (password `flysafe123`) and open the printed AP IP address (default `192.168.4.1`) in a browser to use the web UI.

## Safety notes
- Motors arm only when throttle exceeds the configured minimum and recent commands are present.
- If sensor reads fail or commands stop arriving, the controller disarms and holds minimum throttle.
- Watchdog timers reset tasks that overrun configured budgets; avoid long blocking work in callbacks.

## Development
- PID update rate and watchdog budgets are set in `SystemConfig` within `config.h`.
- PID tuning endpoints are served by `setupWebServer()` in `darong-v1.ino`.
- The main control loop lives in `pidControlTask()` in `darong-v1.ino`.
