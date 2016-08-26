# Arduino_Nano_3oleds

Spectrum analyzer on Arduino Nano and TI CC2500+PA+LNA with SPI and/or I2C OLED's SSD1306.

## Equipment

1. Arduino Nano v. 3 (with 3.3V)
2. TI CC2500+PA+LNA module with external antenna
3. OLED 64x128 SSD1306 I2C
4. Two OLED`s 64x128 SSD1306 SPI

![Arduino_Nano_3oleds_photo]()

## Displays and CC2500+PA+LNA module

Connect OLED's and CC2500+PA+LNA to Arduino Nano as shown on the picture.

![Arduino_Nano_3oleds_scheme](https://github.com/Oestoidea/oled-spectrum-analizer/blob/master/Arduino_Nano_3oleds/fritzing-scheme/Arduino_Nano_3oleds_bb.png)

## Arduino Nano

For correct operation of the display, set the library [Adafruit](https://github.com/adafruit/Adafruit_Python_SSD1306) on Raspberry Pi and add the Python script [RPi_2oleds.py](https://github.com/Oestoidea/oled-spectrum-analizer/blob/master/RPi_2oleds/RPi/RPi_2oleds.py).

```
sudo python3 RPi_2oleds.py
```

_If you have only I2C or SPI display just comment lines with missing connection._


