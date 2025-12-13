#include "config.h"
#include "root_ui.h"
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <ESP32Servo.h>
#include <cmath>
#include <EEPROM.h>
#include <WiFi.h>
#include <WebServer.h>
#include <Wire.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_task_wdt.h>

TaskHandle_t webServerTaskHandle_ = NULL;
TaskHandle_t pidTaskHandle_ = NULL;

bool unregisterCurrentTaskFromWatchdog() {
    esp_err_t err = esp_task_wdt_delete(NULL);
    if (err != ESP_OK) {
        Serial.printf("WARN: Failed to unregister current task from watchdog (err=%d).\n", err);
        return false;
    }
    return true;
}

bool pauseWatchdogForCalibration() {
    if (!unregisterCurrentTaskFromWatchdog()) {
        Serial.println("ERROR: Cannot pause watchdog for calibration.");
        return false;
    }

    if (pidTaskHandle_ != NULL) {
        if (!unregisterPIDTaskFromWatchdog()) {
            Serial.println("ERROR: Cannot pause PID task watchdog for calibration.");
            // Attempt to restore web server watchdog before returning
            registerCurrentTaskWithWatchdog();
            return false;
        }
        vTaskSuspend(pidTaskHandle_);
    }

    return true;
}

bool resumeWatchdogAfterCalibration() {
    bool success = true;

    if (pidTaskHandle_ != NULL) {
        vTaskResume(pidTaskHandle_);
        if (!registerPIDTaskWithWatchdog()) {
            Serial.println("ERROR: Failed to re-register PID task with watchdog after calibration.");
            success = false;
        }
    }

    if (!registerCurrentTaskWithWatchdog()) {
        Serial.println("ERROR: Failed to re-register web server task with watchdog after calibration.");
        success = false;
    }

    return success;
}

bool registerCurrentTaskWithWatchdog() {
    esp_err_t err = esp_task_wdt_add(NULL);
    if (err != ESP_OK) {
        Serial.printf("WARN: Failed to re-register current task with watchdog (err=%d).\n", err);
        return false;
    }
    return true;
}

bool unregisterPIDTaskFromWatchdog() {
    if (pidTaskHandle_ == NULL) {
        return true;
    }

    esp_err_t err = esp_task_wdt_delete(pidTaskHandle_);
    if (err != ESP_OK) {
        Serial.printf("WARN: Failed to unregister PID task from watchdog (err=%d).\n", err);
        return false;
    }
    return true;
}

bool registerPIDTaskWithWatchdog() {
    if (pidTaskHandle_ == NULL) {
        return true;
    }

    esp_err_t err = esp_task_wdt_add(pidTaskHandle_);
    if (err != ESP_OK) {
        Serial.printf("WARN: Failed to re-register PID task with watchdog (err=%d).\n", err);
        return false;
    }
    return true;
}

Adafruit_MPU6050 mpu_;

Servo escFL_F_;
Servo escFR_R_;
Servo escBL_L_;
Servo escBR_B_;

int currentPulseFL_ = ESCConfig::MIN_THROTTLE_PULSE;
int currentPulseFR_ = ESCConfig::MIN_THROTTLE_PULSE;
int currentPulseBL_ = ESCConfig::MIN_THROTTLE_PULSE;
int currentPulseBR_ = ESCConfig::MIN_THROTTLE_PULSE;
bool escInitialized_ = false;

WebServer server_(SystemConfig::WEB_SERVER_PORT);

int baseThrottle = 0;
bool eepromReady_ = false;

enum class FlightState {
    INIT,
    CALIBRATING,
    DISARMED,
    ARMED,
    FAILSAFE,
    LANDING
};

const char* flightStateToString(FlightState state);
void setFlightState(FlightState newState, const char* reason);

FlightState flightState_ = FlightState::INIT;

struct CalibrationData {
    uint32_t magic;
    float accelX_offset;
    float accelY_offset;
    float accelZ_offset;
    float gyroX_offset;
    float gyroY_offset;
    float gyroZ_offset;
};

struct PIDStateData {
    uint32_t magic;
    float kpRoll;
    float kiRoll;
    float kdRoll;
    float kpPitch;
    float kiPitch;
    float kdPitch;
    float kpYaw;
    float kiYaw;
    float kdYaw;
};

// Calibration offsets
float accelX_offset_ = 0.0f;
float accelY_offset_ = 0.0f;
float accelZ_offset_ = 0.0f;
float gyroX_offset_ = 0.0f;
float gyroY_offset_ = 0.0f;
float gyroZ_offset_ = 0.0f;

float gyroX_deg_s_ = 0.0f;
float gyroY_deg_s_ = 0.0f;
float gyroZ_deg_s_ = 0.0f;

float dt_ = 0.0f;

float roll_ = 0.0f;
float pitch_ = 0.0f;
float yaw_ = 0.0f;

float targetRoll_ = 0.0;
float targetPitch_ = 0.0;
float targetYaw_ = 0.0;

unsigned long previousMicros_ = micros();
unsigned long lastDebugPrint_ = 0;

