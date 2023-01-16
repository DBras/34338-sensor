#include <ESP8266WiFi.h>
#include <ThingSpeak.h>
#include <LiquidCrystal_I2C.h>
#include <espnow.h>

#define R_PIN D4
#define B_PIN D0
#define A_PIN A0
#define TOGGLE_BUTTON_PIN D6
#define MOBILE_BUTTON_PIN D3

// Incoming message structure
uint8_t broadcastAddress[] = { 0x84, 0xF3, 0xEB, 0x31, 0x2E, 0x16 };
typedef struct struct_message {
  float temperature;
  float humidity;
  float noiseLevel;
} struct_message;
// Outgoing message structure
typedef struct sent_struct_message {
  bool turnOn;
} sent_struct_message;
struct_message receivedData;
sent_struct_message sentData;

// ThingSpeak & WiFi connection parameters
unsigned long channelID = 2004080;
const char* APIKey = "A5HP1PM53222TH9M";
const char* ssid = "P7";
const char* pass = "dtuiot!!";
WiFiClient client;

// Define time intervals
const long intervalLCD = 100;
const long intervalButton = 100;
const long intervalChange = 3000;
unsigned long prevToggleButton = 0;
unsigned long prevMobileButton = 0;
unsigned long prevLCD = 0;
unsigned long prevRead = 0;
unsigned long analogChange = 0;

// Initiate display
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Button toggle setup
bool buttonToggle = false;
bool mobileButtonToggle = false;
bool allowToggleButton = true;
bool allowMobileButton = true;

// Limiter variables
int prevAnalogRead, currAnalogRead;
float temperatureThreshold = 25;
float noiseThreshold = 63;

// Ready to write to ThingSpeak
bool readyToWrite = false;

void onDataSent(uint8_t* mac_addr, uint8_t sendStatus) {
  // Print status of last packet
  Serial.print("Last Packet Send Status: ");
  if (sendStatus == 0) {
    Serial.println("Delivery success");
  } else {
    Serial.println("Delivery fail");
  }
}

// Function to run when data is received
void onDataRec(uint8_t* mac, uint8_t* incomingData, uint8_t len) {
  // Copy to structure, print data
  memcpy(&receivedData, incomingData, sizeof(receivedData));
  Serial.print("Temp: ");
  Serial.println(receivedData.temperature);
  Serial.print("Hum: ");
  Serial.println(receivedData.humidity);
  Serial.print("Noise: ");
  Serial.println(receivedData.noiseLevel);

  // React if levels are too high
  if (receivedData.temperature >= temperatureThreshold) {
    digitalWrite(R_PIN, HIGH);
  } else {
    digitalWrite(R_PIN, LOW);
  }

  // Flag ready to write to ThingSpeak
  readyToWrite = true;
}

void setup() {
  Serial.begin(115200);
  // LED pins
  pinMode(B_PIN, OUTPUT);
  pinMode(R_PIN, OUTPUT);
  // Potentiometer analog pin
  pinMode(A_PIN, INPUT);
  // Button pin
  pinMode(TOGGLE_BUTTON_PIN, INPUT_PULLUP);
  pinMode(MOBILE_BUTTON_PIN, INPUT_PULLUP);

  // Initialize the lcd
  lcd.init();
  lcd.backlight();

  // Initialize WiFi
  WiFi.mode(WIFI_AP_STA);
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.print("WiFi channel: ");
  Serial.println(WiFi.channel());
  // Initialize ESPNow
  if (esp_now_init() != 0) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  esp_now_set_self_role(ESP_NOW_ROLE_COMBO);
  esp_now_register_send_cb(onDataSent);
  esp_now_add_peer(broadcastAddress, ESP_NOW_ROLE_COMBO, 1, NULL, 0);
  esp_now_register_recv_cb(onDataRec);

  // Begin ThingSpeak
  ThingSpeak.begin(client);
}

void loop() {
  // Time tracking
  unsigned long curr = millis();

  // Write to ThingSpeak if data is ready
  if (readyToWrite) {
    ThingSpeak.setField(1, receivedData.temperature);
    ThingSpeak.setField(2, receivedData.humidity);
    ThingSpeak.setField(3, receivedData.noiseLevel);
    ThingSpeak.writeFields(channelID, APIKey);
    readyToWrite = false;
  }

  // Toggle button & blue LED
  if (digitalRead(TOGGLE_BUTTON_PIN) == false) {
    if (allowToggleButton == true) {
      buttonToggle = !buttonToggle;
      allowToggleButton = false;
      digitalWrite(B_PIN, !digitalRead(B_PIN));
    }
    prevToggleButton = curr;
  } else if (curr - prevToggleButton >= intervalButton) {
    allowToggleButton = true;
  }

  // Toggle mobile
  if (digitalRead(MOBILE_BUTTON_PIN) == false) {
    if (allowMobileButton == true) {
      mobileButtonToggle = !mobileButtonToggle;
      sentData.turnOn = mobileButtonToggle;
      Serial.print("Mobile status: ");
      Serial.println(mobileButtonToggle);
      esp_now_send(broadcastAddress, (uint8_t*)&sentData, sizeof(sentData));
      lcd.setCursor(0, 0);
      if (mobileButtonToggle) {
        lcd.print("Mobile on       ");
      } else {
        lcd.print("Mobile off      ");
      }
      lcd.setCursor(0, 1);
      lcd.print("                ");
      analogChange = curr;
      allowMobileButton = false;
    }
    prevMobileButton = curr;
  } else if (curr - prevMobileButton >= intervalButton) {
    allowMobileButton = true;
  }

  // Update display and set new thresholds
  if (curr - prevLCD >= intervalLCD) {
    prevLCD = curr;
    currAnalogRead = analogRead(A_PIN);

    // Detect change in potentiometer
    if (prevAnalogRead - currAnalogRead < -20
        || prevAnalogRead - currAnalogRead > 20) {
      prevAnalogRead = currAnalogRead;
      analogChange = curr;
      // Change temp or noise threshold depending on button toggle
      if (buttonToggle) {
        // Print new threshold to serial and LCD
        // Division by 100 to get range of 10
        temperatureThreshold = 20.0 + (currAnalogRead / 100);
        Serial.println(temperatureThreshold);
        lcd.setCursor(0, 0);
        lcd.print("Temp Threshold  ");
        lcd.setCursor(0, 1);
        lcd.print(temperatureThreshold);
      } else {
        // Division by 50 to get range of 20
        noiseThreshold = 63.0 + (currAnalogRead / 50);
        Serial.println(noiseThreshold);
        lcd.setCursor(0, 0);
        lcd.print("Noise Threshold ");
        lcd.setCursor(0, 1);
        lcd.print(noiseThreshold);
      }
    }
    // Display current readings on display
    else if (curr - analogChange >= intervalChange) {
      if (buttonToggle) {
        lcd.setCursor(0, 0);
        lcd.print("Current Temp    ");
        lcd.setCursor(0, 1);
        lcd.print(receivedData.temperature);
      } else {
        lcd.setCursor(0, 0);
        lcd.print("Current Noise   ");
        lcd.setCursor(0, 1);
        lcd.print(receivedData.noiseLevel);
      }
    }
  }
}
