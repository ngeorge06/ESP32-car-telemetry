#include <Arduino.h>
#include "data.h"
#include "reader.h"
#include "interface.h"

VehicleData vehicleData;
CANReader canReader;
WebInterface webInterface;

void setup() {
  Serial.begin(115200);

  canReader.begin();

  webInterface.setData(&vehicleData);
  webInterface.begin("ESP32", "12345678");
}

void loop() {
  canReader.update(vehicleData);
  webInterface.handle();
}