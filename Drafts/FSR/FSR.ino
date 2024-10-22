#define FORCE_SENSOR_PIN 35

void setup() {
  Serial.begin(9600);

  // set the ADC attenuation to 11 dB (up to ~3.3V input)
  analogSetAttenuation(ADC_11db);
}

void loop() {
  int analogReading = analogRead(FORCE_SENSOR_PIN);

  if (analogReading <= 10) {
    return;
  }

  Serial.print("The force sensor value = ");
  Serial.print(analogReading);  // print the raw analog reading

  if (analogReading < 10) {  // from 0 to 9
    Serial.println(" -> no pressure");
  } else if (analogReading < 200) {  // from 10 to 199
    Serial.println(" -> light touch");
  } else if (analogReading < 500) {  // from 200 to 499
    Serial.println(" -> light squeeze");
  } else if (analogReading < 800) {  // from 500 to 799
    Serial.println(" -> medium squeeze");
  } else {
    Serial.println(" -> big squeeze");
  }

  delay(1000);
}
