#include <wixel.h>
#include <usb.h>
#include <usb_com.h>
#include <stdio.h>
#include <spi0_master.h>
#include <spi1_master.h>
#include <i2c.h>
#include <radio_registers.h>

int32 CODE param_show_grid = 1;
int32 CODE param_I2C_on = 1;

static int16 XDATA rssiValue[256];
static uint8 XDATA initData[25] =
{
    0xAE,			// display off
    0xD5,0x80,		// set display clock divide ratio/oscillator frequency
    0xA8,0x3F,		// set multiplex ratio(1 to 64)
    0xD3,0x00,
    0x40,
    0x8D,0x14,      // включение внутреннего преобразователя напряжения
    0x20,0x00,		// Set Memory Addressing Mode   00 Horizontal Addressing Mode;
                    //                              01 Vertical Addressing Mode;
                    //                              10 Page Addressing Mode (RESET,0);
                    //                              11 Invalid
    0xA1,			// set segment re-map 0 to 127
    0xC8,			// set COM output scan direction
    0xDA,0x12,		// set COM pins
    0x81,0xCF,		// set contrast control register
    0xD9,0xF1,		// set pre-charge period
    0xDB,0x40,		// set vcomh
    0xA4,			// 0xA4 output follows RAM content; 0xA output ignores RAM content
    0xA6,			// 0xA6 normal / reverse
    /*0x21,0x00,0x7F,
    0x22,0x00,0x07,*/
    0xAF			// display on
};

static uint8 XDATA logo[135] =
{
    0xFF,0xFF,0xFF,0xFF,0xEF,0xFF,0xFF,0xFF,0xFF,
    0x00,0x00,
    0xFF,0xFF,0xFF,0xFF,0xFF,0xFB,0xFF,0xFF,0xFF,
    0x00,0x00,
    0x7F,0xF7,0xBF,0xFF,0xFF,0xDF,0x7F,0xFB,0xDF,
    0x00,0x00,
    0x77,0x9F,0xF7,0x5D,0x57,0xBB,0x6F,0x57,0x9B,
    
    0xFF,0xFF,0xFF,0x7F,0xFF,0xFF,0xF7,0xFF,0xFF,
    0x00,0x00,
    0xEF,0xFF,0xBF,0x6E,0xFF,0xBF,0xED,0x5F,0xF5,
    0x00,0x00,
    0xD7,0x3D,0xA7,0x7F,0x84,0x7B,0x8D,0x57,0x34,
    0x00,0x00,
    0x53,0x9A,0x21,0x0B,0x24,0x0B,0x10,0x05,0x00,
    
    0x7F,0xFB,0x8F,0x7F,0xD7,0x3F,0xF6,0x4D,0xB7,
    0x00,0x00,
    0xB6,0x4D,0x53,0x17,0x6C,0x13,0x4D,0x12,0x55,
    0x00,0x00,
    0x08,0x03,0x14,0x01,0x0A,0x00,0x08,0x03,0x00,
    
    0x8B,0x36,0x89,0x27,0x88,0x13,0x24,0x09,0x02,
    0x00,0x00,
    0x04,0x01,0x04,0x01,0x00,0x01,0x00,0x01,0x00
};

static uint8 XDATA txData[1024] = {0};
static uint8 XDATA rxData[0] = {0};

static uint16 XDATA ZigBee[25] = {0}; // only 13 (12-24) from 16 (11-26)
static uint16 XDATA WiFi[12] = {0}; // only 10 (2-11) from 14 (1-14)

// 4-wire SPI0
#define RES0low 	setDigitalOutput(1,0)	// P0_1 to RES
#define RES0high	setDigitalOutput(1,1)

#define DC0low		setDigitalOutput(2,0)	// P0_2 to D/C (MISO)
#define DC0high		setDigitalOutput(2,1)
											// P0_3 to DIN (MOSI)
#define CS0low		setDigitalOutput(4,0)	// P0_4 to CS0 (!CC)
#define CS0high		setDigitalOutput(4,1)
											// P0_5 to CLK
// 4-wire SPI1
#define RES1low 	setDigitalOutput(13,0)	// P1_3 to RES
#define RES1high	setDigitalOutput(13,1)

