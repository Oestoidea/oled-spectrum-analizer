#include <wixel.h>
#include <usb.h>
#include <usb_com.h>
#include <stdio.h>
#include <spi0_master.h>
#include <spi1_master.h>
#include <i2c.h>
#include <radio_registers.h>

#include "font.h"

int32 CODE param_spi_on = 0;
int32 CODE param_i2c_on = 1;
int32 CODE param_height = 64;

uint16 i2cInitFrequency = 10;
uint16 i2cTxFrequency = 200;

static uint8 XDATA oledInitData[25] = 
{
    0xAE,       // display off
    0xD5,0x80,  // set display clock divide ratio/oscillator frequency
    0xA8,0x3F,  // set multiplex ratio(1 to 64)
    0xD3,0x00,
    0x40,
    0x8D,0x14,  // inclusion of an internal voltage converter
    0x20,0x00,  // Set Memory Addressing Mode 00,Horizontal Addressing Mode;01,Vertical Addressing Mode;10,Page Addressing Mode (RESET,0);11,Invalid
    0xA1,       // set segment re-map 0 to 127
    0xC8,       // set COM output scan direction
    0xDA,0x12,  // set COM pins
    0x81,0xCF,  // set contrast control register
    0xD9,0xF1,  // set pre-charge period
    0xDB,0x40,  // set vcomh
    0xA4,       // 0xA4 output follows RAM content; 0xA output ignores RAM content
    0xA6,       // 0xA6 normal / reverse
    0xAF        // display on
};

static int16 XDATA rssiValue[256];
static int16 XDATA rssiHalfValue[128];

static uint8 XDATA rxData[0] = {0};
static uint8 XDATA txData[1024] = {0};

static uint8 XDATA freqReg[25] = // default 2401 MHz
{
	0x00,
	0x08, 0x10, 0x18, 0x20,
	0x08,
	0x30, 0x38, 0x40, 0x48,
	0x10,
	0x58, 0x60, 0x68, 0x70,
	0x18,
	0x80, 0x88, 0x90, 0x98,
	0x20,
	0xA8, 0xB0, 0xB8, 0xC0
};

static uint8 XDATA adfInitData[24] = // default 2412 MHz - 1st wi-fi channel
{
	0x00, 0x58, 0x00, 0x05,
	0x00, 0x8C, 0x80, 0x3C, 
	0x00, 0x00, 0x04, 0xB3, 
	0x00, 0x00, 0x4E, 0x42, 
	0x08, 0x00, 0x80, 0xC9, 
	0x00, 0x30, 0x00, 0x60
};

// 4-wire SPI0
#define RESlow      setDigitalOutput(1,0)   // P0_1 to RES
#define REShigh     setDigitalOutput(1,1)

#define DClow       setDigitalOutput(2,0)   // P0_2 to D/C (MISO)
#define DChigh      setDigitalOutput(2,1)
                                            // P0_3 to DIN (MOSI)
#define CSlow       setDigitalOutput(4,0)   // P0_4 to CS (!CC)
#define CShigh      setDigitalOutput(4,1)
                                            // P0_5 to CLK
											
// ADF4351 SPI1
                                            // P1_6 to DIN (MOSI)
#define CS1low      setDigitalOutput(14,0)  // P1_4 to CS1 (!CC)
#define CS1high     setDigitalOutput(14,1)
                                            // P1_5 to CLK
	
void updateLeds(void)
{
    usbShowStatusWithGreenLed();
    LED_GREEN(0);
    LED_RED(0);
    LED_YELLOW(0);
}

void i2cInit(void)
{
    i2cPinScl = 10;         // P1_0 to SLC
    i2cPinSda = 11;         // P1_1 to SDA
    i2cSetFrequency(i2cInitFrequency);   // kHz
    i2cSetTimeout(10);      // ms
}

void spi0Init(void)
{
    spi0MasterInit();
    spi0MasterSetFrequency(3000000);
    spi0MasterSetClockPolarity(SPI_POLARITY_IDLE_LOW);  // Sets the clock polarity CPOL = 0 (SPI_POLARITY_IDLE_HIGH)
    spi0MasterSetClockPhase(SPI_PHASE_EDGE_LEADING);    // Sets the clock phase CPHA = 0 (SPI_PHASE_EDGE_TRAILING)
    spi0MasterSetBitOrder(SPI_BIT_ORDER_MSB_FIRST);     // The most-significant bit is transmitted first    
}

