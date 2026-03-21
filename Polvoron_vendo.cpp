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

Servo Choco;
Servo Barnuts;
Servo Milk;

#define SERVO1_PIN 5
#define SERVO2_PIN 6
#define SERVO3_PIN 7

#define BUTTON_CHOCO_PIN 8
#define BUTTON_BARNUTS_PIN 9
#define BUTTON_MILK_PIN 10
#define COIN_PIN 4

int currentBalance = 0;
int lastCoinState = HIGH;
unsigned long lastCoinDebounceTime = 0;
const unsigned long coinDebounceDelay = 30;

int lastBarnuts = HIGH, lastChoco = HIGH, lastMilk = HIGH;

bool showingMsg = false;
unsigned long msgTimer = 0;
bool needsUIUpdate = false;
unsigned long lastPulseTime = 0;

void Starting();
void Scale_Init();
void Items();
void Choices();
void Dispense(int choice);
void CheckCoin();

void setup() {
  Serial.begin(9600);
  Serial.setTimeout(50);

  pinMode(BUTTON_CHOCO_PIN, INPUT_PULLUP);
  pinMode(BUTTON_BARNUTS_PIN, INPUT_PULLUP);
  pinMode(BUTTON_MILK_PIN, INPUT_PULLUP);
  pinMode(COIN_PIN, INPUT_PULLUP);

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED not found");
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

  Choices();
}

void CheckCoin() {
  int coinState = digitalRead(COIN_PIN);

  if (coinState != lastCoinState) {
    lastCoinDebounceTime = millis();
  }

  if ((millis() - lastCoinDebounceTime) > coinDebounceDelay) {
    static int stableCoinState = HIGH;
    if (coinState != stableCoinState) {
      stableCoinState = coinState;
      if (stableCoinState == LOW) {
        currentBalance += 5;
        Serial.print("Coin pulse detected. Balance: ");
        Serial.println(currentBalance);
        lastPulseTime = millis();
        needsUIUpdate = true;
      }
    }
  }

  lastCoinState = coinState;
}

int readChoiceFromButtons() {
  int bBarnuts = digitalRead(BUTTON_BARNUTS_PIN);
  int bChoco = digitalRead(BUTTON_CHOCO_PIN);
  int bMilk = digitalRead(BUTTON_MILK_PIN);

  int choice = 0;

  if (bBarnuts == LOW && lastBarnuts == HIGH) choice = 1;
  else if (bChoco == LOW && lastChoco == HIGH) choice = 2;
  else if (bMilk == LOW && lastMilk == HIGH) choice = 3;

  lastBarnuts = bBarnuts;
  lastChoco = bChoco;
  lastMilk = bMilk;

  if (choice != 0) delay(20);
  return choice;
}

int readChoiceFromSerial() {
  static String line = "";
  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\n' || c == '\r') {
      line.trim();
      if (line.length() == 0) continue;

      int choiceMatch = 0;

      if (line == "1" || line.equalsIgnoreCase("b") || line.equalsIgnoreCase("barn") || line.equalsIgnoreCase("barnuts")) choiceMatch = 1;
      else if (line == "2" || line.equalsIgnoreCase("c") || line.equalsIgnoreCase("choco")) choiceMatch = 2;
      else if (line == "3" || line.equalsIgnoreCase("m") || line.equalsIgnoreCase("milk")) choiceMatch = 3;
      else if (line == "5" || line.equalsIgnoreCase("coin")) choiceMatch = 5;
      else if (line.equalsIgnoreCase("test")) choiceMatch = 99;
      else {
        Serial.println("Invalid input. Use 1/2/3, choco/barnuts/milk, 'test', or 'coin'.");
      }

      line = "";
      return choiceMatch;
    } else {
      line += c;
    }
  }
  return 0;
}