#define DC1low		setDigitalOutput(17,0)	// P1_7 to D/C (MISO)
#define DC1high		setDigitalOutput(17,1)
											// P1_6 to DIN (MOSI)
#define CS1low		setDigitalOutput(14,0)	// P1_4 to CS1 (!CC)
#define CS1high		setDigitalOutput(14,1)
											// P1_5 to CLK
void updateLeds(void)
{
	usbShowStatusWithGreenLed();
	LED_GREEN(0);
	LED_RED(0);
	LED_YELLOW(0);
}

void SPI0Init(void)
{
	spi0MasterInit();
	spi0MasterSetFrequency(3000000);
	spi0MasterSetClockPolarity(SPI_POLARITY_IDLE_LOW);		// Sets the clock polarity CPOL = 0 (SPI_POLARITY_IDLE_HIGH)
	spi0MasterSetClockPhase(SPI_PHASE_EDGE_LEADING);		// Sets the clock phase CPHA = 0 (SPI_PHASE_EDGE_TRAILING)
	spi0MasterSetBitOrder(SPI_BIT_ORDER_MSB_FIRST);			// The most-significant bit is transmitted first
}
	
void SPI1Init(void)
{
	spi1MasterInit();
	spi1MasterSetFrequency(3000000);
	spi1MasterSetClockPolarity(SPI_POLARITY_IDLE_LOW);		// Sets the clock polarity CPOL = 0 (SPI_POLARITY_IDLE_HIGH)
	spi1MasterSetClockPhase(SPI_PHASE_EDGE_LEADING);		// Sets the clock phase CPHA = 0 (SPI_PHASE_EDGE_TRAILING)
	spi1MasterSetBitOrder(SPI_BIT_ORDER_MSB_FIRST);			// The most-significant bit is transmitted first	
}

void I2CInit(void)
{
	i2cPinScl = 10;			// P1_0 to SLC
    i2cPinSda = 11;			// P1_1 to SDA
	i2cSetFrequency(400);	// kHz
	i2cSetTimeout(10);		// ms
}

void oledSPI0Init(void)
{
	uint8 i;
	
	RES0low;
    delayMicroseconds(3);
    RES0high;
	DC0low;
	CS0low;
	
	for (i = 0; i < 25; i++) 
    {
		spi0MasterSendByte(initData[i]);
		delayMicroseconds(3);
    }
	
	CS0high;
}

void oledSPI1Init(void)
{
	uint8 i;

	RES1low;
    delayMicroseconds(3);
    RES1high;
	DC1low;
	CS1low;
	
	for (i = 0; i < 25; i++) 
    {
		spi1MasterSendByte(initData[i]);
		delayMicroseconds(3);
    }
	
	CS1high;
}

void oledI2CInit(void)
{
	BIT nack;
	uint8 i;
	
	for (i = 0; i < 25; i++) 
    {
        i2cStart();
		i2cWriteByte(0x78);
		i2cWriteByte(0x00);
		nack = i2cWriteByte(initData[i]);
		if (i2cTimeoutOccurred)
		{
			delayMs(50);
			i2cTimeoutOccurred = 0;
		}
		else if (nack)
        {
            LED_RED(1);
        }
        i2cStop();
	}
}

void analyzerInit(void)
{
    radioRegistersInit();

    MCSM0 = 0x14;    // Auto-calibrate  when going from idle to RX or TX.
    MCSM1 = 0x00;    // Disable CCA.  After RX, go to IDLE.  After TX, go to IDLE.
    // We leave MCSM2 at its default value = 0x07
    MDMCFG2 = 0x70;   //disable sync word detection
    RFST = 4; //idle radio
}

void oledSPI0FramePreamble(void)
{
	uint8 i;
	uint8 initData[6] =
	{
		0x21,0x00,0x7F,
		0x22,0x00,0x07
	};
	
	DC0low;
	CS0low;
	
	for (i = 0; i < 6; i++) 
    {
		spi0MasterSendByte(initData[i]);
		delayMicroseconds(3);
    }
	
	CS0high;
}

void oledSPI1FramePreamble(void)
{
	uint8 i;
	uint8 initData[6] =
	{
		0x21,0x00,0x7F,
		0x22,0x00,0x07
	};
	
	DC1low;
	CS1low;
	
	for (i = 0; i < 6; i++) 
    {
		spi1MasterSendByte(initData[i]);
		delayMicroseconds(3);
    }
	
	CS1high;
}

