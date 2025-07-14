# LidarBox

This Arduino project logs data

## Dependencies

- [TinyGPSPlus](https://docs.arduino.cc/libraries/tinygpsplus/) for the GPS module
- [LSM6](https://docs.arduino.cc/libraries/lsm6/) for the IMU
- [SD](https://docs.arduino.cc/libraries/sd/) for the SD card module

## Hardware

- Arduino ProMicro (or clone)
- Polulu MinIMU-9 Inertial Measurement Unit
- GlobalSat EM506 GPS Module
- LiDAR module (any of):
  - Benewake TF02-Pro
  - Lightware SF11

## Connections

- MinIMU-9:
  - VIN: VCC
  - GND: GND
  - SDA: D2
  - SCL: D3
- EM506:
  - RX: TX0
  - TX: RX1
- SD Card breakout
  - GND: GND
  - CD: leave open
  - D0: D14
  - SCK: D15
  - VCC: VCC
  - DI: D16
  - CS: D10
- Benewake lidar
  - Red (VCC): 5V
  - Black (GND): GND
  - White (SDA): D2
  - Green (SCL): D3

## Notes

The TF02-Pro is set to serial communication by default, it should be set to communicate via I²C with address 0x10.

