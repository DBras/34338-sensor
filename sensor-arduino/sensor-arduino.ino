/*
  Blink

  Turns an LED on for one second, then off for one second, repeatedly.

  Most Arduinos have an on-board LED you can control. On the UNO, MEGA and ZERO
  it is attached to digital pin 13, on MKR1000 on pin 6. LED_BUILTIN is set to
  the correct LED pin independent of which board is used.
  If you want to know what pin the on-board LED is connected to on your Arduino
  model, check the Technical Specs of your board at:
  https://www.arduino.cc/en/Main/Products

  modified 8 May 2014
  by Scott Fitzgerald
  modified 2 Sep 2016
  by Arturo Guadalupi
  modified 8 Sep 2016
  by Colby Newman

  This example code is in the public domain.

  http://www.arduino.cc/en/Tutorial/Blink
*/

// the setup function runs once when you press reset or power the board
void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(PD2, OUTPUT);
  pinMode(PD3, OUTPUT);
  pinMode(PD4, OUTPUT);
  pinMode(PD5, OUTPUT);
  Serial.begin(115200);
}

// the loop function runs over and over again forever
void loop() {
int v = analogRead(A0);

Serial.println(v);
if(v>300){
    digitalWrite(PD2, HIGH);
    digitalWrite(PD3, LOW);
    digitalWrite(PD4, LOW);
    digitalWrite(PD5, LOW);
    delay(5);
    digitalWrite(PD2, LOW);
    digitalWrite(PD3, HIGH);
    digitalWrite(PD4, LOW);
    digitalWrite(PD5, LOW);
    delay(5);
    digitalWrite(PD2, LOW);
    digitalWrite(PD3, LOW);
    digitalWrite(PD4, HIGH);
    digitalWrite(PD5, LOW);
    delay(5);
    digitalWrite(PD2, LOW);
    digitalWrite(PD3, LOW);
    digitalWrite(PD4, LOW);
    digitalWrite(PD5, HIGH);
    delay(5);
}
  
}