void oledI2CFramePreamble(uint8 startColumn, uint8 stopColumn, uint8 startRow, uint8 stopRow)
{
    i2cSetFrequency(400);
    
    i2cStart();
    i2cWriteByte(0x78);
    i2cWriteByte(0x00);
    i2cWriteByte(0x21);
    i2cWriteByte(startColumn);
    i2cWriteByte(stopColumn);
    i2cWriteByte(0x22);
    i2cWriteByte(startRow);
    i2cWriteByte(stopRow);
    i2cStop();
}

uint16 averageChannel(uint8 first, uint8 last)
{
	uint8 i;
	uint16 rssiChannelSum;
	
	rssiChannelSum = 0;
	for (i = first; i < last + 1; i++)
		rssiChannelSum += rssiValue[i];
	return (uint16) (10 * rssiChannelSum / (last - first + 1));
}

void normalizedWiFi(void)
{
	uint8 i;
	uint16 max = WiFi[2];
    
	for (i = 2; i < 12; i++)
	{
		if (WiFi[i] > max)
			max = WiFi[i];
	}
	
	for(i = 2; i < 12; i++)
		WiFi[i] = (uint16) (20 * (max - WiFi[i]) / max);
}

void normalizedZigBee(void)
{
	uint8 i;
	uint16 max = ZigBee[12];
    
	for (i = 12; i < 25; i++)
	{
		if (ZigBee[i] > max)
			max = ZigBee[i];
	}
	
	for(i = 12; i < 25; i++)
		ZigBee[i] = (uint16) (20 * (max - ZigBee[i]) / max);
}

void checkRadioChannels(void)
{
    uint8 i;
    uint16 channel;
	
    LED_GREEN(1);
	
    for (channel = 0; channel < 256; channel++)
	{
        int32 rssiSum;

        rssiValue[channel] = -115;

        while(MARCSTATE != 1);  //radio should already be idle, but check anyway
        CHANNR = channel;
        RFST = 2;  // radio in RX mode and autocal
        while(MARCSTATE != 13);  //wait for RX mode
        rssiSum = 0;
        for (i = 0; i < 100; i++)
		{
            if (TCON & 2) //radio byte available?
			{
                uint8 rfdata = RFD; // read byte
                TCON &= ~2; //clear ready flag
            }
            rssiSum += radioRssi();
        }
        RFST = 4; //idle radio
        rssiValue[channel] = (int16) (rssiSum/100);
		
		// подготовка к выводу
		
		if (rssiValue[channel] <= -105)
			rssiValue[channel] == 0x00;
		else if (rssiValue[channel] >= -42)
			rssiValue[channel] == 0xFF;
		else
			rssiValue[channel] += 105;
    }  // the above loop takes about 414 ms on average, so about 1.6 ms/channel
	
	WiFi[2] = averageChannel(10,84);
	WiFi[3] = averageChannel(28,102);
	WiFi[4] = averageChannel(45,119);
	WiFi[5] = averageChannel(63,136);
	WiFi[6] = averageChannel(80,154);
	WiFi[7] = averageChannel(98,171);
	WiFi[8] = averageChannel(115,189);
	WiFi[9] = averageChannel(133,206);
	WiFi[10] = averageChannel(150,224);
	WiFi[11] = averageChannel(167,241);
	
	normalizedWiFi();
    
    ZigBee[12] = averageChannel(16,30);
	ZigBee[13] = averageChannel(33,47);
	ZigBee[14] = averageChannel(51,65);
	ZigBee[15] = averageChannel(68,82);
	ZigBee[16] = averageChannel(85,100);
	ZigBee[17] = averageChannel(103,117);
	ZigBee[18] = averageChannel(120,135);
	ZigBee[19] = averageChannel(138,152);
	ZigBee[20] = averageChannel(155,170);
	ZigBee[21] = averageChannel(173,187);
	ZigBee[22] = averageChannel(190,205);
	ZigBee[23] = averageChannel(208,222);
	ZigBee[24] = averageChannel(225,239);
	
	normalizedZigBee();

    LED_GREEN(0);
}

