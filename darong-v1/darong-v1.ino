#include "config.h"
#include "root_ui.h"
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <ESP32Servo.h>
#include <cmath>
#include <WiFi.h>
#include <WebServer.h>
#include <Wire.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_task_wdt.h>

TaskHandle_t webServerTaskHandle_ = NULL;
TaskHandle_t pidTaskHandle_ = NULL;

Adafruit_MPU6050 mpu_;

Servo escFL_F_;
Servo escFR_R_;
Servo escBL_L_;
Servo escBR_B_;

int currentPulseFL_ = ESCConfig::MIN_THROTTLE_PULSE;
int currentPulseFR_ = ESCConfig::MIN_THROTTLE_PULSE;
int currentPulseBL_ = ESCConfig::MIN_THROTTLE_PULSE;
int currentPulseBR_ = ESCConfig::MIN_THROTTLE_PULSE;

WebServer server_(SystemConfig::WEB_SERVER_PORT);

int baseThrottle = 0;

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

float Kp_roll_ = PIDConfig::Kp_roll;
float Ki_roll_ = PIDConfig::Ki_roll;
float Kd_roll_ = PIDConfig::Kd_roll;
float Kp_pitch_ = PIDConfig::Kp_pitch;
float Ki_pitch_ = PIDConfig::Ki_pitch;
float Kd_pitch_ = PIDConfig::Kd_pitch;
float Kp_yaw_ = PIDConfig::Kp_yaw;
float Ki_yaw_ = PIDConfig::Ki_yaw;
float Kd_yaw_ = PIDConfig::Kd_yaw;

bool sensorHealthy_ = true;
unsigned long lastCommandMicros_ = 0;

void markCommandReceived() {
    lastCommandMicros_ = micros();
}

bool hasCommandTimedOut() {
    return (micros() - lastCommandMicros_) > (SystemConfig::COMMAND_TIMEOUT_MS * 1000UL);
}

