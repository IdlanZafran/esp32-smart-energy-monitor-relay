#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFi.h>
#include <WebServer.h>
#include <WiFiManager.h>
#include <LittleFS.h>
#include <time.h> 

// --- Web Server on port 80 ---
WebServer server(80);

// --- OLED Display Settings ---
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define OLED_RESET    -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// --- Pin Definitions ---
const int ZMPT_PIN = 34;  
const int ACS_PIN = 35;   
const int RELAY_PIN = 23; 

// Constants
const float mV_PER_AMP = 185.0; 
const float VOLTAGE_CALIBRATION = 656.4; 
const float ADC_VREF = 3.3; 
const int ADC_RESOLUTION = 4095;

// --- Global Variables ---
float voltageRMS = 0.0;
float current_mA = 0.0;   // Changed to store mA
float powerW = 0.0;      
bool relayState = false; 

// --- Boot Counter Variables ---
bool bootCountReset = false;
const unsigned long BOOT_TIMEOUT = 1500; 

// --- Timers ---
unsigned long previousMillis = 0;
const long interval = 1000; 

// WiFi Reconnect Timer Variables
unsigned long previousWiFiMillis = 0;
const unsigned long WIFI_CHECK_INTERVAL = 10000UL; // 10 seconds

// --- NTP Time Settings ---
const long  gmtOffset_sec = 8 * 3600; 
const int   daylightOffset_sec = 0;   
const char* ntpServer = "pool.ntp.org";

// --- Custom Icons (8x8 pixels) ---
const unsigned char wifi_on_bmp[] PROGMEM = {
  0b00111100, 
  0b01000010, 
  0b10011001, 
  0b00100100, 
  0b01000010, 
  0b00011000, 
  0b00000000, 
  0b00011000  
};

const unsigned char wifi_off_bmp[] PROGMEM = {
  0b10000001, 
  0b01000010, 
  0b00100100, 
  0b00011000, 
  0b00011000, 
  0b00100100, 
  0b01000010, 
  0b10000001  
};

const unsigned char relay_on_bmp[] PROGMEM = {
  0b00111100, 
  0b01111110, 
  0b11111111, 
  0b11111111, 
  0b11111111, 
  0b11111111, 
  0b01111110, 
  0b00111100  
};

const unsigned char relay_off_bmp[] PROGMEM = {
  0b00111100, 
  0b01000010, 
  0b10000001, 
  0b10000001, 
  0b10000001, 
  0b10000001, 
  0b01000010, 
  0b00111100  
};

// Forward declarations
float getVoltageRMS();
float getCurrentRMS();

