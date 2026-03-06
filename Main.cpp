#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "HX711.h"

// OLED display size
#define SCREEN_WIDTH 96
#define SCREEN_HEIGHT 16
#define OLED_RESET -1

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// HX711 pins
#define DOUT 3
#define CLK 2

HX711 scale;

// Calibration factor (change during calibration)
float calibration_factor = 2280.0;

void setup() {
  
  Serial.begin(9600);

  // Initialize OLED
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED not found");
    while(true);
  }

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);

  display.setCursor(0,0);
  display.println("Load Cell Init...");
  display.display();
  delay(2000);

  // Initialize Load Cell
  scale.begin(DOUT, CLK);
  scale.set_scale(calibration_factor);
  scale.tare(); // reset scale

  Serial.println("System Ready");
}

void loop() {

  float weight = scale.get_units(5);

  Serial.print("Weight: ");
  Serial.print(weight,2);
  Serial.println(" g");

  // OLED Display
  display.clearDisplay();

  display.setCursor(0,0);
  display.print("Weight:");

  display.setCursor(0,8);
  display.print(weight,1);
  display.print(" g");

  display.display();

  delay(500);
}
