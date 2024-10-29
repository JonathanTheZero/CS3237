#include "Arduino.h"
#include "SoftwareSerial.h"
#include "DFRobotDFPlayerMini.h"
#include "Arduino.h"
#include <WiFi.h>
#include "ESP32MQTTClient.h"


#define FORCE_SENSOR_PIN_1 35
#define FORCE_SENSOR_PIN_2 32
#define FORCE_SENSOR_PIN_3 33
#define FORCE_SENSOR_PIN_4 34
#define READING_THRESHOLD 10
#define AUDIO_PLAYING_VALUE -1


#define SPEAKER_RX_PORT 27
#define SPEAKER_TX_PORT 26
SoftwareSerial speakerSoftwareSerial(SPEAKER_RX_PORT, SPEAKER_TX_PORT);
DFRobotDFPlayerMini DFAudioPlayer;


const char *ssid = "DESKTOP-496OVRO";
const char *pass = "9Wz7344&"; //temp password
char *server = "mqtt://192.168.137.1:1883";
char *publishTopic[4] = {
  ("esp32/1/fsrs/0"),
  ("esp32/1/fsrs/1"),
  ("esp32/1/fsrs/2"),
  ("esp32/1/fsrs/3")
};
ESP32MQTTClient mqttClient;


void setup() {
  Serial.begin(115200);
  speakerSoftwareSerial.begin(9600);

  // set the ADC attenuation to 11 dB (up to ~3.3V input)
  analogSetAttenuation(ADC_11db);
  initialise_dfplayer();
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
  WiFi.setHostname("ESP32 speaker client");


  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(100);
  }
  Serial.println();
  mqttClient.loopStart();
}


void onMqttConnect(esp_mqtt_client_handle_t client) {
  Serial.println(F("[MQTT] Connected to the MQTT broker and subscribing to topics..."));

  // mqttClient.subscribe(trackTopic, [](const String &payload) {
  //   Serial.print(F("[MQTT] Received: "));
  //   Serial.println(String(trackTopic) + String(" ") + String(payload.c_str()));

  //   int value = payload.toInt();
  //   play_track(value);
  // });
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

  for (int i = 0; i < 4; i++) {
    if (readings[i] <= READING_THRESHOLD) {
      continue;
    }

    Serial.print(F("[FSR] Force sensor "));
    Serial.print(i + 1);
    Serial.print(F(" received value of "));
    Serial.print(readings[i]);

    // Values are arbitrary for now
    if (readings[i] < 10) {
      Serial.println(F(" -> no pressure"));
    } else if (readings[i] < 200) {
      Serial.println(F(" -> light touch"));
    } else if (readings[i] < 500) {
      Serial.println(F(" -> light squeeze"));
    } else if (readings[i] < 800) {
      Serial.println(F(" -> medium squeeze"));
    } else {
      Serial.println(F(" -> big squeeze"));
    }
    String msg = String(i) + F(": ") + String(readings[i]);
    mqttClient.publish(publishTopic[i], msg, 0, false);
  }

  if (readings[0] >= 1000
      && readings[1] >= 1000
      && readings[2] >= 1000
      && readings[3] >= 1000) {
    play_track(2);
  }
}


void initialise_dfplayer() {
  Serial.println(F("[DFPlayer] Initializing DFPlayer ... (May take 3~5 seconds)"));
  delay(1000);

  while (!DFAudioPlayer.begin(speakerSoftwareSerial)) {
    Serial.println(F("[DFPlayer] Unable to begin, retrying in 2 seconds..."));

    if (DFAudioPlayer.available()) {
      printDetail(DFAudioPlayer.readType(), DFAudioPlayer.read());
    }
    delay(2000);
  }

  Serial.println(F("[DFPlayer] DFPlayer Mini online."));
  DFAudioPlayer.volume(30);
  DFAudioPlayer.EQ(5);
}


void play_track(int trackID) {
  if (trackID < 1) {
    Serial.println(F("[DFPlayer] Received track ID is not valid."));
    return;
  }

  // Check if the player is already playing
  int state = DFAudioPlayer.readState();
  if (state == AUDIO_PLAYING_VALUE) {
    Serial.println(F("[DFPlayer] A track is already playing. Skipping play request."));
    delay(250);
    return;
  }

  Serial.print(F("[DFPlayer] Playing track with ID "));
  Serial.println(trackID);
  DFAudioPlayer.play(trackID);
  delay(100);
}


void printDetail(uint8_t type, int value) {
  switch (type) {
    case TimeOut:
      Serial.println(F("Time Out!"));
      break;
    case WrongStack:
      Serial.println(F("Stack Wrong!"));
      break;
    case DFPlayerCardInserted:
      Serial.println(F("Card Inserted!"));
      break;
    case DFPlayerCardRemoved:
      Serial.println(F("Card Removed!"));
      break;
    case DFPlayerCardOnline:
      Serial.println(F("Card Online!"));
      break;
    case DFPlayerPlayFinished:
      Serial.print(F("Number:"));
      Serial.print(value);
      Serial.println(F(" Play Finished!"));
      break;
    case DFPlayerError:
      Serial.print(F("DFPlayerError:"));
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