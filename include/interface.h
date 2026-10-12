#include "data.h"

// wifi access point + web server
class WebInterface {
public:
  void begin(const char* apSsid, const char* apPassword);
  void handle();
  void setData(VehicleData* data);

private:
  VehicleData* vehicleData = nullptr;
};
