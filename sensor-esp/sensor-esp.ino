#include <espnow.h>
#include <ESP8266WiFi.h>
#include <ThingSpeak.h>
#include "DHT.h"
#define DHTPIN D2
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);
//int tempPin = A0;
int noisePin = A0;
int ledPin = 13;
int tempValue = 0;
int noiseValue = 0;
int tickets = 0;
float avgtemp = 0;
float avgnoise = 0;
float avghum = 0;
WiFiClient client;
const char* ssid = "P7";
const char* pass = "dtuiot!!";
unsigned long channelID = 2004080;
const char* APIKey = "A5HP1PM53222TH9M";
const char* server = "api.thingspeak.com";

uint8_t broadcastAddress[] = { 0x84, 0xF3, 0xEB, 0x31, 0xD0, 0x74 };
typedef struct struct_message {
  float avgtemp = 0;
  float avghum = 0;
  float avgnoise = 0;
} struct_message;
typedef struct receive_struct_message {
  bool turnOn;
} receive_struct_message;

struct_message myData;
receive_struct_message receivedData;

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

void OnDataSent(uint8_t* mac_addr, uint8_t sendStatus) {
  Serial.print("Last Packet Send Status: ");
  if (sendStatus == 0) {
    Serial.println("Delivery success");
  } else {
    Serial.println("Delivery fail");
  }
}

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
  WiFi.mode(WIFI_STA);
  int32_t channel = getWiFiChannel(ssid);
  WiFi.printDiag(Serial);
  wifi_promiscuous_enable(1);
  wifi_set_channel(channel);
  wifi_promiscuous_enable(0);
  WiFi.printDiag(Serial);
  dht.begin();
  pinMode(ledPin, OUTPUT);
  pinMode(D7, OUTPUT);
  Serial.begin(115200);
  //WiFi.begin(ssid, pass);
  //pinMode(PD2, OUTPUT);
  //ThingSpeak.begin(client);

  // Init ESP-NOW
  if (esp_now_init() != 0) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
  esp_now_set_self_role(ESP_NOW_ROLE_COMBO);
  esp_now_register_send_cb(OnDataSent);

  // Register peer
  esp_now_add_peer(broadcastAddress, ESP_NOW_ROLE_COMBO, 1, NULL, 0);

  esp_now_register_recv_cb(onDataRec);
}

void loop() {

  delay(20);
  noiseValue = analogRead(noisePin);
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  

  avgtemp = avgtemp + t;
  avgnoise = avgnoise + noiseValue;
  avghum = avghum + h;

  if (tickets == 500) {

    avgtemp = avgtemp / 500;
    avghum = avghum / 500;
    avgnoise = avgnoise / 500;

    Serial.print("curtemp er ");
    Serial.println(t);
    Serial.print("CurNoise er ");
    Serial.println(noiseValue);
    Serial.print("CurHumidity er ");
    Serial.println(h);


    Serial.print("Temperatur er ");
    Serial.println(avgtemp);
    Serial.print("Noise er ");
    Serial.println(avgnoise);
    Serial.print("Humidity er ");
    Serial.println(avghum);
    Serial.println("");
    Serial.println("");
    Serial.println("");
    Serial.println("");
    Serial.println("");
    Serial.println("");
    Serial.println("");
    Serial.println("");


    myData.avgtemp = avgtemp;
    myData.avghum = avghum;
    myData.avgnoise = avgnoise;

    // Send message via ESP-NOW
    esp_now_send(broadcastAddress, (uint8_t*)&myData, sizeof(myData));

    tickets = 0;
    avgtemp = 0;
    avghum = 0;
    avgnoise = 0;
  }

  tickets = tickets + 1;
}
