#include <ArduTFLite.h>
#include "model.hpp"
#include "Arduino.h"
#include <WiFi.h>
#include "ESP32MQTTClient.h"


// The Tensor Arena memory area is used by TensorFlow Lite to store input, output and intermediate tensors
// It must be defined as a global array of byte (or u_int8 which is the same type on Arduino)
// The Tensor Arena size must be defined by trials and errors. We use here a quite large value.
// The alignas(16) directive is used to ensure that the array is aligned on a 16-byte boundary,
// this is important for performance and to prevent some issues on ARM microcontroller architectures.
constexpr int kTensorArenaSize = 8 * 1024;
alignas(16) uint8_t tensor_arena[kTensorArenaSize];
const int inputLength = 8;  // Number of input values
const char *LABELS[] = {
  "no_contact",
  "leaning_back",
  "slouching",
  "backward_lean"
};


#define RX_PORT 27
#define TX_PORT 26

SoftwareSerial speakerSoftwareSerial(RX_PORT, TX_PORT);
DFRobotDFPlayerMini myDFPlayer;


const char *ssid = "DESKTOP-496OVRO";
const char *pass = "9Wz7344&";
const char *server = "mqtt://192.168.137.1:1883";
const char *trackTopic = "esp32/1/track_id";
const char *dataTopic = "esp32/1/data";
ESP32MQTTClient mqttClient;


void setup() {
  speakerSoftwareSerial.begin(9600);
  Serial.begin(115200);
  while (!Serial)
    ;

  initialise_mqtt();
  initialise_dfplayer();
  initialise_model();
}


void loop() {
}


void initialise_model() {
  Serial.println(F("[Model] Initializing TensorFlow Lite Micro Interpreter..."));
  if (!modelInit(__output_model_tflite, tensor_arena, kTensorArenaSize)) {
    Serial.println(F("[Model] Model initialization failed!"));
    while (true)
      ;
  }
  Serial.println(F("[Model] Model initialization done."));
  Serial.println();
}


int make_prediction(float[] values) {
  float inputData[inputLength] = { 19047, 17051, 18310, 16747, 0, 1489, 870, 0 };

  for (int i = 0; i < inputLength; i++) {
    modelSetInput(inputData[i], i);
  }

  if (!modelRunInference()) {
    Serial.println(F("[Model] Inference failed!"));
    return;
  }

  float maxVal = 0;
  int maxIndex = 0;
  for (int i = 0; i < sizeof(LABELS) / sizeof(LABELS[0]); i++) {
    float output = modelGetOutput(i) * 100;
    Serial.print(i);
    Serial.print(LABELS[i]);
    Serial.print(": ");
    Serial.print(output, 2);
    Serial.println("%");

    if (output > maxVal) {
      maxVal = output;
      maxIndex = i;
    }
    if (maxlVal > 50) {  //no bigger prediction possible
      return maxIndex;
    }
  }
  return maxIndex;
}


void initialise_dfplayer() {
  Serial.println(F("[DFPlayer] Initializing DFPlayer ... (May take 3~5 seconds)"));

  delay(1000);
  while (!myDFPlayer.begin(speakerSoftwareSerial)) {
    Serial.println(F("[DFPlayer] Unable to begin, retrying in 2 seconds..."));

    if (myDFPlayer.available()) {
      printDetail(myDFPlayer.readType(), myDFPlayer.read());
    }
    delay(2000);
  }
  Serial.println(F("[DFPlayer] DFPlayer Mini online."));
  myDFPlayer.volume(30);  //Set volume value. From 0 to 30
  myDFPlayer.EQ(5);
}


void play_track(int trackID) {
  if (trackID < 1) {
    Serial.println(F("[DFPlayer] Received track ID is not valid."));
    return;
  }

  Serial.print(F("[DFPlayer] Playing track with ID "));
  Serial.println(trackID);
  myDFPlayer.play(trackID);
  delay(4000);
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
  WiFi.setHostname("ESP32_model_and_speaker");


  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(100);
  }

  Serial.println();
  mqttClient.loopStart();
}


void onMqttConnect(esp_mqtt_client_handle_t client) {
  Serial.println(F("[MQTT] Connected to the MQTT broker and subscribing to topics..."));

  mqttClient.subscribe(trackTopic, [](const String &payload) {
    Serial.print(F("[MQTT] Received: "));
    Serial.println(String(trackTopic) + String(" ") + String(payload.c_str()));

    int value = payload.toInt();
    play_track(value);
  });

  mqttClient.subscribe(dataTopic, [](const String &payload) {
    Serial.print(F("[MQTT] Received: "));
    Serial.println(String(trackTopic) + String(" ") + String(payload.c_str()));

    int index = 0,
        start = 0,
        commaIndex = input.indexOf(',');

    while (commaIndex != -1 && index < inputLength) {
      values[index++] = input.substring(start, commaIndex).toFloat();
      start = commaIndex + 1;
      commaIndex = input.indexOf(',', start);
    }

    values[index] = input.substring(start).toInt();
    int classified = make_prediction(values);
    // TODO: map categories to track IDs
    // play_track(classified);
  });)
}


void handleMQTT(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
  auto *event = static_cast<esp_mqtt_event_handle_t>(event_data);
  mqttClient.onEventCallback(event);
}


void printDetail(uint8_t type, int value) {
  switch (type) {
    case TimeOut:
      Serial.println(F("[DFPlayer] Time Out!"));
      break;
    case WrongStack:
      Serial.println(F("[DFPlayer] Stack Wrong!"));
      break;
    case DFPlayerCardInserted:
      Serial.println(F("[DFPlayer] Card Inserted!"));
      break;
    case DFPlayerCardRemoved:
      Serial.println(F("[DFPlayer] Card Removed!"));
      break;
    case DFPlayerCardOnline:
      Serial.println(F("[DFPlayer] Card Online!"));
      break;
    case DFPlayerPlayFinished:
      Serial.print(F("[DFPlayer] Number: "));
      Serial.print(value);
      Serial.println(F(" Play Finished!"));
      break;
    case DFPlayerError:
      Serial.print(F("[DFPlayer] DFPlayerError:"));
      switch (value) {
        case Busy:
          Serial.println(F("Card not found"));
          break;
        case Sleeping:
          Serial.println(F("Sleeping"));
          break;
        case SerialWrongStack:
          Serial.println(F("Get Wrong Stack"));
          break;
        case CheckSumNotMatch:
          Serial.println(F("Check Sum Not Match"));
          break;
        case FileIndexOut:
          Serial.println(F("File Index Out of Bound"));
          break;
        case FileMismatch:
          Serial.println(F("Cannot Find File"));
          break;
        case Advertise:
          Serial.println(F("In Advertise"));
          break;
        default:
          break;
      }
      break;
    default:
      break;
  }
}