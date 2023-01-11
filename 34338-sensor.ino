#include <ESP8266WiFi.h>
#include <ThingSpeak.h>
#include <LiquidCrystal_I2C.h>

const byte R_PIN = D4;
const byte G_PIN = D3;
const byte B_PIN = D0;
const byte A_PIN = A0;
const byte BUTTON_PIN = D6;

// WiFi connection
const char* ssid = "P7"; 
const char* pass = "dtuiot!!";
WiFiClient client;

// ThingSpeak connection parameters
unsigned long channelID = 2004080;
const char* APIKey = "JXCGVXICZ9YBRNLT";
const char* server = "api.thingspeak.com";

// Define time intervals
const long intervalLCD = 100;
const long intervalRead = 20000;
const long intervalButton = 1000;
unsigned long prevLCD = 0;
unsigned long prevRead = 0;
unsigned long prevButton = 0;

// Initiate display
LiquidCrystal_I2C lcd(0x27,16,2);

// Button toggle setup
bool buttonToggle = false;

// Potentiometer variables
int prevAnalogRead, currAnalogRead;
int temperatureThreshold = 25;

void setup() {
  Serial.begin(115200);
  // LED pins
  pinMode(B_PIN, OUTPUT);
  pinMode(G_PIN, OUTPUT);
  pinMode(R_PIN, OUTPUT);
  // Potentiometer analog pin
  pinMode(A_PIN, INPUT);
  // Button pin
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  // Initialize the lcd
  lcd.init();
  lcd.backlight();
  
  // Initialize WiFi & ThingSpeak connections
  WiFi.begin(ssid, pass);
  ThingSpeak.begin(client);
}

void loop() {
  // Time tracking
  unsigned long curr = millis();
  
  // Toggle button & red LED
  if(digitalRead(BUTTON_PIN) == false
      && prevButton - curr >= intervalButton){
    buttonToggle = !buttonToggle;
    prevButton = curr;
    digitalWrite(B_PIN, !digitalRead(B_PIN));
  }

  //ThingSpeak read
  if(curr-prevRead>=intervalRead){
    prevRead = curr;   

    // Read the first field, print if successful
    long temperature = ThingSpeak.readLongField(channelID, 1, APIKey);
    int statusCode = ThingSpeak.getLastReadStatus();
    if (statusCode == 200) {
      Serial.println(temperature);
      if (temperature >= temperatureThreshold) {
        digitalWrite(R_PIN, HIGH);
      } else {
        digitalWrite(R_PIN, LOW);
      }
    } else {
      Serial.print("An error occurred: ");
      Serial.println(statusCode);
    }

  }

  //LCD Write
  if(curr-prevLCD>=intervalLCD){
    prevLCD=curr;
    currAnalogRead = analogRead(A_PIN);

    // Detect change in potentiometer
    if (prevAnalogRead - currAnalogRead < -20
        || prevAnalogRead - currAnalogRead > 20) {
      prevAnalogRead = currAnalogRead;
      // Change temp or noise threshold depending on button toggle
      if (buttonToggle) {
        // Print new threshold to serial and LCD
        temperatureThreshold = 20.0 + (currAnalogRead / 100);
        Serial.println(temperatureThreshold);
        lcd.setCursor(0,0);
        lcd.print("Temp");
        lcd.setCursor(0,1);
        lcd.print(temperatureThreshold);
      } else {
        Serial.println("Changing sound limit");
        lcd.setCursor(0,0);
        lcd.print("Hum ");
        lcd.setCursor(0,1);
        lcd.print(65);
      }
    }
  }
}
