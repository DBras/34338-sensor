/*

*/

#include <espnow.h>
#include <ESP8266WiFi.h>
#include <ThingSpeak.h>
#include "DHT.h"

#define DHTPIN D2
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

//Set output pins
int noisePin = A0;
int ledPin = 13;

//Setting initial values
int tempValue = 0;
int noiseValue = 0;
int tickets = 0;
float avgtemp = 0;
float avgnoise = 0;
float avghum = 0;

//WiFi connection parameters
WiFiClient client;
const char* ssid = "P7";
const char* pass = "dtuiot!!";

//Message sending structure
uint8_t broadcastAddress[] = { 0x84, 0xF3, 0xEB, 0x31, 0xD0, 0x74 };
typedef struct struct_message {
  float avgtemp = 0;
  float avghum = 0;
  float avgnoise = 0;
} struct_message;

//Incoming message structure
typedef struct receive_struct_message {
  bool turnOn;
} receive_struct_message;
struct_message myData;
receive_struct_message receivedData;

//function finding controller WiFi channel
int32_t getWiFiChannel(const char* ssid) {
  if (int32_t n = WiFi.scanNetworks()) {
    for (uint8_t i = 0; i < n; i++) {
      if (!strcmp(ssid, WiFi.SSID(i).c_str())) {
        return WiFi.channel(i);
      }
    }
  }
  return 0;
}

//Sent data status message
void OnDataSent(uint8_t* mac_addr, uint8_t sendStatus) {
  Serial.print("Last Packet Send Status: ");
  if (sendStatus == 0) {
    Serial.println("Delivery success");
  } else {
    Serial.println("Delivery fail");
  }
}

//Send signal to sensor arduino
void onDataRec(uint8_t * mac, uint8_t *incomingData, uint8_t len) {
  memcpy(&receivedData, incomingData, sizeof(receivedData));
  Serial.print("Received data");
  if (receivedData.turnOn) {
    digitalWrite(D7, HIGH);
  } else {
    digitalWrite(D7, LOW);
  }
}

void setup() {
  //Initialize WiFi
  WiFi.mode(WIFI_STA);
  int32_t channel = getWiFiChannel(ssid);
  WiFi.printDiag(Serial);
  wifi_promiscuous_enable(1);
  wifi_set_channel(channel);
  wifi_promiscuous_enable(0);
  WiFi.printDiag(Serial);
  
  dht.begin();
  Serial.begin(115200);
  
  pinMode(ledPin, OUTPUT);
  pinMode(D7, OUTPUT);
  

  // Initialize ESP-NOW
  if (esp_now_init() != 0) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Once ESPNow is successfully Initialized, we will register for Send CB to
  // get the status of Trasnmitted packet
  esp_now_set_self_role(ESP_NOW_ROLE_COMBO);
  esp_now_register_send_cb(OnDataSent);

  // Register peer
  esp_now_add_peer(broadcastAddress, ESP_NOW_ROLE_COMBO, 1, NULL, 0);

  esp_now_register_recv_cb(onDataRec);
}

void loop() {
  //Reading sensors
  delay(20);
  noiseValue = analogRead(noisePin);
  float CurrentHumidity = dht.readHumidity();
  float CurrentTemperature = dht.readTemperature();
  
  //Adding current sensor values to previous sensor values
  avgtemp = avgtemp + CurrentTemperature;
  avgnoise = avgnoise + noiseValue;
  avghum = avghum + CurrentHumidity;

  //Check how meny values were added and averaging over them in sets of 500
  if (tickets == 500) {
    //Averaging calculations    
    avgtemp = avgtemp / 500;
    avghum = avghum / 500;
    avgnoise = avgnoise / 500;

    //Printing data to serial monitor    
    Serial.print("Current temp is: ");
    Serial.println(CurrentTemperature);
    Serial.print("Current noise is: ");
    Serial.println(noiseValue);
    Serial.print("Current Humidity is: ");
    Serial.println(CurrentHumidity);

    Serial.print("Temperature is: ");
    Serial.println(avgtemp);
    Serial.print("Noise is: ");
    Serial.println(avgnoise);
    Serial.print("Humidity is: ");
    Serial.println(avghum);
    Serial.println("");
    Serial.println("");
    Serial.println("");
    Serial.println("");
    Serial.println("");
    Serial.println("");
    Serial.println("");
    Serial.println("");

    //Writing average values to struct
    myData.avgtemp = avgtemp;
    myData.avghum = avghum;
    myData.avgnoise = avgnoise;

    // Send message via ESP-NOW
    esp_now_send(broadcastAddress, (uint8_t*)&myData, sizeof(myData));

    //Reset values for next batch
    tickets = 0;
    avgtemp = 0;
    avghum = 0;
    avgnoise = 0;
  }

  //Add a ticket to track amount of values
  tickets = tickets + 1;
}
