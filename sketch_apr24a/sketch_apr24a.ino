#include <LiquidCrystal.h>


LiquidCrystal lcd(7, 8, 9, 10, 11, 12);


const int motorBtnPin    = 2;
const int beltSwitchPin  = 3;
const int ldrPin = A2;
const int blueLedPin = 22; 
const int potPin = A3;
const int yellowLedPin = 23;
const int redLedPin      = 4;
const int buzzerPin      = 5;
const int motorRelayPin  = 6;
const int doorSwitchPin = 24;    // Kapı anahtarı
const int pinkLedRedPin  = 25;   // RGB LED'in kırmızı bacağı
const int pinkLedBluePin = 26; 
const int tempSensorPin  = A0;  // LM35 çıkışı
const int fanMotorPin    = A1;  // Klima fanı (DC motor simülasyonu)

// Durum değişkenleri
bool motorRunning       = false;
bool motorButtonPressed = false;
bool fanRunning         = false;
bool activeBeltWarning = false;

void updateLCD(String line1, String line2 = "", int delayTime = 0) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(line1);
  if (line2 != "") {
    lcd.setCursor(0, 1);
    lcd.print(line2);
  }
  if (delayTime > 0) {
    delay(delayTime);
    lcd.clear();
  }
}

void startMotor() {
  motorRunning = true;
  digitalWrite(motorRelayPin, HIGH);
  updateLCD("Motor Calisti");
}

void stopMotor() {
  motorRunning = false;
  digitalWrite(motorRelayPin, LOW);
}

void activateWarning() {
  digitalWrite(redLedPin, HIGH);
  digitalWrite(buzzerPin, HIGH);
  if(activeBeltWarning ){
    return;
  } 
  updateLCD("Emniyet Kemeri", "Takili Degil!");
  activeBeltWarning = true;
}

void clearWarning() {
  digitalWrite(redLedPin, LOW);
  digitalWrite(buzzerPin, LOW);
  if(!activeBeltWarning){
    return;
  }
  lcd.clear();
  activeBeltWarning = false;
}

float readTemperature() {
  int raw = analogRead(tempSensorPin);
  float voltage = raw * (5.0 / 1023.0);
  return voltage * 100.0; // LM35: 10mV/°C
}

void turnOnFan(float temp) {
  if (!fanRunning) {
    digitalWrite(fanMotorPin, HIGH);
    fanRunning = true;
  }
  bool beltOn = digitalRead(beltSwitchPin) == LOW;
  if (beltOn) {
    updateLCD("Sicaklik: " + String(temp, 1) + (char)223 + "C", "Klima Acildi", 500);
  }
}
void turnOffFan() {
  if (fanRunning) {
    digitalWrite(fanMotorPin, LOW);
    fanRunning = false;
   
    if (digitalRead(beltSwitchPin) == LOW) {
      lcd.clear();
    }
  }
}

void checkHeadlights() {
  int lightLevel = analogRead(ldrPin);
  if (lightLevel <= 250) {
    digitalWrite(blueLedPin, HIGH);
    updateLCD("Farlar Acik", "" ,500);
  } else {
    digitalWrite(blueLedPin, LOW);
    updateLCD("Farlar Kapandi", "",500);
  }
}
bool checkYLed= false;
void checkFuelLevel() {
  int potValue = analogRead(potPin);                
  int fuelPercent = map(potValue, 0, 1023, 0, 100);  

  if (fuelPercent == 0) {
    stopMotor();
    digitalWrite(yellowLedPin, LOW);
    turnOffFan();
    digitalWrite(blueLedPin, LOW);
    digitalWrite(redLedPin, LOW);
    digitalWrite(pinkLedRedPin, LOW);
    digitalWrite(pinkLedBluePin, LOW);
    updateLCD("Yakıt Bitti!", "Motor Durdu",1000);
  } else if (fuelPercent < 5) {
    if (checkYLed) {
      digitalWrite(yellowLedPin, HIGH);
      checkYLed = false;
    } else {
      digitalWrite(yellowLedPin, LOW);
      checkYLed = true;

    }
    updateLCD("Kritik: Yakit ", "Az %" + String(fuelPercent), 500);
  } else if (fuelPercent < 10) {
    digitalWrite(yellowLedPin, HIGH);
    updateLCD("Uyari: Yakit", "Dusuk %" + String(fuelPercent), 500);
  } else {
    digitalWrite(yellowLedPin, LOW);
  }
}
void checkDoorStatus() {
  bool doorOpen = digitalRead(doorSwitchPin) == HIGH;
  
  if (doorOpen) {
    digitalWrite(pinkLedRedPin, HIGH);
    digitalWrite(pinkLedBluePin, HIGH);
    digitalWrite(yellowLedPin, LOW);
    turnOffFan();
    digitalWrite(blueLedPin, LOW);
    digitalWrite(redLedPin, LOW);
    updateLCD("Uyari: Kapi Acik", "Motor Calismaz", 200);
    motorButtonPressed = true; 
    stopMotor();
  } else {
    digitalWrite(pinkLedRedPin, LOW);
    digitalWrite(pinkLedBluePin, LOW);
    motorButtonPressed = false; 
  }
}
void setup() {
  pinMode(motorBtnPin,   INPUT_PULLUP);
  pinMode(beltSwitchPin, INPUT_PULLUP);
  pinMode(redLedPin,     OUTPUT);
  pinMode(buzzerPin,     OUTPUT);
  pinMode(yellowLedPin, OUTPUT);
  pinMode(doorSwitchPin, INPUT_PULLUP);
  pinMode(pinkLedRedPin, OUTPUT);  
  pinMode(pinkLedBluePin, OUTPUT);
  pinMode(motorRelayPin, OUTPUT);
  pinMode(blueLedPin, OUTPUT);
  pinMode(ldrPin, INPUT);
  pinMode(tempSensorPin, INPUT); 
  pinMode(fanMotorPin,   OUTPUT);
  lcd.begin(16, 2);
  stopMotor();
  clearWarning();
  turnOffFan();
}

void loop() {
  checkDoorStatus(); 
  bool doorClosed = digitalRead(doorSwitchPin) == LOW;

  if (!doorClosed) {
    stopMotor(); 
    return; 
  }
  bool beltOn       = digitalRead(beltSwitchPin) == LOW;
  bool motorBtnDown = digitalRead(motorBtnPin)    == LOW;

  if (motorBtnDown && !motorButtonPressed && !motorRunning) {
    motorButtonPressed = true; 

    if (!beltOn) {
      stopMotor();
      activateWarning();
    } else {
      clearWarning(); 

      int potValue = analogRead(potPin);                
      int fuelPercent = map(potValue, 0, 1023, 0, 100);  

      if (fuelPercent == 0) {
        
        stopMotor();
        updateLCD("Yakit Bitti", "Motor Calisamaz");
      } else {
        startMotor();
      }
    }
  }

 
  if (!motorBtnDown) {
    motorButtonPressed = false;
  }
  if (!beltOn && motorRunning) {
    activateWarning();
    return;
  } 
  if (beltOn ) {
    clearWarning();
  }
  
  if (motorRunning) {
    float currentTemp = readTemperature();
    if (currentTemp > 25.0) {
      turnOnFan(currentTemp);
    } else {
      turnOffFan();
    }
    checkFuelLevel();
    checkHeadlights();
  }

}