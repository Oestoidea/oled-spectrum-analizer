/*
* ssd1331.h
* A library for RGB OLED module
*/

#define RGB_OLED_WIDTH                      96
#define RGB_OLED_HEIGHT                     64

#define CMD_DRAW_LINE                       0x21
#define CMD_DRAW_RECTANGLE                  0x22
#define CMD_COPY_WINDOW                     0x23
#define CMD_DIM_WINDOW                      0x24
#define CMD_CLEAR_WINDOW                    0x25
#define CMD_FILL_WINDOW                     0x26
    #define DISABLE_FILL    0x00
    #define ENABLE_FILL     0x01
#define CMD_CONTINUOUS_SCROLLING_SETUP      0x27
#define CMD_DEACTIVE_SCROLLING              0x2E
#define CMD_ACTIVE_SCROLLING                0x2F

#define CMD_SET_COLUMN_ADDRESS              0x15
#define CMD_SET_ROW_ADDRESS                 0x75
#define CMD_SET_CONTRAST_A                  0x81
#define CMD_SET_CONTRAST_B                  0x82
#define CMD_SET_CONTRAST_C                  0x83
#define CMD_MASTER_CURRENT_CONTROL          0x87
#define CMD_SET_PRECHARGE_SPEED_A           0x8A
#define CMD_SET_PRECHARGE_SPEED_B           0x8B
#define CMD_SET_PRECHARGE_SPEED_C           0x8C
#define CMD_SET_REMAP                       0xA0
#define CMD_SET_DISPLAY_START_LINE          0xA1
#define CMD_SET_DISPLAY_OFFSET              0xA2
#define CMD_NORMAL_DISPLAY                  0xA4
#define CMD_ENTIRE_DISPLAY_ON               0xA5
#define CMD_ENTIRE_DISPLAY_OFF              0xA6
#define CMD_INVERSE_DISPLAY                 0xA7
#define CMD_SET_MULTIPLEX_RATIO             0xA8
#define CMD_DIM_MODE_SETTING                0xAB
#define CMD_SET_MASTER_CONFIGURE            0xAD
#define CMD_DIM_MODE_DISPLAY_ON             0xAC
#define CMD_DISPLAY_OFF                     0xAE
#define CMD_NORMAL_BRIGHTNESS_DISPLAY_ON    0xAF
#define CMD_POWER_SAVE_MODE                 0xB0
#define CMD_PHASE_PERIOD_ADJUSTMENT         0xB1
#define CMD_DISPLAY_CLOCK_DIV               0xB3
#define CMD_SET_GRAy_SCALE_TABLE            0xB8
#define CMD_ENABLE_LINEAR_GRAY_SCALE_TABLE  0xB9
#define CMD_SET_PRECHARGE_VOLTAGE           0xBB
#define CMD_SET_V_VOLTAGE                   0xBE

#define RGB(R,G,B)                  (((R>>3)<<11) | ((G>>2)<<5) | (B>>3))
enum Color{
    COLOR_BLACK     = RGB(  0,  0,  0), // black
    COLOR_GREY      = RGB(192,192,192), // grey
    COLOR_WHITE     = RGB(255,255,255), // white
    COLOR_RED       = RGB(255,  0,  0), // red
    COLOR_PINK      = RGB(255,192,203), // pink
    COLOR_YELLOW    = RGB(255,255,  0), // yellow
    COLOR_GOLDEN    = RGB(255,215,  0), // golden
    COLOR_BROWN     = RGB(128, 42, 42), // brown
    COLOR_BLUE      = RGB(  0,  0,255), // blue
    COLOR_CYAN      = RGB(  0,255,255), // cyan
    COLOR_GREEN     = RGB(  0,255,  0), // green
    COLOR_PURPLE    = RGB(160, 32,240), // purple
};

enum DisplayMode{
    //reset the above effect and turn the data to ON at the corresponding gray level.
    NormalDisplay   = 0xA4,
    //forces the entire display to be at "GS63"
    DisplayOn       = 0xA5,
    //forces the entire display to be at gray level "GS0"
    DisplayOff      = 0xA6,
    //swap the gray level of display data
    InverseDisplay  = 0xA7
};

enum DisplayPower{
    DimMode         = 0xAC,
    SleepMode       = 0xAE,
    NormalMode      = 0xAF
};

enum ScollingDirection{
    Horizontal      = 0x00,
    Vertical        = 0x01,
    Diagonal        = 0x02
};