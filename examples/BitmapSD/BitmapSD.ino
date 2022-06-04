
/******************************************************************************/
// Created by: SIKTEC.
// Release Version : 1.0.1
// Creation Date: 2022-04-12
// Copyright 2022, SIKTEC.
// 
/******************************************************************************/
/*****************************      NOTES       *******************************
 * This example scans the bitmaps of an SD card and draws them onto the EPD.
 * The color translation is done by the library therefor you don't need to use other tools to prep your bitmaps. 
 * That said, with E-Paper displays it's always a good practice to use 1bpp up to 16 bit color depth for better performance.
 * For the Example to run plug an SD card with some bitmaps in the root folder and uncomment your board type right after 
 * This comment.
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

//The sd card:
SdFat sd_card;
FatFile file;

//The EPD Board Driver:
#if defined(SIKTEC_BOARD_G4) 
    SIKTEC_EPD_G4 *board;
#elif defined(SIKTEC_BOARD_3CU) 
    SIKTEC_EPD_3CU *board;
#elif defined(SIKTEC_BOARD_3CS)
    SIKTEC_EPD_3CS *board;
#endif


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

    //Print to console all file names:
    sd_card.ls(LS_R | LS_DATE | LS_SIZE);

    //Initialize EPD:
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

    // For debugging - check if we are using sram or not:
    if (board->is_using_sram()) {
        Serial.println("EPD Initialized - USING SRAM");
    } else {
        Serial.println("EPD Initialized - NOT USING SRAM");
    }

    //Loop through bitmaps in root and draw them on EPD:
    sd_card.vwd()->rewind();
    char filename[75];
    //Those are the extenssion we will use:
    char ext_lower[] = ".bmp";
    char ext_upper[] = ".BMP";
    int  drawn = 0;
    //Loop through the files in the SD Root directory:
    while (file.openNext(sd_card.vwd(), O_RDONLY)) {

        //Avoid hidden and directories:
        if (!file.isHidden() && !file.isDir()) {

             //Only bitmaps:
            file.getName(filename, 75);
            if (
                strcmp(ext_lower, &filename[strlen(filename)-strlen(ext_lower)]) == 0 ||
                strcmp(ext_upper, &filename[strlen(filename)-strlen(ext_upper)]) == 0
            ) {
                //Parse the Bitmap:
                SIKTEC_EPD_BITMAP bitmapTest = SIKTEC_EPD_BITMAP(&sd_card, file);

                //Check its loaded and supported:
                if (bitmapTest.isValid()) {
                    
                    //Clear the Drawing Buffer:
                    board->clearBuffer();

                    //Draw bitmap directly to the EPD:
                    EPD_BITMAP_STATUS draw = bitmapTest.drawBitmapFiltered(BITMAP_FILTER::AUTO, 0, 0, board, 0, 0, 0, 0);

                    //Check wether bitmap was successfully drawn?
                    if (draw != EPD_BITMAP_STATUS::DONE) {
                        Serial.println("Can't draw this Bitmap... Sorry :(");
                    }

                    // Update display to show the result:
                    board->display(true);

                    drawn++;

                } else {
                    Serial.println("This bitmap is not supported... Sorry :(");
                }
                //Wait so we can see the result:
                delay(4000);
			}
        }
        file.close();
    }

    Serial.print("Finished - Drawn : ");
    Serial.print(drawn);
    Serial.println(" Bitmaps."); 
}


void loop() {
    
    while (1) { ; };
}
