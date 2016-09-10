# Arduino_Nano_3oleds

Spectrum analyzer on Arduino Nano and TI CC2500+PA+LNA with SPI and/or I2C OLED's SSD1306. The spectral width is 2400.01–2503.40 MHz with spacing in 405.456543 kHz on two SPI displays. Displays logo on I2C display. This scheme takes less then 50mA (on 5V).

## Equipment

1. Arduino Nano v3.0 (with 3.3V)
2. TI CC2500+PA+LNA module with external antenna
3. OLED 0.96" 128×64 I2C SSD1306
4. Two OLED`s 0.96" 128×64 SPI SSD1306

![Arduino_Nano_3oleds_photo](https://github.com/Oestoidea/oled-spectrum-analizer/blob/master/Arduino_Nano_3oleds/pics/Arduino_Nano_3oleds.png)

## Displays and CC2500+PA+LNA module

Connect OLED's and CC2500+PA+LNA to Arduino Nano as shown on the picture.

![Arduino_Nano_3oleds_scheme](https://github.com/Oestoidea/oled-spectrum-analizer/blob/master/Arduino_Nano_3oleds/fritzing-scheme/Arduino_Nano_3oleds_bb.png)

## Arduino Nano

Install [Adafruit GFX](https://github.com/adafruit/Adafruit-GFX-Library) and [Adafruit SSD1306](https://github.com/adafruit/Adafruit_SSD1306) libraries in Arduino EDI. This scanner based on [Scanner 2.4 GHz Range of Ready-Made Modules](https://dev.rcopen.com/forum/f8/topic397991) (Russian) written by Valeriy Yatsenkov (aka Rover).

## Connection map

| Arduino Nano | CC2500        |
| ------------ | ------------- |
| D10          | CSN           |
| D11          | SI            |
| D12          | SO            |
| D13          | SCLK          |
| 3V3          | LEN           |
| 3V3          | VCC           |
| GND          | PEN           |
| GND          | GND           |

| Arduino Nano | SPI0 OLED     |
| ------------ | ------------- |
| D9           | CS            |
| D7           | D/C           |
| D6           | DIN (SDA)     |
| D5           | CLK           |
| D4           | RES           |
| 3V3          | VCC           |
| GND          | GND           |

| Arduino Nano | SPI1 OLED     |
| ------------ | ------------- |
| D8           | CS            |
| D7           | D/C           |
| D6           | DIN (SDA)     |
| D5           | CLK           |
| D3           | RES           |
| 3V3          | VCC           |
| GND          | GND           |

| Arduino Nano | I2C OLED      |
| ------------ | ------------- |
| A5 (19)      | SCK           |
| A4 (18)      | SDA           |
| 3V3          | VCC           |
| GND          | GND           |

| Arduino Nano | switch        |
| ------------ | ------------- |
| A3 (17)      | normally open |
| GND          | normally open |

| Arduino Nano | power supply  |
| ------------ | ------------- |
| 5V           | 5V            |
| GND          | GND           |

## Problems

Arduino Nano does not have enough memory, because it was not possible to realize the display (I2C) of available channels. The project requires further optimization.

## Implementation

Prototype is assembled in a clear acrylic case for Rasberry Pi, but can be built more compactly. Button with a red cap — pause.

![Arduino_Nano_3oleds_photo](https://github.com/Oestoidea/oled-spectrum-analizer/blob/master/Arduino_Nano_3oleds/pics/Arduino_Nano_3oleds2.png)
