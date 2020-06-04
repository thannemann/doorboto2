# Hardware

The Doorboto set up consist of  Raspberry Pi running the Doorboto JS code which interfaces with an Arduino via USB/serial communications attached to a relay, bi-color LED indicator, and RFID reader for door access.

## Pinout

Raspberry Pi USB port -> Arduino nano J1 (Micro USB)
 
RELAY digital 4 (and LED if wanting to monitor relay output) also diode in parallel
RED_LED digital 2
GREEN_LED digital 3
RFID SS_PIN    10
RFID RST_PIN   9
RFID MOIS PIN 11
RFID MISO PIN 12
RFID 3.3 3.3V
RFID GND GND

LCD SCL A5
LCD SDA A4
LCD VCC 5V
LCD GND GND

beforeRelay voltage divider A6
afterRelay  voltage divider A7


## BOM
Old UPS to hold up Power to the the system if power is out
5 VDC USB power Supply for the Raspberry PiRasperry Pi 
12 VDC powersupply to power the door latch
Door latch: Ximi technology AT-300A-L (Marked on hardware 2016-3 No201604)
arduino compatiable 5 VDC Relay
Bi-color LED  Red/Green
RFID: MFRC522
Arduino: Arduino Nano
