/*
 * SISTEMA DE CONTROLE INTELIGENTE - ARDUINO R4 WIFI
 * Atualização: AUTO-LOGOUT e CSV Uniforme
 */

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include <SD.h>       
#include <RTClib.h>
#include <MFRC522.h>
#include <DHT.h>
#include <WiFiS3.h>
#include <ArduinoJson.h>

#define DHT_PIN 2           
#define DHT_TYPE DHT11
#define SD_CS_PIN 4        
#define RST_PIN 9           
#define SS_PIN 10           

LiquidCrystal_I2C lcd(0x27, 16, 2);
DHT dht(DHT_PIN, DHT_TYPE);
MFRC522 rfid(SS_PIN, RST_PIN);
RTC_DS3231 rtc;

struct User {
  String uid;
  String name;
  byte age;
  bool isActive;
  unsigned long lastSeen;
};

User users[10] = {
  {"D2D54502", "Pai", 45, false, 0},
  {"9E5C3602", "Mae", 42, false, 0},
  {"D3C0D82C", "Filho", 16, false, 0}
};
const byte NUM_USERS = 3;
User* currentUser = nullptr;

enum LCDState { LCD_USER, LCD_FAN_ON, LCD_FAN_OFF, LCD_TEMP_HUM, LCD_WELCOME };
LCDState lcdState = LCD_WELCOME;
unsigned long lcdStateTimer = 0;
unsigned long lastTempDisplay = 0;
const unsigned long TEMP_DISPLAY_INTERVAL = 5000;
const unsigned long MESSAGE_DISPLAY_TIME = 2000;

byte grauChar[8] = {B00110, B01001, B01001, B00110, B00000, B00000, B00000, B00000};
byte fanOnChar[8] = {B00100, B10101, B01110, B11111, B01110, B10101, B00100, B00000};
byte fanOffChar[8] = {B00100, B00100, B00100, B11111, B00100, B00100, B00100, B00000};
byte userChar[8] = {B01110, B01110, B00100, B01110, B10101, B00100, B01010, B10001};

float currentTemp = 0;
float currentHumidity = 0;
unsigned long lastSensorRead = 0;
const unsigned long SENSOR_INTERVAL = 5000;
unsigned long lastUserCheck = 0;
const unsigned long USER_TIMEOUT = 300000;
bool fanState = false;
String commandBuffer = "";

unsigned long lastDataLog = 0;
const unsigned long LOG_INTERVAL = 300000; 

void setup() {
  Serial.begin(9600);
  
  lcd.init(); lcd.backlight();
  lcd.createChar(0, grauChar); lcd.createChar(1, fanOnChar);
  lcd.createChar(2, fanOffChar); lcd.createChar(3, userChar);
  
  SPI.begin();
  rfid.PCD_Init();
  dht.begin();
  if (!rtc.begin()) rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));

  lcd.clear(); lcd.print("SISTEMA PRONTO"); delay(2000); updateLCDDisplay();
}

void loop() {
  if (millis() - lastDataLog > LOG_INTERVAL) {
    logPeriodicData(); 
    lastDataLog = millis();
  }
  updateLCDDisplay();
  if (millis() - lastSensorRead > SENSOR_INTERVAL) {
    readSensors(); lastSensorRead = millis();
  }
  checkUserTimeout();
  checkRFID();
  processSerialCommands();
  delay(50);
}

// --- FUNÇÃO MODIFICADA: CHECK RFID COM AUTO-LOGOUT ---
void checkRFID() {
  if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) return;
  
  String uid = "";
  for (byte i = 0; i < rfid.uid.size; i++) {
    if (rfid.uid.uidByte[i] < 0x10) uid += "0";
    uid += String(rfid.uid.uidByte[i], HEX);
  }
  uid.toUpperCase();
  
  User* foundUser = nullptr;
  for (int i = 0; i < NUM_USERS; i++) {
    if (users[i].uid == uid) { foundUser = &users[i]; break; }
  }
  
  if (foundUser) {
    // 1. Cenário: A mesma pessoa passou o cartão para sair
    if (foundUser->isActive) {
      Serial.print("Logout manual: "); Serial.println(foundUser->name);
      showUserLogout(foundUser->name);
      logEvent("USER_LOGOUT", foundUser);
      foundUser->isActive = false;
      currentUser = nullptr;
    } 
    // 2. Cenário: Login de nova pessoa
    else {
      // *** MODIFICAÇÃO AQUI ***
      // Antes de logar o novo, verificamos se alguém (incluindo outros) estava logado
      for (int i = 0; i < NUM_USERS; i++) {
        if (users[i].isActive) {
          // Força o registo de logout do utilizador anterior
          logEvent("USER_LOGOUT", &users[i]);
          users[i].isActive = false;
          Serial.print("Auto-Logout de: "); Serial.println(users[i].name);
        }
      }
      
      // Agora fazemos o Login limpo
      Serial.print("Login: "); Serial.println(foundUser->name);
      foundUser->isActive = true;
      foundUser->lastSeen = millis();
      currentUser = foundUser;
      showUserLogin(foundUser->name);
      logEvent("USER_LOGIN", foundUser);
    }
  } else {
    Serial.print("Cartão desconhecido: "); Serial.println(uid);
  }
  
  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
}

