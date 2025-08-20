# BricQT

BricQT is a Qt 6 based client for connecting to **BRIC4** instruments over Bluetooth Low Energy (BLE).  
It provides a clean C++/QML interface for downloading measurement data, visualizing it, and controlling the device.

---

## Features
- BLE discovery and connection using QtBluetooth (`QLowEnergyController`)
- Full support for BRIC4 custom services:
  - Measurement sync (Primary, Metadata, Errors)
  - Device control commands (e.g. `shot`, `scan`, `laser`, `power off`)
  - Last Time characteristic for selective downloads
- C++ structs for strongly typed parsing of BRIC measurement data
- `BricMeasurementModel` (`QAbstractListModel`) with QML role access
- iOS permission handling via a small CoreBluetooth shim
- Cross-platform: macOS, iOS, Android, Linux, Windows (where QtBluetooth is available)
