#include <wixel.h>
#include <usb.h>
#include <usb_com.h>
#include <stdio.h>
#include <spi0_master.h>
#include <i2c.h>
#include <radio_registers.h>
#include <ssd1331.h>

static int16 XDATA rssiValue[256];
static int16 XDATA rssiHalfValue[128];
static int16 XDATA rssiHalfValueMax[128] = {0};
static int16 rssiValueMax = 0;
static uint8 channelValueMax = 0;
static int16 rssiValueMaxCurrent = 0;
static uint8 channelValueMaxCurrent = 0;
static uint8 minZigBeeChannel = 0;
static uint8 minWiFiChannel = 0;
static BIT pause = 1;
static uint8 state = 0;

static int16 colorMax = COLOR_BLUE;
static int16 colorCurrent = COLOR_GOLDEN;
static int16 colorGrid = COLOR_GOLDEN;

static uint8 XDATA rxData[1024] = {0};
static uint8 XDATA txData[1024] = {0};

static uint16 XDATA ZigBee[25] = {0}; // only 13 (12-24) from 16 (11-26)
static uint16 XDATA WiFi[12] = {0}; // only 10 (2-11) from 14 (1-14)

// 4-wire SPI
#define RESlow		setDigitalOutput(1,0)	// P0_1 to RES
#define REShigh		setDigitalOutput(1,1)

#define DClow		setDigitalOutput(2,0)	// P0_2 to D/C (MISO)
#define DChigh		setDigitalOutput(2,1)
											// P0_3 to DIN (MOSI)
#define CSlow		setDigitalOutput(4,0)	// P0_4 to CS (!CC)
#define CShigh		setDigitalOutput(4,1)
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
	spi0MasterSetClockPolarity(SPI_POLARITY_IDLE_LOW);		// Sets the clock polarity CPOL = 0 (SPI_POLARITY_IDLE_HIGH)
	spi0MasterSetClockPhase(SPI_PHASE_EDGE_LEADING);		// Sets the clock phase CPHA = 0 (SPI_PHASE_EDGE_TRAILING)
	spi0MasterSetBitOrder(SPI_BIT_ORDER_MSB_FIRST);			// The most-significant bit is transmitted first	
}

void sendCmd(uint8 c)
{
    DClow;
	CSlow;
	spi0MasterSendByte(c);
	CShigh;
}

void sendData(uint8 c)
{
    DChigh;
	CSlow;
	spi0MasterSendByte(c);
	CShigh;
}

void oledSpiInit(void)
{
    RESlow;
    delayMicroseconds(3);
	REShigh;
   
    sendCmd(CMD_DISPLAY_OFF);          //Display Off
    sendCmd(CMD_SET_CONTRAST_A);       //Set contrast for color A
    sendCmd(0x91);                     //145
    sendCmd(CMD_SET_CONTRAST_B);       //Set contrast for color B
    sendCmd(0x50);                     //80
    sendCmd(CMD_SET_CONTRAST_C);       //Set contrast for color C
    sendCmd(0x7D);                     //125
    sendCmd(CMD_MASTER_CURRENT_CONTROL);//master current control
    sendCmd(0x06);                     //6
    sendCmd(CMD_SET_PRECHARGE_SPEED_A);//Set Second Pre-change Speed For ColorA
    sendCmd(0x64);                     //100
    sendCmd(CMD_SET_PRECHARGE_SPEED_B);//Set Second Pre-change Speed For ColorB
    sendCmd(0x78);                     //120
    sendCmd(CMD_SET_PRECHARGE_SPEED_C);//Set Second Pre-change Speed For ColorC
    sendCmd(0x64);                     //100
    sendCmd(CMD_SET_REMAP);            //set remap & data format
    //sendCmd(0x72);                     //0x72              
    sendCmd(0x70);                     //0x70             
    sendCmd(CMD_SET_DISPLAY_START_LINE);//Set display Start Line
    sendCmd(0x0);
    sendCmd(CMD_SET_DISPLAY_OFFSET);   //Set display offset
    sendCmd(0x0);
    sendCmd(CMD_NORMAL_DISPLAY);       //Set display mode
    //sendCmd(CMD_INVERSE_DISPLAY);       //Set display mode
    sendCmd(CMD_SET_MULTIPLEX_RATIO);  //Set multiplex ratio
    sendCmd(0x3F);
    sendCmd(CMD_SET_MASTER_CONFIGURE); //Set master configuration
    sendCmd(0x8E);
    sendCmd(CMD_POWER_SAVE_MODE);      //Set Power Save Mode
    sendCmd(0x00);                     //0x00
    sendCmd(CMD_PHASE_PERIOD_ADJUSTMENT);//phase 1 and 2 period adjustment
    sendCmd(0x31);                     //0x31
    sendCmd(CMD_DISPLAY_CLOCK_DIV);    //display clock divider/oscillator frequency
    sendCmd(0xF0);
    sendCmd(CMD_SET_PRECHARGE_VOLTAGE);//Set Pre-Change Level
    sendCmd(0x3A);
    sendCmd(CMD_SET_V_VOLTAGE);        //Set vcomH
    sendCmd(0x3E);
    sendCmd(CMD_DEACTIVE_SCROLLING);   //disable scrolling
    sendCmd(CMD_NORMAL_BRIGHTNESS_DISPLAY_ON);//set display on
    
	DChigh;
}

void analyzerInit()
{
    radioRegistersInit();

    MCSM0 = 0x14;    // Auto-calibrate  when going from idle to RX or TX.
    MCSM1 = 0x00;    // Disable CCA.  After RX, go to IDLE.  After TX, go to IDLE.
    // We leave MCSM2 at its default value = 0x07
    MDMCFG2 = 0x70;   //disable sync word detection
    RFST = 4; //idle radio
}


