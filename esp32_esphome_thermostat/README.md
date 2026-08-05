# ESP32 Relay AC X2 Peripheral Connections

## GPIO assignment

| GPIO | ESPHome ID | Peripheral | Signal / function | Connection notes |
|---:|---|---|---|---|
| GPIO2 | `button_minus` | Push button | Decrease target temperature | Connect button between GPIO2 and GND; internal pull-up enabled |
| GPIO4 | `button_mode` | Push button | Thermostat ON/OFF | Connect button between GPIO4 and GND; internal pull-up enabled |
| GPIO5 | `thermostat_display` | TM1637 display | DIO | TM1637 data line |
| GPIO15 | `button_plus` | Push button | Increase target temperature | Connect button between GPIO15 and GND; internal pull-up enabled |
| GPIO16 | `relay_1` | On-board relay 1 | Heating output | Internal ESPHome switch; controlled by the thermostat |
| GPIO17 | `relay_2` | On-board relay 2 | Auxiliary relay output | Exposed as `Relay 2` in Home Assistant |
| GPIO18 | `thermostat_display` | TM1637 display | CLK | TM1637 clock line |
| GPIO23 | `output_led` | On-board LED | General-purpose LED | Exposed as `LED` in Home Assistant |
| GPIO25 | I²C bus | SHT3x sensor | SDA | I²C data line |
| GPIO32 | I²C bus | SHT3x sensor | SCL | I²C clock line |

## Peripheral wiring

### SHT3x temperature and humidity sensor

| SHT3x pin | ESP32 Relay AC X2 connection |
|---|---|
| VCC | 3.3 V |
| GND | GND |
| SDA | GPIO25 |
| SCL | GPIO32 |

- I²C address: `0x44`
- I²C frequency: `50 kHz`

### TM1637 four-digit display

| TM1637 pin | ESP32 Relay AC X2 connection |
|---|---|
| VCC | 3.3 V or 5 V, depending on the module |
| GND | GND |
| CLK | GPIO18 |
| DIO | GPIO5 |

### Push buttons

| Button | GPIO | Wiring |
|---|---:|---|
| Temperature + | GPIO15 | Button between GPIO15 and GND |
| Temperature - | GPIO2 | Button between GPIO2 and GND |
| Mode | GPIO4 | Button between GPIO4 and GND |

The buttons use the ESP32 internal pull-up resistors and are therefore active LOW.

### Relay outputs

| Relay | GPIO | Purpose |
|---|---:|---|
| Relay 1 | GPIO16 | Heating control |
| Relay 2 | GPIO17 | Auxiliary output / reserved |

### On-board LED

| LED | GPIO | Purpose |
|---|---:|---|
| On-board LED | GPIO23 | General-purpose status LED |