void disarmMotors(const char* reason) {
    baseThrottle = 0;
    integralRoll_ = integralPitch_ = integralYaw_ = 0.0f;
    prevErrorRoll_ = prevErrorPitch_ = prevErrorYaw_ = 0.0f;
    writeAllMotors(ESCConfig::MIN_THROTTLE_PULSE);
    if (reason != nullptr) {
        Serial.println(reason);
    }
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

    int watchdogTimeoutSeconds = max(1, (int)std::ceil(SystemConfig::WATCHDOG_TIMEOUT_MS / 1000.0));
    if (esp_task_wdt_init(watchdogTimeoutSeconds, true) != ESP_OK) {
        Serial.println("ERROR: Failed to initialize watchdog timer!");
    }

    // Initialize components
    if (!setupMPU6050()) {
        Serial.println("ERROR: Failed to initialize MPU6050!");
        return;
    }
    
    if (CalibrationConfig::MPU6050_CALIBRATION) {
        calibrateMPU6050();
    }

    setupESC();
    if (CalibrationConfig::ESC_CALIBRATION) {
        calibrateESC();
    }
    writeAllMotors(ESCConfig::MIN_THROTTLE_PULSE);

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
    Serial.print("Connecting to WiFi: ");
    Serial.println(WiFiConfig::SSID);
    
    WiFi.begin(WiFiConfig::SSID, WiFiConfig::PASSWORD);
    
    while (WiFi.status() != WL_CONNECTED) {
        delay(100);
        Serial.print(".");
    }
    
    Serial.println("\nWiFi connected");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
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

        // PID Control Routes
    server_.on("/setKpRoll", HTTP_GET, []() {
        if (server_.hasArg("value")) {
            float kp = server_.arg("value").toFloat();
            Kp_roll_ = constrain(kp, 0.0, 10.0);  // Range: 0-10
            server_.send(200, "text/plain", "OK");
        } else {
            server_.send(400, "text/plain", "Bad Request");
        }
    });

    server_.on("/setKiRoll", HTTP_GET, []() {
        if (server_.hasArg("value")) {
            float ki = server_.arg("value").toFloat();
            Ki_roll_ = constrain(ki, 0.0, 10.0);  // Range: 0-1
            server_.send(200, "text/plain", "OK");
        } else {
            server_.send(400, "text/plain", "Bad Request");
        }
    });

    server_.on("/setKdRoll", HTTP_GET, []() {
        if (server_.hasArg("value")) {
            float kd = server_.arg("value").toFloat();
            Kd_roll_ = constrain(kd, 0.0, 2.0);  // Range: 0-1
            server_.send(200, "text/plain", "OK");
        } else {
            server_.send(400, "text/plain", "Bad Request");
        }
    });

    server_.on("/setKpPitch", HTTP_GET, []() {
        if (server_.hasArg("value")) {
            float kp = server_.arg("value").toFloat();
            Kp_pitch_ = constrain(kp, 0.0, 10.0);  // Range: 0-10
            server_.send(200, "text/plain", "OK");
        } else {
            server_.send(400, "text/plain", "Bad Request");
        }
    });

    server_.on("/setKiPitch", HTTP_GET, []() {
        if (server_.hasArg("value")) {
            float ki = server_.arg("value").toFloat();
            Ki_pitch_ = constrain(ki, 0.0, 10.0);  // Range: 0-1
            server_.send(200, "text/plain", "OK");
        } else {
            server_.send(400, "text/plain", "Bad Request");
        }
    });

    server_.on("/setKdPitch", HTTP_GET, []() {
        if (server_.hasArg("value")) {
            float kd = server_.arg("value").toFloat();
            Kd_pitch_ = constrain(kd, 0.0, 2.0);  // Range: 0-1
            server_.send(200, "text/plain", "OK");
        } else {
            server_.send(400, "text/plain", "Bad Request");
        }
    });

    server_.on("/setKpYaw", HTTP_GET, []() {
        if (server_.hasArg("value")) {
            float kp = server_.arg("value").toFloat();
            Kp_yaw_ = constrain(kp, 0.0, 10.0);  // Range: 0-10
            server_.send(200, "text/plain", "OK");
        } else {
            server_.send(400, "text/plain", "Bad Request");
        }
    });

    server_.on("/setKiYaw", HTTP_GET, []() {
        if (server_.hasArg("value")) {
            float ki = server_.arg("value").toFloat();
            Ki_yaw_ = constrain(ki, 0.0, 1.0);  // Range: 0-1
            server_.send(200, "text/plain", "OK");
        } else {
            server_.send(400, "text/plain", "Bad Request");
        }
    });

    server_.on("/setKdYaw", HTTP_GET, []() {
        if (server_.hasArg("value")) {
            float kd = server_.arg("value").toFloat();
            Kd_yaw_ = constrain(kd, 0.0, 1.0);  // Range: 0-1
            server_.send(200, "text/plain", "OK");
        } else {
            server_.send(400, "text/plain", "Bad Request");
        }
    });

    server_.on("/resetFlight", HTTP_GET, []() {
        integralRoll_ = 0.0f;
        prevErrorRoll_ = 0.0f;
        integralPitch_ = 0.0f;
        prevErrorPitch_ = 0.0f;
        integralYaw_ = 0.0f;
        prevErrorYaw_ = 0.0f;

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

        if (baseThrottle > FlightConfig::MIN_THROTTLE_PERCENT && hasCommandTimedOut()) {
            disarmMotors("Failsafe: Command timeout. Motors disarmed.");
            vTaskDelayUntil(&lastWakeTime, pdMS_TO_TICKS(SystemConfig::PID_UPDATE_INTERVAL_MS));
            continue;
        }

        sensorHealthy_ = updateMPU6050(dt_);
        if (!sensorHealthy_) {
            disarmMotors("Failsafe: Sensor read failed. Motors disarmed.");
            vTaskDelayUntil(&lastWakeTime, pdMS_TO_TICKS(SystemConfig::PID_UPDATE_INTERVAL_MS));
            continue;
        }

        if (baseThrottle > FlightConfig::MIN_THROTTLE_PERCENT) {
            calculateMotorOutputs();
        } else {
            integralRoll_ = 0.0f;
            prevErrorRoll_ = 0.0f;
            integralPitch_ = 0.0f;
            prevErrorPitch_ = 0.0f;
            integralYaw_ = 0.0f;
            prevErrorYaw_ = 0.0f;
            writeAllMotors(ESCConfig::MIN_THROTTLE_PULSE);
        }

        if (CalibrationConfig::ENABLE_DEBUG_PRINT ) { // Print every 50ms instead of every 5ms
            printDebugInfo();
            lastDebugPrint_ = currentMicros;
        }

        uint32_t loopElapsed = millis() - loopStartMs;
        if (loopElapsed > SystemConfig::MAX_TASK_EXECUTION_TIME_MS) {
            Serial.println("Warning: PID task exceeded execution budget");
        }
        esp_task_wdt_reset();

        vTaskDelayUntil(&lastWakeTime, pdMS_TO_TICKS(SystemConfig::PID_UPDATE_INTERVAL_MS));
    }
}

void calculateMotorOutputs() {
    // Calculate PID outputs
    float rollOutput = computeRoll();
    float pitchOutput = computePitch();
    float yawOutput = computeYaw();

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

