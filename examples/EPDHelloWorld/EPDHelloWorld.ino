
/******************************************************************************/
// Created by: SIKTEC.
// Release Version : 1.0.2
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
 * Hello world Example
 * Will Print text on the EPD.
 * Will print a color map.
 * Will draw some circle patterns on the EPD. 
 * FOR THIS EXAMPLE TO WORK:
 * 1. uncomment the correct EPD module you are using.
 * 
*******************************************************************************/

/**********************************************************************************************/
// Select your board:
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
    
    #define CUR_BOARD "ESP32"
    #define EPD_PINS {16, 17, 4, 13, 15} 

#elif defined(ARDUINO_AVR_MEGA) || defined(AVR_MEGA2560) || defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
    
    #define CUR_BOARD "MEGA"
    #define EPD_PINS {8, 9, 10, 11, 12}

#elif defined(ARDUINO_AVR_UNO) ||  defined(ARDUINO_AVR_NANO) || defined(__AVR_ATmega328P__) || defined(__AVR_ATmega168__)
    
    #define CUR_BOARD "328P"
    #define EPD_PINS {9, 8, 7, 6, 5}

#elif defined(ARDUINO_AVR_LEONARDO) || defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega16U4__)
    
    #define CUR_BOARD "LEONARDO"
    #define EPD_PINS {9, 8, 7, 6, 5}

#elif defined(ARDUINO_SAM_DUE) || defined(SAM3X8E)
    
    #define CUR_BOARD "DUE"
    #define EPD_PINS {9, 8, 7, 6, 5}

#endif

epd_pins_t epd_pins = EPD_PINS;

/**********************************************************************************************/
// DECLARE OBJECTS:
/**********************************************************************************************/

//The EPD Board Driver:
#if defined(SIKTEC_BOARD_G4) 
    SIKTEC_EPD_G4 *board;
#elif defined(SIKTEC_BOARD_3CU) 
    SIKTEC_EPD_3CU *board;
#elif defined(SIKTEC_BOARD_3CS)
    SIKTEC_EPD_3CS *board;
#endif


//Forward declaration for helper functions used by this example:
void draw_text(int16_t, int16_t, uint16_t, const char[]);
void color_map(const int16_t, const int16_t, const int16_t, const int16_t, const int16_t);
void sand_circles(const uint16_t, const uint16_t, const uint16_t);

//Setup:
void setup() {

    //Initialize Serial:
    Serial.begin(9600);
    while (!Serial) { delay(10); }

    //Initialize EPD:
    Serial.println(CUR_BOARD);
    Serial.println("Initialize EPD:");

    #if defined(SIKTEC_BOARD_G4) 

        board = new SIKTEC_EPD_G4(epd_pins);
        board->begin(EPD_MODE_GRAYSCALE4);

    #elif defined(SIKTEC_BOARD_3CU) 

        board = new SIKTEC_EPD_3CU(epd_pins);
        board->begin(EPD_MODE_TRICOLOR);

    #elif defined(SIKTEC_BOARD_3CS)

        board = new SIKTEC_EPD_3CS(epd_pins);
        board->begin(EPD_MODE_TRICOLOR);

    #endif

    //For debugging - check if we are using sram or not:
    if (board->is_using_sram()) {
        Serial.println("EPD Initialized - USING SRAM");
    } else {
        Serial.println("EPD Initialized - NOT USING SRAM");
    }

}


void loop() {
    
    //Clear the buffer -> all will be white:
    board->clearBuffer();

    //Set text size:
    board->setTextSize(3);

    //Print header message:
    draw_text(5, 10, EPD_BLACK, "SIKTEC EPD MODULE");
    board->setTextSize(2);

    //Print SRAM status?
    draw_text(5, 45, EPD_BLACK, 
        board->is_using_sram() ? " > SRAM - Enabled." : " > SRAM - Disabled."
    );

    //Color map palette"
    draw_text(5, 70, EPD_BLACK, " > COLOR MAP:");
    color_map(55, 95, 10, 300, 40); 

    //Some nice Circles:
    draw_text(5, 145, EPD_BLACK, " > SAND CIRCLES:");
    sand_circles(232, 12, 14);

    // Draw the buffer on screen and power down:
    board->display(true);

    while (1) { ; };
}

void draw_line_thickness(int x, int y, int x1, int y1, int thickness, uint16_t color) { 
    int   dx = abs(x - x1);
    int   dy = abs(y - y1);
    if (dx == 0) {
        //vertical line:
        board->fillRect(
            x - (thickness / 2), y > y1 ? y1 : y, thickness, dy, EPD_BLACK
        );
    } else if (dy == 0) {
        //Horizontal line:
        board->fillRect(
            x > x1 ? x1 : x, y - (thickness / 2), thickness, dx, EPD_BLACK
        );
    } else {
        //Angled line:
        float ddx = (thickness / 2.0) * (x - x1) / sqrt(sq(x - x1) + sq(y - y1));
        float ddy = (thickness / 2.0) * (y - y1) / sqrt(sq(x - x1) + sq(y - y1));
        board->fillTriangle(x + ddx, y - ddy, x - ddx, y + ddy, x1 + ddx, y1 - ddy, color);
        board->fillTriangle(x - ddx, y + ddy, x1 - ddx, y1 + ddy, x1 + ddx, y1 - ddy, color);
    }
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
            x + (move * i), y, 
            cellW, cellH, 
            EPD_BLACK
        );
        board->fillRect(
            x + (move * i) + 4, y + 4, 
            cellW - 8, cellH - 8, 
            i
        );
    }
}

