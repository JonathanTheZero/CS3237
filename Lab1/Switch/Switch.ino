#define SWITCH 23

byte state = LOW;

void setup() {
  Serial.begin(9600);
  pinMode(SWITCH, INPUT_PULLUP);
}

void loop() {
  if (digitalRead(SWITCH) == LOW) {
    state = !state;
    delay(250);
  }
  if (state) {
    Serial.println("Toggle On");
  } else {
    Serial.println("Toggle Off");
  }
}