float integralRoll_ = 0.0f;
float prevErrorRoll_ = 0.0f;
float integralPitch_ = 0.0f;
float prevErrorPitch_ = 0.0f;
float integralYaw_ = 0.0f;
float prevErrorYaw_ = 0.0f;

float Kp_roll_ = 0.0f;
float Ki_roll_ = 0.0f;
float Kd_roll_ = 0.0f;
float Kp_pitch_ = 0.0f;
float Ki_pitch_ = 0.0f;
float Kd_pitch_ = 0.0f;
float Kp_yaw_ = 0.0f;
float Ki_yaw_ = 0.0f;
float Kd_yaw_ = 0.0f;

bool sensorHealthy_ = true;
unsigned long lastCommandMicros_ = 0;

void markCommandReceived() {
    lastCommandMicros_ = micros();
}

bool hasCommandTimedOut() {
    return (micros() - lastCommandMicros_) > (SystemConfig::COMMAND_TIMEOUT_MS * 1000UL);
}

const char* flightStateToString(FlightState state) {
    switch (state) {
        case FlightState::INIT: return "INIT";
        case FlightState::CALIBRATING: return "CALIBRATING";
        case FlightState::DISARMED: return "DISARMED";
        case FlightState::ARMED: return "ARMED";
        case FlightState::FAILSAFE: return "FAILSAFE";
        case FlightState::LANDING: return "LANDING";
        default: return "UNKNOWN";
    }
}

void resetIntegrators() {
    integralRoll_ = 0.0f;
    prevErrorRoll_ = 0.0f;
    integralPitch_ = 0.0f;
    prevErrorPitch_ = 0.0f;
    integralYaw_ = 0.0f;
    prevErrorYaw_ = 0.0f;
}

void applyMotorOutputsForState() {
    switch (flightState_) {
        case FlightState::INIT:
        case FlightState::CALIBRATING:
        case FlightState::DISARMED:
            baseThrottle = 0;
            if (escInitialized_) {
                writeAllMotors(ESCConfig::MIN_THROTTLE_PULSE);
            }
            break;
        case FlightState::FAILSAFE:
            baseThrottle = 0;
            if (escInitialized_) {
                writeAllMotors(ESCConfig::MIN_THROTTLE_PULSE);
            }
            Serial.println("Failsafe: Motors set to minimum pulse.");
            break;
        case FlightState::LANDING:
            baseThrottle = (int)FlightConfig::MIN_THROTTLE_PERCENT;
            break;
        case FlightState::ARMED:
            break;
    }
}

void setFlightState(FlightState newState, const char* reason) {
    if (flightState_ == newState) {
        return;
    }

    FlightState previousState = flightState_;
    if (previousState == FlightState::ARMED && newState != FlightState::ARMED) {
        resetIntegrators();
    }

    flightState_ = newState;
    Serial.print("Flight state changed: ");
    Serial.print(flightStateToString(previousState));
    Serial.print(" -> ");
    Serial.print(flightStateToString(newState));
    if (reason != nullptr) {
        Serial.print(" | Reason: ");
        Serial.print(reason);
    }
    Serial.println();

    applyMotorOutputsForState();
}

void triggerFailsafe(const char* reason) {
    setFlightState(FlightState::FAILSAFE, reason);
}

bool isSafeToArm() {
    bool neutralAttitude = abs(roll_) <= FlightConfig::ARMING_ATTITUDE_LIMIT_DEG &&
                           abs(pitch_) <= FlightConfig::ARMING_ATTITUDE_LIMIT_DEG;
    bool lowThrottle = baseThrottle <= FlightConfig::ARMING_MAX_THROTTLE_PERCENT;
    return sensorHealthy_ && neutralAttitude && lowThrottle;
}

void requestArm(const char* reason) {
    if (flightState_ != FlightState::DISARMED) {
        Serial.println("Arm request ignored: Flight controller not in DISARMED state.");
        return;
    }

    if (!isSafeToArm()) {
        Serial.println("Arm request denied: Preconditions failed (sensor health, attitude, throttle).");
        return;
    }

    setFlightState(FlightState::ARMED, reason);
}

void requestDisarm(const char* reason) {
    setFlightState(FlightState::DISARMED, reason);
}

void requestLanding(const char* reason) {
    if (flightState_ == FlightState::ARMED) {
        setFlightState(FlightState::LANDING, reason);
    } else if (flightState_ == FlightState::LANDING) {
        Serial.println("Landing already in progress.");
    } else {
        Serial.println("Landing request ignored: Not currently armed.");
    }
}

void disarmMotors(const char* reason) {
    requestDisarm(reason);
}

void setup() {
    doSetup();
}
void loop() {

}