uint16 averageChannel(uint8 first, uint8 last)
{
	uint8 i;
	uint16 rssiChannelSum;
	
	rssiChannelSum = 0;
	for (i = first; i < last + 1; i++)
		rssiChannelSum += rssiValue[i] + 105;
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
		WiFi[i] = (uint8) (20 * (max - WiFi[i]) / max);
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
		ZigBee[i] = (uint8) (20 * (max - ZigBee[i]) / max);
}



/*
uint16 averageChannel(uint8 first, uint8 last)
{
	uint8 i;
	uint16 rssiChannelSum;
	
	rssiChannelSum = 0;
	for (i = first; i < last + 1; i++)
		rssiChannelSum -= rssiValue[i];
	return (uint16) (10 * rssiChannelSum / (last - first + 1));
	//return rssiChannelSum;
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
		//WiFi[i] = (uint16) (20 * (max - WiFi[i])) / max;
		//WiFi[i] = (uint16) ((200 * (max - WiFi[i])) / max / 10);
		//WiFi[i] = (uint16) ((20 - 20 * WiFi[i] / max) / 1);
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
	
	for(i = 12; i < 25; i++){
		ZigBee[i] = (uint16) (20 * (max - ZigBee[i]) / max);
        ZigBeeNormalized[i] = (uint8) (200 * (max - ZigBee[i])) / max;}
}
*/

uint8 minWiFi(void)
{
	uint8 i;
	uint8 minChannelWiFi = 2;
	uint16 min = WiFi[2];
    
	for (i = 2; i < 12; i++)
	{
		if (WiFi[i] < min)
		{
			min = WiFi[i];
            minChannelWiFi = i;
        }
	}
	
    return minChannelWiFi;
}

