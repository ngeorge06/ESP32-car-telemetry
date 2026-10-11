// ===================== include/WebInterface.h =====================
#pragma once
#include "data.h"

class WebInterface {
public:
  void begin(const char* apSsid, const char* apPassword);
  void handle();
  void setData(VehicleData* data);

private:
  VehicleData* vehicleData = nullptr;
};