void doSetup(){
    Serial.begin(SystemConfig::SERIAL_BAUD_RATE);
    Serial.println("Initializing Drone...");
    lastCommandMicros_ = micros();

    setFlightState(FlightState::CALIBRATING, "Boot");

    uint32_t watchdogTimeoutMs = max(1000, SystemConfig::WATCHDOG_TIMEOUT_MS);
    esp_task_wdt_config_t watchdogConfig = {
        .timeout_ms = watchdogTimeoutMs,
        .idle_core_mask = (1 << portNUM_PROCESSORS) - 1,
        .trigger_panic = true,
    };

    if (esp_task_wdt_init(&watchdogConfig) != ESP_OK) {
        Serial.println("ERROR: Failed to initialize watchdog timer!");
    }

    // Initialize components
    if (!setupMPU6050()) {
        Serial.println("ERROR: Failed to initialize MPU6050!");
        return;
    }
    
    eepromReady_ = EEPROM.begin(CalibrationConfig::EEPROM_SIZE);
    if (!eepromReady_) {
        Serial.println("ERROR: Failed to initialize EEPROM for calibration storage!");
    }

    bool calibrationLoaded = eepromReady_ && loadCalibrationFromEEPROM();
    if (!calibrationLoaded) {
        calibrateMPU6050();
        saveCalibrationToEEPROM();
    }

    bool pidLoaded = eepromReady_ && loadPIDFromEEPROM();
    if (!pidLoaded) {
        Serial.println("No PID constants found in EEPROM. Using zeros until updated.");
    }

    setupESC();
    if (CalibrationConfig::ESC_CALIBRATION) {
        calibrateESC();
    }
    writeAllMotors(ESCConfig::MIN_THROTTLE_PULSE);

    setFlightState(FlightState::DISARMED, "Calibration complete");

    setupWiFi();
    setupWebServer();
    if(!runWebServer()){
        Serial.println("ERROR: Failed to create web server task!");
        return;
    }

    if(!runPIDTask()){
        Serial.println("ERROR: Failed to create PID task!");
        return;
    }
    Serial.println("Drone initialized successfully");
}
bool setupMPU6050(){

    if (!mpu_.begin()) {
        return false;
    }

    mpu_.setAccelerometerRange(MPU6050_RANGE_8_G);
    mpu_.setGyroRange(MPU6050_RANGE_500_DEG);
    mpu_.setFilterBandwidth(MPU6050_BAND_21_HZ);
    
    Serial.println("MPU6050 initialized successfully");

    return true;
}

bool loadCalibrationFromEEPROM() {
    CalibrationData data;
    EEPROM.get(0, data);

    if (data.magic != CalibrationConfig::EEPROM_MAGIC) {
        Serial.println("No valid calibration data found in EEPROM. Calibration needed.");
        return false;
    }

    accelX_offset_ = data.accelX_offset;
    accelY_offset_ = data.accelY_offset;
    accelZ_offset_ = data.accelZ_offset;
    gyroX_offset_ = data.gyroX_offset;
    gyroY_offset_ = data.gyroY_offset;
    gyroZ_offset_ = data.gyroZ_offset;

    Serial.println("Loaded MPU6050 calibration from EEPROM:");
    printCalibrationData();
    return true;
}

bool loadPIDFromEEPROM() {
    PIDStateData data;
    EEPROM.get(PIDStorageConfig::EEPROM_OFFSET, data);

    if (data.magic != PIDStorageConfig::EEPROM_MAGIC) {
        Serial.println("No valid PID data found in EEPROM. PID update required.");
        return false;
    }

    Kp_roll_ = data.kpRoll;
    Ki_roll_ = data.kiRoll;
    Kd_roll_ = data.kdRoll;
    Kp_pitch_ = data.kpPitch;
    Ki_pitch_ = data.kiPitch;
    Kd_pitch_ = data.kdPitch;
    Kp_yaw_ = data.kpYaw;
    Ki_yaw_ = data.kiYaw;
    Kd_yaw_ = data.kdYaw;

    Serial.println("Loaded PID constants from EEPROM.");
    return true;
}

void saveCalibrationToEEPROM() {
    if (!eepromReady_) {
        Serial.println("EEPROM not initialized; skipping calibration save.");
        return;
    }

    CalibrationData data;
    data.magic = CalibrationConfig::EEPROM_MAGIC;
    data.accelX_offset = accelX_offset_;
    data.accelY_offset = accelY_offset_;
    data.accelZ_offset = accelZ_offset_;
    data.gyroX_offset = gyroX_offset_;
    data.gyroY_offset = gyroY_offset_;
    data.gyroZ_offset = gyroZ_offset_;

    EEPROM.put(0, data);
    EEPROM.commit();
    Serial.println("Saved MPU6050 calibration to EEPROM.");
}

void savePIDToEEPROM() {
    if (!eepromReady_) {
        Serial.println("EEPROM not initialized; skipping PID save.");
        return;
    }

    PIDStateData data;
    data.magic = PIDStorageConfig::EEPROM_MAGIC;
    data.kpRoll = Kp_roll_;
    data.kiRoll = Ki_roll_;
    data.kdRoll = Kd_roll_;
    data.kpPitch = Kp_pitch_;
    data.kiPitch = Ki_pitch_;
    data.kdPitch = Kd_pitch_;
    data.kpYaw = Kp_yaw_;
    data.kiYaw = Ki_yaw_;
    data.kdYaw = Kd_yaw_;

    EEPROM.put(PIDStorageConfig::EEPROM_OFFSET, data);
    EEPROM.commit();
    Serial.println("Saved PID constants to EEPROM.");
}

