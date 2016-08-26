# Wixel_2oleds

Spectrum analyzer on Pololu Wixel with SPI and/or I2C OLED's SSD1306. This scheme takes less then 10mA (on 5V).

## Equipment

1. Pololu Wixel
2. OLED 64x128 SSD1306 I2C
3. OLED 64x128 SSD1306 SPI

![Wixel_2oleds_photo](https://github.com/Oestoidea/oled-spectrum-analizer/blob/master/Wixel_2oleds/pics/Wixel_2oleds.png)

## Wixel

Put the firmware on Wixel with parameters __spi_on__ and __i2c_on__. For example, to compile and download the firmware with [wixel-sdk](http://pololu.github.io/wixel-sdk/) on OS Windows:

```
C:\wixel-sdk>make load_spi_spectrum S="spi_on=1 i2c_on=1"
```

More information about Wixel apps you can see on [official site](https://www.pololu.com/docs/0J46/10.b).

## Displays

Connect OLED's to Wixel as shown on the picture.

![Wixel_2oleds_scheme](https://github.com/Oestoidea/oled-spectrum-analizer/blob/master/Wixel_2oleds/fritzing-scheme/Wixel_2oleds_bb.png)
