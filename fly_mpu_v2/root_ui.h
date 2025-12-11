#ifndef ROOT_UI_H
#define ROOT_UI_H

const char ROOT_HTML[] = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>ESP32 Quad Motor Control</title>
  <style>
    * {
      box-sizing: border-box;
      margin: 0;
      padding: 0;
    }
    
    body {
      font-family: Arial, sans-serif;
      min-height: 100vh;
      display: flex;
      flex-direction: column;
      align-items: center;
      padding: 20px;
      background-color: #f5f5f5;
    }

    h2 {
      color: #333;
      margin-bottom: 30px;
      text-align: center;
    }

    .container {
      display: flex;
      flex-direction: column;
      align-items: center;
      background-color: white;
      padding: 30px;
      border-radius: 10px;
      box-shadow: 0 2px 10px rgba(0,0,0,0.1);
      width: 100%;
      max-width: 600px;
      gap: 30px;
    }

    .control-section {
      width: 100%;
      display: flex;
      flex-direction: column;
      align-items: center;
      padding: 20px;
      border: 1px solid #eee;
      border-radius: 8px;
    }

    .section-title {
      font-size: 1.2em;
      color: #333;
      margin-bottom: 15px;
      font-weight: bold;
    }

    /* Test Toggle Switch */
    .switch {
      position: relative;
      display: inline-block;
      width: 60px;
      height: 34px;
    }

    .switch input {
      opacity: 0;
      width: 0;
      height: 0;
    }

    .slider {
      position: absolute;
      cursor: pointer;
      top: 0;
      left: 0;
      right: 0;
      bottom: 0;
      background-color: #ccc;
      transition: .4s;
      border-radius: 34px;
    }

    .slider:before {
      position: absolute;
      content: "";
      height: 26px;
      width: 26px;
      left: 4px;
      bottom: 4px;
      background-color: white;
      transition: .4s;
      border-radius: 50%;
    }

    input:checked + .slider {
      background-color: #2196F3;
    }

    input:checked + .slider:before {
      transform: translateX(26px);
    }

    .switch.disabled {
      opacity: 0.5;
      cursor: not-allowed;
    }

    .switch.disabled input {
      cursor: not-allowed;
    }

    /* Speed Control Buttons */
    .speed-control {
      display: flex;
      gap: 20px;
      align-items: center;
    }

    .speed-btn {
      padding: 10px 20px;
      font-size: 1.2em;
      border: none;
      border-radius: 5px;
      cursor: pointer;
      background-color: #2196F3;
      color: white;
      transition: background-color 0.3s;
    }

    .speed-btn:disabled {
      background-color: #cccccc;
      cursor: not-allowed;
    }

    .speed-btn:hover:not(:disabled) {
      background-color: #1976D2;
    }

    .speed-value {
      font-size: 1.2em;
      color: #333;
      min-width: 60px;
      text-align: center;
    }

    /* Throttle Control */
    .throttle-control {
      display: flex;
      flex-direction: column;
      align-items: center;
      width: 100%;
    }

    #masterThrottle {
      width: 50px;
      height: 300px;
      -webkit-appearance: slider-vertical;
      writing-mode: bt-lr;
    }

    #masterThrottle:disabled {
      opacity: 0.5;
      cursor: not-allowed;
    }

    .value-display {
      margin-top: 20px;
      font-size: 1.2em;
      color: #666;
    }

    @media (max-width: 768px) {
      #masterThrottle {
        height: 200px;
      }
    }
  </style>
</head>
<body>
  <h2>ESP32 Quad Motor Control</h2>
  <div class="container">
    <!-- Test Toggle Section -->
    <div class="control-section">
      <div class="section-title">Test Mode</div>
      <label class="switch" id="testSwitch">
        <input type="checkbox" id="testToggle">
        <span class="slider"></span>
      </label>
      <div class="value-display">Throttle: <span id="testValue">0</span>%</div>
    </div>

    <!-- Speed Control Section -->
    <div class="control-section">
      <div class="section-title">Speed Control</div>
      <div class="speed-control">
        <button class="speed-btn" id="decreaseSpeed" disabled>-</button>
        <div class="speed-value" id="speedValue">0%</div>
        <button class="speed-btn" id="increaseSpeed">+</button>
      </div>
    </div>

    <!-- Throttle Control Section -->
    <div class="control-section">
      <div class="section-title">Master Throttle</div>
      <div class="throttle-control">
        <input type="range" min="0" max="100" value="0" id="masterThrottle">
        <div class="value-display">Throttle: <span id="throttleValue">0</span>%</div>
      </div>
    </div>
  </div>

  <script>
    const masterThrottle = document.getElementById("masterThrottle");
    const throttleValue = document.getElementById("throttleValue");
    const testToggle = document.getElementById("testToggle");
    const testSwitch = document.getElementById("testSwitch");
    const testValue = document.getElementById("testValue");
    const increaseSpeed = document.getElementById("increaseSpeed");
    const decreaseSpeed = document.getElementById("decreaseSpeed");
    const speedValue = document.getElementById("speedValue");
    
    let currentSpeed = 0;
    let isTestMode = false;

    // Update all throttle displays
    function updateThrottleDisplays(value) {
      throttleValue.innerText = value;
      speedValue.innerText = value + "%";
      testValue.innerText = value;
      masterThrottle.value = value;
    }

    // Update all controls state
    function updateControlsState() {
      const throttleValue = parseInt(masterThrottle.value);
      
      // Update test mode controls
      // Only disable test mode toggle if manual controls are used (not in test mode)
      const hasManualControl = !isTestMode && (throttleValue > 0 || currentSpeed > 0);
      testToggle.disabled = hasManualControl;
      testSwitch.classList.toggle('disabled', hasManualControl);
      
      // Update speed controls
      // Enable speed controls when not in test mode
      const speedEnabled = !isTestMode;
      increaseSpeed.disabled = !speedEnabled || currentSpeed >= 100;
      decreaseSpeed.disabled = !speedEnabled || currentSpeed <= 0;
      
      // Update throttle control
      masterThrottle.disabled = isTestMode;
    }

    // Test Toggle Handler
    testToggle.onchange = function() {
      if (this.disabled) return;
      
      isTestMode = this.checked;
      if (isTestMode) {
        currentSpeed = 1;
        updateThrottleDisplays(currentSpeed);
        fetch("/set?val=" + currentSpeed);
      } else {
        currentSpeed = 0;
        updateThrottleDisplays(currentSpeed);
        fetch("/set?val=" + currentSpeed);
      }
      updateControlsState();
    }

    // Speed Control Handlers
    increaseSpeed.onclick = function() {
      if (currentSpeed < 100) {
        currentSpeed++;
        updateThrottleDisplays(currentSpeed);
        fetch("/set?val=" + currentSpeed);
        updateControlsState();
      }
    }

    decreaseSpeed.onclick = function() {
      if (currentSpeed > 0) {
        currentSpeed--;
        updateThrottleDisplays(currentSpeed);
        fetch("/set?val=" + currentSpeed);
        updateControlsState();
      }
    }

    // Throttle Slider Handler
    masterThrottle.oninput = function() {
      const value = this.value;
      currentSpeed = parseInt(value);
      updateThrottleDisplays(value);
      fetch("/set?val=" + value);
      updateControlsState();
    }

    // Initial state
    updateThrottleDisplays(0);
    updateControlsState();
  </script>
</body>
</html>
)rawliteral";

#endif // ROOT_UI_H
