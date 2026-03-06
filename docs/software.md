# Software Logic Specification: DRYMASTER PRO R4
**Version:** 1.2 (Implementation-Ready Master Spec)  
**Target Hardware:** Arduino UNO R4 Minima (Renesas RA4M1)  
**Document Type:** Absolute Ground Truth for Code Reconstruction  

## 1. System Architecture and Dependencies
The firmware is implemented as a Finite State Machine (FSM) responsible for coordinating thermal control, safety management, and the graphical user interface.  
The entire system operates on a synchronized main loop with a fixed cycle time of **2000 ms (2 seconds)**.

### 1.1 Required Libraries
* **`R4_TFT_SPI_Touch.h` (Custom):** Core library responsible for managing the ILI9486 display controller and the XPT2046 touch controller via 74HC4094 shift registers. Handles the **480×320 display resolution**.
* **`pid-autotune.h` (AhmedOsam):** Implements the PID controller and automatic calibration using the **relay autotuning method**.
* **`Adafruit_SHT31.h`:** Driver for the **I2C environmental sensor** (SDA/SCL).
* **`OneWire.h` & `DallasTemperature.h`:** Driver stack for the **DS18B20 safety bar temperature sensor** connected to **PIN_DS18B20**.
* **`EEPROM.h`:** Handles persistent storage using the **8KB internal EEPROM**.
* **`bmp30x30.h` (Asset):** Contains **30×30 bitmap icons** used by the dashboard (Temperature, Humidity, Power).

### 1.2 Pin Assignment
* The **pin mapping is fixed by the physical hardware design** and **must not be modified**.
* The firmware **must never refer to pins using identifiers other than the defines listed below**.

Pins defined in the `R4_TFT_SPI_Touch` library:  
#define PIN_PIR_D2      2  
#define PIN_TP_IRQ      3  
#define PIN_TP_CS       4  
#define PIN_SD_CS       5  
#define PIN_TP_BUSY     6  
#define PIN_LCD_DC      7  
#define PIN_LCD_RST     8  
#define PIN_LCD_BL      9  
#define PIN_LCD_CS      10  
#define PIN_SPI_MOSI    11  
#define PIN_SPI_MISO    12  
#define PIN_SPI_SCK     13  

Pins that must be defined at the beginning of the sketch:  
#define PIN_PELTIER     A0  
#define PIN_DS18B20     A1  
#define PIN_SHT31_SDA   A4  
#define PIN_SHT31_SCL   A5  

## 2. Global FSM: States and Navigation
At any moment the system is in exactly **one FSM state**, defined via `enum States`.

| State | Description | Exit Trigger |
| :--- | :--- | :--- |
| **MAIN_PAGE** | Main dashboard displaying real-time values and graphs. | Pressing the "SET" or "TUNE" buttons. |
| **SETPOINT_PAGE** | Adjustment of the temperature target (20–90°C). | "+" and "-" buttons, "SAVE" (writes to EEPROM), or "CANCEL". |
| **INFO_PAGE** | Diagnostics screen showing EEPROM versions, all raw sensor values, PWM state, and setpoint. | Pressing the "BACK" button. |
| **AUTOTUNE_CONFIRM** | Warning screen shown before starting calibration. | "YES" starts autotune; "NO" returns to the Main page. |
| **AUTOTUNE_RUNNING** | Active monitoring of the autotuning process. | Automatic completion or pressing the "ABORT" button. |
| **ALARM_PAGE** | Safety lock state (red screen). | Manual reset via button or automatic recovery once parameters return within safe limits. |

## 3. Sensor Processing and "Aggressive Filtering"
To mitigate electrical noise and sensor fluctuations, the firmware applies **rolling average filtering** on recent measurements.

### 3.1 Rolling Average (8 Samples)
* Every **2000 ms**, new readings are acquired from the **SHT31** and **DS18B20** sensors.
* Each measured variable (**Air Temperature, Air Relative Humidity, Bar Temperature**) is stored in a **circular buffer of 8 samples**.
* The filtering logic computes the **arithmetic mean of the most recent 8 samples**.
* The resulting filtered value is **the only value used by the PID controller and displayed in the UI**, ensuring stable readings.

### 3.2 Absolute Humidity Calculation ($g/m^3$)
Using the filtered **temperature ($T$)** and **relative humidity ($RH$)** values, the firmware computes:

1. **Saturated vapor pressure**

$$E_s = 6.112 \times e^{(17.67 \times T) / (T + 243.5)}$$

2. **Absolute Humidity ($AH$)**

$$AH = (E_s \times (RH / 100.0) \times 216.74) / (273.15 + T)$$

## 4. Memory Management (Circular Wear Leveling)
To protect the internal EEPROM from excessive write cycles, the firmware implements a **Circular Buffer Wear Leveling strategy**, utilizing the entire **8KB EEPROM space**.

* **`Config struct`:** A **15-byte structure** containing:
  * `uint16_t version`
  * `uint8_t setpoint`
  * `float kp, ki, kd`

* **Boot Scan:**  
  At startup, the firmware scans the EEPROM to locate the record with the **highest `version` value**.

* If no valid record is found (blank memory), **default values are loaded**.

* The logic also correctly handles **version rollover**:  
  if both **0xFFFF and 0x0000** versions are present, the firmware searches for the **highest value belonging to the new cycle**.

* **Save Logic:**  
  Configuration data is written **only**:
  - after explicit user confirmation ("SAVE"), or  
  - after a **successful Autotune**.

  The new record is written to the **next EEPROM address**, ensuring even distribution of write cycles across memory.

