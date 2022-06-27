
/******************************************************************************/
// Created by: SIKTEC.
// Release Version : 1.0.1
// Creation Date: 2022-04-12
// Copyright 2022, SIKTEC.
// 
/******************************************************************************/
/*****************************      NOTES       *******************************
 * This draws a cropped bitmap directly from The SDcard on the EPD.
 * The same picture will be tiled on the screen with the different filters applied.
 * Those filters gives you the control on how to map the colors of the bitmap to the actual
 * available colors of the specific EPD.
 * 
 * Filters:
 *     - BITMAP_FILTER::BW              =>   Binary image will convert the image to raw Black & white.
 *     - BITMAP_FILTER::BWR             =>   Binary image will convert the image to Black & white & Red.
 *                                           Red will be drawn for reddish tone pixels.
 *     - BITMAP_FILTER::QUANTIZE        =>   The image will be quantized to all available colors of the EPD.
 *     - BITMAP_FILTER::GRAY4           =>   The image will be converted to a 4 color (2 grays).
 *     - BITMAP_FILTER::DITHER_BW       =>   The image will be dithered with 2 colors (Black & White).
 *     - BITMAP_FILTER::DITHER_GRAY4    =>   The image will be dithered with 4 colors (Black & White & 2 Grays).
 *     * For more information about dithering check the repo and the Dither example.
 * 
 * Filters can be automatically applied or "manually" adjusted check the repo readme for more information.
 * 
 * FOR THIS EXAMPLE TO WORK:
 * 1. uncomment the correct EPD module you are using.
 * 2. copy images (p_example1.bmp) to an SD card and plug it in. 
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
    #define USE_RAM_FOR_DITHERING true  //FASTER

#elif defined(ARDUINO_AVR_MEGA) || defined(AVR_MEGA2560) || defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)

    #define CUR_BOARD "MEGA"
    #define EPD_PINS {8, 9, 10, 11, 12}
    #define SD_CS     14
    #define SD_SPI_SPEED SPI_SCK_MHZ(20)
    #define USE_RAM_FOR_DITHERING true  //FASTER

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
    #define USE_RAM_FOR_DITHERING true  //FASTER

#endif

epd_pins_t epd_pins = EPD_PINS;

/**********************************************************************************************/
// DECLARE:
/**********************************************************************************************/