void spi1Init(void)
{
    spi1MasterInit();
    spi1MasterSetFrequency(70000);
    spi1MasterSetClockPolarity(SPI_POLARITY_IDLE_LOW);  // Sets the clock polarity CPOL = 0 (SPI_POLARITY_IDLE_HIGH)
    spi1MasterSetClockPhase(SPI_PHASE_EDGE_LEADING);    // Sets the clock phase CPHA = 0 (SPI_PHASE_EDGE_TRAILING)
    spi1MasterSetBitOrder(SPI_BIT_ORDER_MSB_FIRST);     // The most-significant bit is transmitted first    
}

void oledI2cInit(void)
{
    BIT nack;
    uint8 i;

    for (i = 0; i < 25; i++) 
    {
        i2cStart();
        i2cWriteByte(0x78);
        i2cWriteByte(0x00);
        nack = i2cWriteByte(oledInitData[i]);
        if (i2cTimeoutOccurred)
        {
            delayMs(50);
            i2cTimeoutOccurred = 0;
        }
        else if (nack)
            LED_RED(1);
        i2cStop();
    }
}

void oledSpi0Init(void)
{
    uint8 i;
    
    RESlow;
    delayMicroseconds(3);
    REShigh;
    DClow;
    CSlow;
    for (i = 0; i < 25; i++) 
    {
        spi0MasterSendByte(oledInitData[i]);
        delayMs(1);
    }
    CShigh;
    DChigh;
}

void adfSpi1Init(void)
{
    uint8 i, j;
    
	for (i = 0; i < 6; i++) 
    {
		CS1low;
		for (j = 0; j < 4; j++) 
			spi1MasterSendByte(adfInitData[4 * i + j]);
		CS1high;
		delayMs(1);
    }
}

void oledI2CFramePreamble(void)
{
    uint8 i;
    uint8 oledInitData[8] =
    {
        0x78,0x00,
        0x21,0x00,0x7F,
        0x22,0x00,0x07
    };
    
    i2cSetFrequency(i2cTxFrequency);
    
    i2cStart();
    for (i = 0; i < 8; i++) 
        i2cWriteByte(oledInitData[i]);
    i2cStop();
}

void oledSPIFramePreamble(void)
{
    uint8 i;
    uint8 oledInitData[6] =
    {
        0x21,0x00,0x7F,
        0x22,0x00,0x07
    };
    
    DClow;
    CSlow;
    
    for (i = 0; i < 6; i++) 
    {
        spi0MasterSendByte(oledInitData[i]);
        delayMicroseconds(3);
    }
    
    CShigh;
}

void analyzerInit()
{
    radioRegistersInit();

    MCSM0 = 0x14;       // Auto-calibrate  when going from idle to RX or TX.
    MCSM1 = 0x00;       // Disable CCA.  After RX, go to IDLE.  After TX, go to IDLE.
    // We leave MCSM2 at its default value = 0x07
    MDMCFG2 = 0x70;     //disable sync word detection
    RFST = 4;           //idle radio
}

void checkRadioChannels(void)
{
    uint16 i;
    uint16 channel;
    
    LED_GREEN(1);
    
    for (channel = 0; channel < 256; channel++)
    {
        int32 rssiSum;

        rssiValue[channel] = -115;

        while(MARCSTATE != 1);      //radio should already be idle, but check anyway
        CHANNR = channel;
        RFST = 2;                   // radio in RX mode and autocal
        while(MARCSTATE != 13);     //wait for RX mode
        rssiSum = 0;
        for (i = 0; i < 100; i++)
        {
            if (TCON & 2)           //radio byte available?
            {
                uint8 rfdata = RFD; // read byte
                TCON &= ~2;         //clear ready flag
            }
            rssiSum += radioRssi();
        }
        RFST = 4;                   //idle radio
        rssiValue[channel] = (int16) (rssiSum/100);
        
        // preparation for output
        
        if (channel % 2 == 0)       // even
        {
            rssiHalfValue[channel/2] = (int16) ((rssiValue[channel] + rssiValue[channel+1]) / 2);
            
            if (rssiHalfValue[channel/2] <= -105)
                rssiHalfValue[channel/2] == 0x00;
            else if (rssiHalfValue[channel/2] >= -42)
                rssiHalfValue[channel/2] == 0xFF;
            else
                rssiHalfValue[channel/2] += 105;
        }
        rssiHalfValue[channel/2] *= 64 / param_height;
   
    }  // the above loop takes about 414 ms on average, so about 1.6 ms/channel

    LED_GREEN(0);
}

