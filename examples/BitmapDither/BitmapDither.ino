
/******************************************************************************/
// Created by: SIKTEC.
// Release Version : 1.0.1
// Creation Date: 2022-04-12
// Copyright 2022, SIKTEC.
// 
/******************************************************************************/
/*****************************      NOTES       *******************************
 * This Example showcase how to apply dithering to a bitmap. This allows drawing 
 * Directly high res bitmaps to the EPD without using "too many memory".
 * Dithering is a method to distribute pixel errors of a quantized image. This way 
 * Pixel color is corrected by "grouping" neighbor pixels and setting them to the right
 * colors to match the current pixel shade. 
 * Dithering is a compute intensive filter and must allocate a buffer of at least - 
 * width (pixels) * 2 (rows) * 2 bytes - For more capable MCUs set the USE_RAM_FOR_DITHERING flag
 * for best performance and for others dont. The library will use the EPD SRAM extenssion chip as
 * the buffer (will be slow).
 * 
 * FOR THIS EXAMPLE TO WORK:
 * 1. uncomment the correct EPD module you are using.
 * 2. copy images (p_example1.bmp, p_example2.bmp, p_example3.bmp)   to an SD card and plug it in.
 * 
*******************************************************************************/

/**********************************************************************************************/
// SELECT YOUR EPD MODULE:
/**********************************************************************************************/
// #define SIKTEC_BOARD_G4
// #define SIKTEC_BOARD_3CU
#define SIKTEC_BOARD_3CS

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
    #define USE_RAM_FOR_DITHERING true //FASTER

#elif defined(ARDUINO_AVR_MEGA) || defined(AVR_MEGA2560) || defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)

    #define CUR_BOARD "MEGA"
    #define EPD_PINS {8, 9, 10, 11, 12}
    #define SD_CS     14
    #define SD_SPI_SPEED SPI_SCK_MHZ(20)
    #define USE_RAM_FOR_DITHERING true //FASTER

#elif defined(ARDUINO_AVR_UNO) ||  defined(ARDUINO_AVR_NANO) || defined(__AVR_ATmega328P__) || defined(__AVR_ATmega168__)

    #define CUR_BOARD "328P"
    #define EPD_PINS {9, 8, 7, 6, 5}
    #define SD_CS     14
    #define SD_SPI_SPEED SPI_SCK_MHZ(4)
    #define USE_RAM_FOR_DITHERING false //SLOWER will use external SRAM.

#elif defined(ARDUINO_AVR_LEONARDO) || defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega16U4__)

    #define CUR_BOARD "LEONARDO"
    #define EPD_PINS {9, 8, 7, 6, 5}
    #define SD_CS     4
    #define SD_SPI_SPEED SPI_SCK_MHZ(4)
    #define USE_RAM_FOR_DITHERING false //SLOWER will use external SRAM.

#elif defined(ARDUINO_SAM_DUE) || defined(SAM3X8E)

    #define CUR_BOARD "DUE"
    #define EPD_PINS {9, 8, 7, 6, 5}
    #define SD_CS     4
    #define SD_SPI_SPEED SPI_SCK_MHZ(20)
    #define USE_RAM_FOR_DITHERING true //FASTER

#endif

epd_pins_t epd_pins = EPD_PINS;

/**********************************************************************************************/
// DECLARE:
/**********************************************************************************************/

//The sd card:
SdFat sd_card;
FatFile file;

//400x300 images to use:
const char filenames[][15] = {
    "p_example3.bmp",
    "p_example2.bmp",
    "p_example1.bmp"
};

//The EPD Board Driver Object:
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

    //Initialize SD Card:
    if (!sd_card.begin(SD_CS, SD_SPI_SPEED)) {
        Serial.println("SD Card initialization Error!");
        sd_card.initErrorHalt(&Serial);
    } else {
        Serial.println("SD Card successfully initialized.");
    }

    //Print to console all file names on SD:
    //sd_card.ls(LS_R | LS_DATE | LS_SIZE);

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

    //A flag to force the dithering buffer (width * 2 * sizeof(int16_t)) to be allocated on RAM
    //The maximum size will be an array of 1600 bytes / 1.6 Kilobute - be carefull not tp crash small micros...
    BITMAP_DITHER_FILTER::FORCE_RAM_BUFFER = USE_RAM_FOR_DITHERING;

    // For debugging - check if we are using sram or not:
    if (board->is_using_sram()) {
        Serial.println("EPD Initialized - USING SRAM");
    } else {
        Serial.println("EPD Initialized - NOT USING SRAM");
    }
}

void loop() {
    
    //Loop through the images and dither them:
    for (uint8_t i = 0; i < 3; ++i) {
        
        //Parse the Bitmap:
        SIKTEC_EPD_BITMAP bitmap = SIKTEC_EPD_BITMAP(&sd_card, filenames[i]);

        if (bitmap.isValid()) {
            unsigned long start = millis();
            //Clear the drawing buffer:
            board->clearBuffer();
            //Draw the bitmap:
            #ifdef SIKTEC_BOARD_G4
                EPD_BITMAP_STATUS draw = bitmap.drawBitmap(BITMAP_FILTER::DITHER_GRAY4, 0, 0, board, 0, 0, 400, 300);
            #else
                EPD_BITMAP_STATUS draw = bitmap.drawBitmap(BITMAP_FILTER::DITHER_BW, 0, 0, board, 0, 0, 400, 300);
            #endif
            // Update display to show the result:
            board->display(true);
            //Finished message:
            if (draw == EPD_BITMAP_STATUS::DONE) {
                //How long it took:
                unsigned long end = millis();
                unsigned long delta = end - start;
                Serial.print("Finished dithering image + drawing => took ms: ");
                Serial.println(delta);
            } else {
                Serial.println("Can't draw this bitmap... Sorry :(");
            }
            //Wait before we draw the next one:
            delay(15000);
        } else {
            Serial.println("This bitmap is not supported or not in the SDcard... Sorry :(");
        }
    }
    //We are done halt forever:
    Serial.println("All Done! :)");
    while (1) { ; };
}
