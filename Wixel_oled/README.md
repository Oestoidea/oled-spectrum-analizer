# Wixel_oled

Spectrum analyzer on Pololu Wixel with SPI OLED's SSD1331. This scheme takes less then 10mA (on 5V).

## Equipment

1. Pololu Wixel ([fritzing part](https://github.com/Oestoidea/oled-spectrum-analizer/blob/master/fritzing-parts/OLED%200.96%20128x64%20I2C%20SSD1306.fzpz))
2. OLED 0.95" 96×64 SPI SSD1331 ([fritzing part](https://github.com/Oestoidea/oled-spectrum-analizer/blob/master/fritzing-parts/OLED%200.95%2096x64%20SPI%20SSD1331.fzpz))

[![Wixel_oled_video](https://github.com/Oestoidea/oled-spectrum-analizer/blob/master/Wixel_oled/pics/Wixel_oled_video.png)](https://www.youtube.com/watch?v=7rlPyKthjLw "Spectrum Analyzer 2.4 GHz on Wixel with OLED SSD1331")

## Wixel

Put the firmware on Wixel. For example, to compile and download the firmware with [wixel-sdk](http://pololu.github.io/wixel-sdk/) on OS Windows:

```
C:\wixel-sdk>make load_Wixel_oled
```

More information about Wixel apps you can see on [official site](https://www.pololu.com/docs/0J46/10.b). This scanner based on [Spectrum Analyzer](https://github.com/pololu/wixel-sdk/tree/dev/david/analyzer/apps/spectrum_analyzer) written by David E. Grayson.

## Displays

Connect OLED's to Wixel as shown on the picture. The initialization and some features borrowed from the project [RGB OLED SSD1331](https://github.com/Seeed-Studio/RGB_OLED_SSD1331) by Lawliet Zou (Seeed Studio).

![Wixel_oled_scheme](https://github.com/Oestoidea/oled-spectrum-analizer/blob/master/Wixel_oled/fritzing-scheme/Wixel_oled_bb.png)

## Buttoms

Buttom __Pause__ stops processes. Buttom __Reset__ puts maximum graph. Buttom __Mode__ toggles between display modes. 

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

| Wixel    | Pause switch  |
| -------- |:-------------:|
| P0_0     | normally open |
| GND      | normally open |

| Wixel    | Reset switch  |
| -------- |:-------------:|
| P1_2     | normally open |
| GND      | normally open |

| Wixel    | Mode switch   |
| -------- |:-------------:|
| P1_3     | normally open |
| GND      | normally open |

| Wixel    | power supply  |
| -------- |:-------------:|
| VIN      | 2.7–6.5V      |
| GND      | GND           |

## Implementation

Prototype is assembled on the breadboard. The right button on this example — reset, the middle button — mode switching, and the left button — pause.

Start screen with the logo appears at reboot.

![Wixel_oled_photo](https://github.com/Oestoidea/oled-spectrum-analizer/blob/master/Wixel_oled/pics/Wixel_oled.png)

The first mode with the current and the maximum spectrum curves and values at the maximum points.

![Wixel_oled_photo](https://github.com/Oestoidea/oled-spectrum-analizer/blob/master/Wixel_oled/pics/Wixel_oled2.png)

The second mode is different information about the free channels instead of the frequency scale.

![Wixel_oled_photo](https://github.com/Oestoidea/oled-spectrum-analizer/blob/master/Wixel_oled/pics/Wixel_oled3.png)

The last mode displays ZigBee and Wi-Fi channels (not all channels fall within the available range). The higher the channel in the histogram, the more free.

![Wixel_oled_photo](https://github.com/Oestoidea/oled-spectrum-analizer/blob/master/Wixel_oled/pics/Wixel_oled4.png)
