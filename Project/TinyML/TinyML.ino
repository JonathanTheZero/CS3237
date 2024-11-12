#include <ArduTFLite.h>
#include "model.hpp"
#include "Arduino.h"
#include <WiFi.h>
#include "ESP32MQTTClient.h"
// #include "SoftwareSerial.h"
// #include "DFRobotDFPlayerMini.h"


// The Tensor Arena memory area is used by TensorFlow Lite to store input, output and intermediate tensors
// It must be defined as a global array of byte (or u_int8 which is the same type on Arduino)
// The Tensor Arena size must be defined by trials and errors. We use here a quite large value.
// The alignas(16) directive is used to ensure that the array is aligned on a 16-byte boundary,
// this is important for performance and to prevent some issues on ARM microcontroller architectures.
constexpr int kTensorArenaSize = 8 * 1024;
alignas(16) uint8_t tensor_arena[kTensorArenaSize];
#define INPUT_LENGTH 8


#define RX_PORT 27
#define TX_PORT 26
#define BEEP_TRACK_ID 1
#define PLEASE_SIT_UPRIGHT_TRACK_ID 2
#define EYES_TOO_CLOSE_TRACK_ID 3
#define PLEASE_STOP_SLOUCHING_TRACK_ID 4
#define IMBALANCE_TRACK_ID 5
#define EYES_TOO_FAR_TRACK_ID 6

// SoftwareSerial speakerSoftwareSerial(RX_PORT, TX_PORT);
// DFRobotDFPlayerMini myDFPlayer;


const char *trackTopic = "esp32/1/track_id";
const char *dataTopic = "esp32/1/data";
const char *ssid = "DESKTOP-496OVRO";
const char *pass = "9Wz7344&";
const char *server = "mqtt://192.168.137.1:1883";
ESP32MQTTClient mqttClient;


void setup() {
  // speakerSoftwareSerial.begin(9600);
  Serial.begin(115200);

  initialise_mqtt();
  // initialise_dfplayer();
  initialise_model();
}


void loop() {

}


void initialise_model() {
  Serial.println(F("[Model] Initializing TensorFlow Lite Micro Interpreter..."));
  if (!modelInit(__output_model_tflite, tensor_arena, kTensorArenaSize)) {
    Serial.println(F("[Model] Model initialization failed!"));
  }
  Serial.println(F("[Model] Model initialization done."));
}


int make_prediction(float values[]) {
  for (uint8_t i = 0; i < INPUT_LENGTH; i++) {
    modelSetInput(values[i], i);
  }

  if (!modelRunInference()) {
    Serial.println(F("[Model] Inference failed!"));
    return -1;
  }

  float maxVal = 0;
  uint8_t maxIndex = 0;
  for (uint8_t i = 0; i < sizeof(LABELS) / sizeof(LABELS[0]); i++) {
    float output = modelGetOutput(i) * 100;
    // Serial.print(i);
    // Serial.print(LABELS[i]);
    // Serial.print(": ");
    // Serial.print(output, 2);
    // Serial.println("%");

    if (output > maxVal) {
      maxVal = output;
      maxIndex = i;
    }
    if (maxVal > 50) {  //no bigger prediction possible
      break;
    }
  }
  Serial.print(F("[Model] Predicted "));
  Serial.print(LABELS[maxIndex]);
  Serial.print(F(" with an accuracy of "));
  Serial.print(maxVal, 2);
  Serial.println(F("%"));
  return maxIndex;
}


// void initialise_dfplayer() {
//   Serial.println(F("[DFPlayer] Initializing DFPlayer ... (May take 3~5 seconds)"));

//   delay(1000);
//   while (!myDFPlayer.begin(speakerSoftwareSerial)) {
//     Serial.println(F("[DFPlayer] Unable to begin, retrying in 2 seconds..."));
//     Serial.println("Retrying speaker in 2s");

//     if (myDFPlayer.available()) {
//       printDetail(myDFPlayer.readType(), myDFPlayer.read());
//     }
//     delay(2000);
//   }
//   Serial.println(F("[DFPlayer] DFPlayer Mini online."));
//   myDFPlayer.volume(30);  //Set volume value. From 0 to 30
//   myDFPlayer.EQ(5);
// }


