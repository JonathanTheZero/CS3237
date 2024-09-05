#include <Arduino.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <HTTPClient.h>
#include <Wire.h>
#include <SparkFun_APDS9960.h>
WiFiMulti wifiMulti;

const char* ssid = "Galaxy A53 5G 1EBE";
const char* password = "ympt4538^";  //one time password, sorry :)

const char* laptopAt = "http://192.168.182.224:3237";


// Light sensor
SparkFun_APDS9960 apds = SparkFun_APDS9960();
uint16_t ambient_light = 0;
uint16_t red_light = 0;
uint16_t green_light = 0;
uint16_t blue_light = 0;

void get_data() {
  if (!apds.readAmbientLight(ambient_light)
      || !apds.readRedLight(red_light)
      || !apds.readGreenLight(green_light)
      || !apds.readBlueLight(blue_light)) {
    Serial.println("Error reading light values");
  } else {
    Serial.print("Ambient: ");
    Serial.print(ambient_light);
    Serial.print(" Red: ");
    Serial.print(red_light);
    Serial.print(" Green: ");
    Serial.print(green_light);
    Serial.print(" Blue: ");
    Serial.println(blue_light);
  }
}


void setup() {

  Serial.begin(115200);

  for (uint8_t t = 4; t > 0; t--) {
    Serial.printf("[SETUP] WAIT %d...\n", t);
    Serial.flush();
    delay(1000);
  }

  wifiMulti.addAP(ssid, password);

  while (wifiMulti.run() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());


  if (apds.init()) {
    Serial.println(F("APDS-9960 initialization complete"));
  } else {
    Serial.println(F("Something went wrong during APDS-9960 init!"));
  }

  // Start running the APDS-9960 light sensor (no interrupts)
  if (apds.enableLightSensor(false)) {
    Serial.println(F("Light sensor is now running"));
  } else {
    Serial.println(F("Something went wrong during light sensor init!"));
  }

  // Wait for initialization and calibration to finish
  delay(500);
}

void loop() {
  get_data();
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("Sending...");

    HTTPClient http;

    String url = laptopAt;
    url += "/data?data1=" + String(ambient_light)
           + "&data2=" + String(red_light)
           + "&data3=" + String(blue_light)
           + "&data4=" + String(green_light);

    http.begin(url);
    int returnCode = http.GET();  //perform a HTTP GET request

    if (returnCode > 0) {
      String payload = http.getString();
      Serial.println(payload);
    }
    http.end();

  } else {
    Serial.println("WiFi disconnected");
  }
  delay(5000);  //Five second delay
}
