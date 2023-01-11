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
unsigned long prevLCD = 0;
unsigned long prevRead = 0;

// Initiate display
LiquidCrystal_I2C lcd(0x27,16,2);

// Button toggle setup
int ButtonToggle = 0;
int ButtonIndex=0;

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
  //Time tracking
  unsigned long curr = millis();
  
  //Button
  if(digitalRead(BUTTON_PIN) == false){
    if(ButtonIndex == 0){
      ButtonToggle++;
      ButtonToggle = ButtonToggle%2;
      digitalWrite(R_PIN,!digitalRead(R_PIN));
      ButtonIndex = 1;
    }
  }
  else{
    ButtonIndex = 0;
  }

  //ThingSpeak read
  if(curr-prevRead>=intervalRead){
    prevRead = curr;   

    // Read the first field, print if successful
    long temperature = ThingSpeak.readLongField(channelID, 1, APIKey);
    int statusCode = ThingSpeak.getLastReadStatus();
    if (statusCode == 200) {
      Serial.println(temperature);
    } else {
      Serial.print("An error occurred: ");
      Serial.println(statusCode);
    }

  }

  //LCD Write
  if(curr-prevLCD>=intervalLCD){
    prevLCD=curr;
    currAnalogRead = analogRead(A_PIN);

    if (prevAnalogRead - currAnalogRead < -20
        || prevAnalogRead - currAnalogRead > 20) {
      temperatureThreshold = 20 + (currAnalogRead / 100);
      Serial.println(temperatureThreshold);
      prevAnalogRead = currAnalogRead;
    }

    if(analogRead(A_PIN)<=400){
      lcd.setCursor(0,1);
      lcd.print("LOW ");
      digitalWrite(B_PIN, HIGH);
      digitalWrite(G_PIN, LOW);
    }
    else{
      lcd.setCursor(0,1);
      lcd.print("HIGH");
      digitalWrite(G_PIN, HIGH);
      digitalWrite(B_PIN, LOW);
    }
    lcd.setCursor(0,0);
    lcd.print("Act");
  }
}
