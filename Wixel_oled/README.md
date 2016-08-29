# Wixel_oled

Spectrum analyzer on Pololu Wixel with SPI OLED's SSD1331. This scheme takes less then 10mA (on 5V).

## Equipment

1. Pololu Wixel
2. OLED 0.95" 96×64 SPI SSD1331

![Wixel_oled_photo]()

## Wixel

Put the firmware on Wixel. For example, to compile and download the firmware with [wixel-sdk](http://pololu.github.io/wixel-sdk/) on OS Windows:

```
C:\wixel-sdk>make load_Wixel_oled
```

More information about Wixel apps you can see on [official site](https://www.pololu.com/docs/0J46/10.b).

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
