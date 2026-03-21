# Polvoron Vendo Machine Documentation

## Project Overview
This Arduino-based Vending Machine controls the dispensing of three different Polvoron flavors (Choco, Barnuts, and Milk) using continuous rotation servos and an HX711 load cell (scale) to verify successful item drops. It operates entirely on a 5-PHP coin acceptor system and utilizes an Adafruit SSD1306 OLED screen for the user interface.

## Hardware Configuration
| Component | Pin | Type | Purpose |
| :--- | :--- | :--- | :--- |
| **OLED Display** | I2C (A4/A5) | SSD1306 | Displays UI, balance, and warnings. |
| **HX711 Scale** | `DOUT: 3`, `CLK: 2` | Load Cell | Verifies item drops using weight (`dropThreshold = 6.0`). |
| **Coin Acceptor**| `Pin 4` | Digital In | Detects 5-PHP coin pulses. |
| **Button: Choco** | `Pin 8` | Digital In (Pull-up) | Selects Choco flavor. |
| **Button: Barnuts**| `Pin 9` | Digital In (Pull-up) | Selects Barnuts flavor. |
| **Button: Milk** | `Pin 10` | Digital In (Pull-up) | Selects Milk flavor. |
| **Servo: Choco** | `Pin 5` | PWM Out | Rotates Choco dispensing coil. |
| **Servo: Barnuts**| `Pin 6` | PWM Out | Rotates Barnuts dispensing coil. |
| **Servo: Milk** | `Pin 7` | PWM Out | Rotates Milk dispensing coil. |

## Core Mechanics

### 1. Coin Verification Engine
Coins are verified using a highly responsive, non-blocking polling engine via the custom `CheckCoin()` function. The loop uses a native 30ms debouncing window `(millis() - lastCoinDebounceTime > coinDebounceDelay)` to prevent ghost electrical signals from being registered as fake coins.

### 2. Item Selection & Dispensing
Pushing any connected flavor button queues the dispenser. The machine requires exactly **5 PHP** of balance to operate.
Once triggered, the machine dispenses **two items** consecutively:
1. The respective continuous rotation servo begins to move (`servo.write(90)`).
2. The `waitForDrop()` function actively reads the HX711 scale weight until the `dropThreshold` is hit or a `3000ms` physical timeout triggers.
3. The servo is completely shut off (`servo.write(0)`) and detached to prevent idle jitter.

### 3. Non-Blocking Loop Architecture
To prevent CPU lockups where the system might otherwise "ignore" a coin drop:
- `CheckCoin()` is injected deep inside all software delays (`safeDelay()`) and `while()` loops (like waiting for the item to drop onto the scale). No matter what the vending machine is actively doing, incoming coins are safely caught and saved to your balance.
- All hardware selection button reads use `HIGH`/`LOW` state detection logic, ensuring the CPU loop never accidentally hangs on a stuck physical button switch.
- The `Insufficient Balance` screen functions entirely via internal timers rather than hard CPU `delay(2000)` pauses, keeping exactly 100% responsiveness available to the coin hardware at all times.

## Serial Monitor Commands
For testing and debugging without physical hardware interaction, you can use the Serial Monitor (set to `9600` baud rate). Send your commands and press enter.

| Command | Action |
| :--- | :--- |
| `coin` or `5` | Simulates inserting a 5 PHP coin (increases balance). |
| `test` | Triggers a sequential hardware test of all three servo mechanisms. |
| `1` or `barnuts`| Simulates selecting the **Barnuts** flavor. |
| `2` or `choco`  | Simulates selecting the **Choco** flavor. |
| `3` or `milk`   | Simulates selecting the **Milk** flavor. |

## Customization Parameters
- `calibration_factor` (HX711): Adjust the `2280.0` value in the code to calibrate the specific grams/units your load cell reads.
- `coinDebounceDelay`: Adjust the `30`ms value if your specific electronic coin acceptor pulses slightly faster or slower.
- `dropThreshold`: Set to `6.0` by default; requires adjustment depending on the actual physical weight registered when a single Polvoron lands on the tray.
