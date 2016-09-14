# Wixel_2oleds_ssd1306

Spectrum analyzer on Pololu Wixel with SPI and/or I2C OLED's SSD1306. This scheme takes less then 10mA (on 5V).

## Equipment

1. Pololu Wixel ([fritzing part](https://github.com/Oestoidea/oled-spectrum-analizer/blob/master/fritzing-parts/OLED%200.96%20128x64%20I2C%20SSD1306.fzpz))
2. OLED 0.96" 128×64 ([fritzing part](https://github.com/Oestoidea/oled-spectrum-analizer/blob/master/fritzing-parts/OLED%200.96%20128x64%20I2C%20SSD1306.fzpz)) or 0.91" 128×32 I2C SSD1306
3. OLED 0.96" 128×64 SPI SSD1306 ([fritzing part](https://github.com/Oestoidea/oled-spectrum-analizer/blob/master/fritzing-parts/OLED%200.96%20128x64%20SPI%20SSD1306.fzpz))

![Wixel_2oleds_ssd1306_photo](https://github.com/Oestoidea/oled-spectrum-analizer/blob/master/Wixel/Wixel_2oleds_ssd1306/pics/Wixel_2oleds_ssd1306.png)

## Wixel

Put the firmware on Wixel with parameters __spi_on__ and __i2c_on__. For example, to compile and download the firmware with [wixel-sdk](http://pololu.github.io/wixel-sdk/) on OS Windows:

```
C:\wixel-sdk>make load_Wixel_2oleds_ssd1306 S="spi_on=1 i2c_on=1"
```

More information about Wixel apps you can see on [official site](https://www.pololu.com/docs/0J46/10.b). This scanner based on [Spectrum Analyzer](https://github.com/pololu/wixel-sdk/tree/dev/david/analyzer/apps/spectrum_analyzer) written by David E. Grayson.

## Displays

Connect OLED's to Wixel as shown on the picture.

![Wixel_2oleds_ssd1306_scheme](https://github.com/Oestoidea/oled-spectrum-analizer/blob/master/Wixel/Wixel_2oleds_ssd1306/fritzing-scheme/Wixel_2oleds_ssd1306_bb.png)

## Connection map

| Wixel    | SPI OLED      |
| -------- |:-------------:|
| P0_1     | RES           |
| P0_2     | D/C           |
| P0_3     | DIN (SDA)     |
| P0_4     | CS            |
| P0_5     | CLK           |
| 3V3      | VCC           |
| GND      | GND           |

| Wixel    | I2C OLED      |
| -------- |:-------------:|
| P1_0     | SCK           |
| P1_1     | SDA           |
| 3V3      | VCC           |
| GND      | GND           |

| Wixel    | switch        |
| -------- |:-------------:|
| P0_0     | normally open |
| GND      | normally open |

| Wixel    | power supply  |
| -------- |:-------------:|
| VIN      | 2.7–6.5V      |
| GND      | GND           |

## Implementation

Wixel, OLED, and external antenna in the housing is shown below. Button on the left size — switch on, and on the right — pause.

![Wixel_2oleds_ssd1306_implementation](https://github.com/Oestoidea/oled-spectrum-analizer/blob/master/Wixel/Wixel_2oleds_ssd1306/pics/Wixel_2oleds_ssd1306_4.png)

![Wixel_2oleds_ssd1306_implementation](https://github.com/Oestoidea/oled-spectrum-analizer/blob/master/Wixel/Wixel_2oleds_ssd1306/pics/Wixel_2oleds_ssd1306_5.png)

![Wixel_2oleds_ssd1306_implementation](https://github.com/Oestoidea/oled-spectrum-analizer/blob/master/Wixel/Wixel_2oleds_ssd1306/pics/Wixel_2oleds_ssd1306_6.png)
