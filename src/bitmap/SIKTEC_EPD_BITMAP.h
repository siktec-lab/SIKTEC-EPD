/******************************************************************************/
// Created by: SIKTEC.
// Release Version : 1.0.2
// Creation Date: 2022-04-12
// Copyright 2022, SIKTEC. 
/******************************************************************************/
/*****************************      NOTES       *******************************
 * Technical BMP sources:
 *  - https://upload.wikimedia.org/wikipedia/commons/7/75/BMPfileFormat.svg 
 *  - https://www.digicamsoft.com/bmp/bmp.html
 *  - https://en.wikipedia.org/wiki/BMP_file_format
 * About Dithering:
 *  - https://tannerhelland.com/2012/12/28/dithering-eleven-algorithms-source-code.html
 *  - https://mncaudill.github.io/3bitdither/
 * Dither Image tools:
 *  - https://legacy.imagemagick.org/Usage/quantize/#dither_error
 * Supported BMP Formats:
 *  1. Windows BMP 1-32 bpp, compressions 0 - 3.
 *  2. OS2 BMP 1-32 bpp, compressions 0 - 3. 
 *  3. Headers - 12 - 124 bytes variants,
 *  4. GIMP Bitmaps.
 *  5. Illustrator Bitmaps.
*******************************************************************************/
/*****************************      Changelog       ****************************
1.0.1:
    -> initial release.
    -> Filters and Dithering procedure.
    -> Multiple formats supported.

Future:
    -> //TODO: implement reversed array bitmaps.
    -> //TODO: implement dithering with 1 - 8 bpp bitmaps.
    -> //TODO: create bitmap procedure. 
*******************************************************************************/


/**  @file SIKTEC_EPD_BITMAP.h */
#pragma once 

/**********************************************************************************************/
// LIB INCLUDES:
/**********************************************************************************************/
#include <Arduino.h>
#include <SPI.h>
#include <SdFat.h>
#include <SIKTEC_EPD.h>
#include "SIKTEC_BITMAP_FILTERS.h"

#ifndef SIKTEC_EPD_DEBUG_BITMAP 
    #define SIKTEC_EPD_DEBUG_BITMAP 0
#endif

#ifndef SIKTEC_EPD_DEBUG_BITMAP_PIXELS
    #define SIKTEC_EPD_DEBUG_BITMAP_PIXELS 0
#endif

#ifndef SIKTEC_EPD_DEBUG_BITMAP_DITHER
    #define SIKTEC_EPD_DEBUG_BITMAP_DITHER 0
#endif

#ifndef SPI_SCK_MHZ
    #define SPI_SCK_MHZ(speedMhz) SPISettings(1000000UL * speedMhz, MSBFIRST, SPI_MODE0) //get the speed in MHz
#endif
#ifndef SPI_SCK_KHZ
    #define SPI_SCK_KHZ(speedKhz) SPISettings(1000UL * speedKhz, MSBFIRST, SPI_MODE0) //get the speed in KHz
#endif

#define BITMAP_TYPE_BM 0x4D42 // 0x4D42 ASCII 'BM' is the Windows BMP signature We support.
#define BITMAP_FILEHEADER_SIZE 14