uint8 power(uint8 base, uint16 n)
{
    uint8 p;
    for (p = 1; n > 0; --n)
        p *= base;
    return p;
}

uint16 letterView(int8 number, uint16 position) // number use ASCII code
{
	uint8 col;
	uint8 reverse;
	
	for (col = 0; col < 6; col++)
	{
		reverse = myFont6[number - 0x20][col];
		
		reverse = (reverse & 0x55) <<  1 | (reverse & 0xAA) >>  1;
		reverse = (reverse & 0x33) <<  2 | (reverse & 0xCC) >>  2;
		reverse = (reverse & 0x0F) <<  4 | (reverse & 0xF0) >>  4;
		
		txData[position] = reverse;
		position--;
	}
	return position;
}

uint8 byteToChannel(uint8 byte)
{
	uint8 wifiChannel;
	if 		(byte >= 1  && byte <= 14)	wifiChannel = 1;
	else if (byte >= 15 && byte <= 19)	wifiChannel = 2;
	else if (byte >= 20 && byte <= 24)	wifiChannel = 3;
	else if (byte >= 25 && byte <= 29)	wifiChannel = 4;
	else if (byte >= 30 && byte <= 34)	wifiChannel = 5;
	else if (byte >= 35 && byte <= 39)	wifiChannel = 6;
	else if (byte >= 40 && byte <= 44)	wifiChannel = 7;
	else if (byte >= 45 && byte <= 49)	wifiChannel = 8;
	else if (byte >= 50 && byte <= 54)	wifiChannel = 9;
	else if (byte >= 55 && byte <= 59)	wifiChannel = 10;
	else if (byte >= 60 && byte <= 64)	wifiChannel = 11;
	else if (byte >= 65 && byte <= 69)	wifiChannel = 12;
	else if (byte >= 70 && byte <= 74)	wifiChannel = 13;
	else if (byte >= 75 && byte <= 93)	wifiChannel = 14;
	else								wifiChannel = 0;
	return wifiChannel;
}

uint8 channelToByte(uint8 wifiChannel)
{
	uint8 byte;
	if (wifiChannel >= 1 && wifiChannel <= 13)
		byte = 5 * wifiChannel + 7;
	else if (wifiChannel == 14)
		byte = 84;
	else
		byte = 0;
	return byte;
}

void adfSet(uint8 byte)
{
	adfInitData[19] = 0xC9;

	if (!(byte % 5))
	{
		adfInitData[19] = 0x29;
		if (byte == 0 || byte == 25 || byte == 50 || byte == 75)
			adfInitData[19] = 0x11;
	}

	if (byte < 25)
	{
		adfInitData[21] = 0x30;
		adfInitData[22] = 0x00;
		adfInitData[23] = freqReg[byte];
	}
	else if (byte < 50)
	{
		adfInitData[21] = 0x30;
		adfInitData[22] = 0x80;
		adfInitData[23] = freqReg[byte - 25];
	}
	else if (byte < 75)
	{
		adfInitData[21] = 0x31;
		adfInitData[22] = 0x00;
		adfInitData[23] = freqReg[byte - 50];
	}
	else
	{
		adfInitData[21] = 0x31;
		adfInitData[22] = 0x80;
		adfInitData[23] = freqReg[byte - 75];
	}

	adfSpi1Init();
}

