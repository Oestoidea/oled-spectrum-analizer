import time

import Adafruit_GPIO.SPI as SPI
import Adafruit_SSD1306

import serial, time
import threading
import sys, argparse

import re
import findserial

from PIL import Image
from PIL import ImageDraw
from PIL import ImageFont

RST = 25
DC = 9
SPI_PORT = 0
SPI_DEVICE = 0

dispI2C = Adafruit_SSD1306.SSD1306_128_64(rst=RST)
dispSPI = Adafruit_SSD1306.SSD1306_128_64(rst=RST, dc=DC, spi=SPI.SpiDev(SPI_PORT, SPI_DEVICE, max_speed_hz=8000000))

dispI2C.begin()
dispSPI.begin()

class SerialPort(threading.Thread):
    def __init__(self, serial_port, iterations):
         threading.Thread.__init__(self)
        self.daemon = True
        self.iterations = iterations
        self.ser = serial.Serial()
        self.ser.port = serial_port
        self.ser.baudrate = 9600
        self.ser.bytesize = serial.EIGHTBITS
        self.ser.stopbits = serial.STOPBITS_ONE
        self.ser.parity = serial.PARITY_NONE    #set parity check: no parity
        self.ser.timeout = 1                    #non-block read
        self.ser.xonxoff = False                #disable software flow control
        self.ser.rtscts = False                 #disable hardware (RTS/CTS) flow control
        self.ser.dsrdtr = False                 #disable hardware (DSR/DTR) flow control
    
    def run(self):
        try:
            self.ser.open()
            if (self.ser.isOpen()):
                print("Port %s is opened." % self.ser.port)
        except serial.SerialException:
            print("Error with opening serial port %s." % self.ser.port)
            exit()

        if self.ser.isOpen():
            self.ser.flushInput()
            self.ser.flushOutput()
            time.sleep(0.5)
            numOfLines = 0

            while True:
                try:
                    response = self.ser.readline().decode("utf-8")
                    if response == '':
                        print("%s: unreachable." % self.ser.port)
                    elif response != '\n':
                        if (numOfLines >= self.iterations):
                            break
                        numOfLines += 1
                        print(''.join(response.splitlines()))
                        result = re.findall(r'-\d{2,3}',response)

                        j = 0
                        result_odd = []
                        for rssi in result:
                            result[j] = 105 + int(rssi)
                            if result[j] > 64:
                                result[j] = 64
                            if j % 2 == 1:
                                result_odd.insert(j // 2, (result[j] + result[j-1]) // 2)
                            j += 1

                        width = dispSPI.width
                        height = dispSPI.height
                        image = Image.new('1', (width, height))
                        draw = ImageDraw.Draw(image)
                        draw.rectangle((0,0,width,height), outline=0, fill=0)
                        padding = 0
                        top = padding
                        bottom = height - padding

                        x = 1
                        while x < 128:
                            draw.line((x, top, x, result_odd[x]), fill=255)
                            x += 1
                        dispI2C.image(image)
                        dispSPI.image(image)
                        dispI2C.display()
                        dispSPI.display()

                except:
                    print("%s: unreachable." % self.ser.port)

            self.ser.close()

        else:
            print("Cannot open serial port.")

def createParser ():
    parser = argparse.ArgumentParser(
        prog = 'specscan',
        description = 'Wi-fi Spectrum on Pololu Wixel',
        epilog = '(c) Oestoidea 2016')
    parser.add_argument ('-i', '--iterations', type = int, default = 3600, help = 'maximum iteration number',
            metavar = 'NUM')
    return parser

if __name__ == '__main__':
    parser = createParser()
    try:
        namespace = parser.parse_args(sys.argv[1:])
    except:
        print('Unknown argument.')

    i = 0
    j = []

    for serial_port in findserial.serial_ports():
        j.insert(i, SerialPort(serial_port, namespace.iterations))
        j[i].start()
        i += 1
    try:
        j[0].join()
    except:
        print('No connected devices.')

    if (i > 0):
        print('%d iterations for %d devices have successfully done.' % (namespace.iterations, i))