void calibrateMPU6050(){
    Serial.println("Calibrating MPU6050...");
    Serial.println("Please keep the drone still and level!");
    delay(2000);

    float accelX_sum = 0, accelY_sum = 0, accelZ_sum = 0;
    float gyroX_sum = 0, gyroY_sum = 0, gyroZ_sum = 0;

    for (int i = 0; i < MPUConfig::CALIBRATION_SAMPLES; i++) {
        sensors_event_t a, g, temp;
        mpu_.getEvent(&a, &g, &temp);

        accelX_sum += a.acceleration.x;
        accelY_sum += a.acceleration.y;
        accelZ_sum += a.acceleration.z;
        gyroX_sum += g.gyro.x;
        gyroY_sum += g.gyro.y;
        gyroZ_sum += g.gyro.z;

        delay(2);
    }

    // Calculate offsets
    accelX_offset_ = accelX_sum / MPUConfig::CALIBRATION_SAMPLES;
    accelY_offset_ = accelY_sum / MPUConfig::CALIBRATION_SAMPLES;
    accelZ_offset_ = accelZ_sum / MPUConfig::CALIBRATION_SAMPLES - 9.80665;
    gyroX_offset_ = gyroX_sum / MPUConfig::CALIBRATION_SAMPLES;
    gyroY_offset_ = gyroY_sum / MPUConfig::CALIBRATION_SAMPLES;
    gyroZ_offset_ = gyroZ_sum / MPUConfig::CALIBRATION_SAMPLES;

    printCalibrationData();

}

void printCalibrationData() {
    Serial.println("MPU6050 Calibration Complete!");
    Serial.println("Offsets:");
    Serial.print("Accel X: "); Serial.println(accelX_offset_);
    Serial.print("Accel Y: "); Serial.println(accelY_offset_);
    Serial.print("Accel Z: "); Serial.println(accelZ_offset_);
    Serial.print("Gyro X: "); Serial.println(gyroX_offset_);
    Serial.print("Gyro Y: "); Serial.println(gyroY_offset_);
    Serial.print("Gyro Z: "); Serial.println(gyroZ_offset_);
}

void setupESC(){
    ESP32PWM::allocateTimer(0);
    ESP32PWM::allocateTimer(1);
    ESP32PWM::allocateTimer(2);
    ESP32PWM::allocateTimer(3);

    escFL_F_.attach(ESCConfig::FL_PIN, ESCConfig::MIN_THROTTLE_PULSE, ESCConfig::MAX_THROTTLE_PULSE);
    escFL_F_.setPeriodHertz(ESCConfig::ESC_FREQ);
    escFR_R_.attach(ESCConfig::FR_PIN, ESCConfig::MIN_THROTTLE_PULSE, ESCConfig::MAX_THROTTLE_PULSE);
    escFR_R_.setPeriodHertz(ESCConfig::ESC_FREQ);
    escBL_L_.attach(ESCConfig::BL_PIN, ESCConfig::MIN_THROTTLE_PULSE, ESCConfig::MAX_THROTTLE_PULSE);
    escBL_L_.setPeriodHertz(ESCConfig::ESC_FREQ);
    escBR_B_.attach(ESCConfig::BR_PIN, ESCConfig::MIN_THROTTLE_PULSE, ESCConfig::MAX_THROTTLE_PULSE);
    escBR_B_.setPeriodHertz(ESCConfig::ESC_FREQ);

    escInitialized_ = true;

    Serial.println("ESC initialized successfully");
}

void calibrateESC() {
    Serial.println("--------------------------------------------------");
    Serial.println("IMPORTANT: Make sure your ESCs are powered on now!");
    delay(2000);

    Serial.println("\nStep 1: Sending maximum signal (2000) to all motors");
    writeAllMotors(ESCConfig::MAX_THROTTLE_PULSE);
    delay(2000);

    Serial.println("\nStep 2: Sending minimum signal (1000) to all motors");
    writeAllMotors(ESCConfig::MIN_THROTTLE_PULSE);
    delay(2000);

    Serial.println("\nESCs Calibration completed!");
}

void setupWiFi() {
    Serial.print("Starting access point: ");
    Serial.println(WiFiConfig::AP_SSID);

    IPAddress local_ip(WiFiConfig::AP_IP[0], WiFiConfig::AP_IP[1], WiFiConfig::AP_IP[2], WiFiConfig::AP_IP[3]);
    IPAddress gateway(WiFiConfig::AP_GATEWAY[0], WiFiConfig::AP_GATEWAY[1], WiFiConfig::AP_GATEWAY[2], WiFiConfig::AP_GATEWAY[3]);
    IPAddress subnet(WiFiConfig::AP_SUBNET[0], WiFiConfig::AP_SUBNET[1], WiFiConfig::AP_SUBNET[2], WiFiConfig::AP_SUBNET[3]);

    WiFi.mode(WIFI_AP);
    if (!WiFi.softAPConfig(local_ip, gateway, subnet)) {
        Serial.println("ERROR: Failed to configure static AP IP");
        return;
    }

    bool apStarted = WiFi.softAP(WiFiConfig::AP_SSID, WiFiConfig::AP_PASSWORD);
    if (!apStarted) {
        Serial.println("ERROR: Failed to start access point");
        return;
    }

    Serial.println("Access point started");
    Serial.print("AP IP address: ");
    Serial.println(WiFi.softAPIP());
}