uint8 minZigBee(void)
{
	uint8 i;
	uint8 minChannelZigBee = 12;
	uint16 min = ZigBee[12];
    
	for (i = 12; i < 25; i++)
	{
		if (ZigBee[i] < min)
        {
			min = ZigBee[i];
            minChannelZigBee = i;
        }
	}
    
    return minChannelZigBee;
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
        rssiValue[channel] = (int16) (rssiSum / 100);
		
		// подготовка к выводу
		
		if (channel % 2 == 0) // even
		{
			rssiHalfValue[channel/2] = (int16) ((rssiValue[channel] + rssiValue[channel+1]) / 2);
			
			if (rssiHalfValue[channel/2] <= -105)
			{
				rssiHalfValue[channel/2] == 0x00;
			}
			else if (rssiHalfValue[channel/2] >= -53) // -42 place for grid
			{
				rssiHalfValue[channel/2] == 0xFF;
			}
			else
			{
				rssiHalfValue[channel/2] += 105;
			}
		}
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
	
	minWiFiChannel = minWiFi();
    normalizedWiFi();
    
    ZigBee[12] = averageChannel(16,30);
	ZigBee[13] = averageChannel(33,47);
	ZigBee[14] = averageChannel(51,65);
	ZigBee[15] = averageChannel(68,82);
	ZigBee[16] = averageChannel(85,99);
	ZigBee[17] = averageChannel(103,117);
	ZigBee[18] = averageChannel(120,134);
	ZigBee[19] = averageChannel(138,152);
	ZigBee[20] = averageChannel(155,169);
	ZigBee[21] = averageChannel(173,187);
	ZigBee[22] = averageChannel(190,204);
	ZigBee[23] = averageChannel(208,222);
	ZigBee[24] = averageChannel(225,239);
	
	minZigBeeChannel = minZigBee();
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

void drawPixel(uint8 x, uint8 y, uint16 color)
{
    //set column point
    sendCmd(CMD_SET_COLUMN_ADDRESS);
    sendCmd(x);
    sendCmd(RGB_OLED_WIDTH-1);
    //set row point
    sendCmd(CMD_SET_ROW_ADDRESS);
    sendCmd(y);
    sendCmd(RGB_OLED_HEIGHT-1);
    //fill 16bit colour
    sendData(color >> 8);
    sendData(color);
}

void drawLine(uint8 x0, uint8 y0, uint8 x1, uint8 y1, uint16 color)
{
    if (x0 >= RGB_OLED_WIDTH)  x0 = RGB_OLED_WIDTH - 1;
    if (y0 >= RGB_OLED_HEIGHT) y0 = RGB_OLED_HEIGHT - 1;
    if (x1 >= RGB_OLED_WIDTH)  x1 = RGB_OLED_WIDTH - 1;
    if (y1 >= RGB_OLED_HEIGHT) y1 = RGB_OLED_HEIGHT - 1;

    sendCmd(CMD_DRAW_LINE);//draw line
    sendCmd(x0);//start column
    sendCmd(y0);//start row
    sendCmd(x1);//end column
    sendCmd(y1);//end row
    sendCmd((uint8)((color>>11)&0x1F));//R
    sendCmd((uint8)((color>>5)&0x3F));//G
    sendCmd((uint8)(color&0x1F));//B
}

void drawFrame(uint8 x0, uint8 y0, uint8 x1, uint8 y1, uint16 outColor, uint16 fillColor)
{
    if (x0 >= RGB_OLED_WIDTH)  x0 = RGB_OLED_WIDTH - 1;
    if (y0 >= RGB_OLED_HEIGHT) y0 = RGB_OLED_HEIGHT - 1;
    if (x1 >= RGB_OLED_WIDTH)  x1 = RGB_OLED_WIDTH - 1;
    if (y1 >= RGB_OLED_HEIGHT) y1 = RGB_OLED_HEIGHT - 1;

    sendCmd(CMD_FILL_WINDOW);//fill window
    sendCmd(ENABLE_FILL);
    sendCmd(CMD_DRAW_RECTANGLE);//draw rectangle
    sendCmd(x0);//start column
    sendCmd(y0);//start row
    sendCmd(x1);//end column
    sendCmd(y1);//end row
    sendCmd((uint8)((outColor>>11)&0x1F));//R
    sendCmd((uint8)((outColor>>5)&0x3F));//G
    sendCmd((uint8)(outColor&0x1F));//B
    sendCmd((uint8)((fillColor>>11)&0x1F));//R
    sendCmd((uint8)((fillColor>>5)&0x3F));//G
    sendCmd((uint8)(fillColor&0x1F));//B
    delayMs(1);
}

void clearWindow(uint8 x0, uint8 y0, uint8 x1, uint8 y1)
{
    sendCmd(CMD_CLEAR_WINDOW);//clear window
    sendCmd(x0);//start column
    sendCmd(y0);//start row
    sendCmd(x1);//end column
    sendCmd(y1);//end row
    delayMs(1);
}

void drawNumber(uint8 x, uint8 y, uint8 number, BIT size, uint16 color)
{
	if (size)
	{
		switch(number)
		{
		case 0:
            drawLine(x + 1, y,     x + 3, y,     color);
            drawLine(x + 1, y + 6, x + 3, y + 6, color);
            drawLine(x,     y + 1, x,     y + 5, color);
            drawLine(x + 4, y + 1, x + 4, y + 5, color);
			break;
		case 1:
            drawLine(x,     y,     x + 2, y,     color);
            drawLine(x + 1, y + 1, x + 1, y + 6, color);
            drawPixel(x, y + 5, color);
			break;
		case 2:
			drawLine(x,     y,     x + 4, y,     color);
            drawPixel(x + 1, y + 1, color);
            drawPixel(x + 2, y + 2, color);
            drawPixel(x + 3, y + 3, color);
            drawLine(x + 4, y + 4, x + 4, y + 5, color);
            drawLine(x + 1, y + 6, x + 3, y + 6, color);
            drawPixel(x, y + 5, color);
			break;
		case 3:
			drawLine(x + 1, y,     x + 3, y,     color);
            drawPixel(x, y + 1, color);
			drawLine(x + 4, y + 1, x + 4, y + 2, color);
            drawPixel(x + 3, y + 3, color);
            drawPixel(x + 2, y + 4, color);
            drawPixel(x + 3, y + 5, color);
            drawLine(x,     y + 6, x + 4, y + 6, color);
			break;
		case 4:
			drawLine(x,     y + 2, x + 4, y + 2, color);
            drawLine(x + 3, y,     x + 3, y + 6, color);
            drawPixel(x, y + 3, color);
            drawPixel(x + 1, y + 4, color);
            drawPixel(x + 2, y + 5, color);
			break;
        case 5:
            drawLine(x + 1, y,     x + 3, y,     color);
            drawPixel(x, y + 1, color);
            drawLine(x + 4, y + 1, x + 4, y + 3, color);
            drawLine(x,     y + 4, x + 3, y + 4, color);
            drawPixel(x, y + 5, color);
			drawLine(x,     y + 6, x + 4, y + 6, color);
			break;
		case 6:
            drawLine(x + 1, y,     x + 3, y,     color);
            drawLine(x + 1, y + 3, x + 3, y + 3, color);
            drawLine(x,     y + 1, x,     y + 4, color);
            drawLine(x + 4, y + 1, x + 4, y + 2, color);
            drawPixel(x + 1, y + 5, color);
            drawLine(x + 2, y + 6, x + 3, y + 6, color);
			break;
		case 7:
            drawLine(x + 1, y,     x + 1, y + 2, color);
            drawPixel(x + 2, y + 3, color);
            drawPixel(x + 3, y + 4, color);
            drawPixel(x + 4, y + 5, color);
            drawLine(x,     y + 6, x + 4, y + 6, color);
			break;
		case 8:
            drawLine(x + 1, y,     x + 3, y,     color);
            drawLine(x + 1, y + 3, x + 3, y + 3, color);
            drawLine(x + 1, y + 6, x + 3, y + 6, color);
            drawLine(x,     y + 1, x,     y + 2, color);
            drawLine(x,     y + 4, x,     y + 5, color);
            drawLine(x + 4, y + 1, x + 4, y + 2, color);
            drawLine(x + 4, y + 4, x + 4, y + 5, color);
			break;
		case 9:
            drawLine(x + 1, y,     x + 2, y,     color);
            drawPixel(x + 3, y + 1, color);
            drawLine(x + 4, y + 2, x + 4, y + 5, color);
            drawLine(x + 1, y + 3, x + 3, y + 3, color);
            drawLine(x + 1, y + 6, x + 3, y + 6, color);
            drawLine(x,     y + 4, x,     y + 5, color);
            break;
		}
	}
	else
	{
		switch(number)
		{
		case 0:
            drawLine(x,     y,     x,     y + 4, color);
            drawLine(x + 2, y,     x + 2, y + 4, color);
            drawPixel(x + 1, y, color);
            drawPixel(x + 1, y + 4, color);
			break;
		case 1:
            drawLine(x + 1, y,     x + 1, y + 4, color);
            drawPixel(x, y + 4, color);
			break;
		case 2:
            drawLine(x,     y,     x + 2, y,     color);
            drawLine(x,     y + 2, x + 2, y + 2, color);
            drawLine(x,     y + 4, x + 2, y + 4, color);
            drawPixel(x, y + 1, color);
            drawPixel(x + 2, y + 3, color);
			break;
		case 3:
            drawLine(x + 2, y,     x + 2, y + 4, color);
            drawLine(x,     y,     x + 1, y,     color);
            drawPixel(x + 1, y + 2, color);
            drawLine(x,     y + 4, x + 1, y + 4, color);
			break;
		case 4:
            drawLine(x,     y + 2, x,     y + 4, color);
            drawLine(x + 2, y,     x + 2, y + 4, color);
            drawPixel(x + 1, y + 2, color);
			break;
		case 5:
            drawLine(x,     y,     x + 2, y,     color);
            drawLine(x,     y + 2, x + 2, y + 2, color);
            drawLine(x,     y + 4, x + 2, y + 4, color);
            drawPixel(x, y + 3, color);
            drawPixel(x + 2, y + 1, color);
			break;
		case 6:
            drawLine(x,     y,     x,     y + 4, color);
            drawLine(x + 2, y,     x + 2, y + 2, color);
            drawPixel(x + 1, y, color);
            drawPixel(x + 1, y + 2, color);
			break;
		case 7:
            drawLine(x + 2, y,     x + 2, y + 4, color);
            drawLine(x,     y + 4, x + 1, y + 4, color);
			break;
		case 8:
            drawLine(x,     y,     x,     y + 4, color);
            drawLine(x + 2, y,     x + 2, y + 4, color);
            drawPixel(x + 1, y, color);
            drawPixel(x + 1, y + 2, color);
            drawPixel(x + 1, y + 4, color);
			break;
		case 9:
            drawLine(x,     y + 2, x,     y + 4, color);
            drawLine(x + 2, y,     x + 2, y + 4, color);
            drawPixel(x + 1, y + 2, color);
            drawPixel(x + 1, y + 4, color);
			break;
		case 10:  // "-"
            drawLine(x,     y + 2, x + 2, y + 2, color);
			break;
		}
	}
}

void drawGrid(uint16 color)
{
    drawLine(3, 61, 3, 63, color);
    drawLine(11, 58, 11, 63, color);
    drawNumber(13, 55, 1, 0, color);
    drawNumber(16, 55, 0, 0, color);
    
    drawLine(20, 61, 20, 63, color);
    drawLine(29, 58, 29, 63, color);
    drawNumber(31, 55, 2, 0, color);
    drawNumber(35, 55, 0, 0, color);
    
    drawLine(38, 61, 38, 63, color);
    drawLine(46, 58, 46, 63, color);
    drawNumber(48, 55, 3, 0, color);
    drawNumber(52, 55, 0, 0, color);
    
    drawLine(55, 61, 55, 63, color);
    drawLine(64, 58, 64, 63, color);
    drawNumber(66, 55, 4, 0, color);
    drawNumber(70, 55, 0, 0, color);
    
    drawLine(73, 61, 73, 63, color);
    drawLine(81, 58, 81, 63, color);
    drawNumber(83, 55, 5, 0, color);
    drawNumber(87, 55, 0, 0, color);
    
    drawLine(90, 61, 90, 63, color);
}

void drawWiFi(uint8 x, uint8 y, uint16 color)        // Wi-Fi text
{
    drawLine(x,     y + 4, x,     y + 6, color);
    drawLine(x + 1, y + 2, x + 1, y + 3, color);
    drawLine(x + 2, y,     x + 2, y + 1, color);
    drawLine(x + 3, y + 2, x + 3, y + 3, color);
    drawLine(x + 4, y + 4, x + 4, y + 5, color);
    drawLine(x + 5, y + 2, x + 5, y + 3, color);
    drawLine(x + 6, y,     x + 6, y + 1, color);
    drawLine(x + 7, y + 2, x + 7, y + 3, color);
    drawLine(x + 8, y + 4, x + 8, y + 6, color);
    drawLine(x + 10, y,    x + 10, y + 4, color);
    drawPixel(x + 10, y + 6, color);
    drawLine(x + 12, y + 2, x + 14, y + 2, color);
    drawLine(x + 16, y,     x + 16, y + 6, color);
    drawLine(x + 17, y + 3, x + 18, y + 3, color);
    drawLine(x + 17, y + 6, x + 19, y + 6, color);
    drawLine(x + 20, y,     x + 20, y + 4, color);
}

void drawZigBee(uint8 x, uint8 y, uint16 color)        // ZigBee text
{
    drawLine(x,     y,     x + 4, y,     color);
    drawLine(x,     y + 6, x + 4, y + 6, color);
    drawLine(x,     y + 1, x + 4, y + 5, color);
    drawLine(x + 6, y,     x + 6, y + 4, color);
    drawPixel(x + 6, y + 6, color);
    drawPixel(x + 8, y, color);
    drawLine(x + 8, y - 1, x + 11, y - 1, color);
    drawPixel(x + 11, y - 2, color);
    drawLine(x + 8, y + 2, x + 8, y + 3, color);
    drawLine(x + 11, y + 2, x + 11, y + 3, color);
    drawLine(x + 9, y + 1, x + 10, y + 1, color);
    drawLine(x + 9, y + 4, x + 10, y + 4, color);
    drawPixel(x + 11, y + 5, color);
    drawLine(x + 13, y,     x + 13, y + 6, color);
    drawLine(x + 14, y,     x + 16, y,     color);
    drawLine(x + 14, y + 3, x + 16, y + 3, color);
    drawLine(x + 14, y + 6, x + 16, y + 6, color);
    drawLine(x + 17, y + 1, x + 17, y + 2, color);
    drawLine(x + 17, y + 4, x + 17, y + 5, color);
    drawLine(x + 19, y + 1, x + 19, y + 3, color);
    drawLine(x + 20, y,     x + 22, y,     color);
    drawLine(x + 20, y + 2, x + 22, y + 2, color);
    drawLine(x + 20, y + 4, x + 21, y + 4, color);
    drawPixel(x + 22, y + 3, color);
    drawLine(x + 24, y + 1, x + 24, y + 3, color);
    drawLine(x + 25, y,     x + 27, y,     color);
    drawLine(x + 25, y + 2, x + 27, y + 2, color);
    drawLine(x + 25, y + 4, x + 26, y + 4, color);
    drawPixel(x + 27, y + 3, color);
}

void drawCh(uint8 x, uint8 y, uint16 color)        // ch text
{
    drawLine(x,     y + 1, x,     y + 3, color);
    drawLine(x + 1, y,     x + 2, y,     color);
    drawLine(x + 1, y + 4, x + 2, y + 4, color);
    drawPixel(x + 3, y + 1, color);
    drawPixel(x + 3, y + 3, color);
    drawPixel(x + 2, y + 5, color);
    drawLine(x + 3, y + 6, x + 4, y + 6, color);
    drawLine(x + 5, y,     x + 5, y + 6, color);
    drawPixel(x + 6, y + 3, color);
    drawPixel(x + 7, y + 4, color);
    drawLine(x + 8, y,     x + 8, y + 3, color);
}

void drawStatistics(uint16 color)
{
    uint8 highDigit;
    uint8 lowDigit;

    highDigit = minWiFiChannel / 10;
    lowDigit = minWiFiChannel % 10;
    
    clearWindow(0, 54, 8, 63);
    
    if (highDigit == 1)
        drawNumber(0, 57, highDigit, 1, color);
    
    drawNumber(4, 57, lowDigit, 1, color);
    drawCh(10, 57, color);
    drawWiFi(20, 57, color);
    
    highDigit = minZigBeeChannel / 10;
    lowDigit = minZigBeeChannel % 10;
    
    clearWindow(46, 54, 56, 63);
    
    if (highDigit == 1)
        drawNumber(48, 57, highDigit, 1, color);
    else
        drawNumber(46, 57, highDigit, 1, color);
    
    drawNumber(52, 57, lowDigit, 1, color);
    drawCh(58, 57, color);
    drawZigBee(68, 57, color);
}

void drawLogo(uint8 x, uint8 y, uint16 color)
{
    drawLine(x,     y,     x,     y + 1, color);
    drawLine(x,     y + 3, x,     y + 4, color);
    drawPixel(x, y + 7, color);
    
    drawLine(x + 1, y,     x + 1, y + 2, color);
    drawPixel(x + 1, y + 4, color);
    drawPixel(x + 1, y + 6, color);
    drawPixel(x + 1, y + 8, color);
    drawPixel(x + 1, y + 10, color);
    
    drawLine(x + 2, y,     x + 2, y + 3, color);
    drawLine(x + 2, y + 5, x + 2, y + 6, color);
    drawPixel(x + 2, y + 12, color);
    
    drawLine(x + 3, y,     x + 3, y + 1, color);
    drawLine(x + 3, y + 3, x + 3, y + 5, color);
    drawLine(x + 3, y + 7, x + 3, y + 9, color);
    drawPixel(x + 3, y + 11, color);
    
    drawLine(x + 4, y,     x + 4, y + 2, color);
    drawPixel(x + 4, y + 4, color);
    drawPixel(x + 4, y + 6, color);
    drawPixel(x + 4, y + 10, color);
    drawPixel(x + 4, y + 13, color);
    
    drawPixel(x + 5, y, color);
    drawLine(x + 5, y + 2, x + 5, y + 4, color);
    drawPixel(x + 5, y + 6, color);
    drawLine(x + 5, y + 8, x + 5, y + 9, color);
    drawPixel(x + 5, y + 11, color);
    
    drawLine(x + 6, y,     x + 6, y + 2, color);
    drawLine(x + 6, y + 4, x + 6, y + 8, color);
    drawPixel(x + 6, y + 13, color);
    
    drawLine(x + 7, y,      x + 7, y + 4,  color);
    drawPixel(x + 7, y + 7, color);
    drawLine(x + 7, y + 10, x + 7, y + 12, color);
    drawPixel(x + 7, y + 15, color);
    
    drawLine(x + 8, y,     x + 8, y + 2, color);
    drawLine(x + 8, y + 4, x + 8, y + 6, color);
    drawLine(x + 8, y + 8, x + 8, y + 9, color);
    drawPixel(x + 8, y + 12, color);
    drawPixel(x + 8, y + 14, color);
    
    
    drawLine(x + 11, y,      x + 11, y + 4,  color);
    drawLine(x + 11, y + 6,  x + 11, y + 7,  color);
    drawPixel(x + 11, y + 10, color);
    drawLine(x + 11, y + 12, x + 11, y + 13, color);
    
    drawLine(x + 12, y,      x + 12, y + 1,  color);
    drawLine(x + 12, y + 3,  x + 12, y + 10, color);
    drawPixel(x + 12, y + 12, color);
    drawPixel(x + 12, y + 14, color);
    drawLine(x + 12, y + 16, x + 12, y + 17, color);
    
    drawLine(x + 13, y,      x + 13, y + 6,  color);
    drawPixel(x + 13, y + 8, color);
    drawLine(x + 13, y + 10, x + 13, y + 11, color);
    drawPixel(x + 13, y + 15, color);
    drawPixel(x + 13, y + 19, color);
    
    drawLine(x + 14, y,      x + 14, y + 4,  color);
    drawLine(x + 14, y + 6,  x + 14, y + 9,  color);
    drawLine(x + 14, y + 11, x + 14, y + 14, color);
    
    drawLine(x + 15, y,      x + 15, y + 7,  color);
    drawPixel(x + 15, y + 10, color);
    drawPixel(x + 15, y + 15, color);
    drawPixel(x + 15, y + 17, color);
    drawPixel(x + 15, y + 19, color);
    
    drawLine(x + 16, y,      x + 16, y + 14, color);
    drawPixel(x + 16, y + 16, color);
    
    drawLine(x + 17, y,      x + 17, y + 10, color);
    drawPixel(x + 17, y + 13, color);
    drawPixel(x + 17, y + 15, color);
    drawPixel(x + 17, y + 18, color);
    drawPixel(x + 17, y + 20, color);
    
    drawLine(x + 18, y,      x + 18, y + 2,  color);
    drawLine(x + 18, y + 4,  x + 18, y + 8,  color);
    drawLine(x + 18, y + 10, x + 18, y + 13, color);
    drawLine(x + 18, y + 16, x + 18, y + 17, color);
    
    drawLine(x + 19, y,      x + 19, y + 6,  color);
    drawLine(x + 19, y + 8,  x + 19, y + 10, color);
    drawPixel(x + 19, y + 12, color);
    drawLine(x + 19, y + 14, x + 19, y + 15, color);
    drawPixel(x + 19, y + 19, color);
    
    
    drawLine(x + 22, y,      x + 22, y + 8,  color);
    drawPixel(x + 22, y + 10, color);
    drawLine(x + 22, y + 12, x + 22, y + 16, color);
    drawPixel(x + 22, y + 18, color);
    drawPixel(x + 22, y + 20, color);
    drawPixel(x + 22, y + 22, color);
    
    drawLine(x + 23, y,      x + 23, y + 12, color);
    drawPixel(x + 23, y + 14, color);
    drawPixel(x + 23, y + 17, color);
    drawPixel(x + 23, y + 20, color);
    drawPixel(x + 23, y + 24, color);
    
    drawLine(x + 24, y,      x + 24, y + 8,  color);
    drawLine(x + 24, y + 10, x + 24, y + 11, color);
    drawLine(x + 24, y + 13, x + 24, y + 16, color);
    drawLine(x + 24, y + 18, x + 24, y + 19, color);
    drawPixel(x + 24, y + 22, color);
    
    drawLine(x + 25, y,      x + 25, y + 1,  color);
    drawLine(x + 25, y + 3,  x + 25, y + 13, color);
    drawLine(x + 25, y + 15, x + 25, y + 17, color);
    drawPixel(x + 25, y + 20, color);
    drawPixel(x + 25, y + 24, color);
    
    drawLine(x + 26, y,      x + 26, y + 15, color);
    drawLine(x + 26, y + 18, x + 26, y + 19, color);
    drawLine(x + 26, y + 21, x + 26, y + 22, color);
    
    drawLine(x + 27, y,      x + 27, y + 7,  color);
    drawLine(x + 27, y + 9,  x + 27, y + 11, color);
    drawLine(x + 27, y + 13, x + 27, y + 14, color);
    drawLine(x + 27, y + 16, x + 27, y + 18, color);
    drawPixel(x + 27, y + 20, color);
    drawPixel(x + 27, y + 24, color);
    
    drawLine(x + 28, y,      x + 28, y + 13, color);
    drawLine(x + 28, y + 15, x + 28, y + 17, color);
    drawPixel(x + 28, y + 20, color);
    drawPixel(x + 28, y + 22, color);
    drawPixel(x + 28, y + 26, color);
    
    drawLine(x + 29, y,      x + 29, y + 16, color);
    drawLine(x + 29, y + 18, x + 29, y + 19, color);
    drawPixel(x + 29, y + 22, color);
    drawPixel(x + 29, y + 24, color);
    
    drawLine(x + 30, y,      x + 30, y + 11, color);
    drawLine(x + 30, y + 13, x + 30, y + 15, color);
    drawLine(x + 30, y + 17, x + 30, y + 18, color);
    drawLine(x + 30, y + 20, x + 30, y + 21, color);
    drawPixel(x + 30, y + 23, color);
    drawPixel(x + 30, y + 26, color);
    
    
    drawLine(x + 33, y,      x + 33, y + 18, color);
    drawLine(x + 33, y + 20, x + 33, y + 21, color);
    drawPixel(x + 33, y + 23, color);
    drawPixel(x + 33, y + 25, color);
    
    drawLine(x + 34, y,      x + 34, y + 16, color);
    drawLine(x + 34, y + 18, x + 34, y + 19, color);
    drawPixel(x + 34, y + 22, color);
    drawPixel(x + 34, y + 24, color);
    drawPixel(x + 34, y + 27, color);
    
    drawLine(x + 35, y,      x + 35, y + 10, color);
    drawLine(x + 35, y + 12, x + 35, y + 15, color);
    drawLine(x + 35, y + 17, x + 35, y + 18, color);
    drawLine(x + 35, y + 20, x + 35, y + 23, color);
    drawPixel(x + 35, y + 26, color);
    drawPixel(x + 35, y + 29, color);
    
    drawLine(x + 36, y,      x + 36, y + 21, color);
    drawLine(x + 36, y + 24, x + 36, y + 25, color);
    drawPixel(x + 36, y + 28, color);
    
    drawLine(x + 37, y,      x + 37, y + 3,  color);
    drawLine(x + 37, y + 5,  x + 37, y + 18, color);
    drawPixel(x + 37, y + 20, color);
    drawLine(x + 37, y + 22, x + 37, y + 23, color);
    drawPixel(x + 37, y + 27, color);
    drawPixel(x + 37, y + 31, color);
    
    drawLine(x + 38, y,      x + 38, y + 14, color);
    drawLine(x + 38, y + 16, x + 38, y + 22, color);
    drawLine(x + 38, y + 24, x + 38, y + 26, color);
    drawPixel(x + 38, y + 29, color);
    
    drawLine(x + 39, y,      x + 39, y + 19, color);
    drawLine(x + 39, y + 23, x + 39, y + 24, color);
    drawPixel(x + 39, y + 27, color);
    drawPixel(x + 39, y + 31, color);
    
    drawLine(x + 40, y,      x + 40, y + 17, color);
    drawLine(x + 40, y + 19, x + 40, y + 23, color);
    drawLine(x + 40, y + 25, x + 40, y + 26, color);
    drawLine(x + 40, y + 28, x + 40, y + 29, color);
    
    drawLine(x + 41, y,      x + 41, y + 22, color);
    drawLine(x + 41, y + 24, x + 41, y + 25, color);
    drawPixel(x + 41, y + 27, color);
    drawPixel(x + 41, y + 31, color);
}

void drawZigBeeSmall(uint8 x, uint8 y, uint16 color)
{
    drawLine(x,     y,      x,     y + 2,  color);
    drawPixel(x, y + 4, color);
    drawLine(x,     y + 10, x,     y + 12, color);
    drawPixel(x + 1, y + 2, color);
    drawPixel(x + 1, y + 10, color);
    drawPixel(x + 1, y + 13, color);
    drawPixel(x + 2, y + 1, color);
    drawPixel(x + 2, y + 4, color);
    drawLine(x + 2, y + 7,  x + 2, y + 8,  color);
    drawLine(x + 2, y + 10, x + 2, y + 12, color);
    drawLine(x + 2, y + 16, x + 2, y + 17, color);
    drawLine(x + 2, y + 21, x + 2, y + 22, color);
    drawPixel(x + 3, y, color);
    drawPixel(x + 3, y + 4, color);
    drawPixel(x + 3, y + 6, color);
    drawPixel(x + 3, y + 8, color);
    drawPixel(x + 3, y + 10, color);
    drawPixel(x + 3, y + 13, color);
    drawPixel(x + 3, y + 15, color);
    drawLine(x + 3, y + 17, x + 3, y + 18, color);
    drawPixel(x + 3, y + 20, color);
    drawLine(x + 3, y + 22, x + 3, y + 23, color);
    drawLine(x + 4, y,      x + 4, y + 2,  color);
    drawPixel(x + 4, y + 4, color);
    drawLine(x + 4, y + 7,  x + 4, y + 8,  color);
    drawLine(x + 4, y + 10, x + 4, y + 12, color);
    drawLine(x + 4, y + 15, x + 4, y + 16, color);
    drawLine(x + 4, y + 20, x + 4, y + 21, color);
    drawPixel(x + 5, y + 8, color);
    drawLine(x + 5, y + 16, x + 5, y + 17, color);
    drawLine(x + 5, y + 21, x + 5, y + 22, color);
    drawLine(x + 6, y + 6,  x + 6, y + 7,  color);
}

void drawWiFiSmall(uint8 x, uint8 y, uint16 color)
{
    drawPixel(x, y, color);
    drawPixel(x, y + 4, color);
    drawPixel(x, y + 6, color);
    drawLine(x,     y + 11, x,     y + 13, color);
    drawPixel(x, y + 15, color);
    drawPixel(x + 1, y, color);
    drawPixel(x + 1, y + 2, color);
    drawPixel(x + 1, y + 4, color);
    drawPixel(x + 1, y + 11, color);
    drawPixel(x + 2, y, color);
    drawPixel(x + 2, y + 2, color);
    drawPixel(x + 2, y + 4, color);
    drawPixel(x + 2, y + 6, color);
    drawLine(x + 2, y + 8,  x + 2, y + 9,  color);
    drawLine(x + 2, y + 11, x + 2, y + 13, color);
    drawPixel(x + 2, y + 15, color);
    drawPixel(x + 3, y, color);
    drawPixel(x + 3, y + 2, color);
    drawPixel(x + 3, y + 4, color);
    drawPixel(x + 3, y + 6, color);
    drawPixel(x + 3, y + 11, color);
    drawPixel(x + 3, y + 15, color);
    drawPixel(x + 4, y + 1, color);
    drawPixel(x + 4, y + 3, color);
    drawPixel(x + 4, y + 6, color);
    drawPixel(x + 4, y + 11, color);
    drawPixel(x + 4, y + 15, color);
}

void drawGridChannel(uint16 color)
{
    uint8 xStart = 8;
    uint8 yStart = 33;
    
    drawZigBeeSmall(0, 36, color);
    
    drawNumber(xStart, yStart, 1, 0, color);
    drawNumber(xStart + 3, yStart, 2, 0, color);
    drawNumber(xStart + 12, yStart, 1, 0, color);
    drawNumber(xStart + 15, yStart, 4, 0, color);
    drawNumber(xStart + 24, yStart, 1, 0, color);
    drawNumber(xStart + 27, yStart, 6, 0, color);
    drawNumber(xStart + 36, yStart, 1, 0, color);
    drawNumber(xStart + 39, yStart, 8, 0, color);
    drawNumber(xStart + 48, yStart, 2, 0, color);
    drawNumber(xStart + 52, yStart, 0, 0, color);
    drawNumber(xStart + 60, yStart, 2, 0, color);
    drawNumber(xStart + 64, yStart, 2, 0, color);
    drawNumber(xStart + 72, yStart, 2, 0, color);
    drawNumber(xStart + 76, yStart, 4, 0, color);
    
    xStart = 16;
    yStart = 1;
    
    drawWiFiSmall(0, 8, color);
    
    drawNumber(xStart, yStart, 2, 0, color);
    drawNumber(xStart + 7, yStart, 3, 0, color);
    drawNumber(xStart + 14, yStart, 4, 0, color);
    drawNumber(xStart + 21, yStart, 5, 0, color);
    drawNumber(xStart + 28, yStart, 6, 0, color);
    drawNumber(xStart + 35, yStart, 7, 0, color);
    drawNumber(xStart + 42, yStart, 8, 0, color);
    drawNumber(xStart + 49, yStart, 9, 0, color);
    drawNumber(xStart + 54, yStart, 1, 0, color);
    drawNumber(xStart + 57, yStart, 0, 0, color);
    drawNumber(xStart + 62, yStart, 1, 0, color);
    drawNumber(xStart + 65, yStart, 1, 0, color);
}

void drawBarchart(uint16 color)
{
    uint8 i;
    uint8 xStartDelta;
    uint8 xStart = 9;
    uint8 yStart = 40;
    uint16 colorOff = COLOR_BLACK;
    
    for (i = 12; i < 25; i++)
	{
		xStartDelta = xStart + 6 * (i - 12);
        drawFrame(xStartDelta, yStart, xStartDelta + 4, yStart + 23, colorOff, colorOff);
        drawFrame(xStartDelta, yStart, xStartDelta + 4, yStart + ZigBee[i], color, color);
        drawPixel(xStartDelta, yStart + ZigBee[i], colorOff);
        drawPixel(xStartDelta + 4, yStart + ZigBee[i], colorOff);
	}
    
    xStart = 15;
    yStart = 8;
    
    for (i = 2; i < 12; i++)
	{
		xStartDelta = xStart + 7 * (i - 2);
        drawFrame(xStartDelta, yStart, xStartDelta + 4, yStart + 23, colorOff, colorOff);
        drawFrame(xStartDelta, yStart, xStartDelta + 4, yStart + WiFi[i], color, color);
        drawPixel(xStartDelta, yStart + WiFi[i], colorOff);
        drawPixel(xStartDelta + 4, yStart + WiFi[i], colorOff);
	}
}

void drawBarchartCurrentMax(void)
{
    uint8 channel;
    uint8 rssiValueMaxTemp;
	uint8 rssiValueMax1;
	uint8 rssiValueMax0;
	uint8 channelValueMaxTemp;
    
    drawLine(95, 0, 95, 54, COLOR_BLACK);
    
    if (rssiHalfValue[0] > rssiHalfValueMax[0]) // max curve on 0 channel
        rssiHalfValueMax[0] = rssiHalfValue[0];
    
    for (channel = 96; channel > 0; channel--)
    {
        if (rssiHalfValue[channel] > rssiHalfValueMax[channel]) // max curve
            rssiHalfValueMax[channel] = rssiHalfValue[channel];
        
        if (rssiHalfValueMax[channel] > rssiValueMax) // max value on max curve
        {
            rssiValueMax = rssiHalfValueMax[channel];
            channelValueMax = channel;
        }
        
        if (rssiHalfValue[channel] > rssiValueMaxCurrent) // max value on current curve
        {
            rssiValueMaxCurrent = rssiHalfValue[channel];
            channelValueMaxCurrent = channel;
        }
        
        drawLine(channel - 1, 0, channel - 1, 54, COLOR_BLACK);
        drawLine(channel, rssiHalfValueMax[channel], channel - 1, rssiHalfValueMax[channel - 1], colorMax);
        drawLine(channel, rssiHalfValue[channel], channel - 1, rssiHalfValue[channel - 1], colorCurrent);
    }
    
    rssiValueMaxTemp = 105 - rssiValueMax;
    rssiValueMax1 = rssiValueMaxTemp / 10;
    rssiValueMax0 = rssiValueMaxTemp % 10;

    channelValueMaxTemp = channelValueMax;
    if ( (96 - channelValueMax) < 12 )
        channelValueMaxTemp -= 12;
    
    drawNumber(channelValueMaxTemp, rssiHalfValueMax[channelValueMax], 10, 0, colorMax);
    drawNumber(channelValueMaxTemp + 4, rssiHalfValueMax[channelValueMax], rssiValueMax1, 0, colorMax);
    drawNumber(channelValueMaxTemp + 8, rssiHalfValueMax[channelValueMax], rssiValueMax0, 0, colorMax);
    
    rssiValueMaxTemp = 105 - rssiValueMaxCurrent;
    rssiValueMax1 = rssiValueMaxTemp / 10;
    rssiValueMax0 = rssiValueMaxTemp % 10;
    
    channelValueMaxTemp = channelValueMaxCurrent;
    if ( (96 - channelValueMaxCurrent) < 12 )
        channelValueMaxTemp -= 12;
    
    drawNumber(channelValueMaxTemp, rssiHalfValue[channelValueMaxCurrent], 10, 0, colorCurrent);
    drawNumber(channelValueMaxTemp + 4, rssiHalfValue[channelValueMaxCurrent], rssiValueMax1, 0, colorCurrent);
    drawNumber(channelValueMaxTemp + 8, rssiHalfValue[channelValueMaxCurrent], rssiValueMax0, 0, colorCurrent);
}
    
void main()
{
	uint8 channel;
    
	systemInit();
    usbInit();
	spiInit();
    delayMicroseconds(100);
    oledSpiInit();
	analyzerInit();
    
    clearWindow(0, 0, 95, 63);
	drawLogo(26, 15, COLOR_WHITE);
    checkRadioChannels();
    delayMs(2000);
    clearWindow(0, 0, 95, 63);
    drawGrid(colorGrid);

    while(1)
	{
		boardService();
        
        rssiValueMaxCurrent = 0;
		
		if (pause) // Measure voltage on P0_0
		{
			updateLeds();
            
			checkRadioChannels();
            
            LED_YELLOW(1);
            switch(state)
            {
            case 0:
                drawBarchartCurrentMax();
                drawGrid(colorGrid);
                break;
            case 1:
                drawBarchartCurrentMax();
                drawStatistics(colorGrid);
                break;
            case 2:
                drawBarchart(colorMax);
                break;
            }
            LED_YELLOW(0);
		}
		else
		{
			LED_RED(1);
		}
        
        if (!isPinHigh(12) && state != 2 && pause) // Measure voltage on P1_2
		{
            for (channel = 96; channel > 0; channel--)
			{
                rssiHalfValueMax[channel] = 0;
                rssiValueMax = 0;
                channelValueMax = 0;
            }
            
            //clearWindow(0, 0, 95, 63);
        }
        
        if (!isPinHigh(13) && pause) // Measure voltage on P1_3
        {
            if (state == 2)
                state = 0;
            else
                state++;
            
            switch(state)
            {
            case 0:
                clearWindow(0, 0, 95, 63);
                drawGrid(colorGrid);
                break;
            case 1:
                clearWindow(0, 54, 95, 63);
                drawStatistics(colorGrid);
                break;
            case 2:
                clearWindow(0, 0, 95, 63);
                drawGridChannel(colorMax);
                break;
            }
          
            while(!isPinHigh(13));
        }
        
        if (!isPinHigh(0)) // Measure voltage on P0_0
        {
            pause = !pause;
            while(!isPinHigh(0));
        }
	}
}