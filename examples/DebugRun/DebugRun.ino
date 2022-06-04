
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
 * This Simple script is used to test EPD's - for a better HelloWorld
 * example load EPDHelloWorld.ino
 * In this example we are testing more advanced SRAM use cases that can be used later
 * For advanced features implementation of the EPD.
*******************************************************************************/

/**********************************************************************************************/
// Select your board:
/**********************************************************************************************/
// #define SIKTEC_BOARD_G4
#define SIKTEC_BOARD_3CU
// #define SIKTEC_BOARD_3CS

/**********************************************************************************************/
// LIB INCLUDES:
/**********************************************************************************************/
#include <Arduino.h>
#include <SPI.h>
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
void draw_lines(int16_t, int16_t, uint16_t, int16_t, uint16_t);

//Just for testing:
const char userData[] = "this is a some data to be stored in user space of SRAM"; 
const size_t userDataLen = sizeof(userData) / sizeof(userData[0]);
epd_sram_space_t free_buffer;

//Setup:
void setup() {

    //Initialize Serial:
    Serial.begin(9600);
    while (!Serial) { delay(10); }
    delay(3000);

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

    //SRAM additional space:
    free_buffer = board->getFreeSramSpace();
    Serial.print("SRAM free additional space: ");
    Serial.print(free_buffer.bytes);
    Serial.println(" Bytes");

    //You can use this additional free space by writing to SRAM:
    board->sram->write(free_buffer.address, (uint8_t *)userData, userDataLen);
}


void loop() {

    //Clear the buffer -> all will be white:
    board->clearBuffer();

    int fill = 60;
    for (int i = 0; i < fill; ++i) {
        draw_liness(1, i * 3, fill*2 - i*2, 1, (i % 2 ? EPD_GRAY : EPD_BLACK));
    }

    // //Draw the buffer on screen and power down:
    board->display(true);

    //Test user space:
    char readBuff[userDataLen];
    board->sram->read(free_buffer.address, (uint8_t *)readBuff, userDataLen);
    Serial.print("Reading data back from SRAM: \n  >> ");
    Serial.println(readBuff);
    
    while (1) { ; };
}

void draw_liness(int16_t sx, int16_t sy, int16_t w, int16_t h, uint16_t color) {
    //Draw the text using GFX library:
    int16_t ey = sy + h;
    int16_t ex = sx + w;
    for (int16_t y = sy; y < ey; ++y) {
        for (int16_t x = sx; x < ex; ++x) {
            board->drawPixel(x, y, color);
        }
    }
}