void loop() {
  CheckCoin();

  if (needsUIUpdate && (millis() - lastPulseTime > 400)) {
    needsUIUpdate = false;
    if (!showingMsg) {
      Choices(); 
    }
  }

  if (showingMsg) {
    if (millis() - msgTimer > 2000) {
      showingMsg = false;
      Choices();
    }
    return;
  }

  int choice = readChoiceFromButtons();

  if (choice == 0) {
    int serialInput = readChoiceFromSerial();

    if (serialInput == 5) {
      currentBalance += 5;
      Serial.print("Coin inserted via Serial. Balance: ");
      Serial.print(currentBalance);
      Serial.println(" PHP");
      Choices();
    } else if (serialInput == 99) {
      Serial.println("--- TESTING ALL SLOTS ---");
      const int servoPins[] = {SERVO1_PIN, SERVO2_PIN, SERVO3_PIN};
      const char* slotNames[] = {"Choco (Pin 5)", "Barnuts (Pin 6)", "Milk (Pin 7)"};

      for (int i = 0; i < 3; ++i) {
        Serial.print("Testing Slot: ");
        Serial.println(slotNames[i]);

        Servo testServo;
        testServo.attach(servoPins[i]);
        testServo.write(90);
        delay(300);
        testServo.write(0);
        delay(300);
        testServo.detach();
        delay(500);
      }

      Serial.println("--- TEST COMPLETE ---");
      Choices();
    } else if (serialInput >= 1 && serialInput <= 3) {
      choice = serialInput;
    }
  }

  if (choice != 0) {
    if (currentBalance >= 5) {
      currentBalance -= 5;
      Serial.println("Purchase accepted. Dispensing 2 items...");
      Choices(); 
      Dispense(choice);
    } else {
      Serial.println("Insufficient balance. 5 pesos required.");

      display.clearDisplay();
      display.setCursor(0,0);
      display.println("Insufficient Bal!");
      display.setCursor(0,8);
      display.print("Current: ");
      display.print(currentBalance);
      display.display();

      showingMsg = true;
      msgTimer = millis();
    }
  }
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
  display.println("1 Barn 2 Choc 3 Milk");
  display.setCursor(0,8);
  display.print("Bal: ");
  display.print(currentBalance);
  display.println(" PHP");
  display.display();

  Serial.println("\n--- Vendo Machine ---");
  Serial.print("Balance: ");
  Serial.print(currentBalance);
  Serial.println(" PHP");
  Serial.println("1 = Barnuts, 2 = Choco, 3 = Milk");
  Serial.println("Note: Type your command and press ENTER.");
}

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

void safeDelay(unsigned long ms) {
  unsigned long start = millis();
  while (millis() - start < ms) {
    CheckCoin();
    delay(5);
  }
}

bool waitForDrop(float threshold, unsigned long timeoutMs) {
  unsigned long start = millis();
  while (millis() - start < timeoutMs) {
    CheckCoin();
    if (scale.get_units() >= threshold) return true;
    delay(10);
  }
  return false;
}

void Dispense(int choice) {
  const float dropThreshold = 6.0; 
  const unsigned long timeoutMs = 3000;

  for (int count = 1; count <= 2; count++) {
    scale.tare();

    Serial.print("Dropping item ");
    Serial.print(count);
    Serial.println(" of 2...");

    switch (choice) {
      case 1: {
        Serial.println("Dispensing Barnuts (servo should move)");
        Barnuts.attach(SERVO2_PIN);
        Barnuts.write(90);
        safeDelay(300);

        if (waitForDrop(dropThreshold, timeoutMs)) {
          Serial.println("Barnuts dropped");
        } else {
          Serial.println("Barnuts drop timeout");
        }

        Barnuts.write(0);
        safeDelay(300);
        Barnuts.detach();
        break;
      }

      case 2:
        Serial.println("Dispensing Choco (servo should move)");
        Choco.attach(SERVO1_PIN);
        Choco.write(90);
        safeDelay(300);

        if (waitForDrop(dropThreshold, timeoutMs)) {
          Serial.println("Choco dropped");
        } else {
          Serial.println("Choco drop timeout");
        }

        Choco.write(0);
        safeDelay(300);
        Choco.detach();
        break;

      case 3:
        Serial.println("Dispensing Milk (servo should move)");
        Milk.attach(SERVO3_PIN);
        Milk.write(90);
        safeDelay(300);

        if (waitForDrop(dropThreshold, timeoutMs)) {
          Serial.println("Milk dropped");
        } else {
          Serial.println("Milk drop timeout");
        }

        Milk.write(0);
        safeDelay(300);
        Milk.detach();
        break;

      default:
        Serial.println("Invalid choice");
    }

    safeDelay(1000); 
  }
}
