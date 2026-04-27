# ESP32 Smart Energy Monitor & Relay Switch
**By [Idlan Zafran Mohd Zaidie](https://github.com/IdlanZafran)**

[![Follow](https://img.shields.io/github/followers/IdlanZafran?label=Follow%20Me&style=social)](https://github.com/IdlanZafran)

[![Version](https://img.shields.io/badge/version-1.0.0-blue.svg)](https://github.com/IdlanZafran/esp32-smart-energy-monitor-relay)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

_If you like this project, don’t forget to:_
- _⭐ Star this repository_
- _👤 Follow my GitHub for future updates_

## 📌 Overview

A professional IoT energy monitoring and smart relay system built using ESP32, featuring a real-time web dashboard, OLED status display, and remote relay control. This project monitors Voltage (V), Current (mA), and Power (W) while providing a live bill estimator tailored for Malaysian (TNB) tariffs.

## 🚀 Features

* **Real-time Monitoring:** High-accuracy RMS calculation for Voltage (ZMPT101B) and Current (ACS712).
* **Web Dashboard:** Modern, responsive UI with live data updates via JSON API.
* **OLED Interface:** 128x32 display showing IP address, NTP time, relay status, and live metrics.
* **Smart Recovery:** 
    * **Auto-Reconnect:** Automatically attempts to restore WiFi connection if dropped.
    * **Rapid-Boot Reset:** 3 quick physical power cycles/resets will wipe WiFi credentials (via WiFiManager).
* **Bill Estimator:** Integrated RM (Ringgit Malaysia) cost estimation based on live load.
* **Interactive UI:** Custom robot face animations during the boot sequence.

## 🛠️ Hardware Requirements

* **Microcontroller:** ESP32 (DevKit V1)
* **Voltage Sensor:** ZMPT101B
* **Current Sensor:** ACS712 (5A/20A/30A)
* **Display:** SSD1306 OLED (128x32 I2C)
* **Switching:** 5V Relay Module (Active High)
* **FileSystem:** LittleFS (used for boot counting/state management)


## 🔧 Installation & Setup

### Option A: PlatformIO (Recommended)

1. Open the project folder in VS Code with the PlatformIO extension.
2. PlatformIO will automatically download the required libraries.
3. Connect your ESP32 and click **Upload**.

---

### Option B: Arduino IDE

1. Ensure the ESP32 Board core is installed in **Boards Manager**.
2. Install the following libraries via **Library Manager (Ctrl + Shift + I)**:
   - Adafruit SSD1306  
   - Adafruit GFX Library  
   - WiFiManager (by tzapu)

3. Open:
```

ESP32_Smart_Energy_Monitor_Relay/ESP32_Smart_Energy_Monitor_Relay.ino

````

4. Select your board (e.g., **DOIT ESP32 DEVKIT V1**)  
5. Click **Upload**

---

## ⚙️ Calibration

### Voltage Calibration
- Measure your AC mains voltage using a multimeter.
- Adjust:
```cpp
VOLTAGE_CALIBRATION
````

until the OLED reading matches your measurement.

---

### Current Sensor Calibration (ACS712)

Update `mV_PER_AMP` based on your sensor model:

* **5A module:** `185.0`
* **20A module:** `100.0`
* **30A module:** `66.0`

---

### Electricity Tariff

* Open `localweb.h`
* Update the value:

  ```javascript
  0.218
  ```

  inside the `estimatedCost` formula to match your local utility rate.

---

## 🖥️ Usage

### First Boot

* OLED displays WiFi setup prompt.
* Connect to the access point:

  ```
  Smart Plug
  ```

---

### WiFi Setup

1. Connect using your phone or laptop.
2. Select your home WiFi network.
3. Enter the password.

---

### Dashboard Access

* After connection, the OLED shows an IP address.
* Enter this IP into any browser on the same network.
* View real-time power monitoring dashboard.

---

### Relay Control

* Use the web dashboard button to toggle power to your connected load.

---

## ⚠️ Safety Warning

**DANGER – HIGH VOLTAGE**

This project involves **110V–240V AC mains electricity**, which can be fatal if handled improperly.

* NEVER touch the circuit while powered.
* Always disconnect power before modifications.
* Use a **non-conductive, grounded enclosure**.
* Ensure proper insulation and safety practices.

> Use this project entirely at your own risk.

---

## 📜 License

This project is licensed under the **MIT License**.

