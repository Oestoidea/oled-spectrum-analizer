# ESP8266_oled_list_ssd1306

Spectrum analyzer ESP8266 on with I2C OLED's SSD1306. The spectral spectral width is limited to thirteen Wi-Fi channels. Displays data only for channels but not spectrum. This scheme takes less then 50mA (on 5V).

## Equipment

1. ESP8266 ([ESP-01 fritzing part](https://github.com/Oestoidea/oled-spectrum-analizer/blob/master/fritzing-parts/ESP8266-1.fzpz))
2. OLED 0.96" 128×64 ([fritzing part](https://github.com/Oestoidea/oled-spectrum-analizer/blob/master/fritzing-parts/OLED%200.96%20128x64%20I2C%20SSD1306.fzpz)) or 0.91" 128×32 I2C SSD1306
3. AMS1117 module (or LM1117)

![ESP8266_oled_list_ssd1306_photo]()

## Display

Connect OLED to ESP8266 as shown on the picture.

![ESP8266_oled_list_ssd1306_scheme]()

## ESP8266

Install [ESP8266](http://esp8266.ru/arduino-ide-esp8266/#fast-start) (Russian) libraries in Arduino EDI. This scanner based on standart WiFiScan from SDK and [ESP8266-I2C-OLED](https://github.com/costonisp/ESP8266-I2C-OLED) written by costonisp.

## Connection map

| ESP8266-1    | OLED          |
| ------------ | ------------- |
| GPIO0        | SCL           |
| GPIO2        | SDA           |

| ESP8266-1    | OLED         | AMS1117       |
| ------------ | ------------ | ------------- |
| 3V3, CH_PD   | 3V3          | 3V3           |
| GND          | GND          | GND           |

## Problems

ESP8266 does not have 5V input supply then we have to use an additional voltage converter (AMS1117).

## Implementation

The prototype is made with plug-in modules for debugging convenience. Additionally, you can display the COM-port connectors (Rx, Tx, GND) to read odnovremennoego list of networks.

![ESP8266_oled_list_ssd1306_photo]()
