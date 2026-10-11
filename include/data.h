// ===================== include/VehicleData.h =====================
#pragma once

struct VehicleData {
  volatile int rpm = 0;
  volatile int speedKph = 0;
  volatile int coolantTempC = 0;
  volatile int intakeTempC = 0;
  volatile int engineLoadPct = 0;
  volatile int throttlePct = 0;
  volatile float mafGramsPerSec = 0;
  volatile bool mafSupported = false;
  volatile int manifoldPressureKpa = 0;
  volatile bool manifoldPressureSupported = false;
  volatile int fuelLevelPct = 0;
  volatile bool fuelLevelSupported = false;
};