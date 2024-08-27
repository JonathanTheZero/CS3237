#define LED 21

// the setup function runs once when you press reset or power the board
void setup() {
  // initialize digital pin 5 as an output.
  pinMode(LED, OUTPUT);
}

// the loop function runs over and over again forever
int frequency_delay = 4000;

void loop() {
  digitalWrite(LED, HIGH);
  if (frequency_delay == 0){
    return;
  }
  delay(frequency_delay);
  digitalWrite(LED, LOW);   
  delay(frequency_delay);                      
  frequency_delay /= 2;
}