// void play_track(uint8_t trackID) {
//   if (trackID < 1) {
//     Serial.println(F("[DFPlayer] Received track ID is not valid."));
//     return;
//   }

//   Serial.print(F("[DFPlayer] Playing track with ID "));
//   Serial.println(trackID);
//   myDFPlayer.play(trackID);
//   delay(4000);
// }


void initialise_mqtt() {
  log_i();
  log_i("setup, ESP.getSdkVersion(): ");
  log_i("%s", ESP.getSdkVersion());


  // mqttClient.enableDebuggingMessages();
  mqttClient.setURI(server);
  mqttClient.enableLastWillMessage("lwt", "I am going offline");
  mqttClient.setKeepAlive(30);
  WiFi.begin(ssid, pass);
  WiFi.setHostname("ESP32_tf_mp3");


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
    // play_track(value);
  });

  mqttClient.subscribe(dataTopic, [](const String &payload) {
    Serial.print(F("[MQTT] Received: "));
    Serial.println(String(trackTopic) + String(" ") + String(payload.c_str()));

    uint8_t index = 0,
            start = 0;
    int commaIndex = payload.indexOf(',');
    float values[INPUT_LENGTH];

    while (commaIndex != -1 && index < INPUT_LENGTH) {
      values[index++] = payload.substring(start, commaIndex).toFloat();
      start = commaIndex + 1;
      commaIndex = payload.indexOf(',', start);
    }

    values[index] = payload.substring(start).toInt();
    int classified = make_prediction(values);
    Serial.print(classified);
    Serial.print("    ");
    Serial.println(LABELS[classified]);
    // play_track(position_to_trackID((Positions)classified));
  });
}


int position_to_trackID(Positions prediction) {
  switch (prediction) {
    case LEANING_BACK:
    case LEANING_FORWARD:
      return PLEASE_SIT_UPRIGHT_TRACK_ID;
    case SLOUCHING:
      return PLEASE_STOP_SLOUCHING_TRACK_ID;
    case LEANING_RIGHT:
    case LEANING_LEFT:
      return IMBALANCE_TRACK_ID;
    case NO_CONTACT:
    default:
      return -1;
  }
}


void handleMQTT(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
  auto *event = static_cast<esp_mqtt_event_handle_t>(event_data);
  mqttClient.onEventCallback(event);
}


// void printDetail(uint8_t type, int value) {
//   switch (type) {
//     case TimeOut:
//       Serial.println(F("[DFPlayer] Time Out!"));
//       break;
//     case WrongStack:
//       Serial.println(F("[DFPlayer] Stack Wrong!"));
//       break;
//     case DFPlayerCardInserted:
//       Serial.println(F("[DFPlayer] Card Inserted!"));
//       break;
//     case DFPlayerCardRemoved:
//       Serial.println(F("[DFPlayer] Card Removed!"));
//       break;
//     case DFPlayerCardOnline:
//       Serial.println(F("[DFPlayer] Card Online!"));
//       break;
//     case DFPlayerPlayFinished:
//       Serial.print(F("[DFPlayer] Number: "));
//       Serial.print(value);
//       Serial.println(F(" Play Finished!"));
//       break;
//     case DFPlayerError:
//       Serial.print(F("[DFPlayer] DFPlayerError:"));
//       switch (value) {
//         case Busy:
//           Serial.println(F("Card not found"));
//           break;
//         case Sleeping:
//           Serial.println(F("Sleeping"));
//           break;
//         case SerialWrongStack:
//           Serial.println(F("Get Wrong Stack"));
//           break;
//         case CheckSumNotMatch:
//           Serial.println(F("Check Sum Not Match"));
//           break;
//         case FileIndexOut:
//           Serial.println(F("File Index Out of Bound"));
//           break;
//         case FileMismatch:
//           Serial.println(F("Cannot Find File"));
//           break;
//         case Advertise:
//           Serial.println(F("In Advertise"));
//           break;
//         default:
//           break;
//       }
//       break;
//     default:
//       break;
//   }
// }