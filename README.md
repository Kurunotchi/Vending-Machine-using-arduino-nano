# Vending-Machine-using-arduino-nano


A simple smart vending machine firmware built using an Arduino Uno platform. It dispenses three items (Choco, Barnuts, Milk) using servo motors and verifies the dispensing action using an HX711 load cell (scale) to ensure the item has dropped. It also features an SSD1306 OLED display for user feedback and allows inputs from both physical buttons and via Serial communication.

## Hardware Components
- **Microcontroller:** Arduino Uno
- **Display:** SSD1306 OLED (96x16 resolution) via I2C interface
- **Actuators:** 3x Micro Servo Motors (for dispensing items)
- **Sensors:** Load Cell with HX711 Amplifier (drop verification)
- **Input:** 3x Push Buttons (Active Low with internal pull-ups)
- **Coin Detector:** Coin acceptor connected to digital pin (pulse-based)

## Pin Configuration

| Component | Arduino Pin | Notes |
| :--- | :--- | :--- |
| **I2C Display** | SDA, SCL | Standard Arduino Uno I2C pins (A4, A5 typically) |
| **HX711 DOUT** | 3 | Data out from load cell |
| **HX711 CLK** | 2 | Clock for load cell |
| **Servo 1 (Choco)** | 5 | PWM |
| **Servo 2 (Barnuts)**| 6 | PWM |
| **Servo 3 (Milk)** | 7 | PWM |
| **Button 1 (Choco)**| 8 | Input pull-up |
| **Button 2 (Barnuts)**| 9 | Input pull-up |
| **Button 3 (Milk)**| 10 | Input pull-up |
| **Coin Pin** | 4 | Input pull-up for coin detection |

## Libraries Used
Dependencies are managed via PlatformIO (`platformio.ini`):
- `adafruit/Adafruit SSD1306` & `adafruit/Adafruit GFX Library` (Display)
- `bogde/HX711` (Load Cell driver)
- `arduino-libraries/Servo` (Servo control)

## Software Logic & Flow

### 1. Initialization (`setup`)
- Serial communication initialized at 9600 baud
- Button and coin pins configured as input with pull-ups
- SSD1306 display initialized and shows startup message "Vendo Machine"
- HX711 scale initialized with calibration factor and tared
- All servo motors tested with a sweep sequence (0° → 90° → 0°) to verify operation
- Display shows item selection menu

### 2. Main Loop (`loop`)
- Continuously checks for coin insertion
- Monitors button presses and serial input for item selection
- Updates OLED display with current balance and options
- Processes purchases when valid selection is made

### 3. Coin Management
- Coins detected via falling edge on COIN_PIN
- Each coin adds 5 PHP to currentBalance
- Balance displayed on OLED and Serial
- Supports coin insertion via hardware pin or serial command "coin"

### 4. Item Selection
- **Physical Buttons:** Debounced button presses for direct selection
- **Serial Input:** Accepts numeric (1-3) or text inputs:
  - "1", "b", "barn", "barnuts" → Barnuts
  - "2", "c", "choco" → Choco
  - "3", "m", "milk" → Milk
  - "5", "coin" → Add coin

### 5. Dispensing Process
- **Cost:** 5 PHP per purchase (dispenses 2 items)
- **Verification:** HX711 scale monitors weight change
- **Process per item:**
  1. Scale tared before each drop
  2. Servo moves from 0° to 90° to push item
  3. Waits for weight increase ≥ 6.0 units within 3-second timeout
  4. Servo returns to 0° and detaches
  5. Process repeats for second item
- **Feedback:** Serial output shows dispensing progress and results

### 6. Error Handling
- Insufficient balance displays error message on OLED
- Drop timeout logged to Serial
- Invalid serial input prompts user for correct format

## Configuration Constants
- **Drop Threshold:** 6.0 units (weight increase to confirm drop)
- **Drop Timeout:** 3000ms (maximum wait time for drop detection)
- **Servo Timing:** 300ms delay for servo movement
- **Coin Value:** 5 PHP per coin
- **Items per Purchase:** 2 items

## Serial Commands
- `1`, `b`, `barn`, `barnuts` - Select Barnuts
- `2`, `c`, `choco` - Select Choco
- `3`, `m`, `milk` - Select Milk
- `5`, `coin` - Simulate coin insertion

## Calibration
- **HX711 Scale:** Adjust `calibration_factor` (currently 2280.0) for accurate weight readings
- **Servo Positions:** 0° = home position, 90° = dispense position
- **Drop Threshold:** Tune based on actual item weights

## Usage Instructions
1. Power on the Arduino
2. Insert coins (5 PHP each) via coin acceptor or serial "coin" command
3. Select item using buttons or serial commands
4. System dispenses 2 items and deducts 5 PHP from balance
5. Monitor Serial output for detailed feedback

## Troubleshooting
- **Display not working:** Check I2C connections and address (0x3C)
- **Scale inaccurate:** Recalibrate `calibration_factor` and ensure proper tare
- **Servos not moving:** Verify PWM pins and power supply
- **Coin not detected:** Check coin pin wiring and pull-up configuration
- **Items not dropping:** Adjust servo angles, timing, or drop threshold
