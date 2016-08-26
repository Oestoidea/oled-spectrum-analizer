This project created to show ISM 2.4 GHz spectrum on 124x64 OLED's

# RPi_2oleds

Raspberry Pi 3 with SPI and  OLED's SSD1306.

## Equipment

1. Raspberry Pi 3 (OS Raspbian)
2. Pololu Wixel
3. OLED 64x128 SSD1306 I2C
4. OLED 64x128 SSD1306 SPI

## Wixel

Download and unzip __wixelcmd__ tool for load the firmware :

```
wget https://www.pololu.com/file/0J872/wixel-arm-linux-gnueabihf-150527.tar.gz
tar -xzvf wixel-arm-linux-gnueabihf-150527.tar.gz
```

Сonnect the Pololu Wixel to Raspberry Pi by USB and check the connection:

```
sudo ./wixelcmd list
```

Put the firmware on Wixel:

```
sudo ./wixelcmd write RPi_2oleds.wxl -a
```

## Displays

Connect OLED's to Raspberry Pi as shown on the picture.



## Raspberry Pi 3

For correct operation of the display, set the library adafruit (https://github.com/adafruit/Adafruit_Python_SSD1306) on Raspberry Pi and add the Python script RPi_2oleds.py.

```
sudo python3 RPi_2oleds.py
```

_If you have only I2C or SPI display just comment lines with missing connection._
