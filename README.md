# wateringsystem-firmware
WeMos D1 based plant wateringsystem.

![circuit diagram](/wateringsystem_bb.png)

## additional libraries
- PubSubClient by Nick O’Leary (Version 2.6.0)
- NTPClient (Version 3.1.0)
- [Rtc by Makuna](https://github.com/Makuna/Rtc) (Version 2.0.1)
- [DHT sensor library by Adafruit](https://github.com/adafruit/DHT-sensor-library) (Version 1.2.3)
- [ArduinoJson](https://github.com/bblanchon/ArduinoJson) (Version 5.9.0)
- [esp8266-oled-ssd1306](https://github.com/squix78/esp8266-oled-ssd1306) (Version 3.2.7)

## MQTT commands
- "wateringsystem/actions/watering" + payload (int value between 1000 and 180000): to activate the pump relay for the paload time in milliseconds
- "wateringsystem/actions/measuring" no payload: starting measurement and send values via MQTT to client
- "wateringsystem/actions/rtc/adjust" no payload: load datetime from NTP
