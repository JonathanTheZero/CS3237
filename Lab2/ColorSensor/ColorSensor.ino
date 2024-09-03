#include <Wire.h>
uint8_t address = 0x39;
uint8_t MSByte = 0, LSByte = 0;
uint16_t clearValue = 0,
         redValue = 0,
         blueValue = 0,
         greenValue = 0;


void setup() {
  Wire.begin();
  Serial.begin(9600);
  Wire.beginTransmission(address);
  Wire.write(0x80);
  Wire.write(0x03);
  Wire.endTransmission();
  delay(500);
}


void loop() {
  Wire.beginTransmission(address);
  Wire.write(0x94);  // Clear Data Low Byte
  Wire.endTransmission();
  Wire.requestFrom(address, 1);
  if (Wire.available()) {
    LSByte = Wire.read();
  }
  Wire.beginTransmission(address);
  Wire.write(0x95);  // Clear Data High Byte
  Wire.endTransmission();
  Wire.requestFrom(address, 1);
  if (Wire.available()) {
    MSByte = Wire.read();
  }
  clearValue = (MSByte << 8) + LSByte;

  Wire.beginTransmission(address);
  Wire.write(0x96);  // Red Data Low Byte
  Wire.endTransmission();
  Wire.requestFrom(address, 1);
  if (Wire.available()) {
    LSByte = Wire.read();
  }
  Wire.beginTransmission(address);
  Wire.write(0x97);  // Red Data High Byte
  Wire.endTransmission();
  Wire.requestFrom(address, 1);
  if (Wire.available()) {
    MSByte = Wire.read();
  }
  redValue = (MSByte << 8) + LSByte;

  Wire.beginTransmission(address);
  Wire.write(0x98);  // Green Data Low Byte
  Wire.endTransmission();
  Wire.requestFrom(address, 1);
  if (Wire.available()) {
    LSByte = Wire.read();
  }
  Wire.beginTransmission(address);
  Wire.write(0x99);  // Green Data High Byte
  Wire.endTransmission();
  Wire.requestFrom(address, 1);
  if (Wire.available()) {
    MSByte = Wire.read();
  }
  greenValue = (MSByte << 8) + LSByte;

  Wire.beginTransmission(address);
  Wire.write(0x9A);  // Blue Data Low Byte
  Wire.endTransmission();
  Wire.requestFrom(address, 1);
  if (Wire.available()) {
    LSByte = Wire.read();
  }

  Wire.beginTransmission(address);
  Wire.write(0x9B);  // Blue Data High Byte
  Wire.endTransmission();
  Wire.requestFrom(address, 1);
  if (Wire.available()) {
    MSByte = Wire.read();
  }
  blueValue = (MSByte << 8) + LSByte;

  // Print all values
  Serial.print("Clear Value: ");
  Serial.print(clearValue);
  Serial.print(" | RGB values: ");
  Serial.print(redValue);
  Serial.print(", ");
  Serial.print(greenValue);
  Serial.print(", ");
  Serial.println(blueValue);

  // Wait 1 second before next reading
  delay(1000);
}