void setupWebServer(){
    server_.on("/", HTTP_GET, []() {
        server_.send(200, "text/html", ROOT_HTML);
    });

    server_.on("/setThrottle", HTTP_GET, []() {
        if (server_.hasArg("value")) {
            int throttle = server_.arg("value").toInt();
            baseThrottle = constrain(throttle, 0, FlightConfig::MAX_THROTTLE_PERCENT);
            markCommandReceived();
            server_.send(200, "text/plain", "OK");
        } else {
            server_.send(400, "text/plain", "Bad Request");
        }
    });

    server_.on("/setRoll", HTTP_GET, []() {
        if (server_.hasArg("value")) {
            float roll = server_.arg("value").toFloat();
            targetRoll_ = constrain(roll, -FlightConfig::MAX_ROLL_ANGLE, FlightConfig::MAX_ROLL_ANGLE);
            markCommandReceived();
            server_.send(200, "text/plain", "OK");
        } else {
            server_.send(400, "text/plain", "Bad Request");
        }
    });

    server_.on("/setPitch", HTTP_GET, []() {
        if (server_.hasArg("value")) {
            float pitch = server_.arg("value").toFloat();
            targetPitch_ = constrain(pitch, -FlightConfig::MAX_PITCH_ANGLE, FlightConfig::MAX_PITCH_ANGLE);
            markCommandReceived();
            server_.send(200, "text/plain", "OK");
        } else {
            server_.send(400, "text/plain", "Bad Request");
        }
    });

    server_.on("/setYaw", HTTP_GET, []() {
        if (server_.hasArg("value")) {
            float yaw = server_.arg("value").toFloat();
            targetYaw_ = constrain(yaw, -FlightConfig::MAX_YAW_RATE, FlightConfig::MAX_YAW_RATE);
            markCommandReceived();
            server_.send(200, "text/plain", "OK");
        } else {
            server_.send(400, "text/plain", "Bad Request");
        }
    });

    server_.on("/getStatus", HTTP_GET, []() {
        char response[128];
        snprintf(response, sizeof(response), "{\"state\":\"%s\",\"throttle\":%d}",
                 flightStateToString(flightState_), baseThrottle);
        server_.send(200, "application/json", response);
    });

    server_.on("/arm", HTTP_POST, []() {
        markCommandReceived();
        requestArm("Web arm command");
        if (flightState_ == FlightState::ARMED) {
            server_.send(200, "text/plain", "Armed");
        } else {
            server_.send(409, "text/plain", "Arm request failed");
        }
    });

    server_.on("/disarm", HTTP_POST, []() {
        markCommandReceived();
        requestDisarm("Web disarm command");
        server_.send(200, "text/plain", "Disarmed");
    });

    server_.on("/land", HTTP_POST, []() {
        markCommandReceived();
        requestLanding("Web land command");
        if (flightState_ == FlightState::LANDING) {
            server_.send(200, "text/plain", "Landing initiated");
        } else {
            server_.send(409, "text/plain", "Landing not initiated");
        }
    });

    server_.on("/getPID", HTTP_GET, []() {
        char response[256];
        snprintf(response, sizeof(response),
                 "{\"kpRoll\":%.3f,\"kiRoll\":%.3f,\"kdRoll\":%.3f,\"kpPitch\":%.3f,\"kiPitch\":%.3f,\"kdPitch\":%.3f,\"kpYaw\":%.3f,\"kiYaw\":%.3f,\"kdYaw\":%.3f}",
                 Kp_roll_, Ki_roll_, Kd_roll_,
                 Kp_pitch_, Ki_pitch_, Kd_pitch_,
                 Kp_yaw_, Ki_yaw_, Kd_yaw_);

        server_.send(200, "application/json", response);
    });

    server_.on("/setPID", HTTP_POST, []() {
        bool hasAllArgs = server_.hasArg("kpRoll") && server_.hasArg("kiRoll") && server_.hasArg("kdRoll") &&
                          server_.hasArg("kpPitch") && server_.hasArg("kiPitch") && server_.hasArg("kdPitch") &&
                          server_.hasArg("kpYaw") && server_.hasArg("kiYaw") && server_.hasArg("kdYaw");

        if (!hasAllArgs) {
            server_.send(400, "text/plain", "Missing PID parameters");
            return;
        }

        Kp_roll_ = constrain(server_.arg("kpRoll").toFloat(), 0.0, 10.0);
        Ki_roll_ = constrain(server_.arg("kiRoll").toFloat(), 0.0, 10.0);
        Kd_roll_ = constrain(server_.arg("kdRoll").toFloat(), 0.0, 2.0);

        Kp_pitch_ = constrain(server_.arg("kpPitch").toFloat(), 0.0, 10.0);
        Ki_pitch_ = constrain(server_.arg("kiPitch").toFloat(), 0.0, 10.0);
        Kd_pitch_ = constrain(server_.arg("kdPitch").toFloat(), 0.0, 2.0);

        Kp_yaw_ = constrain(server_.arg("kpYaw").toFloat(), 0.0, 10.0);
        Ki_yaw_ = constrain(server_.arg("kiYaw").toFloat(), 0.0, 1.0);
        Kd_yaw_ = constrain(server_.arg("kdYaw").toFloat(), 0.0, 1.0);

        markCommandReceived();
        savePIDToEEPROM();
        server_.send(200, "text/plain", "PID constants updated");
    });

    server_.on("/calibrateMPU", HTTP_POST, []() {
        if (!pauseWatchdogForCalibration()) {
            server_.send(500, "text/plain", "Failed to pause watchdog for calibration");
            return;
        }

        setFlightState(FlightState::CALIBRATING, "MPU calibration request");

        calibrateMPU6050();
        saveCalibrationToEEPROM();

        bool watchdogResumed = resumeWatchdogAfterCalibration();

        setFlightState(FlightState::DISARMED, "MPU calibration complete");

        if (!watchdogResumed) {
            server_.send(500, "text/plain", "Calibration complete, but watchdog recovery failed");
            return;
        }

        server_.send(200, "text/plain", "MPU6050 calibration complete");
    });

    server_.on("/calibrateESC", HTTP_POST, []() {
        baseThrottle = 0;
        disarmMotors(NULL);

        setFlightState(FlightState::CALIBRATING, "ESC calibration request");

        if (!pauseWatchdogForCalibration()) {
            server_.send(500, "text/plain", "Failed to pause watchdog for ESC calibration");
            return;
        }

        calibrateESC();

        bool watchdogResumed = resumeWatchdogAfterCalibration();

        setFlightState(FlightState::DISARMED, "ESC calibration complete");

        if (!watchdogResumed) {
            server_.send(500, "text/plain", "ESC calibration complete, but watchdog recovery failed");
            return;
        }

        server_.send(200, "text/plain", "ESC calibration complete");
    });

    server_.on("/resetFlight", HTTP_GET, []() {
        resetIntegrators();
        requestDisarm("Flight reset");

        server_.send(200, "text/plain", "OK");
    });

    server_.on("/getLogs", HTTP_GET, []() {
        char text[256];
        snprintf(text, sizeof(text),
        "Roll: %.1f Pitch: %.1f Yaw: %.1f | Throt: %d | Motors: %d/%d/%d/%d us | dt(ms): %.2f | Kp: %.2f/%.2f/%.2f | Ki: %.2f/%.2f/%.2f | Kd: %.3f/%.3f/%.3f | Corrections: %.3f/%.3f | %.3f/%.3f | %.3f/%.3f",
        roll_, pitch_, yaw_,
        // targetRoll_, targetPitch_, targetYaw_,
        baseThrottle,
        currentPulseFL_, currentPulseFR_, currentPulseBL_, currentPulseBR_,
        dt_ * 1000.0,
        Kp_roll_, Kp_pitch_, Kp_yaw_,
        Ki_roll_, Ki_pitch_, Ki_yaw_,
        Kd_roll_, Kd_pitch_, Kd_yaw_,
        integralRoll_, prevErrorRoll_,
        integralPitch_, prevErrorPitch_,
        integralYaw_, prevErrorYaw_
        );

        server_.send(200, "text/plain", text);
    });
    
    server_.begin();
    Serial.println("Web server setup complete!");
}