namespace SIKtec {

/**
 * @brief different header types and there sizes in bytes
 */
enum BMP_VARIANT {
    NOT_SUPPORTED           = 0,
    BITMAPCOREHEADER_12     = 12,
    OS22XBITMAPHEADER_16    = 16,
    BITMAPINFOHEADER_40     = 40,
    BITMAPINFOHEADE_ILLU_56 = 56, // Wild header - only seen in illustrator 16bpp 565
    OS22XBITMAPHEADER_64    = 64,
    BITMAPV4HEADER_108      = 108, 
    BITMAPV5HEADER_124      = 124  
};

/**
 * @brief BITMAP status flag returned from various methods 
 */
enum EPD_BITMAP_STATUS {
    FILE_NOT_FOUND,
    VALID,
    DONE,
    UNSUPPORTED_BMP_TYPE,
    UNSUPPORTED_BMP_VARIANT,
    ERROR_READ_FILE,
    ERROR_FILE_SIZE,
    COMPRESSION_NOT_SUPPORTED,
    NOT_IMPLEMENTED
};

/**
 * @brief Types of builtin filters implemented by default 
 */
enum BITMAP_FILTER {
    GRAY4,
    BWR,
    BW,
    QUANTIZE,
    DITHER_BW,
    DITHER_GRAY4,
    NONE
};

/**
 * @brief the file header layout shared among all types. 
 */
#pragma pack(push, 1)
typedef struct BMPFileHeader {
    uint16_t  type           = 0;
    uint32_t  size           = 0;
    uint32_t  reserved       = 0;
    uint32_t  array_start    = 0;
} bmp_file_header_t;
#pragma pack(pop)

/**
 * @brief the info header for modern 12 bytes+ headers. 
 */
#pragma pack(push, 1)
typedef struct BMPInfoHeader {
    //12
    uint32_t  header_size      = 0;
    int32_t   width            = 0;
    int32_t   height           = 0;
    uint16_t  color_planes     = 0;
    uint16_t  bpp              = 0; //Bit Per Pixel => 1, 4, 8, 16, 24, 32
    //40
    uint32_t  compression      = 0;
    uint32_t  raw_size         = 0;
    int32_t   horizontal_res   = 0;
    int32_t   vertical_res     = 0;
    uint32_t  colors           = 0;
    uint32_t  important_colors = 0;
    //64
    uint16_t  res_units        = 0;
    uint16_t  ignore_pad       = 0;
    uint16_t  fill_direction   = 0;  // 0 -> lower-left corner. Bits fill from left-to-right, then bottom-to-top
    uint16_t  halftoning_algo  = 0;  // 0 -> none, 1 -> Error Diffusion, 2 -> PANDA, 3 -> Super Circle. 
    uint32_t  halftoning_par1  = 0;  
    uint32_t  halftoning_par2  = 0;  
    uint32_t  color_encoding   = 0;  // 0 -> RGB 
    uint32_t  ignore_ident     = 0;
    //+
    //FOR GIMP support Photoshop and Illustrator
} bmp_info_header_t;
#pragma pack(pop)

/**
 * @brief A struct that wraps the bitmap minimal required info. 
 */
typedef struct BMPDefinition {
    EPD_BITMAP_STATUS   status  = EPD_BITMAP_STATUS::FILE_NOT_FOUND; 
    BMP_VARIANT         variant = BMP_VARIANT::NOT_SUPPORTED; 
    bool                flip = true; // BMP is stored bottom-to-top
    bmp_file_header_t   file_header;
    bmp_info_header_t   info_header;
    uint32_t            palette_size = 0;
    colorBits_t         *palette = nullptr; //BMP format on SD is BGRA this will flip and store ARGB888 or RGB565
} bmp_def_t;


/**
 * @brief A struct that defines how to read the specific loaded bmp. 
 */
typedef struct BMPReadDefinition {
    uint32_t row_bit_size          = 0;
    uint32_t read_width             = 0; 
    uint32_t read_height            = 0; 
    uint32_t start_row              = 0; 
    uint32_t start_col              = 0; 
    uint32_t start_row_address      = 0; 
    uint32_t pixels_per_iteration   = 1;
    uint32_t column_offset_bytes    = 0;
    uint32_t column_skip_bytes      = 0;
} bmp_read_definition_t;

/**
 * @brief A BMP sprite definition struct. 
 */
typedef struct BMPSprite {
    uint16_t columns = 0; // number of expected columns in sprite
    uint16_t rows    = 0; // number of expected rows in sprite
    uint32_t width   = 0; // single sprite entry width  -> auto calculated when defining
    uint32_t height  = 0; // single sprite entry height -> auto calculated when defining
} bmp_sprite_t;


/** @brief the kernel function pointer that can be attached as a filter to the drawing methods */
typedef colorBits_t (*translate_color)(const uint8_t R, const uint8_t G, const uint8_t B);

//------------------------------------------------------------------------//
// SIKTEC_EPD_BITMAP
//------------------------------------------------------------------------//

/**
 * @brief  The SIKTEC_EPD_BITMAP Class handles all the SD reading / parsing / translating and drawing to the EPD.
*/
class SIKTEC_EPD_BITMAP {

public:

