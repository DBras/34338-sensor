/*
 
*/

void setup() {
  // initialize pins for motor output.
  pinMode(PD2, OUTPUT);
  pinMode(PD3, OUTPUT);
  pinMode(PD4, OUTPUT);
  pinMode(PD5, OUTPUT);
  Serial.begin(115200);
}

void loop() {
  int readIn = analogRead(A0);
  Serial.println(readIn);
  //Check for low or high input
  if (readIn > 300) {
    //Start the step loop for the step motor
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