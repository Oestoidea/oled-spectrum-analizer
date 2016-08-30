#include <wixel.h>
#include <usb.h>
#include <usb_com.h>
#include <stdio.h>
#include <spi0_master.h>
#include <i2c.h>
#include <radio_registers.h>

int32 CODE param_spi_on = 1;
int32 CODE param_i2c_on = 1;

static uint8 XDATA initData[25] = 
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

static uint8 XDATA rxData[1024] = {0};
static uint8 XDATA txData[1024] = {0};

// 4-wire SPI
#define RESlow      setDigitalOutput(1,0)   // P0_1 to RES
#define REShigh     setDigitalOutput(1,1)

#define DClow       setDigitalOutput(2,0)   // P0_2 to D/C (MISO)
#define DChigh      setDigitalOutput(2,1)
                                            // P0_3 to DIN (MOSI)
#define CSlow       setDigitalOutput(4,0)   // P0_4 to CS (!CC)
#define CShigh      setDigitalOutput(4,1)
                                            // P0_5 to CLK
void updateLeds(void)
{
    usbShowStatusWithGreenLed();
    LED_GREEN(0);
    LED_RED(0);
    LED_YELLOW(0);
}

void spiInit(void)
{
    spi0MasterInit();
    spi0MasterSetFrequency(3000000);
    spi0MasterSetClockPolarity(SPI_POLARITY_IDLE_LOW);  // Sets the clock polarity CPOL = 0 (SPI_POLARITY_IDLE_HIGH)
    spi0MasterSetClockPhase(SPI_PHASE_EDGE_LEADING);    // Sets the clock phase CPHA = 0 (SPI_PHASE_EDGE_TRAILING)
    spi0MasterSetBitOrder(SPI_BIT_ORDER_MSB_FIRST);     // The most-significant bit is transmitted first    
}

void i2cInit(void)
{
    i2cPinScl = 10;         // P1_0 to SLC
    i2cPinSda = 11;         // P1_1 to SDA
    i2cSetFrequency(400);   // kHz
    i2cSetTimeout(10);      // ms
}

void oledSpiInit(void)
{
    uint8 i;
    
    RESlow;
    delayMicroseconds(3);
    REShigh;
    DClow;
    CSlow;
    for (i = 0; i < 25; i++) 
    {
        spi0MasterSendByte(initData[i]);
        delayMs(1);
    }
    CShigh;
    DChigh;
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
        nack = i2cWriteByte(initData[i]);
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

void oledSPIFramePreamble(void)
{
    uint8 i;
    uint8 initData[6] =
    {
        0x21,0x00,0x7F,
        0x22,0x00,0x07
    };
    
    DClow;
    CSlow;
    
    for (i = 0; i < 6; i++) 
    {
        spi0MasterSendByte(initData[i]);
        delayMicroseconds(3);
    }
    
    CShigh;
}

void oledI2CFramePreamble(void)
{
    uint8 i;
    uint8 initData[8] =
    {
        0x78,0x00,
        0x21,0x00,0x7F,
        0x22,0x00,0x07
    };
    
    i2cSetFrequency(400);
    
    i2cStart();
    for (i = 0; i < 8; i++) 
        i2cWriteByte(initData[i]);
    i2cStop();
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

void main()
{
    BIT nack;
    uint8 k;
    uint16 j;
    uint8 block;
    uint16 channel;
    
    systemInit();
    usbInit();
    if (param_spi_on)
    {
        spiInit();
        delayMicroseconds(100);
        oledSpiInit();
    }
    if (param_i2c_on)
    {
        i2cInit();
        oledI2cInit();
    }
    analyzerInit();
    
    checkRadioChannels();
    checkRadioChannels();

    while(1)
    {
        boardService();
        
        if (isPinHigh(0)) // Measure voltage on P0_0
        {
            updateLeds();
            checkRadioChannels();
            
            for (j = 0; j < 1024; j++)
                txData[j] = 0x00;
            
            j = 0;
            
            for(block = 0; block < 8; block++)
            {
                for(channel = 128; channel > 0; channel--)
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
                        txData[j] = power(2,(rssiHalfValue[channel - 1])) - 1;
                        rssiHalfValue[channel - 1] = 0;
                        j++;
                    }
                }
            }
            
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