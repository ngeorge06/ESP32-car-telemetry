// ===================== src/CANReader.cpp =====================
#include "reader.h"
#include <mcp_can.h>
#include <SPI.h>

#define CAN_CS_PIN 5

static MCP_CAN CAN(CAN_CS_PIN);

bool CANReader::begin() {
  Serial.println("Initializing MCP2515...");
  if (CAN.begin(MCP_ANY, CAN_500KBPS, MCP_8MHZ) != CAN_OK) {
    Serial.println("MCP2515 init FAILED - check wiring");
    return false;
  }
  Serial.println("MCP2515 initialized OK");
  CAN.setMode(MCP_NORMAL);
  return true;
}

void CANReader::requestPID(byte pid) {
  byte data[8] = {0x02, 0x01, pid, 0x00, 0x00, 0x00, 0x00, 0x00};
  CAN.sendMsgBuf(0x7DF, 0, 8, data);
}

int CANReader::pidIndexOf(byte pid) {
  for (int i = 0; i < PID_COUNT; i++) {
    if (pidList[i] == pid) return i;
  }
  return -1;
}

void CANReader::update(VehicleData& data) {
  if (CAN_MSGAVAIL == CAN.checkReceive()) {
    long unsigned int rxId;
    byte len;
    byte buf[8];
    CAN.readMsgBuf(&rxId, &len, buf);

    if (rxId == 0x7E8 && buf[1] == 0x41) {
      byte pid = buf[2];
      int idx = pidIndexOf(pid);
      if (idx != -1) pidConfirmed[idx] = true; // mark real, seen-at-least-once support

      switch (pid) {
        case 0x0C: data.rpm = ((buf[3] * 256) + buf[4]) / 4; break;
        case 0x0D: data.speedKph = buf[3]; break;
        case 0x05: data.coolantTempC = buf[3] - 40; break;
        case 0x0F: data.intakeTempC = buf[3] - 40; break;
        case 0x04: data.engineLoadPct = (buf[3] * 100) / 255; break;
        case 0x11: data.throttlePct = (buf[3] * 100) / 255; break;
        case 0x10: data.mafGramsPerSec = ((buf[3] * 256) + buf[4]) / 100.0; break;
        case 0x0B: data.manifoldPressureKpa = buf[3]; break;
        case 0x2F: data.fuelLevelPct = (buf[3] * 100) / 255; break;
      }
    }
  }

  data.mafSupported = pidConfirmed[pidIndexOf(0x10)];
  data.manifoldPressureSupported = pidConfirmed[pidIndexOf(0x0B)];
  data.fuelLevelSupported = pidConfirmed[pidIndexOf(0x2F)];

  if (millis() - lastRequest > REQUEST_INTERVAL) {
    requestPID(pidList[currentPidIndex]);
    currentPidIndex = (currentPidIndex + 1) % PID_COUNT;
    lastRequest = millis();
  }
}