void webServerTask(void* parameter) {
    esp_task_wdt_add(NULL);
    while (true) {
        unsigned long loopStartMs = millis();
        server_.handleClient();
        uint32_t loopElapsed = millis() - loopStartMs;
        if (loopElapsed > SystemConfig::WEB_SERVER_TIMEOUT_MS) {
            Serial.println("Warning: Web server task exceeded execution budget");
            triggerFailsafe("Failsafe: Web server task exceeded execution budget");
        }
        esp_task_wdt_reset();
        vTaskDelay(pdMS_TO_TICKS(SystemConfig::WEB_SERVER_UPDATE_INTERVAL_MS));
    }
}

bool runWebServer(){
    BaseType_t taskCreated = xTaskCreatePinnedToCore(
        webServerTask,                                  // Task function
        "WebServer",                                    // Task name
        SystemConfig::WEB_SERVER_TASK_STACK_SIZE,       // Stack size
        NULL,                                           // Task parameters
        SystemConfig::WEB_SERVER_TASK_PRIORITY,         // Priority
        &webServerTaskHandle_,                          // Task handle
        SystemConfig::WEB_SERVER_TASK_CORE              // Core ID
    );

    if (taskCreated != pdPASS) {
        return false;
    }
    Serial.println("Web server task created successfully");
    return true;
}
bool runPIDTask(){
    BaseType_t taskCreated = xTaskCreatePinnedToCore(
        pidControlTask,                         // Task function
        "PIDControl",                           // Task name
        SystemConfig::PID_TASK_STACK_SIZE,      // Stack size
        NULL,                                   // Task parameters
        SystemConfig::PID_TASK_PRIORITY,        // Priority
        &pidTaskHandle_,                        // Task handle
        SystemConfig::PID_TASK_CORE             // Core ID
    );
    if (taskCreated != pdPASS) {
            return false;
        }
    Serial.println("PID control task created successfully");
    return true;
}
void pidControlTask(void* parameter) {
    TickType_t lastWakeTime = xTaskGetTickCount();  // Initialize once
    esp_task_wdt_add(NULL);
    while (true) {
        unsigned long loopStartMs = millis();
        unsigned long currentMicros = micros();
        dt_ = (float)(currentMicros - previousMicros_) / 1000000.0;
        dt_ = max(dt_, PIDConfig::MIN_DT_SECONDS);
        previousMicros_ = currentMicros;

        if ((flightState_ == FlightState::ARMED || flightState_ == FlightState::LANDING) && hasCommandTimedOut()) {
            triggerFailsafe("Failsafe: Command timeout.");
            vTaskDelayUntil(&lastWakeTime, pdMS_TO_TICKS(SystemConfig::PID_UPDATE_INTERVAL_MS));
            continue;
        }

        sensorHealthy_ = updateMPU6050(dt_);
        if (!sensorHealthy_) {
            triggerFailsafe("Failsafe: Sensor read failed.");
            vTaskDelayUntil(&lastWakeTime, pdMS_TO_TICKS(SystemConfig::PID_UPDATE_INTERVAL_MS));
            continue;
        }

        switch (flightState_) {
            case FlightState::ARMED:
            case FlightState::LANDING:
                if (baseThrottle > FlightConfig::MIN_THROTTLE_PERCENT) {
                    calculateMotorOutputs();
                } else {
                    resetIntegrators();
                    writeAllMotors(ESCConfig::MIN_THROTTLE_PULSE);
                }
                break;
            case FlightState::INIT:
            case FlightState::CALIBRATING:
            case FlightState::DISARMED:
            case FlightState::FAILSAFE:
            default:
                applyMotorOutputsForState();
                break;
        }

        if (CalibrationConfig::ENABLE_DEBUG_PRINT ) { // Print every 50ms instead of every 5ms
            printDebugInfo();
            lastDebugPrint_ = currentMicros;
        }

        uint32_t loopElapsed = millis() - loopStartMs;
        if (loopElapsed > SystemConfig::MAX_TASK_EXECUTION_TIME_MS) {
            Serial.println("Warning: PID task exceeded execution budget");
            triggerFailsafe("Failsafe: PID task exceeded execution budget");
        }
        esp_task_wdt_reset();

        vTaskDelayUntil(&lastWakeTime, pdMS_TO_TICKS(SystemConfig::PID_UPDATE_INTERVAL_MS));
    }
}

