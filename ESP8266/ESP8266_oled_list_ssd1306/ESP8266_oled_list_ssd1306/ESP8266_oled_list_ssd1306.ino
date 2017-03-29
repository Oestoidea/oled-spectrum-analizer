#include <ESP8266WiFi.h>
#include <Wire.h>
#include "font.h"

#define height 64                       // 64 or 32 for 128x64 or 128x32 OLEDs

#define offset        0x00             // offset=0 for SSD1306 controller
//#define offset        0x02            // offset=2 for SH1106 controller
#define OLEDaddress  0x3c
#define myFont        myFont6         
#define fontWeigth    6

const char* SSID;
const char* RSSI;
const char* CH;

char RSSItmp[4];
char CHtmp[4];

String encryptionTypeStr(uint8_t authmode) {
    switch(authmode) {
        case ENC_TYPE_NONE:
            return "NONE";
        case ENC_TYPE_WEP:
            return "WEP ";
        case ENC_TYPE_TKIP:
            return "TKIP";
        case ENC_TYPE_CCMP:
            return "CCMP";
        case ENC_TYPE_AUTO:
            return "AUTO";
        default:
            return "?   ";
    }
}

void setup(void) {
    Serial.begin(115200);
    Wire.begin(0, 2);                       // Initialize I2C and OLED Display SDA - GPIO0 (D3), SLC - GPIO2 (D4)
    initOLED();                             // Initialize I2C and OLED Display SDA - GPIO7 (SD0), SLC - GPIO6 (CLK)
    resetDisplay();
        
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();

    //sendStrXY("Setup done",0,0);
    delay(100);
    clearDisplay();
    
    Serial.println("Setup done");
    Serial.println();
    Serial.println("MAC: " + WiFi.macAddress());
}

void loop(void) {
    //server.handleClient();    // checks for incoming messages
    // WiFi.scanNetworks will return the number of networks found
   
    int n = WiFi.scanNetworks();
    //Serial.println("scan done");
    int indices[n];
    for (int i = 0; i < n; i++)
        indices[i] = i;
    for (int i = 0; i < n; i++)
        for (int j = i + 1; j < n; j++)
            if (WiFi.RSSI(indices[j]) > WiFi.RSSI(indices[i]))
                std::swap(indices[i], indices[j]);
    
    //clearDisplay();
    if (n == 0)
    {
        sendStrXY("no networks",0,0);
        Serial.println("no networks");
    }
    else
    {
        for (int j = 0; j < height / 8; j++)
        //for (int i = 0; i < n; ++i)
        {
            // Print SSID and RSSI for each network found
            SSID = strcpy((char*)malloc(WiFi.SSID(indices[j]).length()+1), WiFi.SSID(indices[j]).c_str());
            //sprintf(RSSItmp, "%d", WiFi.RSSI(i));
            snprintf(RSSItmp, 4, "%d", WiFi.RSSI(indices[j]));
            RSSI = strcpy((char*)malloc(5), RSSItmp);
            if  (WiFi.channel(indices[j]) >= 10)
                snprintf(CHtmp, 4, "%d", WiFi.channel(indices[j]));
            else
                snprintf(CHtmp, 4, "% d", WiFi.channel(indices[j]));
            CH = strcpy((char*)malloc(5), CHtmp);
            clearRow(j);
            sendStrXY(CH, j, 0);
            sendStrXY(SSID, j, 2);
            sendStrXY(RSSI, j, 13); // prints SSID on OLED
            delay(10);
        }

        Serial.print(n);
        Serial.println(" networks found");
    
        Serial.println("00: (RSSI)[BSSID]             [ch] [encr] SSID [hidden]");
        for (int j = 0; j < n; ++j)
        {
            // Print SSID and RSSI for each network found
            Serial.printf("%02d", j + 1);
            Serial.print(":");
      
            Serial.print(" (");
            Serial.print(WiFi.RSSI(indices[j]));
            Serial.print(")");
      
            Serial.print(" [");
            Serial.print(WiFi.BSSIDstr(indices[j]));
            Serial.print("]");
      
            Serial.print(" [");
            Serial.printf("%02d",(int)WiFi.channel(indices[j]));
            Serial.print("]");
      
            Serial.print(" [");
            Serial.print((String) encryptionTypeStr(WiFi.encryptionType(indices[j])));
            Serial.print("]");
            
            Serial.print(" " + WiFi.SSID(indices[j]));
            // Serial.print((WiFi.encryptionType(indices[i]) == ENC_TYPE_NONE)?" ":"*");
      
            Serial.print(" [");
            Serial.print((String) WiFi.isHidden(indices[j]));
            Serial.print("]");
            
            Serial.println();
            delay(10);
        }
    }
    Serial.println("");
    
    
    // Wait a bit before scanning again
    delay(5000);
}

// Resets display depending on the actual mode.
static void resetDisplay(void)
{
    displayOff();
    clearDisplay();
    displayOn();
}

// Turns display on.
void displayOn(void)
{
    sendCommand(0xAF);        //display on
}

// Turns display off.
void displayOff(void)
{
    sendCommand(0xAE);    //display off
}

// Clears the display by sendind 0 to all the screen map.
static void clearDisplay(void)
{
    unsigned char k;
    for(k = 0; k < height / 8; k++)
    { 
        setXY(k,0);    
        for(int i = 0; i < (128 + 2 * offset); i++)     //locate all COL
        {
            sendChar(0);         //clear all COL
            //delay(10);
        }
    }
}