uint8 power(uint8 base, uint16 n)
{
	uint8 p;
	for (p = 1; n > 0; --n)
		p *= base;
	return p;
}

void drawNumber(uint16 middle, uint8 number, BIT size)
{
	if (size)
	{
		switch(number)
		{
		case 1:
			middle++;		txData[middle] = 0x21;
			middle--;		txData[middle] = 0x7F;
			middle--;		txData[middle] = 0x01;
			break;
		case 2:
			middle += 2;	txData[middle] = 0x21;
			middle--;		txData[middle] = 0x43;
			middle--;		txData[middle] = 0x45;
			middle--;		txData[middle] = 0x49;
			middle--;		txData[middle] = 0x31;
			break;
		case 3:
			middle += 2;	txData[middle] = 0x42;
			middle--;		txData[middle] = 0x41;
			middle--;		txData[middle] = 0x51;
			middle--;		txData[middle] = 0x69;
			middle--;		txData[middle] = 0x46;
			break;
		case 5:
			middle += 2;	txData[middle] = 0x72;
			middle--;		txData[middle] = 0x51;
			middle--;		txData[middle] = 0x51;
			middle--;		txData[middle] = 0x51;
			middle--;		txData[middle] = 0x4E;
			break;
		case 6:
			middle += 2;	txData[middle] = 0x1E;
			middle--;		txData[middle] = 0x29;
			middle--;		txData[middle] = 0x49;
			middle--;		txData[middle] = 0x49;
			middle--;		txData[middle] = 0x06;
			break;
		case 7:
			middle += 2;	txData[middle] = 0x40;
			middle--;		txData[middle] = 0x47;
			middle--;		txData[middle] = 0x48;
			middle--;		txData[middle] = 0x50;
			middle--;		txData[middle] = 0x60;
			break;
		}
	}
	else
	{
		switch(number)
		{
		case 0:
			middle++;		txData[middle] = 0x1F;
			middle--;		txData[middle] = 0x11;
			middle--;		txData[middle] = 0x1F;
			break;
		case 1:
			middle++;		txData[middle] = 0x10;
			middle--;		txData[middle] = 0x1F;
			break;
		case 2:
			middle++;		txData[middle] = 0x17;
			middle--;		txData[middle] = 0x15;
			middle--;		txData[middle] = 0x1D;
			break;
		case 3:
			middle++;		txData[middle] = 0x15;
			middle--;		txData[middle] = 0x15;
			middle--;		txData[middle] = 0x1F;
			break;
		case 4:
			middle++;		txData[middle] = 0x1C;
			middle--;		txData[middle] = 0x04;
			middle--;		txData[middle] = 0x1F;
			break;
		case 5:
			middle++;		txData[middle] = 0x1D;
			middle--;		txData[middle] = 0x15;
			middle--;		txData[middle] = 0x17;
			break;
		case 6:
			middle++;		txData[middle] = 0x1F;
			middle--;		txData[middle] = 0x05;
			middle--;		txData[middle] = 0x07;
			break;
		case 7:
			middle++;		txData[middle] = 0x10;
			middle--;		txData[middle] = 0x10;
			middle--;		txData[middle] = 0x1F;
			break;
		case 8:
			middle++;		txData[middle] = 0x1F;
			middle--;		txData[middle] = 0x15;
			middle--;		txData[middle] = 0x1F;
			break;
		case 9:
			middle++;		txData[middle] = 0x1C;
			middle--;		txData[middle] = 0x14;
			middle--;		txData[middle] = 0x1F;
			break;
		case 10:  // "-"
			middle++;		txData[middle] = 0x04;
			middle--;		txData[middle] = 0x04;
			middle--;		txData[middle] = 0x04;
			break;
		}
	}
}

