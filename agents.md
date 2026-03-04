# 🤖 Project Master Reference: DRYMASTER PRO R4

This document is the **Ground Truth** for the DryMaster Pro R4 project. Any AI agent working on this project MUST strictly adhere to the specifications, pinouts, and coding standards defined herein.

---

## 🛠 1. Hardware Detailed Specifications

### 1.1 Core Control & Display

* **MCU:** Arduino UNO R4 WiFi (Renesas RA4M1).
* **Display:** 3.5" TFT LCD SPI, 480x320 resolution.
* **Driver:** ILI9488 or similar (handled by custom library).
* **Touch:** Capacitive/Resistive (integrated via `R4_TFT_SPI_Touch.h`).


* **Real Time Clock:** Internal R4 RTC (used for timestamping if needed).

### 1.2 Sensors & Actuators

* **Ambient Sensor (SHT31):** High-precision I2C sensor for Air Temperature and Relative Humidity.
* **Safety/Mass Sensor (DS18B20):** OneWire sensor attached to a **1.7kg Aluminum/Steel Bar** to monitor thermal inertia and prevent overheating.
* **Presence Sensor (PIR HC-SR501):** Digital input to trigger the display backlight.
* **Heating Element:** Peltier Cell (High Power) controlled via a MOSFET/Power Relay circuit.
* **Storage:** Internal EEPROM (8192 bytes) for persistent settings and calibration data.

### 1.3 Complete Pinout Mapping (Arduino R4 WiFi)

| Pin | Function | Connection / Component |
| --- | --- | --- |
| **0 (RX)** | Serial | Debug / Programming |
| **1 (TX)** | Serial | Debug / Programming |
| **2** | **PIR_INPUT** | HC-SR501 Out (Backlight trigger) |
| **3** | **TP_IRQ** | Touch Panel Interrupt (TFT) |
| **4** | **TP_CS** | Touch Panel Chip Select (TFT) |
| **5** | **SD_CS** | SD Card Slot Chip Select (TFT Integrated) |
| **6** | **ONE_WIRE_BUS** | DS18B20 Data Pin (Pull-up 4.7k required) |
| **7** | **LCD_DC** | Data/Command Toggle (TFT) |
| **8** | **LCD_RST** | Reset (TFT) |
| **9** | **LCD_BL** | Backlight Control (PWM/AnalogWrite) |
| **10** | **LCD_CS** | Display Chip Select (TFT) |
| **11** | **SPI_MOSI** | Master Out Slave In (SPI Bus) |
| **12** | **SPI_MISO** | Master In Slave Out (SPI Bus) |
| **13** | **SPI_SCK** | Serial Clock (SPI Bus) |
| **A0** | **PELTIER_CTRL** | Peltier Actuator (Gate of MOSFET or Relay) |
| **A4** | **SDA** | I2C Data (SHT31) |
| **A5** | **SCL** | I2C Clock (SHT31) |

---

## 💻 2. Software Architecture & State Machine

### 2.1 External Libraries

* `pid-autotune.h` / `pid.h`: Custom AhmedOsama07 version.
* `R4_TFT_SPI_Touch.h`: Custom/Optimized library for UNO R4 SPI Display.
* `Adafruit_SHT31.h`: For environmental data.
* `DallasTemperature.h` & `OneWire.h`: For DS18B20 safety sensor.
* `EEPROM.h`: Standard library for data persistence.

### 2.2 System States (FSM)

1. **MAIN_PAGE:** Real-time dashboard with scrolling graphs (Temp, Hum, PWM, Setpoint).
2. **SETPOINT_PAGE:** UI for adjusting the target temperature using +/- buttons.
3. **INFO_PAGE:** Hardware health status, sensor diagnostics, and raw data view.
4. **AUTOTUNE_CONFIRM:** Safety gate before starting the calibration process.
5. **AUTOTUNE_RUNNING:** Active PID tuning (Relay oscillation mode) with real-time stats.
6. **ALARM_PAGE:** System Lockout. Triggered if SHT31 > 95°C or DS18B20 > 115°C.

---

## 📜 3. Mandatory Development Rules

**Any code generated for this project must strictly follow these instructions:**

* **Language:** Everything (Variables, Functions, GUI Labels, Serial Prints, and Comments) **MUST be in English**.
* **Verbosity:** Code must be **highly verbose** with comments. Every logical block, formula, or hardware interaction must be explained in detail within the code.
* **Completeness:** Never provide snippets. Every response must contain the **full, complete sketch** ready to be compiled.
* **Strict Method Naming:** Use only the methods defined in `pid.h`.
* `setConstants(kp, ki, kd)` NOT `setTunings`.
* `setSetPoint(val)` NOT `setSetpoint`.
* `setConstrains(low, high)` NOT `setOutputLimits`.
* `compute(input)` NOT `Compute`.


* **Formatting:**
* Use Markdown headers, bold text, and tables for clarity.
* Use LaTeX **only** for complex scientific formulas (e.g., Absolute Humidity calculation). Do **NOT** use LaTeX for units (°C, %) or simple prose.


* **GUI Standards:** All UI strings (e.g., "SAVE", "CANCEL", "SYSTEM ALARM") must be in English.

---

## 🧮 4. Key Algorithms

### 4.1 Absolute Humidity Formula

The system calculates the water vapor density in $g/m^3$ using the following physical relationship:


$$es = 6.112 \cdot e^{\frac{17.67 \cdot T}{T + 243.5}}$$

$$AbsHum = \frac{es \cdot (Rh / 100) \cdot 216.74}{273.15 + T}$$

### 4.2 PID PWM Implementation

The system uses a **Time Proportional Control** on a 2000ms window. The `pidOutput` (0-2000) determines how many milliseconds the Peltier remains ON within that window.

---

**Would you like me to generate the full code again, applying this new level of verbosity and the English-only standard for everything?**
