#include "Arduino.h"
#include <WiFi.h>
#include "ESP32MQTTClient.h"


#define FORCE_SENSOR_PIN_1 33
#define FORCE_SENSOR_PIN_2 32
#define FORCE_SENSOR_PIN_3 35
#define FORCE_SENSOR_PIN_4 34


const char *ssid = "Galaxy A53 5G 1EBE";
const char *pass = "jxjw7723"; //temp password
char *server = "mqtt://192.168.35.224:1883";
char *publishTopic[4] = {
  ("esp32/1/fsrs/0"),
  ("esp32/1/fsrs/1"),
  ("esp32/1/fsrs/2"),
  ("esp32/1/fsrs/3")
};
const char *dataTopic = "esp32/1/fsrs";
ESP32MQTTClient mqttClient;


void setup() {
  Serial.begin(115200);
  initialise_mqtt();
}


void loop() {
  read_values_and_act();
  delay(1000);
}



void initialise_mqtt() {
  log_i();
  log_i("setup, ESP.getSdkVersion(): ");
  log_i("%s", ESP.getSdkVersion());


  mqttClient.enableDebuggingMessages();
  mqttClient.setURI(server);
  mqttClient.enableLastWillMessage("lwt", "I am going offline");
  mqttClient.setKeepAlive(30);
  WiFi.begin(ssid, pass);
  WiFi.setHostname("ESP32_speaker_client");


  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(100);
  }
  Serial.println();
  mqttClient.loopStart();
}


void onMqttConnect(esp_mqtt_client_handle_t client) {
  Serial.println(F("[MQTT] Connected to the MQTT broker and subscribing to topics..."));
}


void handleMQTT(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
  auto *event = static_cast<esp_mqtt_event_handle_t>(event_data);
  mqttClient.onEventCallback(event);
}


void read_values_and_act() {
  int readings[4] = {
    analogRead(FORCE_SENSOR_PIN_1),
    analogRead(FORCE_SENSOR_PIN_2),
    analogRead(FORCE_SENSOR_PIN_3),
    analogRead(FORCE_SENSOR_PIN_4)
  };

  String msg = String(readings[0]) + "," + String(readings[1]) + "," + String(readings[2]) + "," + String(readings[3]);
  mqttClient.publish(dataTopic, msg, 0, false);
  Serial.println(msg);
}