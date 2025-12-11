#include "fly_setup.h"

// Serve throttle control page
void handleRoot() {
  server.send(200, "text/html", ROOT_HTML);
}

// Handle throttle value
void handleSet() {
  if (server.hasArg("val")) {
    int newThrottle = server.arg("val").toInt();
    throttle = constrain(newThrottle, 0, 100);

    server.send(200, "text/plain", "Throttle set");
  } else {
    server.send(400, "text/plain", "Missing val param");
  }
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
  Serial.print("Connecting to WiFi: ");
  Serial.println(ssid);
  // Connect to WiFi network
  WiFi.begin(ssid, password);

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void configureControllerRoutes() {
  // Web routes
  server.on("/", handleRoot);
  server.on("/set", handleSet);
  server.begin();
  Serial.println("Controller setup complete! Open the IP address in your browser.");
}

void initializeMPU6050() {
  // Initialize I2C
  Wire.begin();

  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
    Serial.println("Please check your connections:");
    Serial.println("1. VCC -> 3.3V");
    Serial.println("2. GND -> GND");
    Serial.println("3. SCL -> GPIO 9");
    Serial.println("4. SDA -> GPIO 8");
    mpuInitialized = false;
    return;
  }

  // Configure MPU6050
  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
  mpuInitialized = true;
  Serial.println("MPU6050 initialized successfully!");
} 