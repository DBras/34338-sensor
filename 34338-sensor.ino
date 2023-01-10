#include <ESP8266WiFi.h>
#include <ThingSpeak.h>
#include <LiquidCrystal_I2C.h>

//WiFi connection
const char* ssid = "InderNettet"; 
const char* pass = "#ByensBedsteKager";
WiFiClient client;

//ThingSpeak connection parameters
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

void setup() {
  Serial.begin(115200);
  //LED pins: R=D4, G=D3, B=D0
  pinMode(D0, OUTPUT);
  pinMode(D3, OUTPUT);
  pinMode(D4, OUTPUT);
  //Potentiometer analog A0
  pinMode(A0, INPUT);
  //Button D6
  pinMode(D6, INPUT_PULLUP);

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
  if(digitalRead(D6) == false){
    if(ButtonIndex == 0){
      ButtonToggle++;
      ButtonToggle = ButtonToggle%2;
      digitalWrite(D4,!digitalRead(D4));
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
    if(analogRead(A0)<=400){
      lcd.setCursor(0,1);
      lcd.print("LOW ");
      digitalWrite(D0, HIGH);
      digitalWrite(D3, LOW);
    }
    else{
      lcd.setCursor(0,1);
      lcd.print("HIGH");
      digitalWrite(D3, HIGH);
      digitalWrite(D0, LOW);
    }
    lcd.setCursor(0,0);
    lcd.print("Act");
  }
}
