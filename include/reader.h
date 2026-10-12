#pragma once
#include <Arduino.h>
#include "data.h"

// talks to the car and fills in VehicleData
class CANReader {
public:
  bool begin();
  void update(VehicleData& data);

private:
  void requestPID(byte pid);

  bool canReady = false;
  int currentPidIndex = 0;
  unsigned long lastRequest = 0;
  static const unsigned long REQUEST_INTERVAL = 150;  // ms between requests

  //  PIDs requested
  static const int PID_COUNT = 9;
  byte pidList[PID_COUNT] = {0x0C, 0x0D, 0x05, 0x0F, 0x04, 0x11, 0x10, 0x0B, 0x2F};
};
