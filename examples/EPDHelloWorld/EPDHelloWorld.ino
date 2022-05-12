
/******************************************************************************/
// Created by: SIKTEC.
// Release Version : 1.0.1
// Creation Date: 2022-04-12
// Copyright 2022, SIKTEC.
// 
/******************************************************************************/
/*****************************      NOTES       *******************************
 * ePaper / eInk display driver to easily integrate SIKTEC displays.
 * GFX compatible with optional external SRAM use.
 * Supported drivers / SIKTEC boards:
    -> UC8276 -> IL0398 SIKTEC_EPD_G4
    -> UC8276 -> SIKTEC_EPD_3CU
    -> SSD1619 -> SIKTEC_EPD_3CS
*******************************************************************************/
/*****************************      EXAMPLE       *****************************
 * 
 * 
 * 
 * 
 * 
 * 
*******************************************************************************/
/**********************************************************************************************/
// FOR TESTING & DEBUGGING:
/**********************************************************************************************/
// #define SIKTEC_EPD_DEBUG
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
#include <Adafruit_I2CDevice.h> // Only required when compiling with platformio
#include <Adafruit_GFX.h>       // Only required when compiling with platformio
#include <SIKTEC_EPD.h>

using namespace SIKtec;

/**********************************************************************************************/
// PIN DEFINITION: 
// epd_pins_t { edp_cs, sram_cs, dc, rst, busy }
/**********************************************************************************************/
#if defined(ESP32)
    
    #define EPD_PINS {16, 17, 4, 13, 15}

#elif defined(ARDUINO_AVR_MEGA) || defined(AVR_MEGA2560) || defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
    
    #define EPD_PINS {8, 9, 10, 11, 12}

#elif defined(ARDUINO_AVR_UNO) ||  defined(ARDUINO_AVR_NANO) || defined(__AVR_ATmega328P__) || defined(__AVR_ATmega168__)

    #define EPD_PINS {9, 8, 7, 6, 5}

#elif defined(ARDUINO_AVR_LEONARDO) || defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega16U4__)

    #define EPD_PINS {9, 8, 7, 6, 5}

#elif defined(ARDUINO_SAM_DUE) || defined(SAM3X8E)

    #define EPD_PINS {9, 8, 7, 6, 5}

#endif

epd_pins_t epd_pins = EPD_PINS;
/**********************************************************************************************/
// DECLARE OBJECTS:
/**********************************************************************************************/


// SIKTEC_EPD_G4 *board;
// SIKTEC_EPD_3CU *board;
SIKTEC_EPD_3CS *board;


//Forward declaration for helper functions used by this example:
void draw_text(int16_t, int16_t, uint16_t, const char[]);
void color_map(const int16_t, const int16_t, const int16_t, const int16_t, const int16_t);
void sand_circles(const uint16_t, const uint16_t, const uint16_t);

//Setup: Serial -> EPD initialize.
void setup() {

    //Initialize Serial:
    Serial.begin(115200);
    while (!Serial) { delay(10); }
    
    delay(2000);

    //Initialize EPD:
    ////TODO: remove later for debugging
    
    Serial.println("Initialize EPD:");
    Serial.print("MOSI: ");
    Serial.println(MOSI);
    Serial.print("MISO: ");
    Serial.println(MISO);
    Serial.print("SCK: ");
    Serial.println(SCK);

    // board = new SIKTEC_EPD_G4(epd_pins);
    // board = new SIKTEC_EPD_3CU(epd_pins);
    board = new SIKTEC_EPD_3CS(epd_pins);


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

}


void loop() {
    
    Serial.println("IN LOOP >>> "); //TODO: remove later for debugging
    //Clear the buffer -> all will be white:
    board->clearBuffer();

    // //Set text size:
    board->setTextSize(2);

    #ifdef QC_PRINT_TEXT
        if (board->is_using_sram()) {
            draw_text(5, 10, EPD_BLACK, "SIKTEC EPD QC - SRAM Enabled.");
        } else {
            draw_text(5, 10, EPD_BLACK, "SIKTEC EPD QC - SRAM Disabled.");
        }
    #endif

    #ifdef QC_COLOR_MAP
        color_map(55, 55, 10, 300, 40); 
    #endif

    #ifdef QC_SAND_CIRCLES
        sand_circles(190, 12, 16);
    #endif

    // //Draw the buffer on screen and power down:
    board->display(true);

    while (1) { ; };
}

void draw_text(int16_t x, int16_t y, uint16_t color, const char str[]) {
    
    //Draw the text using GFX library:
    board->setTextColor(color);
    board->setCursor(x, y);
    board->print(str);
}

void sand_circles(const uint16_t y, const uint16_t middleRep, const uint16_t sideRep) {

    //Circles using GFX library:
    for (uint16_t i = 0; i < sideRep; i++) {
        board->drawCircle(100, y, i * 5, (i % 2 != 0 ? EPD_BLACK : EPD_RED));
    }
    for (uint16_t i = 0; i < middleRep; i++) {
        board->drawCircle(200, y, i * 5, (i % 2 != 0 ? EPD_BLACK : EPD_RED));
    }
    for (uint16_t i = 0; i < sideRep; i++) {
        board->drawCircle(300, y, i * 5, (i % 2 != 0 ? EPD_BLACK : EPD_RED));
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

