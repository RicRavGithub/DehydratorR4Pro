# Project Master Reference: DRYMASTER PRO R4
**Absolute Ground Truth for Development and AI Agents.**

## 🛠 1. Hardware Detailed Specifications

### 1.1 Core Control & Display
* **MCU:** Arduino UNO R4 Minima (Renesas RA4M1, 32-bit, 5V logic).
* **Display:** Waveshare 4-inch Touch LCD Shield (SKU: 13587).
* **Resolution:** 480x320.
* **Driver:** ILI9486 (interfaced via SPI through 74HC4094 shift registers).
* **Touch:** XPT2046 Resistive Touch (SPI).
* **Backlight:** PWM controlled via Arduino for Fade-In/Fade-Out effects.

### 1.2 Thermal Actuation (Peltier System)
* **Peltier Cell:** Laird Thermal Systems HiTemp ETX8-12-F1-4040-TA-RT-W6 (survive in temperatures up to +150°C).
* **Protection:**
    * 1N5408 Diode (Cathode to +12V, Anode to Peltier Negative/MOSFET Drain).
    * In-series Thermal Fuse (125°C RY/SF type).
    * In-series KSD302 Thermostat (120°C 16A Normally Closed).
* **Driver:** MOSFET CSD18536KCS (N-Channel, TO-220, Logic Level).
* **Heatsink:** Assmann WSW V8813Y.
* **Resistor Network:** R1 (470Ω) on Gate; R2 (10kΩ) Pull-down soldered directly between Gate and Source.

### 1.3 Sensors
* **Ambient Air:** SHT31-D (I2C) for Temperature and Humidity (High accuracy below 10% RH).
* **Safety Bar:** DS18B20 (OneWire) inserted into a 6mm x 60mm blind hole in the aluminum bar, filled with Arctic MX-4 thermal paste and sealed with red thermal silicone.
* **Presence:** PIR HC-SR501 for backlight management.

### 1.4 Mechanical & Thermodynamic Design
* **Hot Side:** Massive Aluminum Bar (659x100x10 mm) acting as the internal radiator.
* **Cold Side (Condensate):** Pinned copper heatsink with 3.4mm pitch to prevent surface tension blocking droplets.
* **Thermal Interface:** Thermal Grizzly Kryonaut between Peltier and metal surfaces.
* **Sandwich Assembly:**
    * 4x Stainless Steel M2.5x20mm screws.
    * 304 Stainless Steel compression springs (4x10mm, 0.7mm wire) providing ~1.2kg/cm² pressure.
    * Nylon washers for thermal isolation of the screws, M2.5x5x1mm, #2 for each screw.
    * Stainless steel falt washers M2.5x6x0.5mm, to spread the load

### 1.5 PCB
* **Sandwich pcb** to extract signals for external sensors, placed between Arduino the the display shield
* The sandwich PCB includes 2.54 screw connectors, pullup and mosfet resistors and filtering capacitors:
* | Name(s)       | value | Usage
* | R1            | 470Ω  | Gate MOSFET protection for Arduino R4
* | R2, R3, R4    |	4.7kΩ | Pull-up for I2C and OneWire
* | R5	          | 10kΩ  | Pull-down MOSFET, soldered on mosfet pins, so out from the pcb
* | C3	          |	680µF | 12V Arduino power input stabilizer
* | C1, C4, C6, C7|	100nF | Ceramic capacitors for high frequency noise filtering, for sensors and 12V arduino
* | C2, C5, C8	  |	10uF  | Ceramic capacitors for lower frequency noise filtering, for sensors

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
