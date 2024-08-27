#define LEDPIN 21
#define LEDCHANNEL 1
#define LEDFREQ 5000
#define LEDRESOLUTION 8
// PWM DutyCycle
int brightness = 0;
int direction = 1;

void setup() {
  ledcAttachChannel(LEDPIN, LEDFREQ, LEDRESOLUTION, LEDCHANNEL);
  ledcWriteChannel(LEDCHANNEL, brightness);
}


void loop() {
  ledcWriteChannel(LEDCHANNEL, brightness);
  brightness += direction * 10;
  delay(200);
  if (brightness == 200 || brightness == 0) {
    direction *= -1;
  }
}