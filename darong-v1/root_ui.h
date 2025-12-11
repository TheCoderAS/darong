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

    /* PID Control Styles */
    .pid-control {
      display: flex;
      flex-direction: column;
      align-items: center;
      margin: 10px 0;
      width: 100%;
    }

    .pid-control label {
      font-size: 0.9em;
      color: #ffffff;
      margin-bottom: 5px;
      font-weight: 500;
    }

    .pid-control-row {
      display: flex;
      align-items: center;
      justify-content: center;
      gap: 10px;
      width: 100%;
    }

    .pid-value-display {
      background: rgba(0, 0, 0, 0.5);
      padding: 8px 0px;
      border-radius: 8px;
      min-width: 60px;
      text-align: center;
      border: 1px solid rgba(255, 255, 255, 0.1);
      font-weight: 600;
      font-size: 0.875rem;
    }

    .pid-btn {
      width: 35px;
      height: 35px;
      border-radius: 50%;
      border: none;
      background: rgba(255, 255, 255, 0.1);
      color: #ffffff;
      font-size: 18px;
      font-weight: 600;
      cursor: pointer;
      transition: all 0.3s ease;
      display: flex;
      align-items: center;
      justify-content: center;
      border: 1px solid rgba(255, 255, 255, 0.2);
    }

    .pid-btn:hover {
      background: rgba(255, 255, 255, 0.2);
      transform: scale(1.1);
    }

    .pid-btn:active {
      transform: scale(0.95);
    }

    .pid-btn:disabled {
      opacity: 0.5;
      cursor: not-allowed;
      transform: none;
    }

    .reset-btn {
      width: 100%;
      padding: 12px;
      background: rgba(255, 255, 255, 0.1);
      color: #ffffff;
      border: 1px solid rgba(255, 255, 255, 0.2);
      border-radius: 8px;
      cursor: pointer;
      font-size: 1em;
      transition: all 0.3s ease;
      margin-top: 10px;
    }

    .reset-btn:hover {
      background: rgba(255, 255, 255, 0.2);
      transform: translateY(-2px);
    }

    .reset-btn:active {
      transform: translateY(0);
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

  <!-- PID Control Section -->
  <h2 style="margin-top: 20px;">PID Tuning</h2>
  <div class="container">
    <!-- Roll PID Controls -->
    <div class="control-section">
      <div class="section-title">Roll PID</div>

      <div class="pid-control">
        <label>Kp</label>
        <div class="pid-control-row">
          <button class="pid-btn" id="kpRollMinus">-</button>
          <div class="pid-value-display" id="kpRollValue">2.400</div>
          <button class="pid-btn" id="kpRollPlus">+</button>
        </div>
      </div>

      <div class="pid-control">
        <label>Ki</label>
        <div class="pid-control-row">
          <button class="pid-btn" id="kiRollMinus">-</button>
          <div class="pid-value-display" id="kiRollValue">1.900</div>
          <button class="pid-btn" id="kiRollPlus">+</button>
        </div>
      </div>

      <div class="pid-control">
        <label>Kd</label>
        <div class="pid-control-row">
          <button class="pid-btn" id="kdRollMinus">-</button>
          <div class="pid-value-display" id="kdRollValue">0.110</div>
          <button class="pid-btn" id="kdRollPlus">+</button>
        </div>
      </div>
    </div>

    <!-- Pitch PID Controls -->
    <div class="control-section">
      <div class="section-title">Pitch PID</div>

      <div class="pid-control">
        <label>Kp</label>
        <div class="pid-control-row">
          <button class="pid-btn" id="kpPitchMinus">-</button>
          <div class="pid-value-display" id="kpPitchValue">2.400</div>
          <button class="pid-btn" id="kpPitchPlus">+</button>
        </div>
      </div>

      <div class="pid-control">
        <label>Ki</label>
        <div class="pid-control-row">
          <button class="pid-btn" id="kiPitchMinus">-</button>
          <div class="pid-value-display" id="kiPitchValue">1.900</div>
          <button class="pid-btn" id="kiPitchPlus">+</button>
        </div>
      </div>

      <div class="pid-control">
        <label>Kd</label>
        <div class="pid-control-row">
          <button class="pid-btn" id="kdPitchMinus">-</button>
          <div class="pid-value-display" id="kdPitchValue">0.110</div>
          <button class="pid-btn" id="kdPitchPlus">+</button>
        </div>
      </div>
    </div>
    <!-- Yaw PID Controls -->
    <div class="control-section">
      <div class="section-title">Yaw PID</div>

      <div class="pid-control">
        <label>Kp</label>
        <div class="pid-control-row">
          <button class="pid-btn" id="kpYawMinus">-</button>
          <div class="pid-value-display" id="kpYawValue">0.000</div>
          <button class="pid-btn" id="kpYawPlus">+</button>
        </div>
      </div>

      <div class="pid-control">
        <label>Ki</label>
        <div class="pid-control-row">
          <button class="pid-btn" id="kiYawMinus">-</button>
          <div class="pid-value-display" id="kiYawValue">0.000</div>
          <button class="pid-btn" id="kiYawPlus">+</button>
        </div>
      </div>

      <div class="pid-control">
        <label>Kd</label>
        <div class="pid-control-row">
          <button class="pid-btn" id="kdYawMinus">-</button>
          <div class="pid-value-display" id="kdYawValue">0.000</div>
          <button class="pid-btn" id="kdYawPlus">+</button>
        </div>
      </div>
    </div>
  </div>

  <!-- Reset Button -->
  <div class="container" style="margin-top: 20px;">
    <button id="resetPIDBtn" class="reset-btn">Reset PID</button>
    <button id="resetFlightBtn" class="reset-btn">Reset Flight</button>
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

    // Initialize PID control elements
    const pidControls = {
      kpRoll: {
        minus: document.getElementById("kpRollMinus"),
        plus: document.getElementById("kpRollPlus"),
        value: document.getElementById("kpRollValue"),
        currentValue: 2.400,
        min: 0.0,
        max: 10.0,
        step: 0.1
      },
      kiRoll: {
        minus: document.getElementById("kiRollMinus"),
        plus: document.getElementById("kiRollPlus"),
        value: document.getElementById("kiRollValue"),
        currentValue: 1.900,
        min: 0.0,
        max: 10.0,
        step: 0.05
      },
      kdRoll: {
        minus: document.getElementById("kdRollMinus"),
        plus: document.getElementById("kdRollPlus"),
        value: document.getElementById("kdRollValue"),
        currentValue: 0.110,
        min: 0.0,
        max: 2.0,
        step: 0.005
      },
      kpPitch: {
        minus: document.getElementById("kpPitchMinus"),
        plus: document.getElementById("kpPitchPlus"),
        value: document.getElementById("kpPitchValue"),
        currentValue: 2.400,
        min: 0.0,
        max: 10.0,
        step: 0.1
      },
      kiPitch: {
        minus: document.getElementById("kiPitchMinus"),
        plus: document.getElementById("kiPitchPlus"),
        value: document.getElementById("kiPitchValue"),
        currentValue: 1.900,
        min: 0.0,
        max: 10.0,
        step: 0.05
      },
      kdPitch: {
        minus: document.getElementById("kdPitchMinus"),
        plus: document.getElementById("kdPitchPlus"),
        value: document.getElementById("kdPitchValue"),
        currentValue: 0.110,
        min: 0.0,
        max: 2.0,
        step: 0.005
      },
      kpYaw: {
        minus: document.getElementById("kpYawMinus"),
        plus: document.getElementById("kpYawPlus"),
        value: document.getElementById("kpYawValue"),
        currentValue: 0.0,
        min: 0.0,
        max: 10.0,
        step: 0.1
      },
      kiYaw: {
        minus: document.getElementById("kiYawMinus"),
        plus: document.getElementById("kiYawPlus"),
        value: document.getElementById("kiYawValue"),
        currentValue: 0.0,
        min: 0.0,
        max: 10.0,
        step: 0.05
      },
      kdYaw: {
        minus: document.getElementById("kdYawMinus"),
        plus: document.getElementById("kdYawPlus"),
        value: document.getElementById("kdYawValue"),
        currentValue: 0.0,
        min: 0.0,
        max: 2.0,
        step: 0.005
      }
    };

    const resetPIDBtn = document.getElementById("resetPIDBtn");
    const resetFlightBtn = document.getElementById("resetFlightBtn");

    // Initialize stick controls
    const sticks = {
      roll: { element: document.getElementById("rollStick"), handle: document.getElementById("rollStickHandle"), value: document.getElementById("rollValue"), range: 30 },
      pitch: { element: document.getElementById("pitchStick"), handle: document.getElementById("pitchStickHandle"), value: document.getElementById("pitchValue"), range: 30 },
      yaw: { element: document.getElementById("yawStick"), handle: document.getElementById("yawStickHandle"), value: document.getElementById("yawValue"), range: 200 }
    };

    let isLocked = false;

    // Function to update controls state
    function updateControlsState() {
      const disabled = isLocked;

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
      Object.values(pidControls).forEach(control => {
        control.minus.disabled = disabled;
        control.plus.disabled = disabled;
        control.minus.style.opacity = disabled ? '0.5' : '1';
        control.plus.style.opacity = disabled ? '0.5' : '1';
      });

      resetPIDBtn.disabled = disabled;
      resetPIDBtn.style.opacity = disabled ? '0.5' : '1';

      resetFlightBtn.disabled = disabled;
      resetFlightBtn.style.opacity = disabled ? '0.5' : '1';

    }

    // Function to update throttle value
    function updateThrottle(value) {
      if (isLocked) return;
      masterThrottle.value = value;
      throttleValue.innerText = value;
      fetch("/setThrottle?value=" + value);
    }

    // Throttle Control
    masterThrottle.oninput = function () {
      if (isLocked) return;
      updateThrottle(this.value);
    }

    // Throttle Buttons
    decreaseThrottleBtn.onclick = function () {
      if (isLocked) return;
      const currentValue = parseInt(masterThrottle.value);
      if (currentValue > 0) {
        updateThrottle(currentValue - 1);
      }
    }

    increaseThrottleBtn.onclick = function () {
      if (isLocked) return;
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

    // Stick Control Implementation
    Object.entries(sticks).forEach(([key, stick]) => {
      let isDragging = false;
      let startX, startY;
      let currentX = 0, currentY = 0;

      const updateStickPosition = (x, y) => {
        if (isLocked) return;

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
        if (isLocked) return;
        isDragging = true;
        const rect = stick.element.getBoundingClientRect();
        startX = e.clientX - rect.left - rect.width / 2;
        startY = e.clientY - rect.top - rect.height / 2;
        currentX = startX;
        currentY = startY;
        updateStickPosition(currentX, currentY);
      });

      document.addEventListener('mousemove', (e) => {
        if (!isDragging || isLocked) return;
        const rect = stick.element.getBoundingClientRect();
        currentX = e.clientX - rect.left - rect.width / 2;
        currentY = e.clientY - rect.top - rect.height / 2;
        updateStickPosition(currentX, currentY);
      });

      document.addEventListener('mouseup', () => {
        if (!isDragging || isLocked) return;
        isDragging = false;
        currentX = 0;
        currentY = 0;
        updateStickPosition(0, 0);
      });

      // Touch events for mobile
      stick.handle.addEventListener('touchstart', (e) => {
        if (isLocked) return;
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
        if (!isDragging || isLocked) return;
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

    // PID Control Functions
    function updatePIDValue(controlName, newValue) {
      // console.log(controlName)
      if (isLocked) return;

      const control = pidControls[controlName];

      // Constrain value to min/max
      newValue = Math.max(control.min, Math.min(control.max, newValue));

      // Update current value
      control.currentValue = newValue;

      // Update display
      control.value.innerText = newValue.toFixed(3);

      // Send to server
      fetch(`/set${controlName.charAt(0).toUpperCase() + controlName.slice(1)}?value=${newValue}`);
    }

    function incrementPIDValue(controlName) {
      const control = pidControls[controlName];
      const newValue = control.currentValue + control.step;
      updatePIDValue(controlName, newValue, false);
    }

    function decrementPIDValue(controlName) {
      const control = pidControls[controlName];
      const newValue = control.currentValue - control.step;
      updatePIDValue(controlName, newValue, false);
    }

    // PID Control Event Listeners
    Object.entries(pidControls).forEach(([controlName, control]) => {
      control.minus.onclick = function () {
        decrementPIDValue(controlName);
      };

      control.plus.onclick = function () {
        incrementPIDValue(controlName);
      };
    });

    // Reset PID Values
    const defaultsPIDValues = {
      kpRoll: 2.6,
      kiRoll: 2.0,
      kdRoll: 0.115,
      kpPitch: 2.6,
      kiPitch: 1.950,
      kdPitch: 0.115,
      kpYaw: 0.0,
      kiYaw: 0.0,
      kdYaw: 0.0
    };
    resetPIDBtn.onclick = function () {
      if (isLocked) return;
      resetPid();
    };

    const resetPid = () => {
      Object.entries(defaultsPIDValues).forEach(([controlName, defaultValue]) => {
        updatePIDValue(controlName, defaultValue, true);
      });
    }
    resetPid();
    resetFlightBtn.onclick = function () {
      if (isLocked) return;
      fetch("/resetFlight");
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
    updateControlsState();
  </script>
</body>

</html>
)rawliteral";

#endif // ROOT_UI_H
