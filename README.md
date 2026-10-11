# ESP-32 Car Telemetry Dashboard

An ESP32-based system that parses live engine data via my car's OBD-II port and shows it on a dashboard you open on a phone or tablet. This repo also includes a [KiCad design](#Carrier-PCB) for a custom PCB that replaces my breadboard wiring.

Tested on: 2016 VW Touareg (3.6 V6 petrol)

<p align="center">
  <img src="docs/images/dashboard.png" height="320" alt="Live dashboard on a tablet">
  <img src="docs/images/car_breadboard.png" height="320" alt="The breadboard version running in the car">
</p>

## What it does

- Connects to the car's CAN bus (ISO 15765-4, 500 kbit/s, 11-bit IDs) using an MCP2515 controller. It sends Mode 01 requests to `0x7DF` and reads the replies from `0x7E8`.
- Parses response and displays RPM, speed, coolant temperature, intake air temperature, engine load and throttle position. Mass air flow, manifold pressure and fuel level are requested too, and their cards only appear if ECU successfully responds.
- The ESP32 runs its own Wi-Fi access point and a lightweight web server. The dashboard page polls a JSON endpoint (`/data`) every 150 ms.

## Hardware

The breadboard version (ESP32 DevKit V1 + MCP2515 module) works in the car. The carrier PCB is a KiCad 10 design (2 layers, 130 × 44 mm) that plugs both boards into sockets and runs from the car's 12 V. **It is a design only, not fabricated or tested.** ERC and DRC report 0 errors and 0 warnings.

<p align="center">
  <img src="docs/images/PCB.png" width="48%" alt="PCB layout with top and bottom copper">
  <img src="docs/images/3D_model.png" width="48%" alt="3D render of the board">
</p>

![Schematic](docs/images/schematic.png)

- 12 V goes through a fuse, a TVS diode and a reverse-polarity diode into an LM2596S-5 buck regulator.
- A 74AHCT125 buffer handles the 3.3 V to 5 V SPI lines, with a resistor divider on MISO.
- Socket spacings are measured from my own boards, and the ESP32's USB and antenna ends hang over the board edge.


## Run it

```
cd firmware
pio run --target upload
```

SPI pins: MOSI GPIO23, MISO GPIO19, SCK GPIO18, CS GPIO5. Join the `TouaregDash` Wi-Fi network and open `http://192.168.4.1`. The Wi-Fi password is set in `main.cpp`, so change it.

## Issues I faced

**Corrupted CAN data.** A cheap MOSFET level converter between the 3.3 V ESP32 and the 5 V MCP2515 module corrupted the SPI data (frame IDs like `0x9FFFFFFF`). Loopback mode, which uses no car or cable, still showed the corruption, and wiring SPI directly gave clean data. The converter was most likely too slow for the SPI clock. I replaced it with a 1 kΩ / 2 kΩ divider on MISO, and the PCB uses a 74AHCT125 for MOSI, SCK and CS.

**The handbrake fault codes.** Mode 03 and Mode 07 requests were answered by two ECUs (`0x7E8` and `0x7E9`), and both reported zero codes while the dash showed a check-engine light, EPC and a headlight warning. Passive listening and active scans (VW TP 2.0, UDS requests across `0x700` to `0x7BF`) found nothing from the body or brake modules. That fits VW's gateway design, and reading them would need manufacturer-specific diagnostics I couldn't get. So the original problem is unsolved.

**Unsupported data.** My car doesn't report gear, oil temperature or engine torque, so the dashboard doesn't show them.

## Future ideas

- Read the parking-brake module's fault codes with a proper diagnostic tool.
- Fabricate the board after checking the footprints against a 1:1 printout.