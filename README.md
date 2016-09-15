This project created to show ISM 2.4 GHz spectrum on SSD1306 and SSD1331 OLED's on Wixel, Arduino Nano, ESP8266 or Raspberry Pi.

# Structure

This project helps to biuld spectrum analyzer on different modules. It contains next Wixel subprojects:

* [_Pololu Wixel_ with SPI and/or I2C _OLED's SSD1306_](https://github.com/Oestoidea/oled-spectrum-analizer/tree/master/Wixel/Wixel_2oleds_ssd1306)
* [_Pololu Wixel_ with two SPI and I2C _OLED's SSD1306_](https://github.com/Oestoidea/oled-spectrum-analizer/tree/master/Wixel/Wixel_3oleds_ssd1306)
* [_Pololu Wixel_ with SPI _OLED SSD1331_](https://github.com/Oestoidea/oled-spectrum-analizer/tree/master/Wixel/Wixel_oled_ssd1331)

And another subprojects:

* [_Raspberry Pi 3_ and _Pololu Wixel_ with SPI and/or I2C _OLED's SSD1306_](https://github.com/Oestoidea/oled-spectrum-analizer/tree/master/RPi)
* [_Arduino Nano v3_ and _TI CC2500+PA+LNA_ with two SPI and I2C _OLED's SSD1306_](https://github.com/Oestoidea/oled-spectrum-analizer/tree/master/Arduino_Nano)
* [_ESP8266-1_ with I2C _OLED's SSD1306_](https://github.com/Oestoidea/oled-spectrum-analizer/tree/master/ESP8266/ESP8266_oled_spectrum_ssd1306)
* [_ESP8266-1_ with I2C _OLED's SSD1306_](https://github.com/Oestoidea/oled-spectrum-analizer/tree/master/ESP8266/ESP8266_oled_list_ssd1306)

# Projects Comparison Table

| Equipment    | OLED Chip | OLED Size and Interface | Spectrum | Channels | APs |      |
| ------------ | --------- | ----------------------- |:--------:|:--------:|:---:| ---- |
| Wixel        | SSD1306   | 128×64 (SPI, I2C), 128×32 (I2C) | ✔ |  |  |  [🔗](https://github.com/Oestoidea/oled-spectrum-analizer/tree/master/Wixel/Wixel_2oleds_ssd1306) |
| Wixel        | SSD1306   | 128×64 (2×SPI, I2C) | ✔ | ✔ |  | [🔗](https://github.com/Oestoidea/oled-spectrum-analizer/tree/master/Wixel/Wixel_3oleds_ssd1306) |
| Wixel        | SSD1331   | 96×64 (SPI) | ✔ | ✔ |  |  [🔗](https://github.com/Oestoidea/oled-spectrum-analizer/tree/master/Wixel/Wixel_oled_ssd1331) |
| Raspberry Pi 3, Wixel | SSD1306   | 128×64 (SPI, I2C) | ✔ |  |  |  [🔗](https://github.com/Oestoidea/oled-spectrum-analizer/tree/master/RPi) |
| Arduino Nano v3, TI CC2500+PA+LNA | SSD1306   | 128×64 (2×SPI, I2C) | ✔ |  |  |  [🔗](https://github.com/Oestoidea/oled-spectrum-analizer/tree/master/Arduino_Nano) |
| ESP8266-1 | SSD1306   | 128×64 (SPI, I2C) |  | ✔ |  |  [🔗](https://github.com/Oestoidea/oled-spectrum-analizer/tree/master/ESP8266/ESP8266_oled_spectrum_ssd1306) |
| ESP8266-1 | SSD1306   | 128×64 (SPI, I2C), 128×32 (I2C) |  |  | ✔ | [🔗](https://github.com/Oestoidea/oled-spectrum-analizer/tree/master/ESP8266/ESP8266_oled_list_ssd1306) |

# Fritzing Parts

Also you can find additional parts for [Fritzing](http://fritzing.org/home/) used in this project schemes:

* [Pololu Wixel](https://github.com/Oestoidea/oled-spectrum-analizer/blob/master/fritzing-parts/OLED%200.96%20128x64%20I2C%20SSD1306.fzpz) 
* [OLED 0.96" 128×64 SPI SSD1306](https://github.com/Oestoidea/oled-spectrum-analizer/blob/master/fritzing-parts/OLED%200.96%20128x64%20SPI%20SSD1306.fzpz)
* [OLED 0.96" 128×64 I2C SSD1306](https://github.com/Oestoidea/oled-spectrum-analizer/blob/master/fritzing-parts/OLED%200.96%20128x64%20I2C%20SSD1306.fzpz)
* [OLED 0.95" 96×64 SPI SSD1331](https://github.com/Oestoidea/oled-spectrum-analizer/blob/master/fritzing-parts/OLED%200.95%2096x64%20SPI%20SSD1331.fzpz)
* [CC2500+PA+LNA](https://github.com/Oestoidea/oled-spectrum-analizer/blob/master/fritzing-parts/CC2500%2BPA%2BLNA.fzpz)
* [ESP8266-1](https://github.com/Oestoidea/oled-spectrum-analizer/blob/master/fritzing-parts/ESP8266-1.fzpz)

# Future Plans

* To make a combination device with Arduino and ESP8266.
* To add in Raspberry Pi project another low-cost ISM spectrum analyzers (Metageek Wi-Spy 2.4i and 2.4x, Ubiquiti AirView2, Wi-detector, and TI eZ430-RF2500) by USB.
* To connect Cypress CYWUSB6935 module to Arduino and STM32 by SPI.
* To connect nRF24L01 module to Arduino and STM32 by SPI.
* To connect CC2500 module to STM32 with OLED indication.

# Author

Vladimir Sokolov aka Oestoidea

# Licenses

The source code are licensed under the [GNU GPLv3](https://www.gnu.org/licenses/gpl-3.0.html).
The schematics are licensed under the [CC-BY-SA 3.0](http://creativecommons.org/licenses/by-sa/3.0/).
