#include "HX711.h"
#include "Arduino.h"
#include <WiFi.h>
#include "ESP32MQTTClient.h"
// #include <ArduinoJson.h>

#define FORCE_SENSOR_PIN_1 33
#define FORCE_SENSOR_PIN_2 32
#define FORCE_SENSOR_PIN_3 35
#define FORCE_SENSOR_PIN_4 34


const char *ssid = "DESKTOP-496OVRO";
const char *pass = "9Wz7344&";
char *server = "mqtt://192.168.137.1:1883";
char *publishTopic[4] = { "esp32/0/load_cells/0", "esp32/0/load_cells/1", "esp32/0/load_cells/2", "esp32/0/load_cells/3" };
ESP32MQTTClient mqttClient;


HX711 scale0;
HX711 scale1;
HX711 scale2;
HX711 scale3;
HX711 scales[4] = { scale0, scale1, scale2, scale3 };

//uint8_t dataPin = 6;
//uint8_t clockPin = 7;
uint8_t dataPin[4] = { 26, 19, 17, 13 };   //for esp32
uint8_t clockPin[4] = { 25, 18, 16, 12 };  //for esp32


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

  for (int i = 0; i < 4; i++) {
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

  for (int i = 0; i < 4; i++) {
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

  for (int i = 0; i < 4; i++) {
    scales[i].calibrate_scale(15000, 20);
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
  String print_msg = String();
  for (int i = 0; i < 4; i++) {
    float reading = scales[i].get_units(20);
    Serial.print(reading);
    Serial.print(" ");

    String msg = String(i) + " " + String(reading);
    // mqttClient.publish(publishTopic[i], msg, 0, false);
    print_msg += String(reading);
    if (i != 4) {
      msg += F(",");
    }
  }

  mqttClient.publish("esp32/0/load_cells", print_msg, 0, false);


  int fsr_readings[4] = {
    analogRead(FORCE_SENSOR_PIN_1),
    analogRead(FORCE_SENSOR_PIN_2),
    analogRead(FORCE_SENSOR_PIN_3),
    analogRead(FORCE_SENSOR_PIN_4)
  };

  String fsr_msg = String(fsr_readings[0]) + "," + String(fsr_readings[1]) + "," + String(fsr_readings[2]) + "," + String(fsr_readings[3]);
  mqttClient.publish("esp32/0/fsrs", fsr_msg);
  String complete_msg = print_msg + F(", ") + fsr_msg;
  Serial.println(complete_msg);
  mqttClient.publish("esp32/0/data", complete_msg);
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