// Clears the display by sendind 0 to all the screen map.
static void clearRow(unsigned char k)
{
    setXY(k, 0);    
    for(int i = 0; i < (128 + 2 * offset); i++)     //locate all COL
    {
        sendChar(0);         //clear all COL
        //delay(10);
    }
}

// Actually this sends a byte, not a char to draw in the display. 
// Display's chars uses 8 byte font the small ones and 96 bytes
// for the big number font.
static void sendChar(unsigned char data) 
{
    //if (interrupt && !doing_menu) return;   // Stop printing only if interrupt is call but not in button functions
    
    Wire.beginTransmission(OLEDaddress); //begin transmitting
    Wire.write(0x40);                     //data mode
    Wire.write(data);
    Wire.endTransmission();               //stop transmitting
}

// Prints a display char (not just a byte) in coordinates X Y,
// being multiples of 8. This means we have 16 COLS (0-15) 
// and 8 ROWS (0-7).
static void sendCharXY(unsigned char data, int X, int Y)
{
    setXY(X, Y);
    Wire.beginTransmission(OLEDaddress); //begin transmitting
    Wire.write(0x40);                     //data mode
    
    for (int i = 0; i < fontWeigth; i++)
       Wire.write(pgm_read_byte(myFont[data - 0x20] + i));
      
    Wire.endTransmission();               //stop transmitting
}

// Used to send commands to the display.
static void sendCommand(unsigned char com)
{
    Wire.beginTransmission(OLEDaddress);     //begin transmitting
    Wire.write(0x80);                         //command mode
    Wire.write(com);
    Wire.endTransmission();                   //stop transmitting
}

// Set the cursor position in a 16 COL * 8 ROW map.
static void setXY(unsigned char row, unsigned char col)
{
    sendCommand(0xB0 + row);                      //set page address
    sendCommand(offset + (8 * col & 0x0F));       //set low col address
    sendCommand(0x10 + ((8 * col >> 4) & 0x0F));  //set high col address
}

// Prints a string regardless the cursor position.
static void sendStr(unsigned char *string)
{
    unsigned char i = 0;
    while(*string)
    {
        for(i = 0; i < fontWeigth; i++)
            sendChar(pgm_read_byte(myFont[*string - 0x20] + i));
        *string++;
    }
}

// Prints a string in coordinates X Y, being multiples of 8.
// This means we have 16 COLS (0-15) and 8 ROWS (0-7).
static void sendStrXY(const char *string, int X, int Y)
{
    setXY(X,Y);
    unsigned char i = 0;
    while(*string)
    {
        for(i = 0; i < fontWeigth; i++)
            sendChar(pgm_read_byte(myFont[*string - 0x20] + i));
        *string++;
    }
}

// Inits oled and draws logo at startup
static void initOLED(void)
{
    displayOff();
    // Adafruit Init sequence for 128x64 OLED module
    sendCommand(0xAE);    //DISPLAYOFF
    sendCommand(0xD5);    //SETDISPLAYCLOCKDIV
    sendCommand(0x80);    // the suggested ratio 0x80
    sendCommand(0xA8);    //SSD1306_SETMULTIPLEX
    if (height == 64)
        sendCommand(0x3F);  // for 128x64
    else
        sendCommand(0x1F);  // for 128x32
    sendCommand(0xD3);    //SETDISPLAYOFFSET
    sendCommand(0x0);     //no offset
    sendCommand(0x40 | 0x0);    //SETSTARTLINE
    sendCommand(0x8D);    //CHARGEPUMP
    sendCommand(0x14);
    sendCommand(0x20);    //MEMORYMODE
    sendCommand(0x00);    //0x0 act like ks0108
    
    //sendCommand(0xA0 | 0x1);    //SEGREMAP   //Rotate screen 180 deg
    sendCommand(0xA0);
    
    //sendCommand(0xC8);    //COMSCANDEC  Rotate screen 180 Deg
    sendCommand(0xC0);
    
    sendCommand(0xDA);     //COMSCANDEC
    if (height == 64)       //
        sendCommand(0x12);  // for 128x64
    else
        sendCommand(0x02);  // for 128x32
    sendCommand(0x81);     //SETCONTRAS
    if (height == 64)       //
        sendCommand(0xCF);  // for 128x64
    else
        sendCommand(0x8F);  // for 128x32
    sendCommand(0xd9);     //SETPRECHARGE 
    sendCommand(0xF1); 
    sendCommand(0xDB);     //SETVCOMDETECT                
    sendCommand(0x40);
    sendCommand(0xA4);     //DISPLAYALLON_RESUME        
    sendCommand(0xA6);     //NORMALDISPLAY             
    
    clearDisplay();
    sendCommand(0x2e);     // stop scroll
    //----------------------------REVERSE comments----------------------------//
    sendCommand(0xa0);     //seg re-map 0->127(default)
    sendCommand(0xa1);     //seg re-map 127->0
    sendCommand(0xc8);
    delay(1000);
    //----------------------------REVERSE comments----------------------------//
    //sendCommand(0xa7);    //Set Inverse Display  
    //sendCommand(0xae);    //display off
    sendCommand(0x20);     //Set Memory Addressing Mode
    sendCommand(0x00);     //Set Memory Addressing Mode ab Horizontal addressing mode
    //sendCommand(0x02);    // Set Memory Addressing Mode ab Page addressing mode(RESET)  
}
