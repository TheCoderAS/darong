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

    body {
      font-family: 'Segoe UI', Arial, sans-serif;
      min-height: 100vh;
      display: flex;
      flex-direction: column;
      align-items: center;
      justify-content: center;
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
      gap: 20px;
      padding: 20px;
      background: rgba(0, 0, 0, 0.7);
      border-radius: 20px;
      box-shadow: 0 8px 32px rgba(0, 0, 0, 0.5);
      width: 95%;
      max-width: 1200px;
      border: 1px solid rgba(255, 255, 255, 0.1);
    }

    .control-section {
      display: flex;
      flex-direction: column;
      align-items: center;
      padding: 15px;
      background: rgba(0, 0, 0, 0.5);
      border-radius: 15px;
      min-width: 180px;
      border: 1px solid rgba(255, 255, 255, 0.1);
      transition: transform 0.3s ease, box-shadow 0.3s ease;
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
      flex-direction: column;
      align-items: center;
      gap: 10px;
      padding: 10px;
      background: rgba(0, 0, 0, 0.4);
      border-radius: 10px;
      border: 1px solid rgba(255, 255, 255, 0.15);
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
      text-align: center;
    }

    .status-container {
      justify-content: center;
    }

    .status-section {
      min-width: 240px;
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
  <h2>Quadcopter Control Panel</h2>
  <div class="container">
    <!-- Throttle Control -->
    <div class="control-section">
      <div class="section-title">Throttle</div>
      <div class="throttle-control">
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

  <h2 style="margin-top: 20px;">Flight Status</h2>
  <div class="container status-container">
    <div class="control-section status-section">
      <div class="section-title">Flight State</div>
      <div class="state-display">
        <div class="state-chip" id="flightState">INIT</div>
        <div class="flight-actions">
          <button class="flight-action-btn" id="armButton">Arm</button>
          <button class="flight-action-btn" id="disarmButton">Disarm</button>
          <button class="flight-action-btn" id="landButton">Land</button>
        </div>
        <div class="status-note" id="flightStatusNote">Request arming to enable controls.</div>
      </div>
    </div>
  </div>

  <h2 style="margin-top: 20px;">Calibrations</h2>
  <div class="container">
    <div class="control-section">
      <div class="section-title">MPU6050</div>
      <button id="calibrateMPUButton" class="pid-button">Calibrate Sensors</button>
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
    </div>
    <div id="pidStatus" class="pid-status"></div>
  </div>
  <div id="logContainer" style="margin-top: 20px; height: 200px; overflow: auto;">
  </div>
  <script>
    // Initialize elements
    const masterThrottle = document.getElementById("masterThrottle");
    const throttleValue = document.getElementById("throttleValue");
    const flightLockToggle = document.getElementById("flightLockToggle");
    const lockValue = document.getElementById("lockValue");
    const decreaseThrottleBtn = document.getElementById("decreaseThrottle");
    const increaseThrottleBtn = document.getElementById("increaseThrottle");
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
    const pidStatus = document.getElementById("pidStatus");
    const calibrateMPUButton = document.getElementById("calibrateMPUButton");
    const calibrateESCButton = document.getElementById("calibrateESCButton");
    const mpuStatus = document.getElementById("mpuStatus");
    const escStatus = document.getElementById("escStatus");

    // Initialize stick controls
    const sticks = {
      roll: { element: document.getElementById("rollStick"), handle: document.getElementById("rollStickHandle"), value: document.getElementById("rollValue"), range: 30 },
      pitch: { element: document.getElementById("pitchStick"), handle: document.getElementById("pitchStickHandle"), value: document.getElementById("pitchValue"), range: 30 },
      yaw: { element: document.getElementById("yawStick"), handle: document.getElementById("yawStickHandle"), value: document.getElementById("yawValue"), range: 200 }
    };

    let isLocked = false;
    let pidLoaded = false;
    let flightState = 'INIT';

    const isControlsDisabled = () => isLocked || !(flightState === 'ARMED' || flightState === 'LANDING');

    // Function to update controls state
    function updateControlsState() {
      const disabled = isControlsDisabled();

      // Update throttle control
      masterThrottle.disabled = disabled;
      decreaseThrottleBtn.disabled = disabled;
      increaseThrottleBtn.disabled = disabled;

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

      calibrateMPUButton.disabled = disabled;
      calibrateESCButton.disabled = disabled;

      armButton.disabled = isLocked || flightState === 'ARMED' || flightState === 'LANDING';
      disarmButton.disabled = isLocked || flightState === 'DISARMED';
      landButton.disabled = isLocked || flightState === 'LANDING' || flightState === 'DISARMED' || flightState === 'CALIBRATING';

    }

    // Function to update throttle value
    function updateThrottle(value) {
      if (isControlsDisabled()) return;
      masterThrottle.value = value;
      throttleValue.innerText = value;
      fetch("/setThrottle?value=" + value);
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

    // Flight Lock Toggle
    flightLockToggle.onchange = function () {
      isLocked = this.checked;
      lockValue.innerText = isLocked ? "ON" : "OFF";
      updateControlsState();
    }

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

    function refreshStatus() {
      fetch('/getStatus')
        .then(res => res.ok ? res.json() : Promise.reject())
        .then(data => {
          setFlightState(data.state);
          throttleValue.innerText = data.throttle;
          masterThrottle.value = data.throttle;
        })
        .catch(() => {
          flightStatusNote.innerText = 'Unable to read flight status.';
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

    function fetchCurrentPid() {
      pidLoaded = false;
      updateControlsState();
      showPidStatus('Loading current PID values...');
      fetch('/getPID')
        .then(res => res.ok ? res.json() : Promise.reject(new Error('Unable to read PID values')))
        .then(data => {
          loadedPidValues = { ...data };
          setPidFormValues(loadedPidValues);
          pidLoaded = true;
          updateControlsState();
          showPidStatus('PID values loaded.');
        })
        .catch(() => {
          loadedPidValues = { ...zeroPidValues };
          setPidFormValues(loadedPidValues);
          pidLoaded = true;
          updateControlsState();
          showPidStatus('PID values unavailable; using zeros.', true);
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

    calibrateMPUButton.onclick = function () {
      runCalibration(calibrateMPUButton, mpuStatus, '/calibrateMPU', 'MPU calibration');
    };

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
    fetchCurrentPid();
    refreshStatus();
    setInterval(refreshStatus, 1000);
    updateControlsState();
  </script>
</body>

</html>
)rawliteral";

#endif // ROOT_UI_H
