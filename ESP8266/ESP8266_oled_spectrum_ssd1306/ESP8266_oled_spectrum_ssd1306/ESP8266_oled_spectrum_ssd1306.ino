#include <Wire.h>  // Only needed for Arduino 1.6.5 and earlier
#include "SSD1306.h" // alias for `#include "SSD1306Wire.h"`
#include <ESP8266WiFi.h>
#include "images.h"

//SSD1306 display(0x3c, 2, 0); // for ESP-01 SLC - GPIO0 (D3), SDA - GPIO2 (D4)
SSD1306 display(0x3c, 0, 2); // for NodeMCU SDA - GPIO0 (D3), SLC - GPIO2 (D4)

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

void setup()
{
    display.init();
    display.flipScreenVertically();
    display.clear();
    display.drawXbm(34, 14, WiFi_Logo_width, WiFi_Logo_height, WiFi_Logo_bits);
    display.display();
    delay(2000);
    display.clear();
    display.setFont(Open_Sans_9);
    display.drawString(27, 0, "Wi-Fi Channels");
    display.setFont(ArialMT_Plain_10);
    display.drawString(1, 54, "1");
    display.drawString(12, 54, "2");
    display.drawString(22, 54, "3");
    display.drawString(32, 54, "4");
    display.drawString(42, 54, "5");
    display.drawString(52, 54, "6");
    display.drawString(62, 54, "7");
    display.drawString(72, 54, "8");
    display.drawString(82, 54, "9");
    display.drawString(88, 54, "1");
    display.drawString(93, 54, "0");
    display.drawString(99, 54, "1");
    display.drawString(103, 54, "1");
    display.drawString(108, 54, "1");
    display.drawString(113, 54, "2");
    display.drawString(118, 54, "1");
    display.drawString(123, 54, "3");
    display.display();

    Serial.begin(115200);
    // Serial.setDebugOutput(true);
  
    // Set WiFi to station mode and disconnect from an AP if it was previously connected
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
  
    delay(100);
  
    Serial.println("Setup done");
    Serial.println();
    Serial.println("MAC: " + WiFi.macAddress());
}

void loop()
{
    Serial.println("scan start");
  
    int channels[13] = {0};
    
    // WiFi.scanNetworks will return the number of networks found
    int n = WiFi.scanNetworks(false,true);
  
    Serial.println("scan done");
    if (n == 0)
        Serial.println("no networks found");
    else
    {
        // sort by RSSI
        int indices[n];
        for (int i = 0; i < n; i++)
            indices[i] = i;
        for (int i = 0; i < n; i++)
            for (int j = i + 1; j < n; j++)
                if (WiFi.RSSI(indices[j]) > WiFi.RSSI(indices[i]))
                    std::swap(indices[i], indices[j]);
    
        for (int i = 1; i < 14; i++)
        {
            for (int j = 0; j < n; j++)
            {
                if (i == WiFi.channel(indices[j]))
                {
                    channels[i] += 100 + WiFi.RSSI(indices[j]);
                    if (i - 1 > 0)
                    {
                        channels[i - 1] += 75 + 3 * WiFi.RSSI(indices[j]) / 4;
                        if (i - 2 > 0)
                        {
                            channels[i - 2] += 50 + WiFi.RSSI(indices[j]) / 2;
                            if (i - 3 > 0)
                            {
                                channels[i - 3] += 25 + WiFi.RSSI(indices[j]) / 4;
                            }
                        }
                    }
                    if (i + 1 < 14)
                    {
                        channels[i + 1] += 75 + 3 * WiFi.RSSI(indices[j]) / 4;
                        if (i + 2 < 14)
                        {
                            channels[i + 2] += 50 + WiFi.RSSI(indices[j]) / 2;
                            if (i + 3 < 14)
                            {
                                channels[i + 3] += 25 + WiFi.RSSI(indices[j]) / 4;
                            }
                        }
                    }
                }
            }
        }
    
        int maxChannel = 0;
        for (int i = 1; i < 14; i++)
            if (maxChannel < channels[i])
                maxChannel = channels[i];
    
        for (int i = 1; i < 14; i++)
            channels[i] = 40 * channels[i] / maxChannel;
    
        Serial.print(n);
        Serial.println(" networks found");
    
        Serial.println("00: (RSSI)[BSSID]             [ch] [encr] SSID [hidden]");
        for (int i = 0; i < n; ++i)
        {
            // Print SSID and RSSI for each network found
            // Serial.print(i + 1);
            Serial.printf("%02d", i + 1);
            Serial.print(":");
      
            Serial.print(" (");
            Serial.print(WiFi.RSSI(indices[i]));
            Serial.print(")");
      
            Serial.print(" [");
            Serial.print(WiFi.BSSIDstr(indices[i]));
            Serial.print("]");
      
            Serial.print(" [");
            Serial.printf("%02d",(int)WiFi.channel(indices[i]));
            Serial.print("]");
      
            Serial.print(" [");
            Serial.print((String) encryptionTypeStr(WiFi.encryptionType(indices[i])));
            Serial.print("]");
            
            Serial.print(" " + WiFi.SSID(indices[i]));
            // Serial.print((WiFi.encryptionType(indices[i]) == ENC_TYPE_NONE)?" ":"*");
      
            Serial.print(" [");
            Serial.print((String) WiFi.isHidden(indices[i]));
            Serial.print("]");
            
            Serial.println();
            delay(10);
        }
    }
    Serial.println("");
  
    // Wait a bit before scanning again
    delay(1000);
  
    for (int i = 1; i < 14; i++)
    {
        int dx = 10 * (i - 1) - 1;
        display.setColor(BLACK);
        for (int j = 1; j < 10; j++)
            display.drawLine(j + dx, 53, j + dx, 12);
        display.display();
        
        display.setColor(WHITE);
        display.drawLine(1 + dx, 53, 1 + dx, 53 - channels[i]);
        for (int j = 2; j < 9; j++)
            display.drawLine(j + dx, 53, j + dx, 52 - channels[i]);
        display.drawLine(9 + dx, 53, 9 + dx, 53 - channels[i]);
        display.display();
    }
}
