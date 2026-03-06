# Vending-Machine-using-arduino-nano
Arduino-based weight monitoring system for a vendo machine using a load cell and HX711 amplifier. The measured weight is displayed in real time on an I2C OLED (SSD1306) and sent to the Serial Monitor. Useful for detecting item weight, monitoring product dispensing, or validating load conditions in automated vending systems.

OLED Load Cell Monitor for Vendo Machine

This Arduino project measures weight using a load cell and HX711 amplifier and displays the result on an I2C OLED (SSD1306). The system shows real-time weight readings on the OLED screen and sends the data to the Serial Monitor.
This setup can be used in vending machines to detect product weight, confirm item dispensing, or monitor load conditions.

Features

-Real-time weight measurement
-Load Cell + HX711 sensor interface
-OLED display output
-Serial Monitor logging
-Automatic tare (zero reset) on startup
-Adjustable calibration factor

Hardware Required

-Arduino Uno / Nano / Mega
-Load Cell Sensor
-HX711 Load Cell Amplifier
-I2C OLED Display (SSD1306)
-Jumper Wires
-Breadboard (optional)

Wiring

OLED (I2C)

OLED Pin → Arduino
VCC	- 5V
GND	- GND
SDA	- A4
SCL	- A5

HX711 → Arduino
VCC	- 5V
GND -	GND
DT	- D3
SCK	- D2

Load Cell → HX711
Red - E+
Black	- E-
White	- A-
Green	- A+

Libraries Required

Install these libraries using Arduino Library Manager:

Adafruit SSD1306
Adafruit GFX
HX711
Wire

How It Works

The load cell detects force applied to it.
The HX711 module amplifies the signal and converts it to digital data.
The Arduino reads the weight value.
A calibration factor converts the raw data to grams.
The result is displayed on the OLED screen and printed to the Serial Monitor.

Calibration

Adjust this value in the code:
float calibration_factor = 2280.0;

Steps:

Upload the program.
Place a known weight on the load cell.
Adjust the calibration factor until the reading is accurate.

Example Output

-OLED Display
-Weight:
-185.3 g
-Serial Monitor
-Weight: 185.32 g

Applications

-Vending machine item detection
-Digital weighing scale
-Product dispensing validation
-Smart inventory monitoring
-Embedded sensor projects

License

This project is open-source and free to use for educational and development purposes.