void calculateMotorOutputs() {
    // Calculate PID outputs
    float rollOutput = computeRoll();
    float pitchOutput = computePitch();
    // Yaw corrections temporarily disabled
    float yawOutput = 0.0f;

    int fl_f_Adjust = -pitchOutput + rollOutput + yawOutput;  // Motor 1 (Front Left)
    int fr_r_Adjust = -pitchOutput - rollOutput - yawOutput;  // Motor 2 (Front Right)
    int bl_l_Adjust =  pitchOutput + rollOutput - yawOutput;// Motor 3 (Back Left)
    int br_b_Adjust = pitchOutput - rollOutput + yawOutput; // Motor 4 (Back Right)


    // Write to motors
    int basePulse = throttleToPulse(baseThrottle);
    writeMotorsAdjusted(basePulse, fl_f_Adjust, fr_r_Adjust, bl_l_Adjust, br_b_Adjust);
}
float computeRoll() {    
    float error = targetRoll_ - roll_;

    float PTerm = Kp_roll_ * error;
    float ITerm = constrain(integralRoll_+(Ki_roll_*(error+prevErrorRoll_) * (dt_/2)), -PIDConfig::MAX_INTEGRAL, PIDConfig::MAX_INTEGRAL);
    float DTerm = Kd_roll_ * (error - prevErrorRoll_) / dt_;
    float pidOutput = constrain(PTerm + ITerm + DTerm, -PIDConfig::MAX_PID_OUTPUT, PIDConfig::MAX_PID_OUTPUT);
    prevErrorRoll_ = error;
    integralRoll_ = ITerm;

    return pidOutput;
}

float computePitch() {
    float error = targetPitch_ - pitch_;

    float PTerm = Kp_pitch_ * error;
    float ITerm = constrain(integralPitch_+(Ki_pitch_*(error+prevErrorPitch_) * (dt_/2)), -PIDConfig::MAX_INTEGRAL, PIDConfig::MAX_INTEGRAL);
    float DTerm = Kd_pitch_ * (error - prevErrorPitch_) / dt_;
    float pidOutput = constrain(PTerm + ITerm + DTerm, -PIDConfig::MAX_PID_OUTPUT, PIDConfig::MAX_PID_OUTPUT);

    prevErrorPitch_ = error;
    integralPitch_ = ITerm;

    return pidOutput;
}

