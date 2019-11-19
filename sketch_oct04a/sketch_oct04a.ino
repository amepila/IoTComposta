void setup() {   
  // initialize inbuilt LED pin as an output.
  pinMode(13, OUTPUT);
}

// loop function runs over and over again forever
void loop() {
  digitalWrite(13, HIGH);   // turn the LED on by making the pin 13 HIGH
  delay(500);              // wait for a 0.5 second
  digitalWrite(13, LOW);    // turn the LED off by making the pin 13 LOW
  delay(500);              // wait for a 0.5 second
}
