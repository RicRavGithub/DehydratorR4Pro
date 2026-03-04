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

| Name(s) | value | Usage |
| :--- | :--- | :--- |
| **R1**            | 470Ω  | Gate MOSFET protection for Arduino R4 |
| **R2, R3, R4**    |	4.7kΩ | Pull-up for I2C and OneWire |
| **R5**	          | 10kΩ  | Pull-down MOSFET, soldered on mosfet pins, so out from the pcb |
| **C3**	          |	680µF | 12V Arduino power input stabilizer |
| **C1, C4, C6, C7**|	100nF | Ceramic capacitors for high frequency noise filtering, for sensors and 12V arduino |
| **C2, C5, C8**	  |	10uF  | Ceramic capacitors for lower frequency noise filtering, for sensors |
