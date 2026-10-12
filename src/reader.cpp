#include "reader.h"
#include <mcp_can.h>
#include <SPI.h>

#define CAN_CS_PIN 5  // chip select pin of the MCP2515

static MCP_CAN CAN(CAN_CS_PIN);

bool CANReader::begin() {
  Serial.println("Starting MCP2515...");
  
  // 500 kbps, 8 MHz crystal
  if (CAN.begin(MCP_ANY, CAN_500KBPS, MCP_8MHZ) != CAN_OK) {
    Serial.println("MCP2515 failed to start, check the wiring");
    return false;
  }

  CAN.setMode(MCP_NORMAL);
  canReady = true;
  Serial.println("MCP2515 ready");
  return true;
}

void CANReader::requestPID(byte pid) {
  // standard OBD-II request: mode 01 (live data) for one PID, sent to 0x7DF
  byte request[8] = {0x02, 0x01, pid, 0x00, 0x00, 0x00, 0x00, 0x00};
  CAN.sendMsgBuf(0x7DF, 0, 8, request);
}

void CANReader::update(VehicleData& data) {
  if (!canReady) return;

  if (CAN.checkReceive() == CAN_MSGAVAIL) {
    unsigned long id;
    byte len;
    byte buf[8];
    CAN.readMsgBuf(&id, &len, buf);

    // 0x7E8 = reply from the engine ECU
    // 0x41 = reply to a mode 01 request
    if (id == 0x7E8 && len >= 5 && buf[1] == 0x41) {
      byte pid = buf[2];

      // buf[3] is byte A and buf[4] is byte B in the OBD-II formulas
      switch (pid) {
        case 0x0C: data.rpm = ((buf[3] * 256) + buf[4]) / 4; break;
        case 0x0D: data.speedKph = buf[3]; break;
        case 0x05: data.coolantTempC = buf[3] - 40; break;
        case 0x0F: data.intakeTempC = buf[3] - 40; break;
        case 0x04: data.engineLoadPct = (buf[3] * 100) / 255; break;
        case 0x11: data.throttlePct = (buf[3] * 100) / 255; break;
        case 0x10:
          data.mafGramsPerSec = ((buf[3] * 256) + buf[4]) / 100.0;
          data.mafSupported = true;
          break;
        case 0x0B:
          data.manifoldPressureKpa = buf[3];
          data.manifoldPressureSupported = true;
          break;
        case 0x2F:
          data.fuelLevelPct = (buf[3] * 100) / 255;
          data.fuelLevelSupported = true;
          break;
      }
    }
  }

  // ask for the next PID every REQUEST_INTERVAL ms
  if (millis() - lastRequest > REQUEST_INTERVAL) {
    requestPID(pidList[currentPidIndex]);
    currentPidIndex = (currentPidIndex + 1) % PID_COUNT;
    lastRequest = millis();
  }
}
