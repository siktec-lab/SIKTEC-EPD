
/******************************************************************************/
// Created by: SIKTEC.
// Release Version : 1.0.2
// Creation Date: 2022-04-12
// Copyright 2022, SIKTEC.
// 
/******************************************************************************/
/*****************************      NOTES       *******************************
 * This example load a bitmap from SD and uses it as a sprite.
 * please copy the `cards-sprite.bmp` example image and place it in the SD-Card.
 * Make sure the name and path are correct on line 73 (const char filename[]).
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
#include <SdFat.h>

using namespace SIKtec;

/**********************************************************************************************/
// PIN DEFINITION: 
// epd_pins_t { edp_cs, sram_cs, dc, rst, busy }
/**********************************************************************************************/
#if defined(ESP32)
    #define CUR_BOARD "ESP32"
    #define EPD_PINS {16, 17, 4, 13, 15}
    #define SD_CS     14
    #define SD_SPI_SPEED SPI_SCK_MHZ(20)
#elif defined(ARDUINO_AVR_MEGA) || defined(AVR_MEGA2560) || defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
    #define CUR_BOARD "MEGA"
    #define EPD_PINS {8, 9, 10, 11, 12}
    #define SD_CS     14
    #define SD_SPI_SPEED SPI_SCK_MHZ(20)
#elif defined(ARDUINO_AVR_UNO) ||  defined(ARDUINO_AVR_NANO) || defined(__AVR_ATmega328P__) || defined(__AVR_ATmega168__)
    #define CUR_BOARD "328P"
    #define EPD_PINS {9, 8, 7, 6, 5}
    #define SD_CS     14
    #define SD_SPI_SPEED SPI_SCK_MHZ(4)
#elif defined(ARDUINO_AVR_LEONARDO) || defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega16U4__)
    #define CUR_BOARD "LEONARDO"
    #define EPD_PINS {9, 8, 7, 6, 5}
    #define SD_CS     4
    #define SD_SPI_SPEED SPI_SCK_MHZ(4)
#elif defined(ARDUINO_SAM_DUE) || defined(SAM3X8E)
    #define CUR_BOARD "DUE"
    #define EPD_PINS {9, 8, 7, 6, 5}
    #define SD_CS     4
    #define SD_SPI_SPEED SPI_SCK_MHZ(20)
#endif

epd_pins_t epd_pins = EPD_PINS;

/**********************************************************************************************/
// DECLARE:
/**********************************************************************************************/

SdFat sd_card;

const char filename[] = "sprites/cards-sprite.bmp";

//The EPD Board Driver:
#if defined(SIKTEC_BOARD_G4) 
    SIKTEC_EPD_G4 *board;
#elif defined(SIKTEC_BOARD_3CU) 
    SIKTEC_EPD_3CU *board;
#elif defined(SIKTEC_BOARD_3CS)
    SIKTEC_EPD_3CS *board;
#endif

//A pointer to the globally loaded sprite:
SIKTEC_EPD_BITMAP *bitmap;



//Forward declaration for helper functions used by this example:
void draw_lines(int16_t, int16_t, uint16_t, int16_t, uint16_t);

void setup() {

    //Initialize Serial:
    Serial.begin(9600);
    while (!Serial) { delay(10); }
    delay(500);

    //Initialize SD Card:
    if (!sd_card.begin(SD_CS, SD_SPI_SPEED)) {
        Serial.println("SD Card initialization Error!");
        sd_card.initErrorHalt(&Serial);
    } else {
        Serial.println("SD Card successfully initialized.");
    }
    
    //Initialize EPD:
    Serial.println(CUR_BOARD);
    Serial.println("Initialize EPD.");

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

    // For debugging - check if we are using sram or not:
    if (board->is_using_sram()) {
        Serial.println("EPD Initialized - USING SRAM");
    } else {
        Serial.println("EPD Initialized - NOT USING SRAM");
    }

    //Set board Rotation and clear the drawing buffer:
    board->setRotation(1);
    board->clearBuffer();
    
    //Initialize Bitmap helper:
    {
        
        //Load the sprite:
        Serial.print("Loading sprite: ");
        Serial.println(filename);
        SIKTEC_EPD_BITMAP bitmapTest  = SIKTEC_EPD_BITMAP(&sd_card, filename);
        
        //Define the sprite (4 columns, 9 rows)
        bitmapTest.defineBitmapSprite(4, 9);

        //Margin:
        uint16_t epd_x = 5;
        uint16_t epd_y = 5;

        //loop through the ste sprite index and print in on EPD:
        for (int s = 0; s < 36; ++s) {
            
            Serial.print("Draw Sprite: ");
            Serial.println(s);
            bitmapTest.drawBitmapSprite(BITMAP_FILTER::AUTO, epd_x, epd_y, s, board);

            epd_x += bitmapTest.sprite.width + 5; // Adjust position + margin

            if (epd_x > board->width() - (bitmapTest.sprite.width + 5)) {
                epd_x = 5;
                epd_y += bitmapTest.sprite.height + 5;
            }
        }
    }
}


void loop() {

    //Draw the buffer on screen and power down:
    Serial.println("Done - Updating display.");
    board->display(true);

    while (1) { ; };
}