## 5. Thermal Control (PID & Slow PWM)

### 5.1 Slow PWM Logic (Pin PIN_PELTIER)
Due to the system’s **high thermal inertia**, the MOSFET controlling the Peltier module is driven using **low-frequency PWM (0.5 Hz)**.

* **Time Window:** 2000 ms
* **Duty Cycle:**  
  The PID controller outputs a value in the range **0–2000**.  
  This value determines the ON duration of the output within the 2000 ms window:

  - `PIN_PELTIER` stays **HIGH** for a number of milliseconds equal to the PID output value.
  - It then remains **LOW** for the remaining time until the 2000 ms window ends.

* **Safety Lock:**  
  If an alarm condition occurs, pid class disable() method is executed and `PIN_PELTIER` is anyway **forced LOW regardless of PID output**.

### 5.2 PID Autotune
The PID parameters are determined using the **relay oscillation method**.

During this process:

* The **setpoint remains fixed**.
* The system monitors **temperature oscillation peaks**.
* From these oscillations, the firmware calculates the **critical gain** and **critical period**, which are then used to derive optimal **PID coefficients**.

## 6. Graphical User Interface (GUI) & Touch

### 6.1 Dashboard Layout (MAIN_PAGE)

* **Colors:**  
Each physical quantity has a dedicated color used for:
  - text and borders in the four top boxes
  - chart traces
  - footer button colors

| Quantity | Alias #define in sketch | Color name in R4_TFT_SPI_Touch library |
| :--- | :--- | :--- |
| Air Temperature | COL_TEMP_ARIA | COLOR_RED_VIVID |
| Relative Humidity (%) | COL_REL_HUM | COLOR_BLUE_CYAN |
| Absolute Humidity | COL_ABS_HUM | COLOR_GREEN_EMERALD |
| PWM (%) | COL_PWM | COLOR_YELLOW_VIVID |
| Bar Temperature | COL_BAR | COLOR_WHITE |

The software must define the color names as in the second column, as alias of the corrisponding color in the library and use only these in the code.  

* **Top Bar (Icons):**  
Four boxes (**120×40 px**) containing values and measurement units, with **30×30 icons aligned on the right** for:
  - Air Temperature
  - Relative Humidity %
  - Absolute Humidity
  - PWM %

* **Rolling Chart:**  
Central graph updated **every 10 seconds** and storing **235 historical samples**.

  **Displayed traces:**
  - Air Temperature
  - Relative Humidity %
  - Absolute Humidity
  - PWM %

  **Setpoint Indicator:**  
  A **dashed line** (color `COL_TEMP_ARIA`) represents the configured target temperature, allowing visual evaluation of tracking error.

* **Footer Buttons:**  
Rendered using the `drawButton3D()` function to produce a **raised button effect**.

  - Left: **Bar Temperature**
  - Right: **Setpoint**

* **Title:**  
Centered between the two footer buttons: **"DRYMASTER PRO R4"**

### 6.2 Backlight Management (D9)

* **Trigger:**  
  Activated by the **PIR motion sensor (D2)** or by **touch interaction** on the display.

* **Fade-In:**  
  Rapid PWM ramp-up to **25% brightness**.

* **Fade-Out:**  
  After **5 minutes (300,000 ms)** of inactivity, brightness gradually decreases until **complete shutdown**.

### 6.3 Touch Logic (XPT2046)

Touch coordinates are mapped to UI actions as follows:

* **SET:**  
  In `MAIN_PAGE`, `x > 340, y > 270` → transition to `SETPOINT_PAGE`.

* **TUNE:**  
  In `MAIN_PAGE`, `x < 140, y > 270` → transition to `AUTOTUNE_CONFIRM`.

* **INFO:**  
  In `MAIN_PAGE`, `140 < x < 340, y > 270` → transition to `INFO_PAGE`.

* **Setpoint + / -, SAVE, CANCEL:**  
  In `SETPOINT_PAGE`, the variable `tempSetpointTmp` is modified before saving.  
  The user may:
  - save the new value
  - or return to `MAIN_PAGE` without modifying the stored setpoint.

## 7. Watchdog and Hardware Safety

Safety logic **has priority over all other loop operations**.

At every loop iteration, the system checks for the following critical conditions:

1. **Bar Temperature Threshold**  
   If **Bar T > 115°C**

2. **Air Temperature Threshold**  
   If **Air T > 95°C**

3. **Sensor Integrity Failure**  
   Communication error with **SHT31 or DS18B20** resulting in **NaN values**

4. **Thermal Response Too Slow**  
   The bar temperature does not increase quickly enough.

   * At **full load**, the minimum acceptable rate is **1°C per minute**.
   * At lower PWM levels, the threshold is **proportionally scaled**.

   This condition is used to detect a **possible mechanical detachment or failure of heating elements**.

* **Action:**  
  Immediate transition to `ALARM_PAGE`,  
  PID disabling,  
  `Pin PIN_PELTIER` forced **LOW**,  
  **interrupts disabled**,  
  and a **red visual alarm signal** is displayed.

## 8. Simulation Mode

The code must include a  
`#define SIMULATION_MODE 1`  
It's used to test the code, without sensors attached to the system.  

When set to 0, code behaviour is as decribed, while when set to 1:  
1. All Physical values are computed using a sinusoidal with 5 minutes interval  
2. Backlight is fixed to 25% brigthness
3. Sensors are not initialized or used  
4. ALARM_PAGE status is not triggered  
5. All graphical and touch behaviours are as described  
