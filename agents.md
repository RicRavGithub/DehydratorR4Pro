# Project Master Reference: DRYMASTER PRO R4
**Absolute Ground Truth for Development and AI Agents.**

It's a controller of a peltier cell based system, that dehydrates and heats a cabinet containing 3D Print filament spools

## 🛠 1. Hardware Detailed Specifications
Electronic is based on an Arduino R4 Minima with a display shield and extenal sensors and actuators.

Details of hardware used is described in docs/circuit.md

## 🔌 2. Complete Pinout Mapping

| Pin | Function | Connection / Component | Note |
| :--- | :--- | :--- | :--- |
| **5V** | Power | VCC for Display, PIR, SHT31, DS18B20 | Logic Power |
| **GND** | Ground | Common Star Ground | Unified Reference |
| **D2** | Digital IN | PIR HC-SR501 (OUT) | Interrupt for Backlight wake-up |
| **D3** | SPI IRQ | TP_IRQ (Touch Panel) | XPT2046 Interrupt |
| **D4** | SPI CS | TP_CS (Touch Panel) | Touch Chip Select |
| **D5** | SPI CS | SD_CS (Micro SD) | Leave HIGH (Not used) |
| **D6** | Digital IN | TP_BUSY (Touch Panel) | Touch controller status |
| **D7** | Digital OUT | LCD_DC (Data/Command) | ILI9486 Control |
| **D8** | Digital OUT | LCD_RST (Reset) | Hardware Reset |
| **D9** | PWM OUT | LCD_BL (Backlight) | Fade-In/Out Control |
| **D10** | SPI CS | LCD_CS (Chip Select) | ILI9486 Selection |
| **D11** | SPI MOSI | LCD/Touch MOSI | Hardware SPI |
| **D12** | SPI MISO | LCD/Touch MISO | Hardware SPI |
| **D13** | SPI SCK | LCD/Touch CLK | Hardware SPI |
| **A0** | **PWM OUT** | **MOSFET Gate (Peltier)** | **Slow PWM (0.5Hz / 2s window)** |
| **A1** | **OneWire** | **DS18B20 Data** | **Safety Bar Sensor** |
| **SDA** | I2C Data | SHT31-D (SDA) | Ambient Sensor |
| **SCL** | I2C Clock | SHT31-D (SCL) | Ambient Sensor |

## 💻 3. Software Logic & Requirements

### 3.1 Core Processing
* **Library:** Must use `R4_TFT_SPI_Touch` for graphics and touch.
* **Averaging:** Air Temp, RH%, and Bar Temp must be a rolling average of the last **8 readings**.
* **Loop:** Sensors must be polled every 2 seconds.
* **PID:** Slow PWM frequency of 0.5Hz (2000ms window) on Pin A0.
* **Safety Watchdog:**
    * If Aluminum Bar > 115°C OR Air Temp > 95°C: Force Peltier OFF (A0 LOW).
    * Calculate Heating Velocity (°C/min) during Cold Start to verify thermal coupling.

### 3.2 Data Persistence (EEPROM)
* **Wear Leveling:** Circular buffer strategy using a `Config` struct with a `version` counter (uint16_t), containing: Set Point and kp, ki, kd for pid.
* **Timing:** Save to EEPROM only upon explicit user confirmation of new set point ("SAVE" button), or upon autotune completion.

### 3.3 UI & Graphics
* **Backlight:** Fast Fade-In on PIR trigger; slow Fade-Out 5 minutes after the last PIR detection.
* **Main Dashboard:** Near real-time display (every 2s) of Air Temp, RH%, Abs Humidity (g/m³), Bar Temp, and PWM %.
* **Rolling Chart:**
    * Updated every 10 seconds.
    * Traces: Air Temp, RH%, Abs Humidity, PWM %.
    * Extra Trace: A dashed line (same color as Air Temp) showing historical Setpoint values.

## 📜 4. Mandatory AI Development Rules
1. **Language:** Code, comments, GUI, and variables MUST be in English.
2. **Verbosity:** Extreme commenting required. Explain every physical calculation and hardware interaction.
3. **Completeness:** Provide the FULL sketch in every response.
4. **Formatting:** Render units as "90°C" or "10%" directly. Use LaTeX only for complex formulas.
