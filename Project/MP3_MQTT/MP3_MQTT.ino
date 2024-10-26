#include "Arduino.h"
#include "SoftwareSerial.h"
#include "DFRobotDFPlayerMini.h"
#include <WiFi.h>
#include "ESP32MQTTClient.h"


#define RX_PORT 27
#define TX_PORT 26


SoftwareSerial mySoftwareSerial(RX_PORT, TX_PORT);
DFRobotDFPlayerMini myDFPlayer;
void printDetail(uint8_t type, int value);


const char *ssid = "Galaxy A53 5G 1EBE";
const char *pass = "jxjw7723";


char *server = "mqtt://192.168.118.224:1883";
char *subscribeTopic = "sound/track";
ESP32MQTTClient mqttClient;


void setup() {
  mySoftwareSerial.begin(9600);
  Serial.begin(115200);
  delay(1000);

  initialise_mqtt();
  initialise_dfplayer();
}


void loop() {
}


void initialise_dfplayer() {
  Serial.println(F("[DFPlayer] Initializing DFPlayer ... (May take 3~5 seconds)"));

  delay(1000);
  while (!myDFPlayer.begin(mySoftwareSerial)) {
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


void play_track(int trackID){
  if(trackID < 1){
    Serial.println("[DFPlayer] Received track ID is not valid.");
    return;
  }
  Serial.println("[DFPlayer] Playing track with ID " + trackID);
  myDFPlayer.play(value);
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
  WiFi.setHostname("c3test");


  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(100);
  }
  Serial.println();
  mqttClient.loopStart();
}


void onMqttConnect(esp_mqtt_client_handle_t client) {
  if (mqttClient.isMyTurn(client))  // can be omitted if only one client
  {
    Serial.println("[MQTT] Connected to the MQTT broker and subscribing to topics...");

    mqttClient.subscribe(subscribeTopic, [](const String &payload) {
      Serial.println(
        "[MQTT] " + String(subscribeTopic) + String(" ") + String(payload.c_str()));

      int value = payload.toInt();

      // Check if conversion succeeded
      if (value != 0) {
        play_track(value);
      } else {
        Serial.println("Error: Message could not be converted to a valid track ID");
      }
    });
  }
}


void handleMQTT(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
  auto *event = static_cast<esp_mqtt_event_handle_t>(event_data);
  mqttClient.onEventCallback(event);
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
