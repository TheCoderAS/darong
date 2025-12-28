#ifndef ROOT_UI_H
#define ROOT_UI_H

const char ROOT_HTML[] = R"rawliteral(
<!DOCTYPE html>
<html>

<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Quadcopter Control Panel</title>
  <style>
    * {
      box-sizing: border-box;
      margin: 0;
      padding: 0;
    }

    :root {
      --gap-lg: 24px;
      --gap-md: 16px;
    }

    body {
      font-family: 'Segoe UI', Arial, sans-serif;
      min-height: 100vh;
      display: flex;
      flex-direction: column;
      align-items: center;
      justify-content: flex-start;
      gap: var(--gap-lg);
      padding: 20px;
      background: linear-gradient(135deg, #000000 0%, #1a1a1a 50%, #2d2d2d 100%);
      color: #ffffff;
    }

    h2 {
      color: #ffffff;
      margin-bottom: 20px;
      text-align: center;
      font-size: 2em;
      text-shadow: 0 2px 4px rgba(0, 0, 0, 0.3);
      font-weight: bold;
    }

    .container {
      display: flex;
      flex-direction: row;
      align-items: center;
      justify-content: center;
      flex-wrap: wrap;
      gap: clamp(12px, 2vw, 24px);
      padding: 20px;
      background: rgba(0, 0, 0, 0.7);
      border-radius: 20px;
      box-shadow: 0 8px 32px rgba(0, 0, 0, 0.5);
      width: 100%;
      max-width: 1200px;
      border: 1px solid rgba(255, 255, 255, 0.1);
    }

    .page-shell {
      width: 100%;
      max-width: 1280px;
      display: flex;
      flex-direction: column;
      align-items: center;
      gap: var(--gap-lg);
    }

    .loading-overlay {
      position: fixed;
      inset: 0;
      background: rgba(0, 0, 0, 0.75);
      display: flex;
      flex-direction: column;
      justify-content: center;
      align-items: center;
      gap: 16px;
      color: #ffffff;
      font-weight: 600;
      letter-spacing: 0.5px;
      z-index: 10;
    }

    .loader {
      width: 48px;
      height: 48px;
      border: 5px solid rgba(255, 255, 255, 0.2);
      border-top-color: #ffffff;
      border-radius: 50%;
      animation: spin 1s linear infinite;
    }

    @keyframes spin {
      from {
        transform: rotate(0deg);
      }

      to {
        transform: rotate(360deg);
      }
    }

    .control-section {
      display: flex;
      flex-direction: column;
      align-items: center;
      padding: 15px;
      background: rgba(0, 0, 0, 0.5);
      border-radius: 15px;
      min-width: 200px;
      flex: 1 1 240px;
      border: 1px solid rgba(255, 255, 255, 0.1);
      transition: transform 0.3s ease, box-shadow 0.3s ease;
      gap: var(--gap-md);
    }

    .control-section:hover {
      transform: translateY(-5px);
      box-shadow: 0 8px 16px rgba(0, 0, 0, 0.3);
      background: rgba(0, 0, 0, 0.6);
    }

    .section-title {
      font-size: 1.1em;
      color: #ffffff;
      margin-bottom: 15px;
      font-weight: 500;
      text-transform: uppercase;
      letter-spacing: 1px;
    }

    .value-display {
      margin-top: 15px;
      font-size: 1em;
      color: #ffffff;
      background: rgba(0, 0, 0, 0.5);
      padding: 8px 15px;
      border-radius: 8px;
      min-width: 90px;
      text-align: center;
      border: 1px solid rgba(255, 255, 255, 0.1);
    }

    .motor-toggle-grid {
      display: grid;
      grid-template-columns: repeat(2, minmax(120px, 1fr));
      gap: 12px;
      width: 100%;
      margin-top: 10px;
    }

    .motor-toggle {
      padding: 10px;
      border-radius: 8px;
      border: 1px solid rgba(255, 255, 255, 0.2);
      background: rgba(255, 255, 255, 0.08);
      color: #ffffff;
      cursor: pointer;
      transition: background 0.2s ease, transform 0.2s ease, border-color 0.2s ease;
      font-weight: 600;
      text-align: center;
    }

    .motor-toggle:hover:not(:disabled) {
      background: rgba(255, 255, 255, 0.16);
      transform: translateY(-1px);
    }

    .motor-toggle:disabled {
      opacity: 0.6;
      cursor: not-allowed;
      transform: none;
    }

    .motor-toggle.active {
      background: linear-gradient(135deg, #2d2d2d, #1a1a1a);
      border-color: rgba(255, 255, 255, 0.35);
    }

    /* Throttle Control */
    .throttle-control {
      display: flex;
      flex-direction: column;
      align-items: center;
      height: 300px;
    }

    #masterThrottle {
      width: 50px;
      height: 250px;
      -webkit-appearance: slider-vertical;
      writing-mode: bt-lr;
      background: rgba(255, 255, 255, 0.1);
      border-radius: 25px;
      outline: none;
      border: 1px solid rgba(255, 255, 255, 0.2);
    }

    #masterThrottle::-webkit-slider-thumb {
      -webkit-appearance: none;
      width: 30px;
      height: 30px;
      background: #333333;
      border-radius: 50%;
      cursor: pointer;
      transition: all 0.3s ease;
      box-shadow: 0 2px 4px rgba(0, 0, 0, 0.3);
      border: 2px solid #666666;
    }

    #masterThrottle::-webkit-slider-thumb:hover {
      background: #444444;
      transform: scale(1.1);
    }

    .throttle-buttons {
      display: flex;
      gap: 10px;
      margin-top: 10px;
    }

    .throttle-set-buttons {
      display: flex;
      flex-wrap: wrap;
      gap: 8px;
      justify-content: center;
      margin-bottom: 12px;
    }

    .throttle-set-btn {
      min-width: 58px;
      height: 32px;
      border-radius: 16px;
      border: 1px solid rgba(255, 255, 255, 0.2);
      background: rgba(255, 255, 255, 0.08);
      color: #ffffff;
      font-size: 13px;
      font-weight: 600;
      cursor: pointer;
      transition: all 0.3s ease;
    }

    .throttle-set-btn:hover {
      background: rgba(255, 255, 255, 0.2);
      transform: translateY(-1px);
    }

    .throttle-set-btn:active {
      transform: scale(0.97);
    }

    .throttle-set-btn:disabled {
      opacity: 0.5;
      cursor: not-allowed;
      transform: none;
    }

    .throttle-btn {
      width: 40px;
      height: 40px;
      border-radius: 50%;
      border: none;
      background: rgba(255, 255, 255, 0.1);
      color: #ffffff;
      font-size: 20px;
      cursor: pointer;
      transition: all 0.3s ease;
      display: flex;
      align-items: center;
      justify-content: center;
      border: 1px solid rgba(255, 255, 255, 0.2);
    }

    .throttle-btn:hover {
      background: rgba(255, 255, 255, 0.2);
      transform: scale(1.1);
    }

    .throttle-btn:active {
      transform: scale(0.95);
    }

    .throttle-btn:disabled {
      opacity: 0.5;
      cursor: not-allowed;
      transform: none;
    }

    .state-display {
      display: flex;
      flex-direction: row;
      align-items: center;
      gap: 12px;
      padding: 10px 14px;
      background: rgba(0, 0, 0, 0.4);
      border-radius: 12px;
      border: 1px solid rgba(255, 255, 255, 0.15);
      flex: 1;
      min-width: 260px;
    }

    .state-chip {
      padding: 6px 12px;
      border-radius: 999px;
      font-weight: 600;
      background: linear-gradient(135deg, #2c2c2c, #1a1a1a);
      border: 1px solid rgba(255, 255, 255, 0.2);
      box-shadow: 0 2px 6px rgba(0, 0, 0, 0.35);
    }

    .flight-actions {
      display: flex;
      gap: 10px;
      margin-left: auto;
    }

    .flight-action-btn {
      padding: 8px 14px;
      border-radius: 8px;
      border: 1px solid rgba(255, 255, 255, 0.2);
      background: rgba(255, 255, 255, 0.08);
      color: #ffffff;
      cursor: pointer;
      transition: background 0.2s ease, transform 0.2s ease;
      font-weight: 600;
    }

    .flight-action-btn:hover:not(:disabled) {
      background: rgba(255, 255, 255, 0.16);
      transform: translateY(-1px);
    }

    .flight-action-btn:disabled {
      opacity: 0.5;
      cursor: not-allowed;
      transform: none;
    }

    .status-note {
      font-size: 0.9em;
      color: #cccccc;
      text-align: left;
      flex: 1;
      min-width: 200px;
    }

    .status-container {
      justify-content: center;
      width: 100%;
    }

    .status-section {
      min-width: 280px;
      flex: 1;
      display: flex;
      flex-direction: row;
      align-items: center;
      gap: 16px;
      width: 100%;
      flex-wrap: wrap;
    }

    /* PID Control Styles */
    .pid-form {
      display: grid;
      grid-template-columns: repeat(3, minmax(180px, 1fr));
      gap: 15px;
      width: 100%;
    }

    .pid-card {
      background: rgba(0, 0, 0, 0.5);
      border: 1px solid rgba(255, 255, 255, 0.1);
      border-radius: 12px;
      padding: 12px;
    }

    .pid-card h3 {
      margin-bottom: 10px;
      font-size: 1em;
      letter-spacing: 0.5px;
    }

    .pid-row {
      display: grid;
      grid-template-columns: 1fr 1fr;
      gap: 8px;
      margin-bottom: 8px;
      align-items: center;
    }

    .pid-row label {
      font-size: 0.9em;
      color: #ffffff;
    }

    .pid-row input {
      width: 100%;
      padding: 8px;
      border-radius: 6px;
      border: 1px solid rgba(255, 255, 255, 0.2);
      background: rgba(255, 255, 255, 0.08);
      color: #ffffff;
      font-size: 0.95em;
    }

    .pid-actions {
      display: flex;
      gap: 10px;
      justify-content: flex-end;
      margin-top: 10px;
      width: 100%;
    }

    .pid-button {
      padding: 10px 14px;
      border-radius: 10px;
      border: 1px solid rgba(255, 255, 255, 0.2);
      background: rgba(255, 255, 255, 0.12);
      color: #ffffff;
      cursor: pointer;
      font-size: 0.95em;
      transition: all 0.2s ease;
    }

    .pid-button:hover {
      background: rgba(255, 255, 255, 0.18);
      transform: translateY(-1px);
    }

    .pid-button:disabled {
      opacity: 0.5;
      cursor: not-allowed;
      transform: none;
    }

    .pid-button.disabled-static,
    .pid-button.disabled-static:disabled {
      opacity: 0.35;
      cursor: not-allowed;
      filter: grayscale(60%);
    }

    .pid-status {
      margin-top: 8px;
      font-size: 0.9em;
      color: #cccccc;
    }

    .calibration-status {
      margin-top: 10px;
      font-size: 0.9em;
      color: #cccccc;
      text-align: center;
      min-height: 18px;
    }

    /* Roll, Pitch, Yaw Controls */
    .stick-control {
      position: relative;
      width: 120px;
      height: 120px;
      background: rgba(0, 0, 0, 0.5);
      border-radius: 50%;
      margin: 15px 0;
      border: 1px solid rgba(255, 255, 255, 0.2);
      box-shadow: 0 4px 8px rgba(0, 0, 0, 0.3);
      transition: transform 0.3s ease;
    }

    .stick-control:hover {
      transform: scale(1.05);
      background: rgba(0, 0, 0, 0.6);
    }

    .stick {
      position: absolute;
      width: 35px;
      height: 35px;
      background: #333333;
      border-radius: 50%;
      top: 50%;
      left: 50%;
      transform: translate(-50%, -50%);
      cursor: move;
      transition: all 0.3s ease;
      box-shadow: 0 2px 4px rgba(0, 0, 0, 0.3);
      border: 2px solid #666666;
    }

    .stick:hover {
      background: #444444;
      transform: translate(-50%, -50%) scale(1.1);
    }

    /* Flight Lock Switch */
    .switch {
      position: relative;
      display: inline-block;
      width: 50px;
      height: 28px;
      margin: 10px 0;
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
      background-color: rgba(0, 0, 0, 0.5);
      transition: .4s;
      border-radius: 28px;
      box-shadow: 0 2px 4px rgba(0, 0, 0, 0.3);
      border: 1px solid rgba(255, 255, 255, 0.2);
    }

    .slider:before {
      position: absolute;
      content: "";
      height: 22px;
      width: 22px;
      left: 3px;
      bottom: 3px;
      background-color: #666666;
      transition: .4s;
      border-radius: 50%;
      box-shadow: 0 2px 4px rgba(0, 0, 0, 0.3);
    }

    input:checked+.slider {
      background-color: #333333;
    }

    input:checked+.slider:before {
      transform: translateX(22px);
      background-color: #ffffff;
    }

    /* Responsive Design */
    @media (max-width: 1024px) {
      .container {
        gap: 15px;
        padding: 15px;
      }

      .control-section {
        min-width: 150px;
        padding: 12px;
      }

      .stick-control {
        width: 100px;
        height: 100px;
      }

      .stick {
        width: 30px;
        height: 30px;
      }
    }

    @media (max-width: 768px) {
      .container {
        flex-direction: column;
        max-width: 400px;
      }

      .control-section {
        width: 100%;
        min-width: unset;
      }

      .throttle-control {
        height: 250px;
      }

      #masterThrottle {
        height: 200px;
      }
    }

    @media (max-height: 600px) and (orientation: landscape) {
      .container {
        flex-direction: row;
        flex-wrap: wrap;
        justify-content: center;
        max-width: 100%;
        gap: 10px;
      }

      .control-section {
        width: auto;
        min-width: 140px;
        padding: 10px;
      }

      .throttle-control {
        height: 200px;
      }

      #masterThrottle {
        height: 150px;
      }

      .stick-control {
        width: 90px;
        height: 90px;
        margin: 10px 0;
      }

      .stick {
        width: 25px;
        height: 25px;
      }

      .section-title {
        font-size: 1em;
        margin-bottom: 10px;
      }

      .value-display {
        font-size: 0.9em;
        padding: 6px 12px;
        margin-top: 10px;
      }
    }
  </style>
