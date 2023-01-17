/*
  34338 Baby Monitor Controller

  This acts as a controller for the step motor in the sensor unit

  This runs on an Arduino UNO, it expects 4 digital pins for a step motor and an analog pin
  to read from the sensor-ESP
 
  Modified 17 Jan 2023

  GitHub URL:
  https://github.com/DBras/34338-sensor/ 
*/

void setup() {
  // initialize pins for motor output.
  pinMode(PD2, OUTPUT);
  pinMode(PD3, OUTPUT);
  pinMode(PD4, OUTPUT);
  pinMode(PD5, OUTPUT);
  Serial.begin(115200);
}

uint8_t pins[4] = {PD2, PD3, PD4, PD5};

void loop() {
  int readIn = analogRead(A0);
  Serial.println(readIn);
  //Check for low or high input
  if (readIn > 300) {
    //Start the step loop for the step motor
    for(int i=0; i<4; i++) {
      digitalWrite(pins[i], HIGH);
      if (i-1 >= 0) {
        digitalWrite(pins[i-1], LOW);
      }
      else {
        digitalWrite(pins[3], LOW);
      }
      delay(5);
    }
  }
}