    char * name; /**< the BITMAP file name - dynamicly allocated and deleted. */

    bmp_def_t definition; /**< the BITMAP minimal definition struct. */

    bmp_sprite_t sprite; /**< the optional sprite definition struct. */

    SdFat * sd; /**< the optional sprite definition struct. */

    FatFile file; /**< the bitmap file handler instance */

    bool exists = false; /**< flag that indicates whether a file exists and fully loaded or not */

    /** @brief Construct a new siktec epd bitmap::siktec epd bitmap object */
    SIKTEC_EPD_BITMAP(SdFat * sd, FatFile &_file, uint16_t filename_size = 50);

    /** @brief Construct a new siktec epd bitmap::siktec epd bitmap object. */
    SIKTEC_EPD_BITMAP(SdFat * sd, const char * imageName);

    /** @brief Destroy the siktec epd bitmap::siktec epd bitmap object */
    ~SIKTEC_EPD_BITMAP();

    /** @brief True if the file is a bitmap and supported by the library */
    bool isValid();

    /** @brief Return the bitmap width as defined in the info header */
    int32_t width();
    
    /** @brief Return the bitmap height as defined in the info header */
    int32_t height();
    
    /** @brief return the bitmap status flag indicating the result of the bitmap parse attempt. */
    EPD_BITMAP_STATUS bitmapStatus();
    
    /** @brief Define the loaded bitmap as a sprite. */
    void defineBitmapSprite(uint16_t columns, uint16_t rows);

    /** @brief Draws the bitmap sprite index on the given EPD using a predefined filter / color kernel. */
    EPD_BITMAP_STATUS drawBitmapSprite(
        BITMAP_FILTER   filter,
        uint32_t        epd_x,
        uint32_t        epd_y,
        uint16_t        sprite_index,
        SIKTEC_EPD      *epd,
        bool            reloadDefinition = false 
    );
    EPD_BITMAP_STATUS drawBitmapSprite(
        BITMAP_FILTER_IMPLEMENTATION *filter,
        uint32_t        epd_x,
        uint32_t        epd_y,
        uint16_t        sprite_index,
        SIKTEC_EPD      *epd,
        bool            reloadDefinition = false 
    );

    /** @brief Draws a bitmap to the given epd object with predefined filters. */
    EPD_BITMAP_STATUS drawBitmap(
        BITMAP_FILTER builtin_filters,
        uint32_t epd_x,
        uint32_t epd_y,
        SIKTEC_EPD *epd,
        uint32_t bmp_sc  = 0, // start column
        uint32_t bmp_sr  = 0, // start row
        uint32_t bmp_cw  = 0, // clip width 0 means fullwidth
        uint32_t bmp_ch  = 0, // clip height 0 means fullheight 
        bool reloadDefinition = false
    );

    /** @brief Draws the bitmap on the given EPD using a custom color kernel (filter) function. */
    EPD_BITMAP_STATUS drawBitmap(
        BITMAP_FILTER_IMPLEMENTATION *filter,
        uint32_t epd_x,
        uint32_t epd_y,
        SIKTEC_EPD *epd,
        uint32_t bmp_sc  = 0, // start column
        uint32_t bmp_sr  = 0, // start row
        uint32_t bmp_cw  = 0, // clip width 0 means fullwidth
        uint32_t bmp_ch  = 0, // clip height 0 means fullheight 
        bool reloadDefinition = false
    );
    
