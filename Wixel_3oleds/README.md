# Wixel_3oleds

Spectrum analyzer on Pololu Wixel with SPI and/or I2C OLED's SSD1306. This scheme takes less then 10mA (on 5V).

## Equipment

1. Pololu Wixel
2. OLED 64x128 SSD1306 I2C
3. OLED 64x128 SSD1306 SPI

![Wixel_3oleds_photo]()

## Wixel

Put the firmware on Wixel with parameters __show_grid__ (for grids) and __I2C_on__ (for additonal I2C display). For example, to compile and download the firmware with [wixel-sdk](http://pololu.github.io/wixel-sdk/) on OS Windows:

```
C:\wixel-sdk>make load_Wixel_3oleds S="show_grid=1 I2C_on=1"
```

More information about Wixel apps you can see on [official site](https://www.pololu.com/docs/0J46/10.b).

## Displays

Connect OLED's to Wixel as shown on the picture.

![Wixel_3oleds_scheme](https://github.com/Oestoidea/oled-spectrum-analizer/blob/master/Wixel_3oleds/fritzing-scheme/Wixel_3oleds_bb.png)
