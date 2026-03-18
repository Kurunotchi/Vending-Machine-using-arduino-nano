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

float calibration_factor = 2280.0; // Need i-change for weight accuracy

Servo Choco;
Servo Barnuts;
Servo Milk;

#define SERVO1_PIN 5
#define SERVO2_PIN 6
#define SERVO3_PIN 7

#define BUTTON_CHOCO_PIN 8
#define BUTTON_BARNUTS_PIN 9
#define BUTTON_MILK_PIN 10

void Starting();
void Scale_Init();
void Items();
void Choices();
void Dispense(int choice);

void setup() {
  Serial.begin(9600);

  pinMode(BUTTON_CHOCO_PIN, INPUT_PULLUP);
  pinMode(BUTTON_BARNUTS_PIN, INPUT_PULLUP);
  pinMode(BUTTON_MILK_PIN, INPUT_PULLUP);

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED not found");
    while(true);
  }
  Starting();
  Scale_Init();
  Items();

  const int servoPins[] = {SERVO1_PIN, SERVO2_PIN, SERVO3_PIN};
  for (int i = 0; i < 3; ++i) {
    Servo testServo;
    testServo.attach(servoPins[i]);
    testServo.write(90);
    delay(300);
    testServo.write(0);
    delay(300);
    testServo.detach();
  }
}

int readChoiceFromButtons() {
  if (digitalRead(BUTTON_CHOCO_PIN) == LOW) {
    delay(50);
    if (digitalRead(BUTTON_CHOCO_PIN) == LOW) {
      while (digitalRead(BUTTON_CHOCO_PIN) == LOW) {}
      return 1;
    }
  }
  if (digitalRead(BUTTON_BARNUTS_PIN) == LOW) {
    delay(50);
    if (digitalRead(BUTTON_BARNUTS_PIN) == LOW) {
      while (digitalRead(BUTTON_BARNUTS_PIN) == LOW) {}
      return 2;
    }
  }
  if (digitalRead(BUTTON_MILK_PIN) == LOW) {
    delay(50);
    if (digitalRead(BUTTON_MILK_PIN) == LOW) {
      while (digitalRead(BUTTON_MILK_PIN) == LOW) {}
      return 3;
    }
  }
  return 0;
}

int readChoiceFromSerial() {
  if (!Serial.available()) return 0;

  String line = Serial.readStringUntil('\n');
  line.trim();
  if (line.length() == 0) return 0;

  if (line == "1" || line.equalsIgnoreCase("c") || line.equalsIgnoreCase("choco")) {
    return 1;
  }
  if (line == "2" || line.equalsIgnoreCase("b") || line.equalsIgnoreCase("barn") || line.equalsIgnoreCase("barnuts")) {
    return 2;
  }
  if (line == "3" || line.equalsIgnoreCase("m") || line.equalsIgnoreCase("milk")) {
    return 3;
  }

  Serial.println("Invalid serial choice. Use 1/2/3 or choco/barnuts/milk.");
  return 0;
}

void loop() {
  Choices();

  int choice = 0;
  while (choice == 0) {
    choice = readChoiceFromButtons();
    if (choice != 0) break;

    choice = readChoiceFromSerial();
  }

  Dispense(choice);
  delay(200);
}
void Starting () {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);

  display.setCursor(0,0);
  display.println("Vendo Machine");
  display.display();
  delay(2000);
}
void Choices() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0,0);
  display.println("1 Choco 3 Milk");
  display.setCursor(0,8);
  display.println("2 Barn");
  display.display();
  Serial.println("--- Choose item (serial or buttons) ---");
  Serial.println("1 = Choco, 2 = Barnuts, 3 = Milk");
  Serial.println("You can also type: choco / barnuts / milk");}
void Scale_Init(){
  scale.begin(DOUT, CLK);
  scale.set_scale(calibration_factor);
  scale.tare();
  Serial.println("System Ready");
}
void Items() {
  Choco.attach(SERVO1_PIN);
  Choco.write(0);
  Choco.detach();

  Barnuts.attach(SERVO2_PIN);
  Barnuts.write(0);
  Barnuts.detach();

  Milk.attach(SERVO3_PIN);
  Milk.write(0);
  Milk.detach();
}
bool waitForDrop(float threshold, unsigned long timeoutMs) {
  unsigned long start = millis();
  while (millis() - start < timeoutMs) {
    if (scale.get_units() >= threshold) return true;
    delay(50);
  }
  return false;
}

void Dispense(int choice) {
  const float dropThreshold = 6.0; 
  const unsigned long timeoutMs = 3000;

  switch (choice) {
    case 1:
      Serial.println("Dispensing Choco (servo should move)");
      Choco.attach(SERVO1_PIN);
      Choco.write(90);
      delay(300);
      if (waitForDrop(dropThreshold, timeoutMs)) {
        Serial.println("Choco dropped");
      } else {
        Serial.println("Choco drop timeout");
      }
      Choco.write(0);
      Choco.detach();
      break;
    case 2: {
      Serial.println("Dispensing Barnuts (servo should move)");
      Barnuts.attach(SERVO2_PIN);
      Barnuts.write(90);
      delay(300);
      if (waitForDrop(dropThreshold, timeoutMs)) {
        Serial.println("Barnuts dropped");
      } else {
        Serial.println("Barnuts drop timeout");
      }
      Barnuts.write(0);
      Barnuts.detach();
      break;
    }
    case 3:
      Serial.println("Dispensing Milk (servo should move)");
      Milk.attach(SERVO3_PIN);
      Milk.write(90);
      delay(300);
      if (waitForDrop(dropThreshold, timeoutMs)) {
        Serial.println("Milk dropped");
      } else {
        Serial.println("Milk drop timeout");
      }
      Milk.write(0);
      Milk.detach();
      break;
    default:
      Serial.println("Invalid choice");
  }
}
