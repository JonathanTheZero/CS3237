// For use in NUS-SoC
// ADAPTED FROM  https://github.com/martinius96/ESP32-eduroam/blob/master/2022/eduroam/eduroam.ino

#include <WiFi.h>      //Wifi library
#include "esp_wpa2.h"  //wpa2 library for connections to Enterprise networks
byte mac[6];


//Identity for user with password related to his realm (organization)
//Available option of anonymous identity for federation of RADIUS servers or 1st Domain RADIUS servers

#define NUS_NET_IDENTITY "nusstu\<nusnet-ID>"  //ie nusstu\e0123456
#define NUS_NET_USERNAME "<nusnet-ID>"
#define NUS_NET_PASSWORD "<nusnet-password>"


//SSID NAME
const char* ssid = "NUS_STU";  // eduroam SSID

void setup() {
  Serial.begin(115200);
  delay(10);
  Serial.print(F("Connecting to network: "));
  Serial.println(ssid);
  WiFi.disconnect(true);  //disconnect from WiFi to set new WiFi connection
  // Servers without Certificate like Bielefeld works this way, see https://github.com/espressif/arduino-esp32/blob/master/libraries/WiFi/examples/WiFiClientEnterprise/WiFiClientEnterprise.ino Line 30.
  WiFi.begin(ssid, WPA2_AUTH_PEAP, NUS_NET_IDENTITY, NUS_NET_USERNAME, NUS_NET_PASSWORD);  // without CERTIFICATE you can comment out the test_root_ca on top. Seems like root CA is included?
  // The below does NOT work for eduroam like Bielefeld, as the ANONYMOUS IDENTITY gets confused with the IDENTITY!
  //WiFi.begin(ssid, WPA2_AUTH_PEAP, EAP_ANONYMOUS_IDENTITY, EAP_IDENTITY, EAP_PASSWORD); //WITHOUT CERTIFICATE - WORKING WITH EXCEPTION ON RADIUS SERVER

  // Example: a cert-file WPA2 Enterprise with PEAP - client certificate and client key required
  //WiFi.begin(ssid, WPA2_AUTH_PEAP, EAP_IDENTITY, EAP_USERNAME, EAP_PASSWORD, test_root_ca, client_cert, client_key);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(F("."));
  }
  Serial.println("");
  Serial.println(F("WiFi is connected!"));
  Serial.println(F("IP address set: "));
  Serial.println(WiFi.localIP());  //print LAN IP
  WiFi.macAddress(mac);
  Serial.print("MAC address: ");
  Serial.print(mac[0], HEX);
  Serial.print(":");
  Serial.print(mac[1], HEX);
  Serial.print(":");
  Serial.print(mac[2], HEX);
  Serial.print(":");
  Serial.print(mac[3], HEX);
  Serial.print(":");
  Serial.print(mac[4], HEX);
  Serial.print(":");
  Serial.println(mac[5], HEX);
}
void loop() {
  yield();
}
