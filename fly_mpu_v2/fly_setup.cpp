#include "fly_setup.h"

// Serve throttle control page
void handleRoot() {
  server.send(200, "text/html", ROOT_HTML);
}

// Handle throttle value
void handleSet() {
  if (server.hasArg("val")) {
    int newThrottle = server.arg("val").toInt();
    unsigned long now = millis();

    // Rate limiting to avoid overwhelming the control loop
    static unsigned long lastSetHandled = 0;
    const unsigned long MIN_SET_INTERVAL_MS = 50;
    if (now - lastSetHandled < MIN_SET_INTERVAL_MS) {
      server.send(429, "application/json", "{\"status\":\"rate_limited\"}");
      return;
    }
    lastSetHandled = now;

    // Constrain throttle and only allow non-zero throttle when armed
    newThrottle = constrain(newThrottle, 0, 100);
    if (!armed && newThrottle > 0) {
      throttle = 0;
      server.send(403, "application/json", "{\"status\":\"disarmed\"}");
      return;
    }

    throttle = newThrottle;
    markCommandReceived();

    server.send(200, "application/json", "{\"status\":\"ok\",\"throttle\":" + String(throttle) + "}");
  } else {
    server.send(400, "text/plain", "Missing val param");
  }
}

void handleArm() {
  if (!server.hasArg("state")) {
    server.send(400, "application/json", "{\"error\":\"missing_state\"}");
    return;
  }

  String state = server.arg("state");
  if (state != "on" && state != "off") {
    server.send(400, "application/json", "{\"error\":\"invalid_state\"}");
    return;
  }

  if (state == "on") {
    if (!mpuInitialized) {
      server.send(503, "application/json", "{\"error\":\"mpu_unavailable\"}");
      return;
    }
    if (throttle != 0) {
      server.send(409, "application/json", "{\"error\":\"throttle_not_zero\"}");
      return;
    }
    armed = true;
    watchdogTriggered = false;
  } else {
    armed = false;
    throttle = 0;
  }

  markCommandReceived();
  server.send(200, "application/json", "{\"status\":\"ok\",\"armed\":" + String(armed ? "true" : "false") + "}");
}

void handleStatus() {
  String payload = "{";
  payload += "\"armed\":" + String(armed ? "true" : "false") + ",";
  payload += "\"throttle\":" + String(throttle) + ",";
  payload += "\"mpuInitialized\":" + String(mpuInitialized ? "true" : "false") + ",";
  payload += "\"watchdogTriggered\":" + String(watchdogTriggered ? "true" : "false") + ",";
  payload += "\"lastCommandMs\":" + String(millis() - lastCommandMillis) + ",";
  payload += "\"calibrationInProgress\":" + String(calibrationInProgress ? "true" : "false");
  payload += "}";

  server.send(200, "application/json", payload);
}

void handleCalibrateMpu() {
  if (calibrationInProgress) {
    server.send(409, "application/json", "{\"error\":\"calibration_running\"}");
    return;
  }
  if (!mpuInitialized) {
    server.send(503, "application/json", "{\"error\":\"mpu_unavailable\"}");
    return;
  }
  calibrationInProgress = true;
  calibrateMPU6050();
  calibrationInProgress = false;
  server.send(200, "application/json", "{\"status\":\"mpu_calibrated\"}");
}

void handleCalibrateEsc() {
  if (calibrationInProgress) {
    server.send(409, "application/json", "{\"error\":\"calibration_running\"}");
    return;
  }
  if (armed || throttle != 0) {
    server.send(409, "application/json", "{\"error\":\"disarm_first\"}");
    return;
  }
  calibrationInProgress = true;
  caliberateESCs();
  calibrationInProgress = false;
  server.send(200, "application/json", "{\"status\":\"esc_calibrated\"}");
}

void markCommandReceived() {
  lastCommandMillis = millis();
  watchdogTriggered = false;
}

void setupESCs() {
  // Allow allocation of all timers
  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  ESP32PWM::allocateTimer(2);
  ESP32PWM::allocateTimer(3);

  // Attach ESCs to their respective pins
  escFL.attach(escFLPin);
  escFR.attach(escFRPin);
  escBL.attach(escBLPin);
  escBR.attach(escBRPin);
}

void setupAndConnectWifi() {
  Serial.print("Starting access point: ");
  Serial.println(AP_SSID);

  if (!WiFi.softAP(AP_SSID, AP_PASSWORD)) {
    Serial.println("[ERROR] Failed to start hotspot, retrying...");
    delay(1000);
    WiFi.softAP(AP_SSID, AP_PASSWORD);
  }

  IPAddress IP = WiFi.softAPIP();
  Serial.print("Hotspot ready. Connect and open: http://");
  Serial.println(IP);
}

void configureControllerRoutes() {
  // Web routes
  server.on("/", handleRoot);
  server.on("/set", handleSet);
  server.on("/arm", handleArm);
  server.on("/status", handleStatus);
  server.on("/calibrate/mpu", handleCalibrateMpu);
  server.on("/calibrate/esc", handleCalibrateEsc);
  server.begin();
  Serial.println("Controller setup complete! Open the IP address in your browser.");
}

void initializeMPU6050() {
  // Initialize I2C
  Wire.begin();
  bool initialized = false;
  for (int attempt = 0; attempt < 3 && !initialized; attempt++) {
    if (mpu.begin()) {
      initialized = true;
      break;
    }
    Serial.print("Failed to find MPU6050 chip (attempt ");
    Serial.print(attempt + 1);
    Serial.println(")");
    delay(500);
  }

  if (!initialized) {
    Serial.println("MPU6050 unavailable after retries. Disarming for safety.");
    Serial.println("Please check your connections:");
    Serial.println("1. VCC -> 3.3V");
    Serial.println("2. GND -> GND");
    Serial.println("3. SCL -> GPIO 9");
    Serial.println("4. SDA -> GPIO 8");
    mpuInitialized = false;
    armed = false;
    throttle = 0;
    return;
  }

  // Configure MPU6050
  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
  mpuInitialized = true;
  Serial.println("MPU6050 initialized successfully!");
} 