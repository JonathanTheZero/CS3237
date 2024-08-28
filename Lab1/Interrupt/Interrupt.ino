#define PIN 23
#define DEBOUNCE_TIME 200000  // 200ms

volatile bool pressed = false;
volatile int pressCount = 0;
volatile unsigned long lastPressTime = 0;

void IRAM_ATTR isr() {
  if (micros() - lastPressTime > DEBOUNCE_TIME) {
    pressCount++;
    pressed = true;
    lastPressTime = micros();
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(PIN, INPUT_PULLUP);
  attachInterrupt(PIN, isr, FALLING);
}

void loop() {
  if (pressed) {
    Serial.printf("Button has been pressed %u times\n", pressCount);
    pressed = false;
  }
}