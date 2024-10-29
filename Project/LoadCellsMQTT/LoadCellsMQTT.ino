#include "HX711.h"
#include "Arduino.h"
#include <WiFi.h>
#include "ESP32MQTTClient.h"


const char *ssid = "DESKTOP-496OVRO";
const char *pass = "9Wz7344&";
char *server = "mqtt://192.168.137.1:1883";
char *publishTopic[2] = { "esp32/0/load_cells/0", "esp32/0/load_cells/1" };

ESP32MQTTClient mqttClient;


HX711 scale0;
HX711 scale1;
HX711 scales[2] = { scale0, scale1 };


//uint8_t dataPin = 6;
//uint8_t clockPin = 7;
uint8_t dataPin[2] = { 26, 13 };   //for esp32
uint8_t clockPin[2] = { 25, 12 };  //for esp32


void setup() {
  Serial.begin(115200);
  Serial.println(__FILE__);
  Serial.print("LIBRARY VERSION: ");
  Serial.println(HX711_LIB_VERSION);
  Serial.println();

  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }

  Serial.println();

  WiFi.setHostname("load_cells");

  mqttClient.enableDebuggingMessages();

  mqttClient.setURI(server);
  mqttClient.enableLastWillMessage("lwt", "I am going offline");
  mqttClient.setKeepAlive(30);
  mqttClient.loopStart();

  for (int i = 0; i < 2; i++) {
    scales[i].begin(dataPin[i], clockPin[i]);

    Serial.print("UNITS ");
    Serial.print(i);
    Serial.print(": ");
    Serial.println(scales[i].get_units(20));
  }

  Serial.println("\nEmpty the scale, press a key to continue");
  while (!Serial.available())
    ;
  while (Serial.available()) Serial.read();

  for (int i = 0; i < 2; i++) {
    scales[i].tare(20);
    Serial.print("UNITS ");
    Serial.print(i);
    Serial.print(": ");
    Serial.println(scales[i].get_units(20));
  }

  Serial.println("\nPut 1000 gram in the scale, press a key to continue");
  while (!Serial.available())
    ;
  while (Serial.available()) Serial.read();

  for (int i = 0; i < 2; i++) {
    scales[i].calibrate_scale(1700, 20);
    Serial.print("UNITS ");
    Serial.print(i);
    Serial.print(": ");
    Serial.println(scales[i].get_units(20));
  }

  Serial.println("\nScale is calibrated, press a key to continue");
  // Serial.println(scale.get_scale());
  // Serial.println(scale.get_offset());
  while (!Serial.available())
    ;
  while (Serial.available()) Serial.read();
}


void loop() {
  for (int i = 0; i < 2; i++) {
    Serial.print("UNITS ");
    Serial.print(i);
    Serial.print(": ");

    float reading = scales[i].get_units(20);
    Serial.println(reading);

    String msg = String(i) + " " + String(reading);
    mqttClient.publish(publishTopic[i], msg, 0, false);
  }
  delay(250);
}


void onMqttConnect(esp_mqtt_client_handle_t client) {
  if (mqttClient.isMyTurn(client))  // can be omitted if only one client
  {
  }
}


void handleMQTT(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
  auto *event = static_cast<esp_mqtt_event_handle_t>(event_data);
  mqttClient.onEventCallback(event);
}