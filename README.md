# BBs Counter 2.0

An ATtiny84-based control board for airsoft guns (ASG). It counts fired BBs with an infrared sensor, monitors the state of a 2S LiPo battery, and keeps the BB count of the last five games in EEPROM.

- **Hardware version:** 2.0
- **Software version:** 3.0
- **Board revision (EasyEDA schematic):** 3.0

## Features

- **BB counting:** an IR emitter and photodiode detect each BB. An LM393 comparator, with a trimmer setting the threshold, feeds a falling-edge external interrupt (INT0).
- **Battery monitoring:**
  - Total pack voltage and the voltage of each cell.
  - Absolute percentage (8.40 V = 100 %) and relative percentage (6.20 V = 0 %, 8.40 V = 100 %).
  - Warning when the difference between the two cells exceeds 0.2 V.
  - Warning when the relative charge falls below 25 %.
- **Game history:** the BB count of the last five games is stored in EEPROM. A new game is detected automatically.
- **Settings menu:** adjustable voltage sampling interval (10 s, 20 s, 30 s), display refresh interval (0.5 s, 1 s, 3 s), and EEPROM save interval (2, 5, 10 minutes). The settings are saved to EEPROM.
- **I²C OLED display:** the display can be replaced with an I²C LCD, but the libraries and the current draw must be checked first.

## Hardware

| Block | Details |
|---|---|
| MCU | ATtiny84 (8 KB flash, 512 B SRAM), internal 8 MHz oscillator |
| Power | 2S LiPo (7.4 V nominal, 6.4 V critical, 8.4 V max); fuse; MCP1702T-3.3 LDO (3.3 V, 250 mA) |
| Voltage sensing | Two resistor dividers (62 kΩ / 27 kΩ for the pack, 16 kΩ / 27 kΩ for cell 1) read by the 10-bit ADC, referenced to VCC |
| BB sensing | IR LED and photodiode, LM393 comparator, trimmer threshold (about 0.41 V on the inverting input), 4.7 kΩ pull-up on the interrupt pin |
| Display | I²C OLED, 4-pin JST connector, 10 kΩ pull-ups on SDA/SCL |
| Inputs | Two push buttons (AVANTI, CONFERMA) and a 3-position selector switch |
| Connectors | JST-XH for battery, JST-XH for IR/photodiode, JST-XH for OLED, 1×6 header for ISP programming, 1×2 header for voltage testing |

Passive components are 0805 SMD. The board was designed in EasyEDA. Total cost is about 12.81 € per board, including the display (about 7 €) and shipping.

## Repository contents

- `Code/Code.ino`: Arduino sketch for the ATtiny84.
- `schematic.png`: schematic.
- `PCB.pdf`: PCB layout.
- `BOM.pdf`: bill of materials.
- `Total Cost.pdf`: cost breakdown per board.
- `Report.pdf`: project report (in Italian).

## Requirements

- Arduino IDE with the ATtiny core (ATtiny84 support).
- Libraries: `TinyOzOLED`, `TinyWireM`, `USI_TWI_Master`, `EEPROM` (included with the Arduino IDE).

## Programming

The ATtiny84 is programmed in ISP mode, through the 1×6 header on the board:

- With an Arduino Mega running the *ArduinoISP* sketch, or
- With an FTDI USB-to-serial programmer.

Pinout of the programming header: VCC, GND, MOSI, MISO, SCK, RESET.

## Operation

1. Connect the battery through the JST-XH connector.
2. The board shows the splash screen and reads the last saved game data.
3. **Main screen:** pellet count, pack voltage, cell 1 voltage, cell 2 voltage, and charge percentages.
4. **AVANTI:** shows the last five games. Press CONFERMA to return to the main screen.
5. **Selector switch in position 2 (settings):** AVANTI moves between settings, CONFERMA changes the value. The new values are saved when you return to the main screen.

## Notes

- Each voltage reading is the average of 4 samples taken at the configured interval.
- The IR interrupt counts only falling edges, to avoid false counts if the signal stays low.

## Authors and history

Developed as a personal project. The hardware is at version 2.0 and the software is at version 3.0.