void setup() {
  Serial.begin(115200);

  // Initialize Relay Pin
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW); 
  
  Wire.begin(21, 22);
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }

  // --- LittleFS Boot Counter Logic ---
  if (!LittleFS.begin(true)) { 
    Serial.println("LittleFS Mount Failed");
  }

  int bootCount = 0;
  File readFile = LittleFS.open("/boot_count.txt", "r");
  if (readFile) {
    bootCount = readFile.readString().toInt();
    readFile.close();
  }

  bootCount++; 
  Serial.printf("Boot Count: %d\n", bootCount);

  // --- 1. DISPLAY BOOT COUNT FIRST (0.5 Seconds) ---
  display.clearDisplay();
  display.setTextSize(3); 
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(55, 4); 
  display.print(bootCount);
  display.display();
  delay(500); 

  // --- 2. CHECK FOR RAPID BOOTS (Reset Logic) ---
  if (bootCount >= 3) {
    Serial.println("3 rapid boots detected! Resetting WiFi...");
    
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(28, 12); 
    display.println("Resetting...");
    display.display();
    delay(1500); 

    WiFiManager wifiManager;
    wifiManager.resetSettings(); 
    
    bootCount = 0; 
  }

  File writeFile = LittleFS.open("/boot_count.txt", "w");
  if (writeFile) {
    writeFile.print(String(bootCount));
    writeFile.close();
  }

  // --- 3. CUTE ROBOT FACE ANIMATION ---
  
  display.clearDisplay();
  display.drawRoundRect(42, 4, 12, 18, 4, SSD1306_WHITE); 
  display.drawRoundRect(74, 4, 12, 18, 4, SSD1306_WHITE); 
  display.fillRoundRect(56, 20, 16, 8, 4, SSD1306_WHITE);
  display.display();
  delay(1200);

  display.clearDisplay();
  display.drawRoundRect(42, 10, 12, 6, 2, SSD1306_WHITE); 
  display.drawRoundRect(74, 10, 12, 6, 2, SSD1306_WHITE); 
  display.fillRoundRect(56, 20, 16, 8, 4, SSD1306_WHITE); 
  display.display();
  delay(150);

  display.clearDisplay();
  display.drawLine(42, 13, 54, 13, SSD1306_WHITE); 
  display.drawLine(74, 13, 86, 13, SSD1306_WHITE); 
  display.fillRoundRect(56, 20, 16, 8, 4, SSD1306_WHITE);
  display.display();
  delay(500);

  display.clearDisplay();
  display.drawRoundRect(42, 10, 12, 6, 2, SSD1306_WHITE); 
  display.drawRoundRect(74, 10, 12, 6, 2, SSD1306_WHITE); 
  display.fillRoundRect(56, 20, 16, 8, 4, SSD1306_WHITE);
  display.display();
  delay(150);
  
  display.clearDisplay();
  display.drawRoundRect(42, 4, 12, 18, 4, SSD1306_WHITE); 
  display.drawRoundRect(74, 4, 12, 18, 4, SSD1306_WHITE); 
  display.fillRoundRect(56, 20, 16, 8, 4, SSD1306_WHITE);
  display.display();
  delay(1200);

  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("Connect to WiFi AP:");
  display.setCursor(0, 15); 
  display.setTextSize(2); 
  display.println("Smart Plug");
  display.display();

  WiFiManager wifiManager;
  
  if (!wifiManager.autoConnect("Smart Plug")) {
    Serial.println("Failed to connect");
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("WiFi Failed!");
    display.setCursor(0, 10);
    display.println("Restarting...");
    display.display();
    delay(3000);
    ESP.restart(); 
  }
  
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("WiFi Connected!");
  display.setCursor(0, 10);
  display.println(WiFi.localIP());
  display.display();
  delay(3000);

  previousWiFiMillis = millis();
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  server.on("/", []() {
    File file = LittleFS.open("/index.html", "r");
    if (!file) {
      server.send(500, "text/plain", "Failed to open file");
      return;
    }
    server.streamFile(file, "text/html");
    file.close();
  });  
  // JSON Endpoint updated for mA
  server.on("/data", []() {
    String json = "{\"voltage\":\"" + String(voltageRMS, 1) + 
                  "\", \"current\":\"" + String(current_mA, 1) + 
                  "\", \"power\":\"" + String(powerW, 1) + 
                  "\", \"relay\":" + (relayState ? "true" : "false") + "}";
    server.send(200, "application/json", json);
  });

  server.on("/toggle", []() {
    relayState = !relayState;
    digitalWrite(RELAY_PIN, relayState ? HIGH : LOW);
    server.send(200, "text/plain", "OK");
  });

  server.begin();
}