</head>

<body>
  <div class="loading-overlay" id="loadingOverlay" aria-live="polite">
    <div class="loader" aria-hidden="true"></div>
    <div id="loadingMessage">Preparing interface...</div>
  </div>
  <div class="page-shell">
    <h2>Quadcopter Control Panel</h2>
    <div class="container status-container" style="margin-top: 10px;">
      <div class="control-section status-section">
        <div class="state-display" aria-label="Flight state">
          <div class="state-chip" id="flightState">INIT</div>
          <div class="status-note" id="flightStatusNote">Request arming to enable controls.</div>
        </div>
        <div class="flight-actions">
          <button class="flight-action-btn" id="armButton">Arm</button>
          <button class="flight-action-btn" id="disarmButton">Disarm</button>
          <button class="flight-action-btn" id="landButton">Land</button>
        </div>
      </div>
    </div>
    <div class="container">
    <!-- Throttle Control -->
    <div class="control-section">
      <div class="section-title">Throttle</div>
      <div class="throttle-control">
        <div class="throttle-set-buttons">
          <button class="throttle-set-btn" data-throttle="20">20%</button>
          <button class="throttle-set-btn" data-throttle="25">25%</button>
          <button class="throttle-set-btn" data-throttle="30">30%</button>
          <button class="throttle-set-btn" data-throttle="35">35%</button>
          <button class="throttle-set-btn" data-throttle="40">40%</button>
          <button class="throttle-set-btn" data-throttle="45">45%</button>
        </div>
        <input type="range" min="0" max="100" value="0" id="masterThrottle">
        <div class="throttle-buttons">
          <button class="throttle-btn" id="decreaseThrottle">-</button>
          <button class="throttle-btn" id="increaseThrottle">+</button>
        </div>
        <div class="value-display">Throttle: <span id="throttleValue">0</span>%</div>
      </div>
    </div>

    <!-- Roll Control -->
    <div class="control-section">
      <div class="section-title">Roll</div>
      <div class="stick-control" id="rollStick">
        <div class="stick" id="rollStickHandle"></div>
      </div>
      <div class="value-display">Roll: <span id="rollValue">0</span>°</div>
    </div>

    <!-- Pitch Control -->
    <div class="control-section">
      <div class="section-title">Pitch</div>
      <div class="stick-control" id="pitchStick">
        <div class="stick" id="pitchStickHandle"></div>
      </div>
      <div class="value-display">Pitch: <span id="pitchValue">0</span>°</div>
    </div>

    <!-- Yaw Control -->
    <div class="control-section">
      <div class="section-title">Yaw</div>
      <div class="stick-control" id="yawStick">
        <div class="stick" id="yawStickHandle"></div>
      </div>
      <div class="value-display">Yaw: <span id="yawValue">0</span>°</div>
    </div>

    <!-- Test Mode -->
    <div class="control-section">
      <div class="section-title">Flight Lock</div>
      <label class="switch">
        <input type="checkbox" id="flightLockToggle">
        <span class="slider"></span>
      </label>
      <div class="value-display">Lock: <span id="lockValue">OFF</span></div>
    </div>
  </div>

  <h2 style="margin-top: 20px;">Motor Test</h2>
  <div class="container">
    <div class="control-section" style="flex: 1; min-width: 240px;">
      <div class="section-title">Test Mode</div>
      <label class="switch">
        <input type="checkbox" id="testModeToggle">
        <span class="slider"></span>
      </label>
      <div class="value-display">Test Mode: <span id="testModeValue">OFF</span></div>
      <div class="motor-toggle-grid">
        <button class="motor-toggle" data-motor="fl">Front Left</button>
        <button class="motor-toggle" data-motor="fr">Front Right</button>
        <button class="motor-toggle" data-motor="bl">Back Left</button>
        <button class="motor-toggle" data-motor="br">Back Right</button>
      </div>
      <div class="status-note" style="margin-top: 10px;">Selected motors follow master throttle only when test mode is ON.</div>
    </div>
  </div>

  <h2 style="margin-top: 20px;">Calibrations</h2>
  <div class="container">
    <div class="control-section">
      <div class="section-title">MPU6050</div>
      <button id="calibrateMPUButton" class="pid-button disabled-static" disabled aria-disabled="true">Calibrate Sensors</button>
      <div id="mpuStatus" class="calibration-status">Idle</div>
    </div>

    <div class="control-section">
      <div class="section-title">ESCs</div>
      <button id="calibrateESCButton" class="pid-button">Calibrate ESCs</button>
      <div id="escStatus" class="calibration-status">Idle</div>
    </div>
  </div>

  <!-- PID Control Section -->
  <h2 style="margin-top: 20px;">PID Tuning</h2>
  <div class="container">
    <div class="pid-form">
      <div class="pid-card">
        <h3>Roll</h3>
        <div class="pid-row">
          <label for="kpRoll">Kp</label>
          <input type="number" step="0.001" id="kpRoll" />
        </div>
        <div class="pid-row">
          <label for="kiRoll">Ki</label>
          <input type="number" step="0.001" id="kiRoll" />
        </div>
        <div class="pid-row">
          <label for="kdRoll">Kd</label>
          <input type="number" step="0.001" id="kdRoll" />
        </div>
      </div>
      <div class="pid-card">
        <h3>Pitch</h3>
        <div class="pid-row">
          <label for="kpPitch">Kp</label>
          <input type="number" step="0.001" id="kpPitch" />
        </div>
        <div class="pid-row">
          <label for="kiPitch">Ki</label>
          <input type="number" step="0.001" id="kiPitch" />
        </div>
        <div class="pid-row">
          <label for="kdPitch">Kd</label>
          <input type="number" step="0.001" id="kdPitch" />
        </div>
      </div>
      <div class="pid-card">
        <h3>Yaw</h3>
        <div class="pid-row">
          <label for="kpYaw">Kp</label>
          <input type="number" step="0.001" id="kpYaw" />
        </div>
        <div class="pid-row">
          <label for="kiYaw">Ki</label>
          <input type="number" step="0.001" id="kiYaw" />
        </div>
        <div class="pid-row">
          <label for="kdYaw">Kd</label>
          <input type="number" step="0.001" id="kdYaw" />
        </div>
      </div>
    </div>
  </div>

  <!-- Reset Button -->
  <div class="container" style="margin-top: 20px; flex-direction: column; align-items: stretch; gap: 10px;">
    <div class="pid-actions">
      <button id="savePIDBtn" class="pid-button">Save PID</button>
      <button id="resetPIDBtn" class="pid-button">Reset PID</button>
      <button id="resetFlightBtn" class="pid-button">Reset Flight</button>
      <button id="restartBtn" class="pid-button">Restart Drone</button>
    </div>
    <div id="pidStatus" class="pid-status"></div>
  </div>
  <div id="logContainer" style="margin-top: 20px; height: 200px; overflow: auto;">
  </div>
  </div>
  <script>
    // Initialize elements
    const masterThrottle = document.getElementById("masterThrottle");
    const throttleValue = document.getElementById("throttleValue");
    const flightLockToggle = document.getElementById("flightLockToggle");
    const lockValue = document.getElementById("lockValue");
    const testModeToggle = document.getElementById("testModeToggle");
    const testModeValue = document.getElementById("testModeValue");
    const motorToggleButtons = Array.from(document.querySelectorAll('.motor-toggle'));
    const decreaseThrottleBtn = document.getElementById("decreaseThrottle");
    const increaseThrottleBtn = document.getElementById("increaseThrottle");
    const throttleSetButtons = Array.from(document.querySelectorAll('.throttle-set-btn'));
    const flightStateDisplay = document.getElementById("flightState");
    const flightStatusNote = document.getElementById("flightStatusNote");
    const armButton = document.getElementById("armButton");
    const disarmButton = document.getElementById("disarmButton");
    const landButton = document.getElementById("landButton");
    const pidInputs = {
      kpRoll: document.getElementById("kpRoll"),
      kiRoll: document.getElementById("kiRoll"),
      kdRoll: document.getElementById("kdRoll"),
      kpPitch: document.getElementById("kpPitch"),
      kiPitch: document.getElementById("kiPitch"),
      kdPitch: document.getElementById("kdPitch"),
      kpYaw: document.getElementById("kpYaw"),
      kiYaw: document.getElementById("kiYaw"),
      kdYaw: document.getElementById("kdYaw")
    };

    const savePIDBtn = document.getElementById("savePIDBtn");
    const resetPIDBtn = document.getElementById("resetPIDBtn");
    const resetFlightBtn = document.getElementById("resetFlightBtn");
    const restartBtn = document.getElementById("restartBtn");
    const pidStatus = document.getElementById("pidStatus");
    const calibrateMPUButton = document.getElementById("calibrateMPUButton");
    const calibrateESCButton = document.getElementById("calibrateESCButton");
    const mpuStatus = document.getElementById("mpuStatus");
    const escStatus = document.getElementById("escStatus");
    const loadingOverlay = document.getElementById("loadingOverlay");
    const loadingMessage = document.getElementById("loadingMessage");

    if (calibrateMPUButton) {
      calibrateMPUButton.disabled = true;
      calibrateMPUButton.classList.add('disabled-static');
      calibrateMPUButton.title = 'MPU calibration is unavailable';
    }

    // Initialize stick controls
    const sticks = {
      roll: { element: document.getElementById("rollStick"), handle: document.getElementById("rollStickHandle"), value: document.getElementById("rollValue"), range: 30 },
      pitch: { element: document.getElementById("pitchStick"), handle: document.getElementById("pitchStickHandle"), value: document.getElementById("pitchValue"), range: 30 },
      yaw: { element: document.getElementById("yawStick"), handle: document.getElementById("yawStickHandle"), value: document.getElementById("yawValue"), range: 200 }
    };

    const delay = (ms) => new Promise(resolve => setTimeout(resolve, ms));

    let isInitialLoading = true;
    let isLocked = false;
    let isTestMode = false;
    const motorTestState = { fl: false, fr: false, bl: false, br: false };
    let pidLoaded = false;
    let flightState = 'INIT';
    let statusRefreshTimer = null;

    const isControlsDisabled = () => isInitialLoading || isLocked || !(flightState === 'ARMED' || flightState === 'LANDING');

    function showLoader(message = 'Loading...') {
      loadingOverlay.style.display = 'flex';
      loadingMessage.innerText = message;
    }

    function hideLoader() {
      loadingOverlay.style.display = 'none';
    }

    // Function to update controls state
    function updateControlsState() {
      const disabled = isControlsDisabled();

      // Update throttle control
      masterThrottle.disabled = disabled;
      decreaseThrottleBtn.disabled = disabled;
      increaseThrottleBtn.disabled = disabled;
      throttleSetButtons.forEach(button => {
        button.disabled = disabled;
      });

      // Update stick controls
      Object.values(sticks).forEach(stick => {
        stick.handle.style.pointerEvents = disabled ? 'none' : 'auto';
        stick.handle.style.opacity = disabled ? '0.5' : '1';
      });

      // Update PID controls
      Object.values(pidInputs).forEach(input => {
        input.disabled = disabled;
        input.style.opacity = disabled ? '0.6' : '1';
      });

      savePIDBtn.disabled = disabled || !pidLoaded;
      savePIDBtn.style.opacity = (disabled || !pidLoaded) ? '0.5' : '1';

      resetPIDBtn.disabled = disabled || !pidLoaded;
      resetPIDBtn.style.opacity = (disabled || !pidLoaded) ? '0.5' : '1';

      resetFlightBtn.disabled = disabled;
      resetFlightBtn.style.opacity = disabled ? '0.5' : '1';

      restartBtn.disabled = isInitialLoading;
      restartBtn.style.opacity = isInitialLoading ? '0.5' : '1';

      calibrateMPUButton.disabled = true;
      calibrateESCButton.disabled = disabled;

      testModeToggle.disabled = disabled;
      motorToggleButtons.forEach(button => {
        button.disabled = disabled;
        button.classList.toggle('active', motorTestState[button.dataset.motor]);
      });

      armButton.disabled = isLocked || flightState === 'ARMED' || flightState === 'LANDING';
      disarmButton.disabled = isLocked || flightState === 'DISARMED';
      landButton.disabled = isLocked || flightState === 'LANDING' || flightState === 'DISARMED' || flightState === 'CALIBRATING';

    }

    // Function to update throttle value
    function updateThrottle(value) {
      if (isControlsDisabled()) return;
      masterThrottle.value = value;
      throttleValue.innerText = value;
      const throttleEndpoint = isTestMode ? '/setTestThrottle' : '/setThrottle';
      fetch(`${throttleEndpoint}?value=${value}`);
    }

    // Throttle Control
    masterThrottle.oninput = function () {
      if (isControlsDisabled()) return;
      updateThrottle(this.value);
    }

    // Throttle Buttons
    decreaseThrottleBtn.onclick = function () {
      if (isControlsDisabled()) return;
      const currentValue = parseInt(masterThrottle.value);
      if (currentValue > 0) {
        updateThrottle(currentValue - 1);
      }
    }

    increaseThrottleBtn.onclick = function () {
      if (isControlsDisabled()) return;
      const currentValue = parseInt(masterThrottle.value);
      if (currentValue < 100) {
        updateThrottle(currentValue + 1);
      }
    }

    throttleSetButtons.forEach(button => {
      button.addEventListener('click', () => {
        if (isControlsDisabled()) return;
        const targetValue = parseInt(button.dataset.throttle, 10);
        if (!Number.isNaN(targetValue)) {
          updateThrottle(targetValue);
        }
      });
    });

    // Flight Lock Toggle
    flightLockToggle.onchange = function () {
      isLocked = this.checked;
      lockValue.innerText = isLocked ? "ON" : "OFF";
      updateControlsState();
    }

    function setTestMode(enabled) {
      if (!enabled) {
        const throttleEndpoint = isTestMode ? '/setTestThrottle' : '/setThrottle';
        masterThrottle.value = 0;
        throttleValue.innerText = 0;
        fetch(`${throttleEndpoint}?value=0`);
      }

      if (isControlsDisabled()) return;
      isTestMode = enabled;
      testModeToggle.checked = enabled;
      testModeValue.innerText = enabled ? 'ON' : 'OFF';
      motorToggleButtons.forEach(btn => {
        btn.disabled = isControlsDisabled();
        btn.classList.toggle('active', motorTestState[btn.dataset.motor]);
      });

      fetch(`/setTestMode?enabled=${enabled ? 1 : 0}`, { method: 'POST' })
        .catch(() => { testModeValue.innerText = enabled ? 'ON*' : 'OFF*'; });
    }

    function sendMotorToggle(motorKey, enabled) {
      if (isControlsDisabled()) return;
      motorTestState[motorKey] = enabled;
      motorToggleButtons.forEach(btn => {
        if (btn.dataset.motor === motorKey) {
          btn.classList.toggle('active', enabled);
        }
      });
      fetch(`/setTestMotor?motor=${motorKey}&enabled=${enabled ? 1 : 0}`, { method: 'POST' });
    }

    testModeToggle.onchange = function () {
      setTestMode(this.checked);
    };

    motorToggleButtons.forEach(button => {
      button.onclick = function () {
        if (isControlsDisabled()) return;
        const motorKey = this.dataset.motor;
        const nextState = !motorTestState[motorKey];
        sendMotorToggle(motorKey, nextState);
        this.classList.toggle('active', nextState);
      };
    });

    function setFlightState(newState) {
      flightState = newState;
      flightStateDisplay.innerText = newState;
      switch (newState) {
        case 'ARMED':
          flightStatusNote.innerText = 'Controls enabled. Fly safe!';
          break;
        case 'LANDING':
          flightStatusNote.innerText = 'Landing sequence in progress.';
          break;
        case 'DISARMED':
          flightStatusNote.innerText = 'Request arming to enable controls.';
          break;
        case 'CALIBRATING':
          flightStatusNote.innerText = 'Calibration in progress...';
          break;
        case 'FAILSAFE':
          flightStatusNote.innerText = 'Failsafe active. Disarm before rearming.';
          break;
        default:
          flightStatusNote.innerText = 'Idle.';
      }
      updateControlsState();
    }

    function refreshStatus(options = {}) {
      const { isInitial = false } = options;

      if (isInitial) {
        showLoader('Loading flight status...');
      }

      return fetch('/getStatus')
        .then(res => res.ok ? res.json() : Promise.reject())
        .then(data => {
          setFlightState(data.state);
          throttleValue.innerText = data.throttle;
          masterThrottle.value = data.throttle;
          if (isInitial) {
            loadingMessage.innerText = 'Flight status loaded.';
          }
          if (typeof data.testMode !== 'undefined') {
            isTestMode = data.testMode;
            testModeToggle.checked = isTestMode;
            testModeValue.innerText = isTestMode ? 'ON' : 'OFF';
          }
          if (data.motorMask !== undefined) {
            const mask = data.motorMask;
            motorTestState.fl = !!(mask & 0x1);
            motorTestState.fr = !!(mask & 0x2);
            motorTestState.bl = !!(mask & 0x4);
            motorTestState.br = !!(mask & 0x8);
            motorToggleButtons.forEach(btn => {
              btn.classList.toggle('active', motorTestState[btn.dataset.motor]);
            });
          }
        })
        .catch(() => {
          flightStatusNote.innerText = 'Unable to read flight status.';
          if (isInitial) {
            loadingMessage.innerText = 'Failed to load flight status.';
          }
        });
    }

    function postCommand(url, successMessage) {
      if (isLocked) return;
      flightStatusNote.innerText = 'Request sent...';
      fetch(url, { method: 'POST' })
        .then(res => res.ok ? res.text() : Promise.reject(res.text()))
        .then(msg => {
          flightStatusNote.innerText = msg || successMessage;
          refreshStatus();
        })
        .catch(() => {
          flightStatusNote.innerText = 'Command failed.';
        });
    }

    armButton.onclick = function () {
      postCommand('/arm', 'Arming...');
    };

    disarmButton.onclick = function () {
      postCommand('/disarm', 'Disarmed.');
    };

    landButton.onclick = function () {
      postCommand('/land', 'Landing initiated.');
    };

    // Stick Control Implementation
    Object.entries(sticks).forEach(([key, stick]) => {
      let isDragging = false;
      let startX, startY;
      let currentX = 0, currentY = 0;

      const updateStickPosition = (x, y) => {
        if (isControlsDisabled()) return;

        const rect = stick.element.getBoundingClientRect();
        const centerX = rect.width / 2;
        const centerY = rect.height / 2;

        // Calculate distance from center
        const distance = Math.sqrt(x * x + y * y);
        const maxDistance = centerX;

        // Normalize and constrain
        if (distance > maxDistance) {
          x = (x / distance) * maxDistance;
          y = (y / distance) * maxDistance;
        }

        // Update stick position
        stick.handle.style.transform = `translate(calc(-50% + ${x}px), calc(-50% + ${y}px))`;

        // Calculate and update value
        const normalizedX = (x / maxDistance) * stick.range;
        const normalizedY = (y / maxDistance) * stick.range;

        // Update display and send to server
        const value = key === 'yaw' ? normalizedX : normalizedY;
        stick.value.innerText = value.toFixed(1);

        fetch(`/set${key.charAt(0).toUpperCase() + key.slice(1)}?value=${value}`);
      };

      stick.handle.addEventListener('mousedown', (e) => {
        if (isControlsDisabled()) return;
        isDragging = true;
        const rect = stick.element.getBoundingClientRect();
        startX = e.clientX - rect.left - rect.width / 2;
        startY = e.clientY - rect.top - rect.height / 2;
        currentX = startX;
        currentY = startY;
        updateStickPosition(currentX, currentY);
      });

      document.addEventListener('mousemove', (e) => {
        if (!isDragging || isControlsDisabled()) return;
        const rect = stick.element.getBoundingClientRect();
        currentX = e.clientX - rect.left - rect.width / 2;
        currentY = e.clientY - rect.top - rect.height / 2;
        updateStickPosition(currentX, currentY);
      });

      document.addEventListener('mouseup', () => {
        if (!isDragging || isControlsDisabled()) return;
        isDragging = false;
        currentX = 0;
        currentY = 0;
        updateStickPosition(0, 0);
      });

      // Touch events for mobile
      stick.handle.addEventListener('touchstart', (e) => {
        if (isControlsDisabled()) return;
        e.preventDefault();
        isDragging = true;
        const rect = stick.element.getBoundingClientRect();
        startX = e.touches[0].clientX - rect.left - rect.width / 2;
        startY = e.touches[0].clientY - rect.top - rect.height / 2;
        currentX = startX;
        currentY = startY;
        updateStickPosition(currentX, currentY);
      });

      document.addEventListener('touchmove', (e) => {
        if (!isDragging || isControlsDisabled()) return;
        e.preventDefault();
        const rect = stick.element.getBoundingClientRect();
        currentX = e.touches[0].clientX - rect.left - rect.width / 2;
        currentY = e.touches[0].clientY - rect.top - rect.height / 2;
        updateStickPosition(currentX, currentY);
      });

      document.addEventListener('touchend', () => {
        if (!isDragging || isLocked) return;
        isDragging = false;
        currentX = 0;
        currentY = 0;
        updateStickPosition(0, 0);
      });
    });

    const zeroPidValues = {
      kpRoll: 0,
      kiRoll: 0,
      kdRoll: 0,
      kpPitch: 0,
      kiPitch: 0,
      kdPitch: 0,
      kpYaw: 0,
      kiYaw: 0,
      kdYaw: 0
    };

    let loadedPidValues = { ...zeroPidValues };

    function showPidStatus(message, isError = false) {
      pidStatus.textContent = message;
      pidStatus.style.color = isError ? '#ff9f9f' : '#cccccc';
    }

    function setPidFormValues(values) {
      Object.entries(values).forEach(([key, value]) => {
        if (pidInputs[key]) {
          pidInputs[key].value = Number(value).toFixed(3);
        }
      });
    }

    function collectPidValues() {
      const values = {};
      for (const [key, input] of Object.entries(pidInputs)) {
        const parsed = parseFloat(input.value);
        if (Number.isNaN(parsed)) {
          showPidStatus(`Invalid number for ${key}`, true);
          return null;
        }
        values[key] = parsed;
      }
      return values;
    }

    function fetchCurrentPid(options = {}) {
      const { isInitial = false } = options;
      pidLoaded = false;
      updateControlsState();

      if (isInitial) {
        showLoader('Loading PID values...');
      }

      showPidStatus('Loading current PID values...');
      return fetch('/getPID')
        .then(res => res.ok ? res.json() : Promise.reject(new Error('Unable to read PID values')))
        .then(data => {
          loadedPidValues = { ...data };
          setPidFormValues(loadedPidValues);
          pidLoaded = true;
          updateControlsState();
          showPidStatus('PID values loaded.');
          if (isInitial) {
            loadingMessage.innerText = 'PID values loaded.';
          }
        })
        .catch(() => {
          loadedPidValues = { ...zeroPidValues };
          setPidFormValues(loadedPidValues);
          pidLoaded = true;
          updateControlsState();
          showPidStatus('PID values unavailable; using zeros.', true);
          if (isInitial) {
            loadingMessage.innerText = 'Failed to load PID values.';
          }
        });
    }

    function sendPidUpdate(values) {
      const body = new URLSearchParams();
      Object.entries(values).forEach(([key, value]) => body.append(key, value));

      showPidStatus('Saving PID values...');
      fetch('/setPID', {
        method: 'POST',
        headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
        body: body.toString()
      })
        .then(res => {
          if (!res.ok) {
            throw new Error('Failed to save PID values');
          }
          showPidStatus('PID values saved successfully.');
        })
        .catch(err => {
          showPidStatus(err.message, true);
        });
    }

    savePIDBtn.onclick = function () {
      if (isLocked || !pidLoaded) return;
      const values = collectPidValues();
      if (values) {
        sendPidUpdate(values);
      }
    };

    resetPIDBtn.onclick = function () {
      if (isLocked || !pidLoaded) return;
      setPidFormValues(loadedPidValues);
      sendPidUpdate(loadedPidValues);
    };

    resetFlightBtn.onclick = function () {
      if (isLocked) return;
      fetch("/resetFlight");
    };

    restartBtn.onclick = function () {
      if (isLocked) return;
      flightStatusNote.innerText = 'Restarting...';
      fetch("/restart", { method: 'POST' });
    };

    function runCalibration(button, statusElement, url, label) {
      if (isLocked) return;
      button.disabled = true;
      statusElement.textContent = `${label} in progress...`;

      fetch(url, { method: 'POST' })
        .then(res => res.ok ? res.text() : Promise.reject(new Error(`${label} failed`)))
        .then(message => {
          statusElement.textContent = message || `${label} complete.`;
        })
        .catch(err => {
          statusElement.textContent = err.message;
        })
        .finally(() => {
          button.disabled = isLocked;
        });
    }

    calibrateESCButton.onclick = function () {
      runCalibration(calibrateESCButton, escStatus, '/calibrateESC', 'ESC calibration');
    };

    const hookCall = () => {
      setInterval(() => {
        if (parseInt(throttleValue.innerText, 10) > 0) {
          fetch('/getLogs')
            .then(res => res.text())
            .then(data => {
              const p = document.createElement("p");
              p.innerText = data;
              p.style.fontSize = "12px";
              p.style.color = "#eee";
              const logContainer = document.getElementById("logContainer");
              logContainer.appendChild(p);
              logContainer.scrollTop = logContainer.scrollHeight;
            })
            .catch(err => {
              const p = document.createElement("p");
              p.innerText = `ERROR: ${err.message}`;
              p.style.fontSize = "12px";
              p.style.color = "red";
              const logContainer = document.getElementById("logContainer");
              logContainer.appendChild(p);
              logContainer.scrollTop = logContainer.scrollHeight;
            });
        }
      }, 500);
    };

    // hookCall();


    // Initial state
    setPidFormValues(loadedPidValues);
    function startStatusRefresh() {
      if (statusRefreshTimer) {
        clearInterval(statusRefreshTimer);
      }
      statusRefreshTimer = setInterval(() => refreshStatus(), 1000);
    }

    async function runInitialLoad() {
      showLoader('Preparing interface...');
      updateControlsState();

      try {
        await delay(1000);
        await refreshStatus({ isInitial: true });
        await delay(1000);
        await fetchCurrentPid({ isInitial: true });
      } finally {
        isInitialLoading = false;
        hideLoader();
        updateControlsState();
        startStatusRefresh();
      }
    }

    runInitialLoad();
  </script>
</body>

</html>
)rawliteral";

#endif // ROOT_UI_H
