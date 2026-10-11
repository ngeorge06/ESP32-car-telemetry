// ===================== include/CANReader.h =====================
#pragma once
#include <Arduino.h>
#include "data.h"

class CANReader {
public:
  bool begin();
  void update(VehicleData& data);

private:
  void requestPID(byte pid);

  int currentPidIndex = 0;
  unsigned long lastRequest = 0;
  static const unsigned long REQUEST_INTERVAL = 150;

  static const int PID_COUNT = 9;
  byte pidList[PID_COUNT] = {0x0C, 0x0D, 0x05, 0x0F, 0x04, 0x11, 0x10, 0x0B, 0x2F};

  // one response-seen flag per PID, set true the FIRST time that PID ever
  // gets a real answer - never cleared once true
  bool pidConfirmed[PID_COUNT] = {false, false, false, false, false, false, false, false, false};

  int pidIndexOf(byte pid);
};