void loop() {
  server.handleClient();
  unsigned long currentMillis = millis();

  if (!bootCountReset && millis() > BOOT_TIMEOUT) {
    bootCountReset = true;
    File writeFile = LittleFS.open("/boot_count.txt", "w");
    if (writeFile) {
      writeFile.print("0");
      writeFile.close();
    }
  }

// --- WiFi Auto-Reconnect Logic ---
  if (WiFi.status() != WL_CONNECTED) {
    // If disconnected, try to reconnect every 10 seconds
    if (currentMillis - previousWiFiMillis >= WIFI_CHECK_INTERVAL) {
      Serial.println("WiFi disconnected. Attempting to reconnect...");
      WiFi.disconnect(); // Clear any hung states
      WiFi.reconnect();  // Attempt to reconnect using saved credentials
      previousWiFiMillis = currentMillis; 
    }
  } else {
    // If connected, keep the timer matching currentMillis
    // This ensures that if WiFi drops, it waits exactly 10s before the first attempt
    previousWiFiMillis = currentMillis; 
  }

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    // --- SENSOR UPDATES ---
    if (relayState) {
      voltageRMS = getVoltageRMS();
      current_mA = getCurrentRMS();
      
      // Calculate Power in Watts (Voltage * Amps)
      powerW = voltageRMS * (current_mA / 1000.0); 
    } else {
      voltageRMS = 0.0;
      current_mA = 0.0;
      powerW = 0.0;
    }

    bool wifiConnected = (WiFi.status() == WL_CONNECTED);

    struct tm timeinfo;
    char timeString[6] = "--:--";
    if (getLocalTime(&timeinfo, 10)) { 
      strftime(timeString, sizeof(timeString), "%H:%M", &timeinfo);
    }

    display.clearDisplay();
    display.setTextSize(1);
    
    if (relayState) {
      display.drawBitmap(0, 0, relay_on_bmp, 8, 8, SSD1306_WHITE);
    } else {
      display.drawBitmap(0, 0, relay_off_bmp, 8, 8, SSD1306_WHITE);
    }
    
    if (wifiConnected) {
      IPAddress ip = WiFi.localIP();
      display.setCursor(14, 0);
      display.printf("%d.%d", ip[2], ip[3]); 
    } else {
      display.setCursor(14, 0);
      display.print("No IP");
    }

    display.setCursor(84, 0);
    display.print(timeString);
    
    if (wifiConnected) {
      display.drawBitmap(118, 0, wifi_on_bmp, 8, 8, SSD1306_WHITE);
    } else {
      display.drawBitmap(118, 0, wifi_off_bmp, 8, 8, SSD1306_WHITE);
    }
    
    display.setCursor(0, 12); 
    display.printf("Voltage: %.1f V", voltageRMS);

    // Updated OLED formatting for mA
    display.setCursor(0, 24); 
    display.printf("I: %.1f mA", current_mA); 
    
    display.setCursor(72, 24); // Shifted slightly right to fit longer mA string
    display.printf("P: %.1f W", powerW); 
    
    display.display();
  }
}

float getVoltageRMS() {
  uint32_t start_time = millis();
  uint64_t sum_sq = 0; 
  uint64_t sum = 0;    // FIX: Changed to 64-bit to prevent overflow
  uint32_t count = 0;
  
  while ((millis() - start_time) < 100) {
    uint32_t sensorValue = analogRead(ZMPT_PIN);
    sum += sensorValue;
    sum_sq += (uint64_t)sensorValue * sensorValue;
    count++;
  }
  
  if (count == 0) return 0.0;
  
  double mean = (double)sum / count;
  double mean_sq = (double)sum_sq / count;
  double variance = mean_sq - (mean * mean);
  if (variance < 0) variance = 0;
  
  double adc_rms = sqrt(variance);
  
  float sensor_Vrms = (adc_rms * ADC_VREF) / ADC_RESOLUTION; 
  float actual_Vrms = sensor_Vrms * VOLTAGE_CALIBRATION;
  
  //if (actual_Vrms < 1.0) actual_Vrms = 0.0; 
  
  return actual_Vrms;
}

float getCurrentRMS() {
  uint32_t start_time = millis();
  uint64_t sum_sq = 0; 
  uint64_t sum = 0;    // FIX: Changed to 64-bit to prevent overflow
  uint32_t count = 0;
  
  while ((millis() - start_time) < 100) {
    uint32_t sensorValue = analogRead(ACS_PIN);
    sum += sensorValue;
    sum_sq += (uint64_t)sensorValue * sensorValue;
    count++;
  }
  
  if (count == 0) return 0.0;
  
  double mean = (double)sum / count;
  double mean_sq = (double)sum_sq / count;
  double variance = mean_sq - (mean * mean);
  if (variance < 0) variance = 0;
  
  double adc_rms = sqrt(variance);
  
  float sensor_Vrms = (adc_rms * ADC_VREF) / ADC_RESOLUTION; 
  float actual_Irms = sensor_Vrms / (mV_PER_AMP / 1000.0); // Math yields Amps
  
  float actual_ImA = actual_Irms * 1000.0; // Convert to mA
  
  // FIX: Deadband lowered to 10mA to allow your 45mA lamp to pass!
  if (actual_ImA < 38.0) actual_ImA = 0.0; 
  
  return actual_ImA;
}