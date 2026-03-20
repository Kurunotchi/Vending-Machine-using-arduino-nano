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
#define COIN_PIN 4
int currentBalance = 0;
int lastCoinState = HIGH;
unsigned long lastCoinDebounceTime = 0;
const unsigned long coinDebounceDelay = 30; // Protects from ghost coins / bouncing
// Button states
int lastBarnuts = HIGH, lastChoco = HIGH, lastMilk = HIGH;
// Non-blocking msg variables
bool showingMsg = false;
unsigned long msgTimer = 0;
void Starting();
void Scale_Init();
void Items();
void Choices();
void Dispense(int choice);
void CheckCoin();
void setup() {
  Serial.begin(9600);
  Serial.setTimeout(50); // Prevent Serial String reading from freezing the loop for 1 full second
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
  
  // Non-blocking Debounce
  if (coinState != lastCoinState) {
    lastCoinDebounceTime = millis();
  }
  
  if ((millis() - lastCoinDebounceTime) > coinDebounceDelay) {
    // State is stable
    static int stableCoinState = HIGH;
    if (coinState != stableCoinState) {
      stableCoinState = coinState;
      if (stableCoinState == LOW) {
        currentBalance += 5;
        Serial.print("Coin inserted. Balance: ");
        Serial.print(currentBalance);
        Serial.println(" PHP");
        
        // Don't disrupt the warning message on screen, naturally return to choices
        if (!showingMsg) {
          Choices(); 
        }
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
  // Non-blocking button check (Replaces the dangerous while(LOW){} freeze loop)
  if (bBarnuts == LOW && lastBarnuts == HIGH) choice = 1;
  else if (bChoco == LOW && lastChoco == HIGH) choice = 2;
  else if (bMilk == LOW && lastMilk == HIGH) choice = 3;
  lastBarnuts = bBarnuts;
  lastChoco = bChoco;
  lastMilk = bMilk;
  
  if (choice != 0) delay(20); // Minor debounce edge 
  return choice;
}
int readChoiceFromSerial() {
  if (!Serial.available()) return 0;
  String line = Serial.readStringUntil('\n');
  line.trim();
  if (line.length() == 0) return 0;
  if (line == "1" || line.equalsIgnoreCase("b") || line.equalsIgnoreCase("barn") || line.equalsIgnoreCase("barnuts")) return 1;
  if (line == "2" || line.equalsIgnoreCase("c") || line.equalsIgnoreCase("choco")) return 2;
  if (line == "3" || line.equalsIgnoreCase("m") || line.equalsIgnoreCase("milk")) return 3;
  if (line == "5" || line.equalsIgnoreCase("coin")) return 5;
  Serial.println("Invalid input. Use 1/2/3, choco/barnuts/milk, or 'coin'.");
  return 0;
}
void loop() {
  // 1: ALWAYS process fast pulses FIRST
  CheckCoin();
  // 2: Screen freeze handling without blocking CPU loop
  if (showingMsg) {
    if (millis() - msgTimer > 2000) {
      showingMsg = false;
      Choices(); // Automatically revert after 2 seconds safely
    }
    return; // Don't allow button presses while UI is flashing warning
  }
  // 3: Read choices
  int choice = readChoiceFromButtons();
  
  if (choice == 0) {
    int serialInput = readChoiceFromSerial();
    if (serialInput == 5) {
      currentBalance += 5;
      Serial.print("Coin inserted via Serial. Balance: ");
      Serial.print(currentBalance);
      Serial.println(" PHP");
      Choices();
    } else if (serialInput >= 1 && serialInput <= 3) {
      choice = serialInput;
    }
  }
  // 4: Handle Purchase Match
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
      
      // Start non-blocking timer instead of delay(2000)
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
  
  Serial.println("--- Vendo Machine ---");
  Serial.print("Balance: ");
  Serial.print(currentBalance);
  Serial.println(" PHP");
  Serial.println("1 = Barnuts, 2 = Choco, 3 = Milk");
  Serial.println("Insert coin (or type 'coin'), then select an item.");
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
// Replaces standard delay inside dispense functions to safely catch coins!
void safeDelay(unsigned long ms) {
  unsigned long start = millis();
  while (millis() - start < ms) {
    CheckCoin(); // Continuously listen for arriving coins even while waiting
    delay(5);
  }
}
bool waitForDrop(float threshold, unsigned long timeoutMs) {
  unsigned long start = millis();
  while (millis() - start < timeoutMs) {
    CheckCoin(); // Never miss a coin, even while waiting 3 seconds for it to fall
    if (scale.get_units() >= threshold) return true;
    delay(10); // Limits aggressive CPU load, but keeps checking fast
  }
  return false;
}
void Dispense(int choice) {
  const float dropThreshold = 6.0; 
  const unsigned long timeoutMs = 3000;
  // Dispense exactly 2 items
  for (int count = 1; count <= 2; count++) {
    scale.tare(); // tare scale before each drop
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
    
    // Delay slightly between drops, while reading coins
    safeDelay(1000); 
  }
}