void drawChannel(uint8 start, uint8 end, uint8 startRow, uint16 value)
{
	uint8 block;
	uint8 line;
	uint16 row;
	
	end++;
    
	for (block = startRow; block < (startRow + 3); block++)
	{
		row = 128 * block - 1;
		if (value > 8) 
		{
			for (line = start; line < end; line++)
				txData[row + (128 - line)] = 0xFF;
			value -= 8;
		}
		else if (value == 0)
		{
			for (line = start; line < end; line++)
				txData[row + (128 - line)] = 0x00;
		}
		else if (value == 1)
		{
			txData[row + (128 - start)] = 0x00;
			for (line = start + 1; line < end - 1; line++)
				txData[row + (128 - line)] = 0x01;
			txData[row + (128 - (end - 1))] = 0x00;
			value = 0;
		}
		else
		{
			txData[row + (128 - start)] = power(2, (value - 1)) - 1;
			for (line = start + 1; line < end - 1; line++)
				txData[row + (128 - line)] = power(2, value) - 1;
			txData[row + (128 - (end - 1))] = power(2, (value - 1)) - 1;
			value = 0;
		}
	}
}

void drawLogoZigBee(void) // Logo "ZigBee"
{
    uint16 row;
    
    row = 128 * 8 - 1;
    row--;		txData[row] = 0x0C;
    row--;		txData[row] = 0x1A;
    row--;		txData[row] = 0x06;
    row--;		txData[row] = 0x0C;
    
    row = 128 * 7 - 1;
                txData[row] = 0x03;
    row--;		txData[row] = 0x64;
    row--;		txData[row] = 0xD3;
    row--;		txData[row] = 0x34;
    row--;		txData[row] = 0x63;
    
    row = 128 * 6 - 1;
                txData[row] = 0x81;
    row--;		txData[row] = 0xB8;
    row--;		txData[row] = 0xA5;
    row--;		txData[row] = 0xA5;
    row--;		txData[row] = 0xB9;
    row--;		txData[row] = 0x20;
    row--;		txData[row] = 0x18;
    
    row = 128 * 5 - 1;
                txData[row] = 0x70;
    row--;		txData[row] = 0x40;
    row--;		txData[row] = 0x20;
    row--;		txData[row] = 0x10;
    row--;		txData[row] = 0x70;
}

void drawScreenI2C(void)
{
    uint16 j;
    uint16 row;
    uint8 k;
    
    LED_RED(1);
			
    for (j = 0; j < 1024; j++)
        txData[j] = 0x00;
    
    // ZigBee
    
    drawLogoZigBee();    
    
    row = 128 * 4 - 1;
    drawNumber(row + (128 -   9), 1, 0); // 12
    drawNumber(row + (128 -  12), 2, 0);
    drawNumber(row + (128 -  18), 1, 0); // 13
    drawNumber(row + (128 -  21), 3, 0);
    drawNumber(row + (128 -  27), 1, 0); // 14
    drawNumber(row + (128 -  30), 4, 0);
    drawNumber(row + (128 -  36), 1, 0); // 15
    drawNumber(row + (128 -  39), 5, 0);
    drawNumber(row + (128 -  45), 1, 0); // 16
    drawNumber(row + (128 -  48), 6, 0);
    drawNumber(row + (128 -  54), 1, 0); // 17
    drawNumber(row + (128 -  57), 7, 0);
    drawNumber(row + (128 -  63), 1, 0); // 18
    drawNumber(row + (128 -  66), 8, 0);
    drawNumber(row + (128 -  72), 1, 0); // 19
    drawNumber(row + (128 -  75), 9, 0);
    drawNumber(row + (128 -  81), 2, 0); // 20
    drawNumber(row + (128 -  85), 0, 0);
    drawNumber(row + (128 -  90), 2, 0); // 21
    drawNumber(row + (128 -  94), 1, 0);
    drawNumber(row + (128 -  99), 2, 0); // 22
    drawNumber(row + (128 - 103), 2, 0);
    drawNumber(row + (128 - 108), 2, 0); // 23
    drawNumber(row + (128 - 112), 3, 0);
    drawNumber(row + (128 - 117), 2, 0); // 24
    drawNumber(row + (128 - 121), 4, 0);
    
    // Wi-Fi
    
    // Logo "Wi-Fi"
    
    row = 128 * 3 - 1;
                txData[row] = 0xB8;
    row--;		txData[row] = 0x08;
    row--;		txData[row] = 0xBB;
    row--;		txData[row] = 0x88;
    row--;		txData[row] = 0x88;
    
    row = 128 * 2 - 1;
                txData[row] = 0x51;
    row--;		txData[row] = 0x15;
    row--;		txData[row] = 0x55;
    row--;		txData[row] = 0x55;
    row--;		txData[row] = 0x4A;
    
    row = 0;
    drawNumber(row + (128 -  24), 2, 0); // 2
    drawNumber(row + (128 -  33), 3, 0); // 3
    drawNumber(row + (128 -  42), 4, 0); // 4
    drawNumber(row + (128 -  51), 5, 0); // 5
    drawNumber(row + (128 -  60), 6, 0); // 6
    drawNumber(row + (128 -  69), 7, 0); // 7
    drawNumber(row + (128 -  78), 8, 0); // 8
    drawNumber(row + (128 -  87), 9, 0); // 9
    drawNumber(row + (128 -  94), 1, 0); // 10
    drawNumber(row + (128 -  97), 0, 0);
    drawNumber(row + (128 - 103), 1, 0); // 11
    drawNumber(row + (128 - 106), 1, 0);
    
    oledI2CFramePreamble(0x00, 0x7F, 0x00, 0x07);
    
    for (k = 0; k < 64; k++)
    {
        i2cStart();
        i2cWriteByte(0x78);
        i2cWriteByte(0x40);
        for (j = 0; j < 16; j++) 
            i2cWriteByte(txData[16 * k + j]);
        i2cStop();
    }
}

