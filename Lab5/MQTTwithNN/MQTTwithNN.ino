#include "Arduino.h"
#include <WiFi.h>
#include "ESP32MQTTClient.h"
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#include <ESP32Servo.h>

#define uS_TO_S_FACTOR 1000000ULL
#define TIME_TO_SLEEP 20

#define SERVO_PIN 18
Servo myservo;
int pos;

#define DHTPIN 16
#define DHTTYPE DHT11
DHT_Unified dht(DHTPIN, DHTTYPE);


const char *ssid = "Galaxy A53 5G 1EBE";
const char *pass = "jxjw7723";
const char *server = "mqtt://192.168.138.224:1883";
const char *subscribeTopic = "receiving/esp";
const char *topic = "weather/data";

String responses[] = {
  "open",
  "closed",
  "partial"
};

ESP32MQTTClient mqttClient;
bool has_received;

void setup() {
  Serial.begin(115200);
  delay(1000);

  print_wakeup_reason();

  log_i();
  log_i("setup, ESP.getSdkVersion(): ");
  log_i("%s", ESP.getSdkVersion());

  mqttClient.enableDebuggingMessages();
  dht.begin();

  mqttClient.setURI(server);
  mqttClient.enableLastWillMessage("lwt", "I am going offline");
  mqttClient.setKeepAlive(30);

  WiFi.begin(ssid, pass);
  WiFi.setHostname("c3test");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
  Serial.println("");

  has_received = false;

  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  ESP32PWM::allocateTimer(2);
  ESP32PWM::allocateTimer(3);
  myservo.setPeriodHertz(50);
  myservo.attach(SERVO_PIN, 1000, 2000);

  //start in the middle
  pos = 90;
  myservo.write(pos);


  mqttClient.loopStart();
  delay(1000);

  // Send data until an answer has been registered
  while (!has_received) {
    sendWeatherData();
    delay(2000);
  }

  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  Serial.println("Going to sleep now");
  Serial.flush();
  esp_deep_sleep_start();
}

void loop() {
  // this is not called
}

void sendWeatherData() {
  sensors_event_t event;
  float temp,
    humidity;

  dht.temperature().getEvent(&event);
  if (isnan(event.temperature)) {
    Serial.println(F("Error reading temperature!"));
    return;
  } else {
    temp = event.temperature;
  }

  dht.humidity().getEvent(&event);
  if (isnan(event.relative_humidity)) {
    Serial.println(F("Error reading humidity!"));
    return;
  } else {
    humidity = event.relative_humidity;
  }

  mqttClient.publish(
    topic,
    "Temperature: " + String(temperature) + "°C, Humidity: " + String(humidity) + "%",
    0,
    false);
  Serial.println("Data has been sent!");
}

void onMqttConnect(esp_mqtt_client_handle_t client) {
  if (mqttClient.isMyTurn(client)) {
    Serial.println("Connected to the MQTT broker and subscribing to topics...");

    mqttClient.subscribe(subscribeTopic, [](const String &payload) {
      String message = String(payload.c_str());
      Serial.println(String(subscribeTopic) + String(" ") + message);

      if (message.equals(responses[0])) {
        // open  if too hot
        moveToRight();
        has_received = true;
      } else if (message.equals(responses[1])) {
        // close window if too cold
        moveToLeft();
        has_received = true;
      } else if (message.equals(responses[2])) {
        // moderate temperature else
        moveToMiddle();
        has_received = true;
      }
    });
  }
}


void print_wakeup_reason() {
  esp_sleep_wakeup_cause_t wakeup_reason;

  wakeup_reason = esp_sleep_get_wakeup_cause();

  switch (wakeup_reason) {
    case ESP_SLEEP_WAKEUP_EXT0:
      Serial.println("Wakeup caused by external signal using RTC_IO");
      break;
    case ESP_SLEEP_WAKEUP_EXT1:
      Serial.println("Wakeup caused by external signal using RTC_CNTL");
      break;
    case ESP_SLEEP_WAKEUP_TIMER:
      Serial.println("Wakeup caused by timer");
      break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD:
      Serial.println("Wakeup caused by touchpad");
      break;
    case ESP_SLEEP_WAKEUP_ULP:
      Serial.println("Wakeup caused by ULP program");
      break;
    default:
      Serial.printf("Wakeup was not caused by deep sleep: %d\n", wakeup_reason);
      break;
  }
}

void handleMQTT(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
  auto *event = static_cast<esp_mqtt_event_handle_t>(event_data);
  mqttClient.onEventCallback(event);
}


void moveToRight() {
  for (pos; pos <= 180; pos += 2) {
    myservo.write(pos);
    delay(10);
  }
}


void moveToLeft() {
  for (pos; pos >= 0; pos -= 2) {
    myservo.write(pos);
    delay(10);
  }
}


void moveToMiddle() {
  for (pos; pos != 90; pos > 90 ? pos-- : pos++) {
    myservo.write(pos);
    delay(10);
  }
}