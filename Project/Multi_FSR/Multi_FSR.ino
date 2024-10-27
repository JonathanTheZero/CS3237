#include "Arduino.h"
#include "SoftwareSerial.h"
#include "DFRobotDFPlayerMini.h"


#define SPEAKER_RX_PORT 27
#define SPEAKER_TX_PORT 26
#define FORCE_SENSOR_PIN_1 35
#define FORCE_SENSOR_PIN_2 32
#define FORCE_SENSOR_PIN_3 33
#define FORCE_SENSOR_PIN_4 25


SoftwareSerial mySoftwareSerial(SPEAKER_RX_PORT, SPEAKER_TX_PORT);
DFRobotDFPlayerMini myDFPlayer;


void setup() {
  Serial.begin(115200);

  // set the ADC attenuation to 11 dB (up to ~3.3V input)
  analogSetAttenuation(ADC_11db);
  initialise_dfplayer();
}


void loop() {
  int readings[4] = {
    analogRead(FORCE_SENSOR_PIN_1),
    analogRead(FORCE_SENSOR_PIN_2),
    analogRead(FORCE_SENSOR_PIN_3),
    analogRead(FORCE_SENSOR_PIN_4)
  };

  for (int i = 0; i < 4; i++) {
    if (readings[i] <= 10) {
      continue;
    }

    Serial.print(F("[FSR] The force sensor "));
    Serial.print(i + 1);
    Serial.print(F(" value = "));
    Serial.print(readings[i]);

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
  }

  if (readings[0] >= 1000
      && readings[1] >= 1000
      && readings[2] >= 1000
      && readings[3] >= 1000) {
    play_track(2);
  }

  delay(1000);
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


void play_track(int trackID) {
  if (trackID < 1) {
    Serial.println(F("[DFPlayer] Received track ID is not valid."));
    return;
  }
  Serial.print(F("[DFPlayer] Playing track with ID "));
  Serial.println(trackID);
  myDFPlayer.play(trackID);
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