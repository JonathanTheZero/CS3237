#define AUDIO_PIN 34
#define LED 21

void setup() {
  Serial.begin(9600);
  pinMode(LED, OUTPUT);
}

void loop() {
  int signal_strength = analogRead(AUDIO_PIN);
  Serial.println(signal_strength);

  if (signal_strength > 0) {
    digitalWrite(LED, HIGH);
  } else {
    digitalWrite(LED, LOW);
  }
  delay(100);
}