void drawLogoI2C(void) // Logo
{
    uint16 j;
    uint8 k = 0;
    
    for (j = 0; j < 1024; j++)
    {
        if ((j >= 299 && j <= 340) || (j >= 427 && j <= 468) || (j >= 555 && j <= 585) || (j >= 683 && j <= 702))
        {
            txData[j] = logo[k];
            k++;
        }
        else
            txData[j] = 0x00;
    }
    
    oledI2CFramePreamble(0x00, 0x7F, 0x00, 0x07);
    
    for (k = 0; k < 64; k++)
    {
        i2cStart();
        i2cWriteByte(0x78);
        i2cWriteByte(0x40);
        for (j = 0; j < 16; j++) 
            i2cWriteByte(txData[16 * k + j]);
        i2cStop();
    }
}

void main()
{	
	uint8 delay;
	uint16 row;
	uint16 i;
	uint16 j;
	uint8 block;
	uint8 channel;
	BIT nack;
	uint8 k;
	uint8 t;
	
	// grid (Pololu Wixel [2403.47–2476.50 MHz with spacing in 286.4 kHz] on CC2511F32\r\n)
	// from 0: 5 2404.90 | 23 2410.06 | 40 2414.93 | 58 2420.08 | 75 2424.95 | 93 2430.11 | 110 2434.97 
	// 128 2440.13 | 145 2445.00 | 163 2450.15 | 180 2455.02 | 197 2459.89 | 215 2465.05 | 232 2469.91 | 250 2475.07
	uint8 gridShort0[4] = { 5, 40, 75, 110 };
	uint8 gridShort1[4] = { 145, 180, 215, 215 }; // 255
	uint8 gridLong0[4] = { 23, 58, 93, 93 }; // 128
	uint8 gridLong1[4] = { 129, 163, 197, 232 };
	
	systemInit();
    usbInit();
	if (param_I2C_on)
	{
		I2CInit();
		oledI2CInit();
        if (isPinHigh(1))
        {
            drawLogoI2C();
            delayMs(2000);
        }
        drawScreenI2C();
	}
	SPI0Init();
	SPI1Init();
    delayMicroseconds(100);
	oledSPI0Init();
	oledSPI1Init();
	analyzerInit();
	
	for (delay = 0; delay < 3; delay++)
		checkRadioChannels();

    while(1)
	{
		boardService();
		
		if (isPinHigh(0)) // Measure voltage on P0_0
		{
			updateLeds();
			checkRadioChannels();
			
			i = 0;
			j = 0;
			
            if (param_I2C_on)
			{
				LED_RED(1);
			
				for (j = 0; j < 1024; j++)
					txData[j] = 0x00;
                   
                drawChannel( 19,  26, 1, WiFi[2]); // 109
				drawChannel( 28,  35, 1, WiFi[3]);
				drawChannel( 37,  44, 1, WiFi[4]);
				drawChannel( 46,  53, 1, WiFi[5]);
				drawChannel( 55,  62, 1, WiFi[6]);
				drawChannel( 64,  71, 1, WiFi[7]);
				drawChannel( 73,  80, 1, WiFi[8]);
				drawChannel( 82,  89, 1, WiFi[9]);
				drawChannel( 91,  98, 1, WiFi[10]);
				drawChannel(100, 107, 1, WiFi[11]); // 21
                
                oledI2CFramePreamble(0x10, 0x6F, 0x01, 0x03); // 16 - 111 2-1

                for (t = 1; t < 4; t++)
				{
                    for (k = (8 * t + 1); k < (8 * (t + 1) - 1); k++) // 9 - 14 1-2
                    {
                        i2cStart();
                        i2cWriteByte(0x78);
                        i2cWriteByte(0x40);
                        for (j = 0; j < 16; j++) 
                        {
                            nack = i2cWriteByte(txData[16 * k + j]);
                            if (i2cTimeoutOccurred)
                            {
                                delayMs(50);
                                i2cTimeoutOccurred = 0;
                            }
                            else if (nack)
                                LED_RED(1);
                        }
                        i2cStop();
                    }
                }
                
                drawLogoZigBee();
                
                drawChannel(  8,  14, 5, ZigBee[12]); // 121
                drawChannel( 17,  23, 5, ZigBee[13]);
                drawChannel( 26,  32, 5, ZigBee[14]);
                drawChannel( 35,  41, 5, ZigBee[15]);
                drawChannel( 44,  50, 5, ZigBee[16]);
                drawChannel( 53,  59, 5, ZigBee[17]);
                drawChannel( 62,  68, 5, ZigBee[18]);
                drawChannel( 71,  77, 5, ZigBee[19]);
                drawChannel( 80,  86, 5, ZigBee[20]);
                drawChannel( 89,  95, 5, ZigBee[21]);
                drawChannel( 98, 104, 5, ZigBee[22]);
                drawChannel(107, 113, 5, ZigBee[23]);
                drawChannel(116, 122, 5, ZigBee[24]); // 6
                
                oledI2CFramePreamble(0x00, 0x7F, 0x05, 0x07); // 0x79
                
                for (t = 5; t < 8; t++)
				{
                    for (k = (8 * t); k < (8 * (t + 1)); k++) // 8 - 17
                    {
                        i2cStart();
                        i2cWriteByte(0x78);
                        i2cWriteByte(0x40);
                        for (j = 0; j < 16; j++) 
                        {
                            nack = i2cWriteByte(txData[16 * k + j]);
                            if (i2cTimeoutOccurred)
                            {
                                delayMs(50);
                                i2cTimeoutOccurred = 0;
                            }
                            else if (nack)
                                LED_RED(1);
                        }
                        i2cStop();
                    }
                }
                LED_RED(0);
			}
            
			LED_YELLOW(1);
			
			// SPI0

			if (param_show_grid) // add grid and numbers
			{
				for (j = 0; j < 1024; j++)
					txData[j] = 0x00;
				
				// horizontal numbers
				
				drawNumber(1024 - 128 - gridLong0[0], 1, 1);
				drawNumber(1024 - 128 - gridLong0[1], 2, 1);
				drawNumber(1024 - 128 - gridLong0[2], 3, 1);
				
				// vertical numbers
				
				row = 128 * 7 - 1; // -47
				drawNumber(row + 2, 7, 0);
				drawNumber(row + 6, 4, 0);
				drawNumber(row + 10, 10, 0);
				
				row = 128 * 6 - 1; // -55
				drawNumber(row + 2, 5, 0);
				drawNumber(row + 6, 5, 0);
				drawNumber(row + 10, 10, 0);
				
				row = 128 * 5 - 1; // -63
				drawNumber(row + 2, 3, 0);
				drawNumber(row + 6, 6, 0);
				drawNumber(row + 10, 10, 0);
				
				row = 128 * 4 - 1; // -71
				drawNumber(row + 2, 1, 0);
				drawNumber(row + 6, 7, 0);
				drawNumber(row + 10, 10, 0);
				
				row = 128 * 3 - 1; // -79
				drawNumber(row + 2, 9, 0);
				drawNumber(row + 6, 7, 0);
				drawNumber(row + 10, 10, 0);
				
				row = 128 * 2 - 1; // -87
				drawNumber(row + 2, 7, 0);
				drawNumber(row + 6, 8, 0);
				drawNumber(row + 10, 10, 0);
				
				row = 128 * 1 - 1; // -95
				drawNumber(row + 2, 5, 0);
				drawNumber(row + 6, 9, 0);
				drawNumber(row + 10, 10, 0);
				
				// horizontal scale
				
				for (i = 0; i < 4; i++) // grid
				{
					j = 1024 - gridShort0[i];
					txData[j] = 0xF0;
					j = 1024 - gridLong0[i];
					txData[j] = 0xFE;
				}
			}
			
			j = 0;
		
			for (block = 0; block < 8; block++)
			{
				for (channel = 128; channel > 0; channel--)
				{
					if (rssiValue[channel - 1] > 8) 
					{
						txData[j] = 0xFF;
						rssiValue[channel - 1] -= 8;
						j++;
					}
					else if (rssiValue[channel - 1] == 0)
						j++;
					else
					{
						txData[j] = power(2,(rssiValue[channel - 1])) - 1;
						rssiValue[channel - 1] = 0;
						j++;
					}
				}
			}
			
			oledSPI0FramePreamble();
            
			DC0high;
			CS0low;
			spi0MasterTransfer(txData, rxData, 1024);
			delayMs(6);
			CS0high;
			
			// SPI1

			if (param_show_grid) // add grid and numbers
			{
				for (j = 0; j < 1024; j++)
					txData[j] = 0x00;
				
				// horizontal numbers
				
				drawNumber(1024 - gridLong1[1], 5, 1);
				drawNumber(1024 - gridLong1[2], 6, 1);
				drawNumber(1024 - gridLong1[3], 7, 1);
								
				// vertical numbers
				
				row = 128 * 7 - 1; // -47
				drawNumber(row + 2, 7, 0);
				drawNumber(row + 6, 4, 0);
				drawNumber(row + 10, 10, 0);
				
				row = 128 * 6 - 1; // -55
				drawNumber(row + 2, 5, 0);
				drawNumber(row + 6, 5, 0);
				drawNumber(row + 10, 10, 0);
				
				row = 128 * 5 - 1; // -63
				drawNumber(row + 2, 3, 0);
				drawNumber(row + 6, 6, 0);
				drawNumber(row + 10, 10, 0);
				
				row = 128 * 4 - 1; // -71
				drawNumber(row + 2, 1, 0);
				drawNumber(row + 6, 7, 0);
				drawNumber(row + 10, 10, 0);
				
				row = 128 * 3 - 1; // -79
				drawNumber(row + 2, 9, 0);
				drawNumber(row + 6, 7, 0);
				drawNumber(row + 10, 10, 0);
				
				row = 128 * 2 - 1; // -87
				drawNumber(row + 2, 7, 0);
				drawNumber(row + 6, 8, 0);
				drawNumber(row + 10, 10, 0);
				
				row = 128 * 1 - 1; // -95
				drawNumber(row + 2, 5, 0);
				drawNumber(row + 6, 9, 0);
				drawNumber(row + 10, 10, 0);
				
				// horizontal scale
				
				for (i = 0; i < 4; i++) // grid
				{
					j = 1024 - (gridShort1[i] - 128);
					txData[j] = 0xF0;
					j = 1024 - (gridLong1[i] - 128);
					txData[j] = 0xFE;
				}
				
				j = 0;
				
				for (block = 0; block < 8; block++)
				{
					for (channel = 255; channel >= 128; channel--)
					{
						if (rssiValue[channel] > 8) 
						{
							txData[j] = 0xFF;
							rssiValue[channel] -= 8;
							j++;
						}
						else if (rssiValue[channel] == 0)
							j++;
						else
						{
							txData[j] = power(2, (rssiValue[channel])) - 1;
							rssiValue[channel] = 0;
							j++;
						}
					}
				}
				
				oledSPI1FramePreamble();
                
				DC1high;
				CS1low;
				spi1MasterTransfer(txData, rxData, 1024);
				delayMs(6);
				CS1high;
			}
			LED_YELLOW(0);
		}
		else
			LED_RED(0);
            LED_YELLOW(0);
            LED_GREEN(1);
	}
}