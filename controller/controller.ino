/*
  34338 Baby Monitor Controller

  This acts as a controller unit for the baby monitor system.

  This runs on an ESP-8266. It expects 2 LEDs, 2 buttons, one potentiometer,
  and one LCD screen of size 16x2. It is also necessary to have an open WiFi-hotspot.
  The credentials for this can be changed below.
  Data is received from another ESP running the sensor modules. To send on/off signals
  for the mobile connected to the sensor-ESP, the MAC-address of this ESP must be changed
  below.
  When data is received, it is uploaded to a ThingSpeak channel with ID and APIKey
  which can be changed below.

  Modified 17 Jan 2023

  GitHub URL:
  https://github.com/DBras/34338-sensor/
*/

#include <ESP8266WiFi.h>
#include <ThingSpeak.h>
#include <LiquidCrystal_I2C.h>
#include <espnow.h>

#define R_PIN D4
#define B_PIN D0
#define A_PIN A0
#define BUZZER_PIN D7
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
const long buzzerTimer = 1000;
unsigned long currentTime;
unsigned long prevToggleButton = 0;
unsigned long prevMobileButton = 0;
unsigned long prevLCD = 0;
unsigned long prevRead = 0;
unsigned long analogChange = 0;
unsigned long buzzerStartTime = 0;

// Initiate display
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Button toggle setup
bool buttonToggle = false;
bool mobileButtonToggle = false;
bool allowToggleButton = true;
bool allowMobileButton = true;

// Limiter variables
int prevAnalogRead, currentAnalogRead;
float temperatureThreshold = 25;
float noiseThreshold = 63;

// Ready to write to ThingSpeak
bool readyToWrite = false;

// Buzzer initialization
bool buzz = false;

//Write to LCD with float
void writeLCD(String topLine, float bottomLine){
  lcd.setCursor(0, 1);
  if (bottomLine != NULL) {
    lcd.print(bottomLine);
  } else {
    lcd.clear();
  }
  for (int i = topLine.length(); i <= 16; i++) {
    topLine += " ";
  }
  lcd.setCursor(0, 0);
  lcd.print(topLine);
}

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
  
  //Buzzer for an amount of time
  if (receivedData.noiseLevel >= noiseThreshold) {
    buzz = true;
    buzzerStartTime = currentTime;
  } else {
    buzz = false;
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
  // Buzzer pin
  pinMode(BUZZER_PIN, OUTPUT);

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
  currentTime = millis();

  // Buzzer time controller
  if (buzz) {
    if (currentTime - buzzerStartTime <= buzzerTimer) {
      digitalWrite(BUZZER_PIN, HIGH);
    } else {
      buzz = false;
      digitalWrite(BUZZER_PIN, LOW);
    }
  }

  // Toggle button & blue LED
  if (digitalRead(TOGGLE_BUTTON_PIN) == false) {
    if (allowToggleButton == true) {
      buttonToggle = !buttonToggle;
      allowToggleButton = false;
      digitalWrite(B_PIN, !digitalRead(B_PIN));
    }
    prevToggleButton = currentTime;
  } else if (currentTime - prevToggleButton >= intervalButton) {
    allowToggleButton = true;
  }

  // Toggle mobile
  if (digitalRead(MOBILE_BUTTON_PIN) == false) {
    if (allowMobileButton == true) {
      // Toggle and send to other ESP
      mobileButtonToggle = !mobileButtonToggle;
      sentData.turnOn = mobileButtonToggle;
      Serial.print("Mobile status: ");
      Serial.println(mobileButtonToggle);
      esp_now_send(broadcastAddress, (uint8_t*)&sentData, sizeof(sentData));
      // Write to LCD
      if (mobileButtonToggle) {
        writeLCD("Mobile on", NULL);
      } else {
        writeLCD("Mobile off", NULL);
      }
      analogChange = currentTime;
      allowMobileButton = false;
    }
    prevMobileButton = currentTime;
  } else if (currentTime - prevMobileButton >= intervalButton) {
    allowMobileButton = true;
  }

  // Update display and set new thresholds
  if (currentTime - prevLCD >= intervalLCD) {
    prevLCD = currentTime;
    currentAnalogRead = analogRead(A_PIN);

    // Detect change in potentiometer
    if (prevAnalogRead - currentAnalogRead < -20
        || prevAnalogRead - currentAnalogRead > 20) {
      prevAnalogRead = currentAnalogRead;
      analogChange = currentTime;
      // Change temp or noise threshold depending on button toggle
      if (buttonToggle) {
        // Print new threshold to serial and LCD
        // Division by 100 to get range of 10
        temperatureThreshold = 20.0 + (currentAnalogRead / 100);
        Serial.println(temperatureThreshold);
        writeLCD("Temp Threshold", temperatureThreshold);
      } else {
        // Division by 50 to get range of 20
        noiseThreshold = 63.0 + (currentAnalogRead / 50);
        Serial.println(noiseThreshold);
        writeLCD("Noise Threshold", noiseThreshold);
      }
    }
    // Display current readings on display
    else if (currentTime - analogChange >= intervalChange) {
      if (buttonToggle) {
        writeLCD("Current Temp", receivedData.temperature);
      } else {
        writeLCD("Current Noise", receivedData.noiseLevel);
      }
    }
  }

  // Write to ThingSpeak if data is ready
  if (readyToWrite) {
    ThingSpeak.setField(1, receivedData.temperature);
    ThingSpeak.setField(2, receivedData.humidity);
    ThingSpeak.setField(3, receivedData.noiseLevel);
    ThingSpeak.writeFields(channelID, APIKey);
    readyToWrite = false;
  }
}
