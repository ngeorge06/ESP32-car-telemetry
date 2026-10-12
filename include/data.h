#pragma once

// everything the dashboard shows
struct VehicleData {
  int rpm = 0;
  int speedKph = 0;
  int coolantTempC = 0;
  int intakeTempC = 0;
  int engineLoadPct = 0;
  int throttlePct = 0;

  // these only show up on the dashboard if the car actually answers
  float mafGramsPerSec = 0;
  bool mafSupported = false;
  int manifoldPressureKpa = 0;
  bool manifoldPressureSupported = false;
  int fuelLevelPct = 0;
  bool fuelLevelSupported = false;
};
