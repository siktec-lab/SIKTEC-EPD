
/**********************************************************************************************/
// FOR TESTING & DEBUGGING:
/**********************************************************************************************/
// #define SIKTEC_EPD_DEBUG
#define DECLARE_WITH_SRAM true
#define QC_PRINT_TEXT
#define QC_COLOR_MAP
#define QC_SAND_CIRCLES

/**********************************************************************************************/
// TEST WITH TIS BOARD:
/**********************************************************************************************/
// #define SIKTEC_BOARD_G4
// #define SIKTEC_BOARD_3CU
#define SIKTEC_BOARD_3CS


/**********************************************************************************************/
// LIB INCLUDES:
/**********************************************************************************************/
#include <Arduino.h>
#include <Adafruit_I2CDevice.h>
#include <SIKTEC_EPD.h>


/**********************************************************************************************/
// PIN DEFINITION:
/**********************************************************************************************/
#define EPD_CS      16
#define EPD_DC      4
#if DECLARE_WITH_SRAM
    #define SRAM_CS 17
#else
    #define SRAM_CS -1
#endif
#define EPD_RESET   13 // can set to -1 and share with microcontroller Reset!
#define EPD_BUSY    15 // can set to -1 to not use a pin (will wait a fixed delay)

/**********************************************************************************************/
// DECLARE OBJECTS:
/**********************************************************************************************/

using namespace SIKtec;

//SIKTEC_EPD_G4 *board;
//SIKTEC_EPD_3CU *board;
SIKTEC_EPD_3CS *board;


//Forward declaration for helper functions used by this example:
void draw_text(int16_t, int16_t, uint16_t, const char[]);
void color_map(const int16_t, const int16_t, const int16_t, const int16_t, const int16_t);
void sand_circles(const int16_t, const int16_t, const int16_t);

//Setup: Serial -> EPD initialize.
void setup() {

    //Initialize Serial:
    Serial.begin(115200);
    while (!Serial) { delay(10); }
    
    delay(1000);

    //Initialize EPD:
    Serial.println("Initialize EPD:");
    //board = new SIKTEC_EPD_G4(EPD_DC, EPD_RESET, EPD_CS, SRAM_CS, EPD_BUSY);
    //board = new SIKTEC_EPD_3CU(EPD_DC, EPD_RESET, EPD_CS, SRAM_CS, EPD_BUSY);
    board = new SIKTEC_EPD_3CS(EPD_DC, EPD_RESET, EPD_CS, SRAM_CS, EPD_BUSY);

    //For debugging - check if we are using sram or not:
    if (board->is_using_sram()) {
        Serial.println("EPD Initialized - USING SRAM");
    } else {
        Serial.println("EPD Initialized - NOT USING SRAM");
    }

    //Start communication with EPD and set the color mode:
    //This can be called multiple time to change the color mode on dynamically
    
    // board->begin(EPD_MODE_MONO);
    board->begin(EPD_MODE_TRICOLOR);
    // board->begin(EPD_MODE_GRAYSCALE4);

    delay(1000);
}


void loop() {

    //Clear the buffer -> all will be white:
    board->clearBuffer();

    //Set text size:
    board->setTextSize(2);

    #ifdef QC_PRINT_TEXT
        if (board->is_using_sram()) {
            draw_text(5, 5, EPD_BLACK, "SIKTEC EPD QC - SRAM Enabled.");
        } else {
            draw_text(5, 5, EPD_BLACK, "SIKTEC EPD QC - SRAM Disabled.");
        }
    #endif

    #ifdef QC_COLOR_MAP
        color_map(50, 35, 10, 300, 40); 
    #endif

    #ifdef QC_SAND_CIRCLES
        sand_circles(240, 6, 10);
    #endif

    //Draw the buffer on screen and power down:
    board->display(true);

    while (1) { ; };
}

void draw_text(int16_t x, int16_t y, uint16_t color, const char str[]) {
    
    //Draw the text using GFX library:
    board->setTextColor(color);
    board->setCursor(x, y);
    board->print(str);
}

void sand_circles(const int16_t y, const int16_t middleRep, const int16_t sideRep) {

    //Circles using GFX library:
    for (uint16_t i = 0; i < middleRep; i++) {
        board->drawCircle(200, y, i * 5, (i % 2 != 0 ? EPD_BLACK : EPD_RED));
    }
    for (uint16_t i = 0; i < sideRep; i++) {
        board->drawCircle(140, y, i * 5, (i % 2 != 0 ? EPD_BLACK : EPD_RED));
    }
    for (uint16_t i = 0; i < sideRep; i++) {
        board->drawCircle(260, y, i * 5, (i % 2 != 0 ? EPD_BLACK : EPD_RED));
    }
}

void color_map(const int16_t x, const int16_t y, const int16_t gutter, const int16_t width, const int16_t height) {
    int16_t cellW = (width - (gutter * EPD_NUM_COLORS - 1)) / EPD_NUM_COLORS;  
    int16_t cellH = height;
    int16_t move  = cellW + gutter;

    //Rectangles using GFX library:
    for (uint8_t i = 0; i < EPD_NUM_COLORS; ++i) {
        board->drawRect(
            x + (move * i), 
            y, 
            cellW, 
            cellH, 
            EPD_BLACK
        );
        board->fillRect(
            x + (move * i) + 4, 
            y + 4, 
            cellW - 8, 
            cellH - 8, 
            i
        );
    }
}

