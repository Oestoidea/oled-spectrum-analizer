# ESP8266_oled_list_ssd1306

Spectrum analyzer ESP8266 on with I2C OLED's SSD1306. Displays first four APs with maximum RSSI. These change every five seconds. This scheme takes less then 50mA (on 5V).

## Equipment

1. ESP8266 ([ESP-01 fritzing part](https://github.com/Oestoidea/oled-spectrum-analizer/blob/master/fritzing-parts/ESP8266-1.fzpz))
2. OLED 0.96" 128×64 ([fritzing part](https://github.com/Oestoidea/oled-spectrum-analizer/blob/master/fritzing-parts/OLED%200.96%20128x64%20I2C%20SSD1306.fzpz)) or 0.91" 128×32 I2C SSD1306
3. AMS1117 module (or LM1117)

![ESP8266_oled_list_ssd1306_photo](https://github.com/Oestoidea/oled-spectrum-analizer/blob/master/ESP8266/ESP8266_oled_list_ssd1306/pics/ESP8266_oled_list_ssd1306.png)

## Display

Connect OLED to ESP8266 as shown on the picture (on example with 0.91" 128×32 I2C OLED).

![ESP8266_oled_list_ssd1306_scheme](https://github.com/Oestoidea/oled-spectrum-analizer/blob/master/ESP8266/ESP8266_oled_list_ssd1306/fritzing-scheme/ESP8266_oled_list_ssd1306_bb.png)

## ESP8266

Install [ESP8266](http://esp8266.ru/arduino-ide-esp8266/#fast-start) (Russian) libraries in Arduino IDE. This scanner based on [ESP8266-I2C-OLED](https://github.com/costonisp/ESP8266-I2C-OLED) written by costonisp.

## Connection Map

| ESP8266-1    | or NodeMCU   | OLED          |
| ------------ | ------------ | ------------- |
| GPIO0        | D3           | SDA           |
| GPIO2        | D4           | SCL           |

| ESP8266-1    | OLED         | AMS1117       |
| ------------ | ------------ | ------------- |
| 3V3, CH_PD   | 3V3          | 3V3           |
| GND          | GND          | GND           |

## Problems

ESP8266 does not have 5V input supply then we have to use an additional voltage converter (AMS1117).

## Implementation

The prototype is made with plug-in modules for debugging convenience. Additionally, you can display the COM-port connectors (Rx, Tx, GND) to read odnovremennoego list of networks.

![ESP8266_oled_list_ssd1306_photo](https://github.com/Oestoidea/oled-spectrum-analizer/blob/master/ESP8266/ESP8266_oled_list_ssd1306/pics/ESP8266_oled_list_ssd1306_2.png)

You can also use the NodeMCU card to connect the display. With the UART-TTL CH340G chip:

![ESP8266_oled_list_ssd1306_photo](https://github.com/Oestoidea/oled-spectrum-analizer/blob/master/ESP8266/ESP8266_oled_list_ssd1306/pics/ESP8266_oled_list_ssd1306_3.png)

With the UART-TTL CP2102 chip :

![ESP8266_oled_list_ssd1306_photo](https://github.com/Oestoidea/oled-spectrum-analizer/blob/master/ESP8266/ESP8266_oled_list_ssd1306/pics/ESP8266_oled_list_ssd1306_4.png)