//The sd card:
SdFat sd_card;
FatFile file;
const char filename[] = "p_example1.bmp";

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

    BITMAP_DITHER_FILTER::FORCE_RAM_BUFFER = USE_RAM_FOR_DITHERING;
    
    // For debugging - check if we are using sram or not:
    if (board->is_using_sram()) {
        Serial.println("EPD Initialized - USING SRAM");
    } else {
        Serial.println("EPD Initialized - NOT USING SRAM");
    }

    //Parse the Bitmap:
    SIKTEC_EPD_BITMAP bitmap = SIKTEC_EPD_BITMAP(&sd_card, filename);
    if (bitmap.isValid()) {
        unsigned long start = millis();

        //Clear the drawing buffer:
        board->clearBuffer();

        //Draw filters examples:
        EPD_BITMAP_STATUS drawBW            = bitmap.drawBitmap(BITMAP_FILTER::BW, 0, 0, board, 50, 60, 130, 147);
        EPD_BITMAP_STATUS drawBWR           = bitmap.drawBitmap(BITMAP_FILTER::BWR, 133, 0, board, 50, 60, 130, 147);
        EPD_BITMAP_STATUS drawQUANT         = bitmap.drawBitmap(BITMAP_FILTER::QUANTIZE, 266, 0, board, 50, 60, 130, 147);
        EPD_BITMAP_STATUS drawGRAY4         = bitmap.drawBitmap(BITMAP_FILTER::GRAY4, 0, 150, board, 50, 60, 130, 147);
        EPD_BITMAP_STATUS drawDITHERBW      = bitmap.drawBitmap(BITMAP_FILTER::DITHER_BW, 133, 150, board, 50, 60, 130, 147);
        EPD_BITMAP_STATUS drawDITHERGARY4   = bitmap.drawBitmap(BITMAP_FILTER::DITHER_GRAY4, 266, 150, board, 50, 60, 130, 147);

        // Update display to show the result:
        board->display(true);

        //Finished message:
        unsigned long end = millis();
        unsigned long delta = end - start;
        Serial.print("Finished drawing images filtered => took ms: ");
        Serial.println(delta);
    } else {
        Serial.println("This bitmap is not supported or not in the SDcard... Sorry :(");
    }

    // ADJUSTING FILTERS INSTED OF AUTO - EXAMPLES:
    /*  
        // BLACK & WHITE:
        uint16_t colormap[3][4] = {
            { 0,     0,      0,  EPD_BLACK },
            { 31,    63,     31, EPD_WHITE },
            { 31,    0,      0,  EPD_RED   }
        };
        BitmapFilter_BW BW_filter(40); // pass the desired threshold 0 - 200~ 
        BW_filter.setColorMap(colormap, 3);
        EPD_BITMAP_STATUS drawBW = bitmap.drawBitmap(&BW_filter, 0, 0, board, 50, 60, 130, 147);

        // BLACK & WHITE:
        BitmapFilter_BWR BWR_filter(2, 45, 10); // (Red index in color map), threshold 0 - 200~, reddish 0 - 200~
        BWR_filter.setColorMap(colormap, 3);
        EPD_BITMAP_STATUS drawBWR = bitmap.drawBitmap(&BWR_filter, 133, 0, board, 50, 60, 130, 147);

        // QUANTIZE:
        BitmapFilter_QUANT QUANT_filter;
        QUANT_filter.setColorMap(colormap, 3);
        EPD_BITMAP_STATUS drawQUANT = bitmap.drawBitmap(&QUANT_filter, 266, 0, board, 50, 60, 130, 147);

        // GRAY4:
        uint16_t colormapGrays[5][4] = {
            {0,     0,      0,      EPD_BLACK   },
            {31,    63,     31,     EPD_WHITE   },
            {11,    22,     11,     EPD_DARK    },
            {20,    40,     20,     EPD_LIGHT   }
        };
        BitmapFilter_GRAY4 GRAY4_filter(40); // pass the desired threshold 0 - 200~ 
        GRAY4_filter.setColorMap(colormapGrays, 4);
        EPD_BITMAP_STATUS drawGRAY4 = bitmap.drawBitmap(&GRAY4_filter, 0, 150, board, 50, 60, 130, 147);

        // DITHER BW:
        BitmapFilter_DITHER_BW DITHER_BW_filter(EPD_BLACK, EPD_WHITE, 1.1); // color black, color white, threshold 0.00 - 2.00
        DITHER_BW_filter.setWeightVector(DITHER_WEIGHTS_VECTOR_FLOYD); 
        // weights vector:
        //    - DITHER_WEIGHTS_VECTOR_FLOYD     7, 3, 5, 1
        //    - DITHER_WEIGHTS_VECTOR_HORIZON   5, 3, 5, 5
        //    - DITHER_WEIGHTS_VECTOR_BALANCED  4, 4, 4, 4
        //       * you can use your own - sum of the 4 must be 16.
        EPD_BITMAP_STATUS drawDITHER_BW = bitmap.drawBitmapDithered(&DITHER_BW_filter, 133, 150, board, 50, 60, 130, 147);

        // DITHER GRAY4:
        uint16_t colormap4gray[5][4] = {
            {0,     0,      0,      EPD_BLACK   },
            {15,    15,     15,     EPD_RED     },
            {15,    15,     15,     EPD_DARK    },
            {25,    25,     25,     EPD_LIGHT   },
            {40,    40,     40,     EPD_WHITE   }
        };
        BitmapFilter_DITHER_GRAY4 DITHER_GRAY4_filter(0.85); // threshold 0.00 - 2.00
        DITHER_GRAY4_filter.setWeightVector(DITHER_WEIGHTS_VECTOR_FLOYD); 
        DITHER_GRAY4_filter.setColorMap(colormap4gray, 5);
        EPD_BITMAP_STATUS drawDITHER_GRAY4 = bitmap.drawBitmap(&DITHER_GRAY4_filter, 266, 150, board, 50, 60, 130, 147);
    */
}

void loop() {
    //We are done halt forever:
    Serial.println("All Done! :)");
    while (1) { ; };
}
