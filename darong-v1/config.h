#ifndef CONFIG_H
#define CONFIG_H

// System Configuration
struct SystemConfig {
    static constexpr unsigned long SERIAL_BAUD_RATE = 115200;
    
    // PID Control Task Configuration
    static constexpr int PID_TASK_STACK_SIZE = 8192;
    static constexpr int PID_TASK_PRIORITY = 1;  // Higher priority than web server
    static constexpr int PID_TASK_CORE = 0;      // Run on Core 0
    static constexpr int PID_UPDATE_INTERVAL_MS = 4; // 200Hz update rate

    // Web Server Task Configuration
    static constexpr int WEB_SERVER_PORT = 80;
    static constexpr int WEB_SERVER_TASK_STACK_SIZE = 8192;
    static constexpr int WEB_SERVER_TASK_PRIORITY = 2;
    static constexpr int WEB_SERVER_TASK_CORE = 1;  // Run on Core 1
    static constexpr int WEB_SERVER_UPDATE_INTERVAL_MS = 5;  // Faster web server response

    // System Stability Configuration
    static constexpr int MAX_TASK_EXECUTION_TIME_MS = 8;  // Max time for PID task
    static constexpr int WEB_SERVER_TIMEOUT_MS = 50;      // Max time for web server cycle
    static constexpr int WATCHDOG_TIMEOUT_MS = 100;       // Watchdog timeout
    static constexpr int COMMAND_TIMEOUT_MS = 500;        // Failsafe timeout for incoming commands
};

// Configuration Flags
struct CalibrationConfig {
    static constexpr int ESC_CALIBRATION_DELAY_MS = 2000;
    static constexpr bool ESC_CALIBRATION = false;
    static constexpr bool MPU6050_CALIBRATION = false;
    static constexpr bool ENABLE_DEBUG_PRINT = true;
    static constexpr uint32_t EEPROM_MAGIC = 0x4D505543; // 'MPUC'
    static constexpr size_t EEPROM_SIZE = 256;
};

struct PIDStorageConfig {
    static constexpr uint32_t EEPROM_MAGIC = 0x50494431; // 'PID1'
    static constexpr size_t EEPROM_OFFSET = 64; // Reserve space after calibration block
};

// WiFi Access Point Configuration
struct WiFiConfig {
    static constexpr const char* AP_SSID = "Darong-AP";
    static constexpr const char* AP_PASSWORD = "flysafe123"; // Minimum 8 characters for WPA2
};

// ESC Configuration
struct ESCConfig {
    static constexpr int FL_PIN = 16;  // X -> Front Left // + ->Front
    static constexpr int FR_PIN = 15;  // X -> Front Right // + -> Right
    static constexpr int BL_PIN = 14;  // X -> Back Left // + -> Left
    static constexpr int BR_PIN = 13;  // X -> Back Right // + -> Back
    
    static constexpr int MIN_THROTTLE_PULSE = 1000;
    static constexpr int MAX_THROTTLE_PULSE = 2000;
    static constexpr int ESC_FREQ = 500;
};

// PID Configuration
struct PIDConfig {
    // Roll PID
    static constexpr float Kp_roll = 2.4;
    static constexpr float Ki_roll = 1.9;
    static constexpr float Kd_roll = 0.110;
    
    // Pitch PID
    static constexpr float Kp_pitch = 2.4;
    static constexpr float Ki_pitch = 1.9;
    static constexpr float Kd_pitch = 0.110;

    // Yaw PID
    static constexpr float Kp_yaw = 2.0;
    static constexpr float Ki_yaw = 0.0;
    static constexpr float Kd_yaw = 0.0;
    
    // PID Limits
    static constexpr float MAX_INTEGRAL = 400.0;
    static constexpr float MAX_PID_OUTPUT = 400.0;
    static constexpr float MIN_PID_OUTPUT = -400.0;
    static constexpr float MIN_DT_SECONDS = 0.001f; // Minimum dt used in PID math
};

// Flight Configuration
struct FlightConfig {
    // Angle Limits
    static constexpr float MAX_ROLL_ANGLE = 30.0;
    static constexpr float MAX_PITCH_ANGLE = 30.0;
    static constexpr float MAX_YAW_RATE = 200.0;

    // Arming Preconditions
    static constexpr float ARMING_ATTITUDE_LIMIT_DEG = 5.0;
    static constexpr float ARMING_MAX_THROTTLE_PERCENT = 5.0;

    // Throttle Settings
    static constexpr float MIN_THROTTLE_PERCENT = 5.0;
    static constexpr float MAX_THROTTLE_PERCENT = 80.0;
};

// MPU6050 Configuration
struct MPUConfig {
    static constexpr int CALIBRATION_SAMPLES = 1000;
    static constexpr float COMPLEMENTARY_FILTER_ALPHA = 0.98;
    static constexpr float GYRO_THRESHOLD = 0.1;  // deg/s
    static constexpr float ACCEL_THRESHOLD = 0.1; // m/s^2
};

#endif // CONFIG_H 