float computeYaw() {
    float error = targetYaw_ - yaw_;
    float PTerm = Kp_yaw_ * error;
    float ITerm = constrain(integralYaw_+(Ki_yaw_*(error+prevErrorYaw_) * (dt_/2)), -PIDConfig::MAX_INTEGRAL, PIDConfig::MAX_INTEGRAL);
    float DTerm = Kd_yaw_ * (error - prevErrorYaw_) / dt_;
    float pidOutput = constrain(PTerm + ITerm + DTerm, -PIDConfig::MAX_PID_OUTPUT, PIDConfig::MAX_PID_OUTPUT);

    prevErrorYaw_ = error;
    integralYaw_ = ITerm;

    return pidOutput;
}

void writeAllMotors(int pulse) {
    currentPulseFL_ = pulse;
    currentPulseFR_ = pulse;
    currentPulseBL_ = pulse;
    currentPulseBR_ = pulse;

    escFL_F_.writeMicroseconds(pulse);
    escFR_R_.writeMicroseconds(pulse);
    escBL_L_.writeMicroseconds(pulse);
    escBR_B_.writeMicroseconds(pulse);
}

void writeMotorsAdjusted(int basePulse, int flAdjust, int frAdjust, int blAdjust, int brAdjust) {
    currentPulseFL_ = constrain(basePulse + flAdjust, ESCConfig::MIN_THROTTLE_PULSE, ESCConfig::MAX_THROTTLE_PULSE);
    currentPulseFR_ = constrain(basePulse + frAdjust, ESCConfig::MIN_THROTTLE_PULSE, ESCConfig::MAX_THROTTLE_PULSE);
    currentPulseBL_ = constrain(basePulse + blAdjust, ESCConfig::MIN_THROTTLE_PULSE, ESCConfig::MAX_THROTTLE_PULSE);
    currentPulseBR_ = constrain(basePulse + brAdjust, ESCConfig::MIN_THROTTLE_PULSE, ESCConfig::MAX_THROTTLE_PULSE);

    escFL_F_.writeMicroseconds(currentPulseFL_);
    escFR_R_.writeMicroseconds(currentPulseFR_);
    escBL_L_.writeMicroseconds(currentPulseBL_);
    escBR_B_.writeMicroseconds(currentPulseBR_);
}

int throttleToPulse(int throttlePercent) {
    return map(throttlePercent, 0, 100, ESCConfig::MIN_THROTTLE_PULSE, ESCConfig::MAX_THROTTLE_PULSE);
}

bool updateMPU6050(float effectiveDt) {
    sensors_event_t a, g, temp;
    if (!mpu_.getEvent(&a, &g, &temp)) {
        return false;
    }

    // Apply calibration offsets
    float accelX = a.acceleration.x - accelX_offset_;
    float accelY = a.acceleration.y - accelY_offset_;
    float accelZ = a.acceleration.z - accelZ_offset_;
    float gyroX = g.gyro.x - gyroX_offset_;
    float gyroY = g.gyro.y - gyroY_offset_;
    float gyroZ = g.gyro.z - gyroZ_offset_;

    // Convert gyro to degrees/s
    gyroX_deg_s_ = gyroX * 180.0f / PI;
    gyroY_deg_s_ = gyroY * 180.0f / PI;
    gyroZ_deg_s_ = gyroZ * 180.0f / PI;

    // Calculate angles from accelerometer
    float accelRoll = atan2(accelY, accelZ) * 180.0f / PI;
    float accelPitch = atan2(-accelX, sqrt(accelY * accelY + accelZ * accelZ)) * 180.0f / PI;

    // Complementary filter
    roll_ = MPUConfig::COMPLEMENTARY_FILTER_ALPHA * (roll_ + gyroX_deg_s_ * effectiveDt) +
            (1.0f - MPUConfig::COMPLEMENTARY_FILTER_ALPHA) * accelRoll;
    pitch_ = MPUConfig::COMPLEMENTARY_FILTER_ALPHA * (pitch_ + gyroY_deg_s_ * effectiveDt) +
                (1.0f - MPUConfig::COMPLEMENTARY_FILTER_ALPHA) * accelPitch;
    yaw_ += gyroZ_deg_s_ * effectiveDt;

    return true;

}

void printDebugInfo() {
        char text[256];
        snprintf(text, sizeof(text),
        "Roll: %.1f Pitch: %.1f Yaw: %.1f | Throt: %d | Motors: %d/%d/%d/%d us | dt(ms): %.2f | Kp: %.2f/%.2f/%.2f | Ki: %.2f/%.2f/%.2f | Kd: %.3f/%.3f/%.3f | Corrections: %.3f/%.3f | %.3f/%.3f | %.3f/%.3f",
        roll_, pitch_, yaw_,
        baseThrottle,
        currentPulseFL_, currentPulseFR_, currentPulseBL_, currentPulseBR_,
        dt_ * 1000.0,
        // targetRoll_, targetPitch_, targetYaw_,
        Kp_roll_, Kp_pitch_, Kp_yaw_,
        Ki_roll_, Ki_pitch_, Ki_yaw_,
        Kd_roll_, Kd_pitch_, Kd_yaw_,
        integralRoll_, prevErrorRoll_,
        integralPitch_, prevErrorPitch_,
        integralYaw_, prevErrorYaw_
        );

    Serial.println(text);
}

