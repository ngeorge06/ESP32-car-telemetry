#include <Arduino.h>
#include "data.h"
#include "reader.h"
#include "interface.h"
#include "secrets.h"

VehicleData vehicleData;
CANReader canReader;
WebInterface webInterface;

void setup() {
  Serial.begin(115200);

  canReader.begin();

  webInterface.setData(&vehicleData);
  webInterface.begin(WIFI_SSID, WIFI_PASSWORD);
}

void loop() {
  canReader.update(vehicleData);
  webInterface.handle();
}
