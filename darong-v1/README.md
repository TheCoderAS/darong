# Darong Flight Controller (ESP32)

This project provides a minimal ESP32-based quadcopter controller with a web UI for adjusting throttle and PID gains. It uses an MPU6050 for attitude estimation, drives four ESCs via PWM, and exposes HTTP endpoints for tuning.

## Features
- Complementary-filter attitude estimation using MPU6050 gyro/accelerometer data.
- PID-based roll, pitch, and yaw stabilization with live gain tuning through a single form submission (yaw corrections to motor outputs are currently paused).
- Simple web dashboard for throttle and attitude commands.
- Safety additions: command timeout disarms, MPU6050 read failures disarm, and watchdog coverage for PID and web tasks.
- MPU6050 calibration offsets and PID gains are persisted to EEPROM so reboots reuse the last valid values.
- Manual calibration routes and UI buttons for MPU6050 and ESC calibration.
- PID and web-server tasks are pinned to separate ESP32 cores for predictable timing.
- The controller always boots as a Wi-Fi hotspot (soft AP) so the dashboard is reachable without existing Wi-Fi.

## Running
1. Configure pins and PID defaults in `config.h` (the board hosts its own access point by default). The UI loads PID gains from the controller; if the `/getPID` endpoint is unreachable it will fall back to zeros.
2. Flash `darong-v1.ino` to an ESP32 board with an attached MPU6050 and four ESCs.
3. Connect your phone or laptop to the `Darong-AP` Wi-Fi network (password `flysafe123`) and open the printed AP IP address (default `192.168.4.1`) in a browser to use the web UI.

### API authentication
Sensitive endpoints use HTTP Basic authentication configured in `AuthConfig` within `config.h`. The controller prompts for credentials when you load the root page (`/`), and the browser reuses the authenticated session for subsequent requests.

Protected endpoints include `/arm`, `/setThrottle`, `/setPID`, `/setTestMode`, `/setTestMotor`, and `/restart`. Requests without valid credentials receive a basic-auth challenge.

### Web UI PID tuning
- The PID card shows Kp/Ki/Kd inputs for roll, pitch, and yaw. Enter the desired values and click **Save PID** to send them in a single request to the controller (`/setPID`). The Save and Reset buttons stay disabled until the initial load finishes. PID values are persisted to EEPROM when saved.
- The controller exposes `/getPID` for the UI to preload the latest in-memory gains; **Reset PID** restores the most recently loaded gains (zeros if the controller could not be reached during load) and posts them back to the controller.
- Use the **Calibrate Sensors** button to trigger `/calibrateMPU` (saves offsets to EEPROM) and **Calibrate ESCs** to trigger `/calibrateESC`. Both operations temporarily pause the PID task for clean calibration and disarm motors before ESC calibration.

## Safety notes
- Motors arm only when throttle exceeds the configured minimum and recent commands are present.
- If sensor reads fail or commands stop arriving, the controller disarms and holds minimum throttle.
- Watchdog timers reset tasks that overrun configured budgets; avoid long blocking work in callbacks.

## Development
- PID update rate and watchdog budgets are set in `SystemConfig` within `config.h`.
- PID tuning endpoints are served by `setupWebServer()` in `darong-v1.ino`.
- The main control loop lives in `pidControlTask()` in `darong-v1.ino`.
