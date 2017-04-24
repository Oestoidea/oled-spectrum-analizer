/* spectrum analyzer:
*
* The receivers(s) (the Wixel(s) loaded with spectrum_analyzer) will
* report the signal strength on all 256 channels.
*
* Maximum signal strength is reported in dBm on the USB serial connection.
* This will be a number typically between -100 and -30, indicating the signal strength.
* Heavily modified from test_radio_signal_rx app by S. James Remington
*
*/

#include <wixel.h>
#include <radio_registers.h>
#include <stdio.h>
#include <usb.h>
#include <usb_com.h>

static int16 XDATA rssiValue[256];

void frequentTasks(void);

void updateLeds()
{
    usbShowStatusWithGreenLed();
    // Yellow LED is controlled by checkRadioChannels
    LED_RED(0);
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

void checkRadioChannels()
{
    uint16 i;
    uint16 channel;

    LED_YELLOW(1);
    
    for(channel=0; channel<256; channel++) {
        int32 rssiSum;

        rssiValue[channel] = -115;

        while(MARCSTATE != 1);      //radio should already be idle, but check anyway
        CHANNR = channel;
        RFST = 2;                   // radio in RX mode and autocal
        while(MARCSTATE != 13);     //wait for RX mode
        rssiSum = 0;
        for (i=0; i<100; i++)
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

        frequentTasks();
    }  // the above loop takes about 414 ms on average, so about 1.6 ms/channel

    LED_YELLOW(0);
}

void putchar(char c)
{
    while(!usbComTxAvailable())
        frequentTasks();

    usbComTxSendByte(c);
}

void reportResults(int iterationFirst, int iterationLast)
{
    uint16 i;
    uint32 time;
    
    time = getMs();
    
    if (iterationFirst == 0)
    {
        printf("%02X-%02X-%02X-%02X #%d, 0x%04X%04X ms [ ",
                (uint16)serialNumber[3], (uint16)serialNumber[2], (uint16)serialNumber[1], (uint16)serialNumber[0], iterationLast, (uint16)(time >> 16), (uint16)time);
    }
    else
    {
        printf("%02X-%02X-%02X-%02X #%d%04d, 0x%04X%04X ms [ ",
                (uint16)serialNumber[3], (uint16)serialNumber[2], (uint16)serialNumber[1], (uint16)serialNumber[0], iterationFirst, iterationLast, (uint16)(time >> 16), (uint16)time);
    }
    
    for (i = 0; i < 256; i++)
        printf("%d ", rssiValue[i]);

    printf("]\r\n\n");
}

void frequentTasks()
{
    boardService();
    usbComService();
    updateLeds();
}

void main()
{
    uint32 iterationFirst;
    uint32 iterationLast;
    
    int mute;
    mute = 1;
    
    iterationFirst = 0;
    iterationLast = 0;
    
    systemInit();
    usbInit();
    analyzerInit();
    
    setDigitalInput(0, 1); // pull-up P0_0
    
    //Data rate = 350 kbps
    //Modulation = MSK
    //Channel 0 frequency = 2403.47 MHz
    //Channel spacing = 286.4 kHz
    //Channel bandwidth = 600 kHz
    /*
    printf("Pololu Wixel [2403.47–2476.50 MHz with spacing in 286.4 kHz] on CC2511F32\r\n");
    printf("Serial number: %02X-%02X-%02X-%02X\r\n\n", (uint16)serialNumber[3], (uint16)serialNumber[2], (uint16)serialNumber[1], (uint16)serialNumber[0]);
    printf("Frequency list [ Channel number   Frequency, MHz ]:\r\n");
    printf("[ 
    0 2403.47 | 1 2403.76 | 2 2404.04 | 3 2404.33 | 4 2404.62 | 5 2404.90 | 6 2405.19 | 7 2405.47 | 8 2405.76 | 9 2406.05 |
    10 2406.33 | 11 2406.62 | 12 2406.91 | 13 2407.19 | 14 2407.48 | 15 2407.77 | 16 2408.05 | 17 2408.34 | 18 2408.63 | 19 2408.91 | 
    20 2409.20 | 21 2409.48 | 22 2409.77 | 23 2410.06 | 24 2410.34 | 25 2410.63 | 26 2410.92 | 27 2411.20 | 28 2411.49 | 29 2411.78 | 
    30 2412.06 | 31 2412.35 | 32 2412.63 | 33 2412.92 | 34 2413.21 | 35 2413.49 | 36 2413.78 | 37 2414.07 | 38 2414.35 | 39 2414.64 | 
    40 2414.93 | 41 2415.21 | 42 2415.50 | 43 2415.79 | 44 2416.07 | 45 2416.36 | 46 2416.64 | 47 2416.93 | 48 2417.22 | 49 2417.50 | 
    50 2417.79 | 51 2418.08 | 52 2418.36 | 53 2418.65 | 54 2418.94 | 55 2419.22 | 56 2419.51 | 57 2419.79 | 58 2420.08 | 59 2420.37 | 
    60 2420.65 | 61 2420.94 | 62 2421.23 | 63 2421.51 | 64 2421.80 | 65 2422.09 | 66 2422.37 | 67 2422.66 | 68 2422.95 | 69 2423.23 | 
    70 2423.52 | 71 2423.80 | 72 2424.09 | 73 2424.38 | 74 2424.66 | 75 2424.95 | 76 2425.24 | 77 2425.52 | 78 2425.81 | 79 2426.10 | 
    80 2426.38 | 81 2426.67 | 82 2426.95 | 83 2427.24 | 84 2427.53 | 85 2427.81 | 86 2428.10 | 87 2428.39 | 88 2428.67 | 89 2428.96 | 
    90 2429.25 | 91 2429.53 | 92 2429.82 | 93 2430.11 | 94 2430.39 | 95 2430.68 | 96 2430.96 | 97 2431.25 | 98 2431.54 | 99 2431.82 | 
    100 2432.11 | 101 2432.40 | 102 2432.68 | 103 2432.97 | 104 2433.26 | 105 2433.54 | 106 2433.83 | 107 2434.11 | 108 2434.40 | 109 2434.69 | 
    110 2434.97 | 111 2435.26 | 112 2435.55 | 113 2435.83 | 114 2436.12 | 115 2436.41 | 116 2436.69 | 117 2436.98 | 118 2437.27 | 119 2437.55 | 
    120 2437.84 | 121 2438.12 | 122 2438.41 | 123 2438.70 | 124 2438.98 | 125 2439.27 | 126 2439.56 | 127 2439.84 | 128 2440.13 | 129 2440.42 | 
    130 2440.70 | 131 2440.99 | 132 2441.27 | 133 2441.56 | 134 2441.85 | 135 2442.13 | 136 2442.42 | 137 2442.71 | 138 2442.99 | 139 2443.28 | 
    140 2443.57 | 141 2443.85 | 142 2444.14 | 143 2444.43 | 144 2444.71 | 145 2445.00 | 146 2445.28 | 147 2445.57 | 148 2445.86 | 149 2446.14 | 
    150 2446.43 | 151 2446.72 | 152 2447.00 | 153 2447.29 | 154 2447.58 | 155 2447.86 | 156 2448.15 | 157 2448.43 | 158 2448.72 | 159 2449.01 | 
    160 2449.29 | 161 2449.58 | 162 2449.87 | 163 2450.15 | 164 2450.44 | 165 2450.73 | 166 2451.01 | 167 2451.30 | 168 2451.59 | 169 2451.87 | 
    170 2452.16 | 171 2452.44 | 172 2452.73 | 173 2453.02 | 174 2453.30 | 175 2453.59 | 176 2453.88 | 177 2454.16 | 178 2454.45 | 179 2454.74 | 
    180 2455.02 | 181 2455.31 | 182 2455.59 | 183 2455.88 | 184 2456.17 | 185 2456.45 | 186 2456.74 | 187 2457.03 | 188 2457.31 | 189 2457.60 | 
    190 2457.89 | 191 2458.17 | 192 2458.46 | 193 2458.75 | 194 2459.03 | 195 2459.32 | 196 2459.60 | 197 2459.89 | 198 2460.18 | 199 2460.46 | 
    200 2460.75 | 201 2461.04 | 202 2461.32 | 203 2461.61 | 204 2461.90 | 205 2462.18 | 206 2462.47 | 207 2462.75 | 208 2463.04 | 209 2463.33 | 
    210 2463.61 | 211 2463.90 | 212 2464.19 | 213 2464.47 | 214 2464.76 | 215 2465.05 | 216 2465.33 | 217 2465.62 | 218 2465.91 | 219 2466.19 | 
    220 2466.48 | 221 2466.76 | 222 2467.05 | 223 2467.34 | 224 2467.62 | 225 2467.91 | 226 2468.20 | 227 2468.48 | 228 2468.77 | 229 2469.06 | 
    230 2469.34 | 231 2469.63 | 232 2469.91 | 233 2470.20 | 234 2470.49 | 235 2470.77 | 236 2471.06 | 237 2471.35 | 238 2471.63 | 239 2471.92 | 
    240 2472.21 | 241 2472.49 | 242 2472.78 | 243 2473.07 | 244 2473.35 | 245 2473.64 | 246 2473.92 | 247 2474.21 | 248 2474.50 | 249 2474.78 | 
    250 2475.07 | 251 2475.36 | 252 2475.64 | 253 2475.93 | 254 2476.22 | 255 2476.50 ]\r\n\n");
    */
    while(1)
    {
        if (usbComRxAvailable() == 0)
        {
            if (isPinHigh(0) && mute == 1) // Measure voltage on P0_0
            {
                LED_RED(0);
                frequentTasks();
                checkRadioChannels();
                // problem with 16-bit number, used two registers
                if (iterationLast == 10000)
                {
                    iterationLast -= 10000;
                    iterationFirst++;
                }
                reportResults(iterationFirst, iterationLast);
                iterationLast++;
            }
            else
                LED_RED(1);
        }
        else
        {
            switch(usbComRxReceiveByte())
            {
            case 's': //stop
                mute = 0;
                break;
            case 'c': //start
                mute = 1;
                break;
            }
        }
        
        usbComService();
    }
}