void main()
{
    BIT nack;
    uint8 k;
    uint16 j;
    uint8 block;
    uint16 channel;
	uint16 position;
	uint8 byte = 12; // 1st wi-fi channel
	uint8 wifiChannel = 1;
    
    systemInit();
    usbInit();
	
	setDigitalInput(0, 1); // pull-up
	setDigitalInput(12, 1);
	setDigitalInput(13, 1);
	
    if (param_i2c_on)
    {
        i2cInit();
        oledI2cInit();
    }
    if (param_spi_on)
    {
        spi0Init();
        delayMicroseconds(100);
        oledSpi0Init();
    }
	spi1Init();
	
    analyzerInit();
    
    checkRadioChannels();
    checkRadioChannels();

    while(1)
    {
        boardService();
		
		usbComService(); // check for COM port command
		if (usbComRxAvailable())
		{
			byte = usbComRxReceiveByte();
			if (byte > 99)
				byte = 0;
			if (usbComTxAvailable())
				usbComTxSendByte(0xFF); // OK
			
			wifiChannel = byteToChannel(byte);
		}
		
		adfSet(byte);
		
		// first time check
		if (!isPinHigh(12)) // Measure voltage on P1_2
			{
				wifiChannel++;
				if (wifiChannel > 14)
					wifiChannel = 1;
				byte = channelToByte(wifiChannel);
				adfSet(byte);
			}

		if (!isPinHigh(13)) // Measure voltage on P1_3
			{
				wifiChannel--;
				if (wifiChannel < 1)
					wifiChannel = 14;
				byte = channelToByte(wifiChannel);
				adfSet(byte);
			}
        
        if (isPinHigh(0)) // Measure voltage on P0_0
        {
            updateLeds();
            checkRadioChannels();
            
            for (j = 0; j < 1024; j++)
                txData[j] = 0x00;
            
            j = 0;
            
            for (block = 0; block < 8; block++)
            {
                for (channel = 128; channel > 0; channel--)
                {
                    if (rssiHalfValue[channel - 1] > 8) {
                        txData[j] = 0xFF;
                        rssiHalfValue[channel - 1] -= 8;
                        j++;
                    }
                    else if (rssiHalfValue[channel - 1] == 0)
                        j++;
                    else
                    {
                        txData[j] = power(2, (rssiHalfValue[channel - 1])) - 1;
                        rssiHalfValue[channel - 1] = 0;
                        j++;
                    }
                }
            }
            
			// second time check
			if (!isPinHigh(12)) // Measure voltage on P1_2
				{
					wifiChannel++;
					if (wifiChannel > 14)
						wifiChannel = 1;
					byte = channelToByte(wifiChannel);
					adfSet(byte);
				}

			if (!isPinHigh(13)) // Measure voltage on P1_3
				{
					wifiChannel--;
					if (wifiChannel < 1)
						wifiChannel = 14;
					byte = channelToByte(wifiChannel);
					adfSet(byte);
				}
		
			position = 1023;

			position = letterView(0x30 + 2, position); //frequency
			position = letterView(0x30 + 4, position);
			position = letterView(0x30 + byte / 10, position);
			position = letterView(0x30 + byte % 10, position);
			
			position = letterView(0x20, position); // sp
			position = letterView(0x4D, position); // MHz
			position = letterView(0x48, position);
			position = letterView(0x7A, position);

			position = letterView(0x20, position); // sp	
			position = letterView(0x20, position); // sp
			
			if (wifiChannel < 10) // channel
			{
				position = letterView(0x20, position); // sp
				position = letterView(0x30 + wifiChannel, position);
			}
			else
			{
				position = letterView(0x30 + wifiChannel / 10, position);
				position = letterView(0x30 + wifiChannel % 10, position);
			}
			
			position = letterView(0x20, position); // sp
			position = letterView(0x63, position); // ch
			position = letterView(0x68, position);
			
            if (param_spi_on)
            {
                oledSPIFramePreamble();
                
                DChigh;
                CSlow;
                spi0MasterTransfer(txData, rxData, 1024);
                delayMs(6);
                CShigh;
            }
            
            if (param_i2c_on)
            {
                oledI2CFramePreamble();
                
                for (k = 0; k < 64; k++) {
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
        }
        else
            LED_RED(1);
    }
}