# Waveshare ESP32-S3-RS485-CAN — ESPHome Modbus TCP ↔ RTU gateway

This project wraps the working eModbus bridge architecture from the original
`main.cpp` in an ESPHome external component.

## Target hardware

Waveshare ESP32-S3-RS485-CAN:

- RS485 TX: GPIO17
- RS485 RX: GPIO18
- RS485 DE/~RE: GPIO21
- UART: UART1

## Preserved defaults from the original firmware

- Modbus RTU: 9600 baud, 8N1
- Modbus slave/unit ID: 1
- Modbus TCP port: 502
- Maximum TCP clients: 4
- TCP inactivity timeout: 5000 ms
- RTU response timeout: 2000 ms
- All Modbus function codes are forwarded by `ANY_FUNCTION_CODE`
- eModbus 1.7.4

## Installation

1. Copy this whole directory under your ESPHome configuration directory.
2. Copy `secrets.example.yaml` to `secrets.yaml` and edit Wi-Fi credentials.
3. Validate:

   ```bash
   esphome config waveshare-modbus-gateway.yaml
   ```

4. Compile:

   ```bash
   esphome compile waveshare-modbus-gateway.yaml
   ```

5. First flash can be done over USB. Afterwards ESPHome OTA is available.

## Why Arduino + PlatformIO?

The eModbus library used by the original firmware is an Arduino/ESP32 library.
ESPHome 2026.x defaults increasingly toward ESP-IDF/native tooling. This YAML
explicitly selects the Arduino framework and PlatformIO toolchain to stay as
close as possible to the already-tested standalone firmware environment.

## Architecture

```text
Modbus TCP client
       |
       | TCP/502 over Wi-Fi
       v
ESP32-S3 + eModbus ModbusBridgeWiFi
       |
       | Modbus RTU, UART1
       v
GPIO17 TX / GPIO18 RX / GPIO21 DE~RE
       |
       v
Waveshare isolated RS485 interface
       |
       v
Modbus RTU slave ID 1
```

## Notes

- Do not add an ESPHome `uart:` component on UART1 for these pins. This external
  component owns UART1 directly because eModbus expects an Arduino
  `HardwareSerial` object.
- ESPHome handles Wi-Fi reconnection, so the original firmware's explicit
  restart-on-Wi-Fi-loss logic is intentionally not copied.
- The component currently uses fixed serial framing 8N1, matching the original
  firmware. Baud rate is configurable from YAML.
- If you need several RTU slave IDs behind one gateway, the component can be
  extended to attach a range or several mappings.


## Waveshare board memory/USB settings

The supplied configuration mirrors the known-good PlatformIO environment for this board:

- 16 MB flash
- 8 MB OPI/Octal PSRAM
- `board_build.arduino.memory_type = qio_opi`
- `default_16MB.csv` partition table
- USB CDC enabled at boot (`ARDUINO_USB_MODE=1`, `ARDUINO_USB_CDC_ON_BOOT=1`)
- `lib_ldf_mode = deep+`
- upload speed 921600

`upload_port` is intentionally not hard-coded in the ESPHome YAML. ESPHome can use `/dev/ttyACM0` when passed on the command line for the first flash, and OTA afterward.
