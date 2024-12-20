#include "Arduino.h"
#include "SoftwareSerial.h"
#include "DFRobotDFPlayerMini.h"


#define RX_PORT 27
#define TX_PORT 26


SoftwareSerial mySoftwareSerial(RX_PORT, TX_PORT);
DFRobotDFPlayerMini myDFPlayer;
void printDetail(uint8_t type, int value);


void setup() {
  mySoftwareSerial.begin(9600);
  Serial.begin(115200);
  delay(1000);

  Serial.println();
  Serial.println(F("DFRobot DFPlayer Mini Demo"));
  Serial.println(F("Initializing DFPlayer ... (May take 3~5 seconds)"));

  //Use softwareSerial to communicate with mp3.
  delay(1000);
  while (!myDFPlayer.begin(mySoftwareSerial)) {
    Serial.println(F("Unable to begin, retrying in 2 seconds..."));

    if (myDFPlayer.available()) {
      printDetail(myDFPlayer.readType(), myDFPlayer.read());
    }
    delay(2000);
  }
  Serial.println(F("DFPlayer Mini online."));
  delay(1000);
  myDFPlayer.volume(30);  //Set volume value. From 0 to 30
  myDFPlayer.EQ(5);
  myDFPlayer.play(2);     //Play the first mp3
  Serial.println("Sound playing...");
}


void loop() {
  static unsigned long timer = millis();

  if (millis() - timer > 3000) {
    timer = millis();
    //myDFPlayer.next();  //Play next mp3 every 3 second.
  }

  if (myDFPlayer.available()) {
    printDetail(myDFPlayer.readType(), myDFPlayer.read());  //Print the detail message from DFPlayer to handle different errors and states.
  }
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
