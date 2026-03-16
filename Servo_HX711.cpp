#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "HX711.h"
#include <Servo.h>  

#define SCREEN_WIDTH 96
#define SCREEN_HEIGHT 16
#define OLED_RESET -1

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define DOUT 3
#define CLK 2

HX711 scale;

float calibration_factor = 2280.0;

Servo servo1;
Servo servo2;
Servo servo3;

#define SERVO1_PIN 5
#define SERVO2_PIN 6
#define SERVO3_PIN 7

void setup() {
  Serial.begin(9600);

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

  scale.begin(DOUT, CLK);
  scale.set_scale(calibration_factor);
  scale.tare(); // reset scale

  Serial.println("System Ready");

  servo1.attach(SERVO1_PIN);
  servo2.attach(SERVO2_PIN);
  servo3.attach(SERVO3_PIN);

  servo1.write(0);
  servo2.write(0);
  servo3.write(0);
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

  if (weight >= 58.0) {
    servo1.write(90);
    servo2.write(90);
    servo3.write(90);
  } else {
    servo1.write(0);
    servo2.write(0);
    servo3.write(0);
  }

  delay(500);
}