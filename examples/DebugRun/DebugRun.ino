
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


// SIKTEC_EPD_G4 *board;
// SIKTEC_EPD_3CU *board;
SIKTEC_EPD_3CS *board;


//Forward declaration for helper functions used by this example:

void draw_lines(int16_t, int16_t, uint16_t, int16_t, uint16_t);
const char userData[] = "this is a some data to be stored in user space of SRAM"; 
const size_t userDataLen = sizeof(userData) / sizeof(userData[0]);
epd_sram_space_t free_buffer;
//Setup: Serial -> EPD initialize.
void setup() {

    //Initialize Serial:
    Serial.begin(115200);
    while (!Serial) { delay(10); }
    
    delay(3000);

    //Initialize EPD:
    Serial.println(CUR_BOARD);
    Serial.println("Initialize EPD:");

    // board = new SIKTEC_EPD_G4(epd_pins);
    // board = new SIKTEC_EPD_3CU(epd_pins);
    board = new SIKTEC_EPD_3CS(epd_pins);

    //For debugging - check if we are using sram or not:
    if (board->is_using_sram()) {
        Serial.println("EPD Initialized - USING SRAM");
    } else {
        Serial.println("EPD Initialized - NOT USING SRAM");
    }

    free_buffer = board->getFreeSramSpace();
    Serial.printf("Available SRAM For user: %u, %u, %u \n", free_buffer.kbit, free_buffer.bytes, free_buffer.address);

    

    board->sram->write(free_buffer.address, (uint8_t *)userData, sizeof(userData) / sizeof(userData[0]));
    //Start communication with EPD and set the color mode: 0x7FFF
    //This can be called multiple time to change the color mode on dynamically    
    // board->begin(EPD_MODE_MONO);
    board->begin(EPD_MODE_TRICOLOR);
    // board->begin(EPD_MODE_GRAYSCALE4);

}


void loop() {
    
    Serial.println("IN LOOP >>>>>>>> "); //TODO: remove later for debugging
    //Clear the buffer -> all will be white:
    board->clearBuffer();

    int fill = 10;
    for (int i = 0; i < fill; ++i) {
        draw_liness(1, i * 3, fill*2 - i*2, 1, (i % 2 ? EPD_GRAY : EPD_BLACK));
    }

    //board->drawBitmap()
    // draw_liness(0, 0, 10, 10, EPD_BLACK);
    // draw_liness(12, 0, 10, 10, EPD_GRAY);
    // //Set text size:
    // draw_liness(0, 0, 1, 1, EPD_BLACK);
    // board->debugPixel(0, 0);
    // board->debugPixel(1, 0);
    // board->debugPixel(0, 1);
    // board->debugPixel(1, 1);

    // draw_liness(3, 0, 1, 1, EPD_GRAY);
    // board->debugPixel(3, 0);
    // board->debugPixel(4, 0);
    // board->debugPixel(3, 1);
    // board->debugPixel(4, 1);

    // //Draw the buffer on screen and power down:
    board->display(true);


    //Test user space:
    char readBuff[userDataLen];
    board->sram->read(free_buffer.address, (uint8_t *)readBuff, userDataLen);

    Serial.printf("data after all -> %s <<<\n", readBuff);
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