// --- RESTO DAS FUNÇÕES MANTIDAS IGUAIS ---

void setupLCD() {
  // Já incluído no setup simplificado acima, mas mantido a estrutura
}

void updateLCDDisplay() {
  unsigned long currentTime = millis();
  if (currentTime - lastTempDisplay >= TEMP_DISPLAY_INTERVAL && lcdState == LCD_USER) {
    showTemperatureHumidity(); lastTempDisplay = currentTime; return;
  }
  if (lcdState == LCD_FAN_ON || lcdState == LCD_FAN_OFF || lcdState == LCD_TEMP_HUM) {
    if (currentTime - lcdStateTimer >= MESSAGE_DISPLAY_TIME) {
      lcdState = currentUser ? LCD_USER : LCD_WELCOME; displayCurrentState();
    }
    return;
  }
  if (lcdState == LCD_USER || lcdState == LCD_WELCOME) displayCurrentState();
}

void displayCurrentState() {
  lcd.clear();
  if (currentUser) {
    lcd.setCursor(0, 0); lcd.write(3); lcd.print(" "); lcd.print(currentUser->name);
    lcd.setCursor(14, 1); if (fanState) lcd.write(1); else lcd.print(" ");
    lcdState = LCD_USER;
  } else {
    lcd.setCursor(0, 0); lcd.print(" Passe o cartao ");
    lcd.setCursor(0, 1); lcd.print(" para entrar... ");
    lcdState = LCD_WELCOME;
  }
}

void showTemperatureHumidity() {
  lcd.clear();
  lcd.print("T:"); lcd.print(currentTemp, 1); lcd.write(0); lcd.print("C H:"); lcd.print(currentHumidity, 0); lcd.print("%");
  lcdState = LCD_TEMP_HUM; lcdStateTimer = millis();
}

void showFanOn() {
  lcd.clear(); lcd.setCursor(0,0); lcd.write(1); lcd.print(" FAN ON"); lcdState = LCD_FAN_ON; lcdStateTimer = millis();
}

void showFanOff() {
  lcd.clear(); lcd.setCursor(0,0); lcd.write(2); lcd.print(" FAN OFF"); lcdState = LCD_FAN_OFF; lcdStateTimer = millis();
}

void showUserLogin(String userName) {
  lcd.clear(); lcd.print("Ola "); lcd.print(userName); delay(1500); lcdState = LCD_USER;
}

void showUserLogout(String userName) {
  lcd.clear(); lcd.print("Adeus "); lcd.print(userName); delay(1500); lcdState = LCD_WELCOME;
}

void readSensors() {
  float h = dht.readHumidity(); float t = dht.readTemperature();
  if (!isnan(h) && !isnan(t)) { currentTemp = t; currentHumidity = h; }
}

void checkUserTimeout() {
  if (millis() - lastUserCheck < 60000) return;
  lastUserCheck = millis();
  for (int i = 0; i < NUM_USERS; i++) {
    if (users[i].isActive && (millis() - users[i].lastSeen > USER_TIMEOUT)) {
      logEvent("USER_TIMEOUT", &users[i]);
      users[i].isActive = false;
      if (currentUser == &users[i]) { currentUser = nullptr; displayCurrentState(); }
    }
  }
}

void processSerialCommands() {
  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\n') { processCommand(commandBuffer); commandBuffer = ""; }
    else commandBuffer += c;
  }
}

void processCommand(String cmd) {
  cmd.trim();
  if (cmd == "FAN_ON") { fanState = true; showFanOn(); } 
  else if (cmd == "FAN_OFF") { fanState = false; showFanOff(); }
}

void sendToPC(DateTime& now, User* user, String action) {
  Serial.print("CSV_DATA,"); 
  Serial.print(now.unixtime()); Serial.print(",");
  Serial.print(user ? user->uid : "SYSTEM"); Serial.print(",");
  Serial.print(user ? user->name : "Ninguem"); Serial.print(",");
  Serial.print(action); Serial.print(",");
  Serial.print(currentTemp); Serial.print(",");
  Serial.print(currentHumidity); Serial.print(",");
  Serial.println(fanState ? "1" : "0"); 
}

void logEvent(String action, User* user) {
  DateTime now = rtc.now();
  sendToPC(now, user, action);
  if (user) user->lastSeen = millis();
}

void logPeriodicData() {
  readSensors();
  DateTime now = rtc.now();
  sendToPC(now, currentUser, "AUTO_LOG");
}