    /** @brief Draws the bitmap on the given EPD while applying a dithering algorithm. */
    EPD_BITMAP_STATUS drawBitmapDithered(
        BITMAP_DITHER_FILTER *filter,
        uint32_t epd_x,
        uint32_t epd_y,
        SIKTEC_EPD *epd,
        uint32_t bmp_sc  = 0, // start column
        uint32_t bmp_sr  = 0, // start row
        uint32_t bmp_cw  = 0, // clip width 0 means fullwidth
        uint32_t bmp_ch  = 0, // clip height 0 means fullheight 
        bool reloadDefinition = false
    );

    /** @brief Return a bitmap definition parsed from the headers */
    bmp_def_t getBitmapDefinition(bool createPalette = false);

    #if SIKTEC_EPD_DEBUG_BITMAP
        /** @brief prints to serial the parsed bitmap definition struct */
        void debug_bitmapDefinition();
    #endif
    #if SIKTEC_EPD_DEBUG_BITMAP_DITHER
        /** @brief  dumps to serial the allocated dithering buffer state */
        void printDitherBuffer(const uint16_t buffer, int16_t *ram_buffer,  SIKTEC_EPD *epd, const uint16_t width);
    #endif
    
private:
    
    /** @brief will return all needed values to define how and where to read the bitmap array based on the given coordinates **/
    bmp_read_definition_t prepareBitmapReadDefinition(const uint32_t bmp_sr, const uint32_t bmp_sc, const uint32_t loadWidth, const uint32_t loadHeight);

    /** @brief return a single pixel color directly from the bitmap **/
    colorBits_t getBitmapPixel(const bmp_read_definition_t bitmap_read, const int16_t x, const int16_t y, BITMAP_FILTER_IMPLEMENTATION *filter);

    /** @brief Will parse and traverse the pixel array and draw them on the given EPD. */
    void proccessUncompressed(
        uint32_t epd_x, uint32_t epd_y, 
        const bmp_read_definition_t bitmap_read,
        SIKTEC_EPD *epd,
        BITMAP_FILTER_IMPLEMENTATION *filter = nullptr
    );

    /** @brief Will apply the Filter / Kernel to a parsed pixel and return the color format to use. */
    colorBits_t pixelColorProccess(uint16_t rgb565, BITMAP_FILTER_IMPLEMENTATION *filter = nullptr);

    /** @brief Will apply the Filter / Kernel to a parsed pixel and return the color format to use. */
    colorBits_t pixelColorProccess(uint32_t rgb888, BITMAP_FILTER_IMPLEMENTATION *filter = nullptr);

    /** @brief Return the bitmap type (Header Format) - Based on the header size. */
    BMP_VARIANT bitmapVariant();

    /** @brief Converts a 32bit color (888) to 16bit color (565). */
    uint16_t color32to16(uint8_t R8, uint8_t G8, uint8_t B8);

    /** @brief Converts a 32bit color (A888) to 16bit color (565). */
    uint16_t color32to16(uint32_t C32);

    /** @brief Converts a 16bit color (565) to 32bit color (0888). */
    uint32_t color16to32(uint8_t R5, uint8_t G6, uint8_t B5);

    /** @brief Converts a 16bit color (565) to 32bit color (888). */
    uint32_t color16to32(uint16_t C16);
    
    /** @brief Rewind file pointer to seek pos 0. */
    void rewind();

    /** @brief Set a seek position (cursor) of the current open file. */
    bool seekSet(uint32_t pos);

    /** @brief Reads a byte from current seek position - Increments the cursor. */
    uint8_t read8();

    /** @brief Reads a 16bit number (2 bytes) from current seek position - Increments the cursor. */
    uint16_t read16();

    /** @brief Reads a 24bit number (3 bytes) from current seek position - Increments the cursor. */
    uint32_t read24();
    
    /** @brief Reads 24bit numbers (3 bytes each) from current seek position into an array - Increments the cursor. */
    void read24(uint32_t *output, const size_t length);

    /** @brief Reads a 32bit number (4 bytes) from current seek position - Increments the cursor. */
    uint32_t read32();

    /** @brief Reads 32bit numbers (4 bytes) from current seek position into an array - Increments the cursor. */
    void read32(uint32_t *output, const size_t length);

};

}