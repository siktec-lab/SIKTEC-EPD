/******************************************************************************/
// Created by: SIKTEC.
// Release Version : 1.0.5
// Creation Date: 2022-04-12
// Copyright 2022, SIKTEC.
/******************************************************************************/

/**  @file SIKTEC_EPD_BITMAP.cpp */

#include "SIKTEC_EPD_BITMAP.h"

#ifndef PRINT_DEBUG_BUFFER
#define PRINT_DEBUG_BUFFER(__template, ...) \
    sprintf(debug_message, __template, __VA_ARGS__); \
    Serial.print(debug_message)
#endif

namespace SIKtec {

/**
 * @brief extern definition from SIKTEC_EPD for debugging.
 */
extern char debug_message[];
extern const int debug_message_len;

/**
 * @brief Construct a new siktec epd bitmap::siktec epd bitmap object.
 * 
 * @param sd                An SdFat initialized initialized pointer.
 * @param _file             a FatFile Reference that points to the bitmap.
 * @param filename_size     Default 50 - The file name size of the file.
 */
SIKTEC_EPD_BITMAP::SIKTEC_EPD_BITMAP(SdFat * sd, FatFile &_file, uint16_t filename_size) {
    this->sd = sd;
    this->name = new char[filename_size];
    _file.getName(this->name, filename_size);
    this->exists = _file.isFile();
    this->file = _file;
    if (this->exists) {
        this->definition = this->getBitmapDefinition(true);
        #if SIKTEC_EPD_DEBUG_BITMAP
            this->debug_bitmapDefinition();
        #endif
    } else {
        #if SIKTEC_EPD_DEBUG_BITMAP
            PRINT_DEBUG_BUFFER("File '%s' is not bmp type.", this->name);
        #endif
    }
}

/**
 * @brief Construct a new siktec epd bitmap::siktec epd bitmap object.
 * 
 * @param sd                An SdFat initialized initialized pointer.
 * @param imageName         char * file name to load from SD.
 */
SIKTEC_EPD_BITMAP::SIKTEC_EPD_BITMAP(SdFat * sd, const char * imageName) {
    this->sd = sd;
    size_t nameSize = strlen(imageName) + 1;
    this->name = new char[nameSize];
    strcpy(this->name, imageName);
    this->exists = this->sd->exists(imageName);
    if (this->exists) {
        this->definition = this->getBitmapDefinition(true);
        #if SIKTEC_EPD_DEBUG_BITMAP
            this->debug_bitmapDefinition();
        #endif
    } else {
        #if SIKTEC_EPD_DEBUG_BITMAP
            PRINT_DEBUG_BUFFER("BITMAP '%s' Does not exist.", this->name);
        #endif
    }
}

/**
 * @brief Destroy the siktec epd bitmap::siktec epd bitmap object
 * Will delete palette if its allocated and the file name copy.
 */
SIKTEC_EPD_BITMAP::~SIKTEC_EPD_BITMAP() {
    //Release name and allocated palette array:
    delete[] this->name;
    delete[] this->definition.palette;
}

/**
 * @brief True if the file is a bitmap and supported by the library.
 * 
 * @return bool  
 */
bool SIKTEC_EPD_BITMAP::isValid() {
    return this->bitmapStatus() == EPD_BITMAP_STATUS::VALID;
}

/**
 * @brief Return the bitmap width as defined in the info header.
 * 
 * @return int32_t the bitmap width.
 */
int32_t SIKTEC_EPD_BITMAP::width() {
    return this->definition.info_header.width;
}

/**
 * @brief Return the bitmap height as defined in the info header.
 * 
 * @return int32_t the bitmap height.
 */
int32_t SIKTEC_EPD_BITMAP::height() {
    return this->definition.info_header.height;
}

/**
 * @brief Return the bitmap status flag indicating the result of the bitmap parse attempt.
 * 
 * @return enum EPD_BITMAP_STATUS
 */
EPD_BITMAP_STATUS SIKTEC_EPD_BITMAP::bitmapStatus() {
    return this->definition.status;
}

/**
 * @brief Define the loaded bitmap as a sprite.
 * 
 * @param columns   uint16_t number of sprite columns
 * @param rows      uint16_t number of sprite rows.
 * @return void
 */
void SIKTEC_EPD_BITMAP::defineBitmapSprite(uint16_t columns, uint16_t rows) {
    this->sprite.columns = columns;
    this->sprite.rows    = rows;
    this->sprite.width   =  this->width() / (uint32_t)columns;
    this->sprite.height  =  this->height() / (uint32_t)rows;
}

/**
 * @brief Draws the bitmap sprite index on the given EPD using a predefined filter / color kernel.
 * 
 * @param filter        the filter to be used enum BITMAP_FILTER
 * @param epd_x         uint32_t the top-left X position on the EPD.
 * @param epd_y         uint32_t the top-left Y position on the EPD.
 * @param sprite_index  uint16_t the index of the sprite position.
 * @param epd           SIKTEC_EPD * the pointer to the epd to draw on.
 * @param reloadDefinition bool default False - whether to reload definition or not.
 * @return EPD_BITMAP_STATUS 
 */
EPD_BITMAP_STATUS SIKTEC_EPD_BITMAP::drawBitmapSprite(
    BITMAP_FILTER   filter,
    uint32_t        epd_x,
    uint32_t        epd_y,
    uint16_t        sprite_index,
    SIKTEC_EPD      *epd,
    bool            reloadDefinition
) {
    uint16_t     row  = sprite_index / this->sprite.columns;
    uint16_t     col  =  sprite_index - (row * this->sprite.columns);
    return this->drawBitmap(
        filter,
        epd_x, epd_y, 
        epd,
        col * this->sprite.width,
        row * this->sprite.height,
        this->sprite.width,
        this->sprite.height,
        reloadDefinition
    );
}

/**
 * @brief Draws the bitmap sprite index on the given EPD using a predefined filter / color kernel.
 * 
 * @param filter        BITMAP_FILTER_IMPLEMENTATION - the filter object pointer.
 * @param epd_x         uint32_t the top-left X position on the EPD.
 * @param epd_y         uint32_t the top-left Y position on the EPD.
 * @param sprite_index  uint16_t the index of the sprite position.
 * @param epd           SIKTEC_EPD * the pointer to the epd to draw on.
 * @param reloadDefinition bool default False - whether to reload definition or not.
 * @return EPD_BITMAP_STATUS - consider DONE as successfull.
 */
EPD_BITMAP_STATUS SIKTEC_EPD_BITMAP::drawBitmapSprite(
    BITMAP_FILTER_IMPLEMENTATION *filter,
    uint32_t        epd_x,
    uint32_t        epd_y,
    uint16_t        sprite_index,
    SIKTEC_EPD      *epd,
    bool            reloadDefinition
) {
    uint16_t     row  = sprite_index / this->sprite.columns;
    uint16_t     col  =  sprite_index - (row * this->sprite.columns);
    return this->drawBitmap(
        filter,
        epd_x, epd_y, 
        epd,
        col * this->sprite.width,
        row * this->sprite.height,
        this->sprite.width,
        this->sprite.height,
        reloadDefinition
    );
}

/**
 * @brief draws a bitmap directly to the epd applying a filter.
 * 
 * @param builtin_filters   - the builtin filter to apply - BITMAP_FILTER enum.
 * @param epd_x             - uint32_t the top-left X position on the EPD.
 * @param epd_y             - uint32_t the top-left Y position on the EPD.
 * @param epd               - SIKTEC_EPD * the pointer to the epd to draw on.
 * @param bmp_sc            - bitmap start column (from left)
 * @param bmp_sr            - bitmap start row (from top)
 * @param bmp_cw            - bitmap width to draw  - 0 for full width.
 * @param bmp_ch            - bitmap height to draw  - 0 for full height.
 * @param reloadDefinition  - bool default False - whether to reload definition or not.
 * @return EPD_BITMAP_STATUS - consider DONE as successfull.
 */
EPD_BITMAP_STATUS SIKTEC_EPD_BITMAP::drawBitmap(
    BITMAP_FILTER   builtin_filters,
    uint32_t        epd_x,
    uint32_t        epd_y,
    SIKTEC_EPD      *epd,
    uint32_t        bmp_sc, // start column
    uint32_t        bmp_sr, // start row
    uint32_t        bmp_cw, // clip width 0 means fullwidth
    uint32_t        bmp_ch, // clip height 0 means fullheight 
    bool            reloadDefinition
) {

    //If we want a dither filter BW:
    if (builtin_filters == BITMAP_FILTER::DITHER_BW) {
        BitmapFilter_DITHER_BW DITHERBW_filter(EPD_BLACK, EPD_WHITE, 1.15);
        return this->drawBitmapDithered(&DITHERBW_filter, epd_x, epd_y, epd, bmp_sc, bmp_sr, bmp_cw, bmp_ch, reloadDefinition);
    }

    //If we want a dither filter GRAY4:
    if (builtin_filters == BITMAP_FILTER::DITHER_GRAY4) {
        #ifdef BITMAP_COLOR_RESULT_888 
        uint16_t colormap_dither[5][4] = { // Use grey levels 0 - 255
            { 0,     0,      0,      EPD_BLACK   },
            { 90,    90,     90,     EPD_RED     },
            { 90,    90,     90,     EPD_DARK    },
            { 180,   180,    180,     EPD_LIGHT   },
            { 255,   255,    255,     EPD_WHITE   }
        };
    #else 
        uint16_t colormap_dither[5][4] = { // Use grey levels 0 - 41
            {0,     0,      0,      EPD_BLACK   },
            {15,    15,     15,     EPD_RED     },
            {15,    15,     15,     EPD_DARK    },
            {25,    25,     25,     EPD_LIGHT   },
            {40,    40,     40,     EPD_WHITE   }
        };
    #endif 
        BitmapFilter_DITHER_GRAY4 DITHERGRAY4_filter(1.15);
        DITHERGRAY4_filter.setColorMap(colormap_dither, 5);
        return this->drawBitmapDithered(&DITHERGRAY4_filter, epd_x, epd_y, epd, bmp_sc, bmp_sr, bmp_cw, bmp_ch, reloadDefinition);
    }


    //Builtin filters:
    #ifdef BITMAP_COLOR_RESULT_888 
        uint16_t colormap[5][4] = {
            {0,     0,      0,      EPD_BLACK   },
            {255,   255,    255,    EPD_WHITE   },
            {170,   170,    170,    EPD_DARK    },
            {85,    85,     85,     EPD_LIGHT   },
            {255,   0,      0,      EPD_RED     }
        };
    #else 
        uint16_t colormap[5][4] = {
            {0,     0,      0,      EPD_BLACK   },
            {31,    63,     31,     EPD_WHITE   },
            {11,    22,     11,     EPD_DARK    },
            {20,    40,     20,     EPD_LIGHT   },
            {31,    0,      0,      EPD_RED     }
        };
    #endif 

    BITMAP_FILTER_IMPLEMENTATION *filter;

    switch (builtin_filters) {
        case BITMAP_FILTER::GRAY4: {
            filter = new BitmapFilter_GRAY4(30);
            filter->setColorMap(colormap, 4);
        } break;
        case BITMAP_FILTER::BWR: {
            filter = new BitmapFilter_BWR(4, 45, 10);
            filter->setColorMap(colormap, 5);
        } break;
        case BITMAP_FILTER::BW: {
            filter = new BitmapFilter_BW(40);
            filter->setColorMap(colormap, 2);
        } break;
        case BITMAP_FILTER::QUANTIZE: {
            filter = new BitmapFilter_QUANT();
            filter->setColorMap(colormap, 5);
        } break;
        default: {
            filter = new BitmapFilter_GRAY4(30);
            filter->setColorMap(colormap, 4);
        }
    }

    EPD_BITMAP_STATUS result = this->drawBitmap(
        filter,
        epd_x, epd_y, 
        epd, 
        bmp_sc, // start column
        bmp_sr, // start row
        bmp_cw, // clip width 0 means fullwidth
        bmp_ch, // clip height 0 means fullheight 
        reloadDefinition
    );

    //Delete the filter....
    delete filter;

    //Return
    return result;
}


/**
 * @brief Draws the bitmap on the given EPD using a custom color kernel (filter) function.
 * 
 * @param filter    BITMAP_FILTER_IMPLEMENTATION - the filter object pointer.
 * @param epd_x     uint32_t the top-left X position on the EPD.
 * @param epd_y     uint32_t the top-left Y position on the EPD.
 * @param epd       SIKTEC_EPD * the pointer to the epd to draw on.
 * @param bmp_sc    uint32_t the bitmap starting point X / Column (From Left).
 * @param bmp_sr    uint32_t the bitmap starting point Y / Row (From Top).
 * @param bmp_cw    uint32_t The width to draw (clip width) - 0 for full width.
 * @param bmp_ch    uint32_t The width to draw (clip height) - 0 for full height.
 * @param reloadDefinition bool default False - whether to reload definition or not.
 * @return EPD_BITMAP_STATUS  - consider DONE as successfull.
 */
EPD_BITMAP_STATUS SIKTEC_EPD_BITMAP::drawBitmap(
    BITMAP_FILTER_IMPLEMENTATION *filter,
    uint32_t epd_x,
    uint32_t epd_y,
    SIKTEC_EPD *epd,
    uint32_t bmp_sc, // start column
    uint32_t bmp_sr, // start row
    uint32_t bmp_cw, // clip width 0 means fullwidth
    uint32_t bmp_ch, // clip height 0 means fullheight
    bool reloadDefinition
) {

    //Reload the header? only if changes could have been done....
    if (reloadDefinition) {
        this->definition = this->getBitmapDefinition(true);
    }

    // Make sure we are ready to go:
    if (!this->isValid()) {
        return this->bitmapStatus();
    } 

    //Early exit if we dont need to do anything:
    if (epd && ((epd_x >= (uint32_t)epd->width()) || (epd_y >= (uint32_t)epd->height()))) 
        return EPD_BITMAP_STATUS::DONE;

    //Open file:
    //NOTE: Shlomi removed this - its not necessary and can cause problems.
    //this->sd->chvol(); // set this card to be the current active volume.
    if (!this->file.isOpen() && !this->file.open(this->name, O_RDONLY)) {
        this->definition.status = EPD_BITMAP_STATUS::ERROR_READ_FILE;
        return EPD_BITMAP_STATUS::ERROR_READ_FILE;
    }
    
    //Shrink clip if it goes out of bmp bounds:
    uint32_t bmpWidth = (bmp_cw && bmp_cw < (uint32_t)this->width()) ? bmp_cw : this->width();
    if (bmpWidth + bmp_sc > (uint32_t)this->width()) {
        bmpWidth -= bmpWidth + bmp_sc - this->width();
    }
    uint32_t bmpHeight  = bmp_ch && bmp_ch < (uint32_t)this->height() ? bmp_ch : this->height();
    if (bmpHeight + bmp_sr > (uint32_t)this->height()) {
        bmpHeight -= bmpHeight + bmp_sr - this->height();
    }

    uint32_t loadWidth  = bmpWidth;
    uint32_t loadHeight = bmpHeight; 
    
    //Avoid drawing outside of epd boundaries:
    if (loadWidth + epd_x > (unsigned)epd->width()) 
        loadWidth = epd->width() - epd_x;

    if (loadHeight + epd_y > (unsigned)epd->height()) 
        loadHeight = epd->height() - epd_y;

    //starting row and columns:
    uint32_t bmpStartRow = this->height() - bmp_sr - loadHeight;
    uint32_t bmpStartCol  = bmp_sc;

    //the read definition for the bitmap file:
    bmp_read_definition_t bmp_read = this->prepareBitmapReadDefinition(bmpStartRow, bmpStartCol, loadWidth, loadHeight);

    //FUTURE: add support for other compressions atleast RLE....
    if ( this->definition.info_header.compression == 0 ) {
        //Simple uncompressed bitmap:
        this->proccessUncompressed(
            epd_x, epd_y, bmp_read, epd, filter
        );
    } else if (   this->definition.info_header.compression == 3 
                && (this->definition.info_header.bpp == 16 || this->definition.info_header.bpp == 32)
    ) {
        // Observed in GIMP Bitmaps -> bpp 16, 32 is marked compressed....
        // Its a not:
        this->proccessUncompressed(
            epd_x, epd_y, bmp_read, epd, filter
        );
    } else {
        //Not supported:
        this->file.close();
        return EPD_BITMAP_STATUS::NOT_IMPLEMENTED; 
    }

    this->file.close();
    return EPD_BITMAP_STATUS::DONE;
}

/**
 * @brief Draws the bitmap on the given EPD using a dithering filter.
 * 
 * @param filter    BITMAP_DITHER_FILTER - the dither filter object pointer.
 * @param epd_x     uint32_t the top-left X position on the EPD.
 * @param epd_y     uint32_t the top-left Y position on the EPD.
 * @param epd       SIKTEC_EPD * the pointer to the epd to draw on.
 * @param bmp_sc    uint32_t the bitmap starting point X / Column (From Left).
 * @param bmp_sr    uint32_t the bitmap starting point Y / Row (From Top).
 * @param bmp_cw    uint32_t The width to draw (clip width) - 0 for full width.
 * @param bmp_ch    uint32_t The width to draw (clip height) - 0 for full height.
 * @param reloadDefinition bool default False - whether to reload definition or not.
 * @return EPD_BITMAP_STATUS   - consider DONE as successfull.
 */
EPD_BITMAP_STATUS SIKTEC_EPD_BITMAP::drawBitmapDithered(
    BITMAP_DITHER_FILTER *filter,
    uint32_t epd_x,
    uint32_t epd_y,
    SIKTEC_EPD *epd,
    uint32_t bmp_sc, // start column
    uint32_t bmp_sr, // start row
    uint32_t bmp_cw, // clip width 0 means fullwidth
    uint32_t bmp_ch, // clip height 0 means fullheight
    bool reloadDefinition
) {

    //Reload the header? only if changes could have been done....
    if (reloadDefinition) {
        this->definition = this->getBitmapDefinition(true);
    }

    // Make sure we are ready to go:
    if (!this->isValid()) {
        return this->bitmapStatus();
    } 

    //Early exit if we dont need to do anything:
    if (epd && ((epd_x >= (uint32_t)epd->width()) || (epd_y >= (uint32_t)epd->height()))) 
        return EPD_BITMAP_STATUS::DONE;

    //Open file:
    //NOTE: Shlomi removed this - its not necessary and can cause problems.
    //this->sd->chvol(); // set this card to be the current active volume.
    if (!this->file.isOpen() && !this->file.open(this->name, O_RDONLY)) {
        this->definition.status = EPD_BITMAP_STATUS::ERROR_READ_FILE;
        return EPD_BITMAP_STATUS::ERROR_READ_FILE;
    }
    
    //Shrink clip if it goes out of bmp bounds:
    uint32_t bmpWidth = (bmp_cw && bmp_cw < (uint32_t)this->width()) ? bmp_cw : this->width();
    if (bmpWidth + bmp_sc > (uint32_t)this->width()) {
        bmpWidth -= bmpWidth + bmp_sc - this->width();
    }
    uint32_t bmpHeight  = bmp_ch && bmp_ch < (uint32_t)this->height() ? bmp_ch : this->height();
    if (bmpHeight + bmp_sr > (uint32_t)this->height()) {
        bmpHeight -= bmpHeight + bmp_sr - this->height();
    }

    uint32_t loadWidth  = bmpWidth;
    uint32_t loadHeight = bmpHeight; 
    
    //Avoid drawing outside of epd boundaries:
    if (loadWidth + epd_x > (unsigned)epd->width()) 
        loadWidth = epd->width() - epd_x;

    if (loadHeight + epd_y > (unsigned)epd->height()) 
        loadHeight = epd->height() - epd_y;

    //starting row and columns:
    uint32_t bmpStartRow = this->height() - bmp_sr - loadHeight;
    uint32_t bmpStartCol  = bmp_sc;

    //Early exit if not supported:
    if (
        (this->definition.info_header.compression != 0 &&  this->definition.info_header.compression != 3)
        || (
            this->definition.info_header.compression == 3 
            && this->definition.info_header.bpp != 16
            && this->definition.info_header.bpp != 32
        )
    ) {
        //Not supported:
        this->file.close();
        return EPD_BITMAP_STATUS::NOT_IMPLEMENTED; 
    }

    bmp_read_definition_t bmp_read = this->prepareBitmapReadDefinition(bmpStartRow, bmpStartCol, loadWidth, loadHeight);
    
    //Allocate Buffer:
    uint16_t sram_buffer;
    int16_t *ram_buffer = nullptr;
    bool in_sram = BITMAP_DITHER_FILTER::FORCE_RAM_BUFFER ? false : epd->is_using_sram();
    if (in_sram) {
        in_sram = epd->is_using_sram();
        sram_buffer = epd->allocateSramArrayBuffer(
            (uint16_t)(loadWidth * 2), 
            sizeof(int16_t)
        );
    } else {
        in_sram = false;
        ram_buffer = new int16_t[loadWidth * 2];
    }

    //fill buffer:
    int16_t     bmpColorPixel;
    uint8_t     color_buf[2];
    for (uint16_t row = 0; row < 2; ++row) {
        for (uint16_t col = 0; col < loadWidth; ++col) {
            bmpColorPixel = (int16_t)this->getBitmapPixel(bmp_read, col, loadHeight - row - 1, filter);
            if (in_sram) {
                color_buf[0] = bmpColorPixel >> 8;
                color_buf[1] = bmpColorPixel & 0xFF;
                epd->setSramArrayBufferElement(sram_buffer, col + row * loadWidth, color_buf, 1);
            } else {
                ram_buffer[col + row * loadWidth] = bmpColorPixel;
            }
        }
    }
    #if SIKTEC_EPD_DEBUG_BITMAP_DITHER
        Serial.println("Initial Dither Buffer:");
        this->printDitherBuffer(sram_buffer, ram_buffer, epd, loadWidth);
    #endif

    //Dither and shift:
    uint32_t finalColumn = loadWidth - 1;
    uint32_t finalRow    = loadHeight - 1; 
    int16_t  dither_kernel[5];
    int16_t  epd_col      = (int16_t)epd_x;
    int16_t  epd_row      = (int16_t)epd_y;
    for (uint16_t row = 0; row < loadHeight; ++row) {
        for (uint16_t col = 0; col < loadWidth; ++col) {
            //Current pixel:
            if (in_sram) {
                epd->getSramArrayBufferElement(sram_buffer, col, color_buf, 1);
                dither_kernel[0] = (int16_t)((color_buf[0] << 8) | color_buf[1]);
            } else {
                dither_kernel[0] = ram_buffer[col];
            }
            //Right:
            if (col < finalColumn) {
                if (in_sram) {
                    epd->getSramArrayBufferElement(sram_buffer, col + 1, color_buf, 1);
                    dither_kernel[1] = (int16_t)((color_buf[0] << 8) | color_buf[1]);
                } else {
                    dither_kernel[1] = ram_buffer[col + 1];
                }
            } else {
                dither_kernel[1] = 0;
            }
            //Down Left:
            if (col > 0 && row < finalRow) {
                if (in_sram) {
                    epd->getSramArrayBufferElement(sram_buffer, col + loadWidth - 1, color_buf, 1);
                    dither_kernel[2] = (int16_t)((color_buf[0] << 8) | color_buf[1]);
                } else {
                    dither_kernel[2] = ram_buffer[col + loadWidth - 1];
                }
            } else {
                dither_kernel[2] = 0;
            }
            //Down:
            if (row < finalRow) {
                if (in_sram) {
                    epd->getSramArrayBufferElement(sram_buffer, col + loadWidth, color_buf, 1);
                    dither_kernel[3] = (int16_t)((color_buf[0] << 8) | color_buf[1]);
                } else {
                    dither_kernel[3] = ram_buffer[col + loadWidth];
                }
            } else {
                dither_kernel[3] = 0;
            }
            //Down Right:
            if (row < finalRow && col < finalColumn) {
                if (in_sram) {
                    epd->getSramArrayBufferElement(sram_buffer, col + loadWidth + 1, color_buf, 1);
                    dither_kernel[4] = (int16_t)((color_buf[0] << 8) | color_buf[1]);
                } else {
                    dither_kernel[4] = ram_buffer[col + loadWidth + 1];
                }
            } else {
                dither_kernel[4] = 0;
            }

            //Dither the pixels - distributes the error:
            filter->dither(dither_kernel);
            
            //save it back to buffer:
            if (in_sram) {
                color_buf[0] = dither_kernel[0] >> 8;
                color_buf[1] = dither_kernel[0] & 0xFF;
                epd->setSramArrayBufferElement(sram_buffer, col, color_buf, 1);
                if (col < finalColumn) {
                    color_buf[0] = dither_kernel[1] >> 8;
                    color_buf[1] = dither_kernel[1] & 0xFF;
                    epd->setSramArrayBufferElement(sram_buffer, col + 1, color_buf, 1);
                }
                if (col > 0 && row < finalRow) {
                    color_buf[0] = dither_kernel[2] >> 8;
                    color_buf[1] = dither_kernel[2] & 0xFF;
                    epd->setSramArrayBufferElement(sram_buffer, col + loadWidth - 1, color_buf, 1);
                }
                if (row < finalRow) {
                    color_buf[0] = dither_kernel[3] >> 8;
                    color_buf[1] = dither_kernel[3] & 0xFF;
                    epd->setSramArrayBufferElement(sram_buffer, col + loadWidth, color_buf, 1);
                }
                if (row < finalRow && col < finalColumn) {
                    color_buf[0] = dither_kernel[4] >> 8;
                    color_buf[1] = dither_kernel[4] & 0xFF;
                    epd->setSramArrayBufferElement(sram_buffer, col + loadWidth + 1, color_buf, 1);
                }
                // #if SIKTEC_EPD_DEBUG_BITMAP_DITHER
                //     PRINT_DEBUG_BUFFER("Dither Buffer Pixel: %d %d \n", col, row);
                //     this->printDitherBuffer(sram_buffer, ram_buffer, epd, loadWidth);
                // #endif
            } else {
                ram_buffer[col] = dither_kernel[0];
                if (col < finalColumn) {
                    ram_buffer[col + 1] = dither_kernel[1];
                }
                if (col > 0 && row < finalRow) {
                    ram_buffer[col + loadWidth - 1] = dither_kernel[2];
                }
                if (row < finalRow) {
                    ram_buffer[col + loadWidth] = dither_kernel[3];
                }
                if (row < finalRow && col < finalColumn) {
                    ram_buffer[col + loadWidth + 1] = dither_kernel[4];
                }
            }
        }
        #if SIKTEC_EPD_DEBUG_BITMAP_DITHER
            PRINT_DEBUG_BUFFER("After Dither Buffer Row: %d \n", row);
            this->printDitherBuffer(sram_buffer, ram_buffer, epd, loadWidth);
        #endif
        //draw raw and shift:
        for (uint16_t col = 0; col < loadWidth; ++col) {
            if (in_sram) {
                //Draw:
                epd->getSramArrayBufferElement(sram_buffer, col, color_buf, 1);
                epd->drawPixel(epd_col++, epd_row, (color_buf[0] << 8) | color_buf[1]);
                //Shift:
                epd->getSramArrayBufferElement(sram_buffer, col + loadWidth, color_buf, 1);
                epd->setSramArrayBufferElement(sram_buffer, col, color_buf, 1);
                //Add New Row to buffer:
                if (loadHeight - row > 1) {
                    bmpColorPixel = (int16_t)this->getBitmapPixel(bmp_read, col, loadHeight - row - 2, filter);
                    color_buf[0] = bmpColorPixel >> 8;
                    color_buf[1] = bmpColorPixel & 0xFF;
                    epd->setSramArrayBufferElement(sram_buffer, col + loadWidth, color_buf, 1);
                }
            } else {
                //Draw:
                epd->drawPixel(epd_col++, epd_row, ram_buffer[col]);
                //Shift:
                ram_buffer[col] = ram_buffer[col + loadWidth];
                //Add New Row to buffer:
                if (loadHeight - row > 1) {
                    ram_buffer[col + loadWidth] = (int16_t)this->getBitmapPixel(bmp_read, col, loadHeight - row - 2, filter);
                }
            }
        }
        #if SIKTEC_EPD_DEBUG_BITMAP_DITHER
            PRINT_DEBUG_BUFFER("After Dither Buffer Shift: %d \n", row);
            this->printDitherBuffer(sram_buffer, ram_buffer, epd, loadWidth);
        #endif
        epd_row++; 
        epd_col = (int16_t)epd_x;
    }

    //Release:
    if (in_sram) {
        epd->releaseSramArrayBuffer();
    } else {
        delete ram_buffer;
    }
    
    this->file.close();
    return EPD_BITMAP_STATUS::DONE;
}

/**
 * @brief Return a bitmap definition parsed from the headers
 * will auto detect which bitmap it is and which format to use.
 * 
 * @param createPalette default False - Whether to create a color palette or not.
 * @return bmp_def_t
 */
bmp_def_t SIKTEC_EPD_BITMAP::getBitmapDefinition(bool createPalette) {
    bool closeAfter = false;
    bmp_def_t def;
    if (!this->file.isOpen()) {
        this->sd->chvol();
        closeAfter = this->file.open(this->name, O_RDONLY);
    }
    if (this->file.isOpen()) {
        if (this->file.fileSize() > sizeof(bmp_def_t)) {
            this->file.rewind();
            //read file header directly to struct:
            this->file.read(&def.file_header, sizeof(bmp_file_header_t));
            //check supported type:
            if (def.file_header.type == BITMAP_TYPE_BM) {
                //Check supported variant: 
                def.variant = this->bitmapVariant();
                this->seekSet(BITMAP_FILEHEADER_SIZE); //set cur 0;
                if (def.variant != BMP_VARIANT::NOT_SUPPORTED) {
                    
                    //Parse info header variant:
                    if (def.variant == BMP_VARIANT::BITMAPCOREHEADER_12) {
                        //NOTE: we do this manually as 12 sized headers has a different structure:
                        def.info_header.header_size  = this->read32();
                        def.info_header.width        = (uint32_t)this->read16();
                        def.info_header.height       = (uint32_t)this->read16();
                        def.info_header.color_planes = this->read16();
                        def.info_header.bpp          = this->read16();
                    } else {
                        //NOTE: we limit this as we (for now) dont use all the extra fields 
                        // That for some formats are used. In the future while adding compression support and masking 
                        // We need to change that 
                        size_t read_size = def.variant <= 64 ? def.variant : 64;
                        this->file.read(&def.info_header, read_size);                            
                    }
                    
                    //Set height fixed:
                    if (def.info_header.height < 0) {
                        // RARE: If bmpHeight is negative, image is in top-down order.
                        def.info_header.height = -def.info_header.height; 
                        def.flip = false;
                    }

                    //How many colors? defined or calculate from bpp
                    //Some BMP docs says that biColors can be greater then zero which 
                    //Defines the palette color size - But in this case they wont be used 
                    //unless its bpp <= 8.
                    // If colors is defind then we override bbb ^ 2 and use the defined color size....
                    if (def.info_header.bpp <= 8 && def.info_header.colors == 0) {
                        def.palette_size = 1 << def.info_header.bpp;
                    } else if (def.info_header.bpp <= 8 && def.info_header.colors) {
                        if (def.info_header.colors <= (1 << def.info_header.bpp)) {
                            def.palette_size = def.info_header.colors;
                        } else { 
                            def.palette_size = (1 << def.info_header.bpp);
                        }
                    } else {
                        def.palette_size = 0;
                    }

                    //Set palette if needed:
                    if (createPalette && def.palette_size > 0) {
                        def.palette = new colorBits_t[def.palette_size];
                        //Store the color as 32bit uinteger -
                        //BMP format is BGRA or BGR this will flip and store ARGB
                        //First make sure what is the used color size:
                        uint32_t pos_palette_array = BITMAP_FILEHEADER_SIZE + def.info_header.header_size;
                        uint32_t bytes_to_pixels_array = def.file_header.array_start - pos_palette_array;

                        this->seekSet(pos_palette_array); //set pos at end of header just in case there is padding to avoid;

                        if (bytes_to_pixels_array <  def.palette_size * 4) {
                            for (uint32_t i = 0; i < def.palette_size; ++i) {
                                def.palette[i] = bitmap_color_result == BITMAP_COLOR_MODE::COLOR565 
                                                    ? this->color32to16(this->read24())
                                                    : this->read24();
                            }
                        } else {
                            for (uint32_t i = 0; i < def.palette_size; ++i) {
                                def.palette[i] = bitmap_color_result == BITMAP_COLOR_MODE::COLOR565 
                                                    ? this->color32to16(this->read32())
                                                    : this->read32();
                            }
                        }
                    }

                    def.status = EPD_BITMAP_STATUS::VALID;

                } else {
                    def.status = EPD_BITMAP_STATUS::UNSUPPORTED_BMP_VARIANT;
                }
            } else {
                def.status = EPD_BITMAP_STATUS::UNSUPPORTED_BMP_TYPE;
            }
        } else {
            def.status = EPD_BITMAP_STATUS::ERROR_FILE_SIZE;
        }
    } else {
        def.status = EPD_BITMAP_STATUS::ERROR_READ_FILE;
    }
    if (closeAfter) {
        this->file.close();
    }
    return def;
}

/**
 * @brief Return the bitmap type (Header Format) - Based on the header size.
 * 
 * @return BMP_VARIANT 
 */
BMP_VARIANT SIKTEC_EPD_BITMAP::bitmapVariant() {
    if (!this->file.isOpen()) 
        return BMP_VARIANT::NOT_SUPPORTED;
    this->seekSet(BITMAP_FILEHEADER_SIZE); //set cur 0;
    uint32_t header_size = this->read32();
    switch (header_size) {
        case BMP_VARIANT::BITMAPCOREHEADER_12: 
            return BMP_VARIANT::BITMAPCOREHEADER_12;
        case BMP_VARIANT::OS22XBITMAPHEADER_16: 
            return BMP_VARIANT::OS22XBITMAPHEADER_16;
        case BMP_VARIANT::BITMAPINFOHEADER_40:
            return BMP_VARIANT::BITMAPINFOHEADER_40;
        case BMP_VARIANT::BITMAPINFOHEADE_ILLU_56:
            return BMP_VARIANT::BITMAPINFOHEADE_ILLU_56;
        case BMP_VARIANT::OS22XBITMAPHEADER_64:
            return BMP_VARIANT::OS22XBITMAPHEADER_64;
        case BMP_VARIANT::BITMAPV4HEADER_108:
            return BMP_VARIANT::BITMAPV4HEADER_108;
        case BMP_VARIANT::BITMAPV5HEADER_124:
            return BMP_VARIANT::BITMAPV5HEADER_124;
        default :  
            return BMP_VARIANT::NOT_SUPPORTED;
    }
}

/**
 * @brief returns struct with all needed values for reading a bitmap at a specific region
 * 
 * @param bmp_sr        - uint32_t the bitmap starting point Y / Row (From Top).
 * @param bmp_sc        - uint32_t the bitmap starting point X / Col (From Left).
 * @param loadWidth     - uint32_t The width to draw
 * @param loadHeight    - uint32_t The height to draw
 * @return bmp_read_definition_t 
 */
bmp_read_definition_t SIKTEC_EPD_BITMAP::prepareBitmapReadDefinition(const uint32_t bmp_sr, const uint32_t bmp_sc, const uint32_t loadWidth, const uint32_t loadHeight) {

    bmp_read_definition_t bmp_read;
    bmp_read.start_row      = bmp_sr;       //user defined stating row
    bmp_read.start_col      = bmp_sc;       //user defined stating column
    bmp_read.read_width     = loadWidth;    //user defined and adjusted width
    bmp_read.read_height    = loadHeight;   //user defined and adjusted height
    //BGR or 1-bit bitmap row format size:        
    bmp_read.row_bit_size = ((this->definition.info_header.bpp * this->width() + 31) / 32) * 4; // padded.
    //Set starting row address:
    bmp_read.start_row_address = this->definition.file_header.array_start + bmp_read.start_row * bmp_read.row_bit_size;
    //A small helper value that defines how many pixels are store in a byte:
    switch (this->definition.info_header.bpp) {
        case 1:     { bmp_read.pixels_per_iteration = 8; } break;
        case 4:     { bmp_read.pixels_per_iteration = 2; } break;
        default:    { bmp_read.pixels_per_iteration = 1; }
    }
    //Column offest:
    switch (this->definition.info_header.bpp) {
        case 1: { 
            bmp_read.column_offset_bytes    = bmp_read.start_col / 8; 
            bmp_read.column_skip_bytes      = bmp_read.start_col - bmp_read.column_offset_bytes * 8;
        } break;
        case 4: { 
            bmp_read.column_offset_bytes    = bmp_read.start_col / 2; 
            bmp_read.column_skip_bytes      = bmp_read.start_col - bmp_read.column_offset_bytes * 2;
        } break;
        case 8: { 
            bmp_read.column_offset_bytes    = bmp_read.start_col; 
            bmp_read.column_skip_bytes      = 0;
        } break;
        case 16: { 
            bmp_read.column_offset_bytes    = bmp_read.start_col * 2; 
            bmp_read.column_skip_bytes      = 0;
        } break;
        case 24: { 
            bmp_read.column_offset_bytes    = bmp_read.start_col * 3; 
            bmp_read.column_skip_bytes      = 0;
        } break;
        case 32: { 
            bmp_read.column_offset_bytes    = bmp_read.start_col * 4; 
            bmp_read.column_skip_bytes      = 0;
        } break;
    }
    return bmp_read;
}

/**
 * @brief returns a single pixel color directly from the bitmap
 * 
 * @param bitmap_read - The read operation definition
 * @param x           - The requested x - column (from left)
 * @param y           - The requested y - row (from top)
 * @param filter      - apply a filter to the pixel. 
 * @return colorBits_t - pixel color bitfield
 */
colorBits_t SIKTEC_EPD_BITMAP::getBitmapPixel(const bmp_read_definition_t bitmap_read, const int16_t x, const int16_t y, BITMAP_FILTER_IMPLEMENTATION *filter) {

    uint32_t pixels = 0; //read buffer
    uint32_t row = bitmap_read.start_row + y;
    uint32_t col = bitmap_read.start_col + x;
    uint32_t address = bitmap_read.start_row_address + y * bitmap_read.row_bit_size;

    //Point file to pixel array at start of row:
    if (this->definition.info_header.bpp == 1) {
        // TODO: Implement this
        return 0;
    } else if (this->definition.info_header.bpp == 4) {
        // TODO: Implement this
        return 0;
    } else if (this->definition.info_header.bpp == 8) {
        address += x;
        this->seekSet(address + bitmap_read.column_offset_bytes);
        //We assume its an index
        uint8_t pixel1 = this->read8();
        return this->pixelColorProccess((colorBits_t)(pixel1 < this->definition.palette_size ? this->definition.palette[pixel1] : 0xFFFF),  filter);
    } else if (this->definition.info_header.bpp == 16) {
        address += x * 2;
        this->seekSet(address + bitmap_read.column_offset_bytes);
        //We assume RGB 565 as the color: 00000 000000 00000
        uint16_t pixel1 = this->read16();
        return this->pixelColorProccess(pixel1, filter);
    } else if (this->definition.info_header.bpp == 24) {
        address += x * 3;
        this->seekSet(address + bitmap_read.column_offset_bytes);
        //We assume RGB 888 as the color: 00000000 00000000 00000000
        uint32_t pixel1 = this->read24();
        return this->pixelColorProccess(pixel1, filter);
    } else if (this->definition.info_header.bpp == 32) {
        address += x * 4;
        this->seekSet(address + bitmap_read.column_offset_bytes);
        //We assume ARGB 8888 as the color: 00000000 00000000 00000000 00000000
        uint32_t pixel1 = this->read32();
        return this->pixelColorProccess(pixel1, filter);
    }
    return 0;
}

/**
 * @brief Will parse and traverse the pixel array and draw them on the given EPD.
 * 
 * @param epd_x         uint32_t the top-left X position on the EPD.
 * @param epd_y         uint32_t the top-left Y position on the EPD.
 * @param bmp_r         uint32_t the bitmap Startin point Y / Row (From top).
 * @param bmp_c         uint32_t the bitmap Startin point X / Column (From Left).
 * @param loadWidth     uint32_t The width to draw (clip width) - 0 for full width.
 * @param loadHeight    uint32_t The width to draw (clip height) - 0 for full height.
 * @param epd           SIKTEC_EPD * the pointer to the epd to draw on.
 * @param kernel        translate_color - the kernel function pointer.
 * @return void 
 */
void SIKTEC_EPD_BITMAP::proccessUncompressed(
    uint32_t epd_x, uint32_t epd_y, 
    const bmp_read_definition_t bitmap_read,
    SIKTEC_EPD *epd,
    BITMAP_FILTER_IMPLEMENTATION *filter
) {

    //We assume file is open -> this can be called only from draw which handles the file before.
    int16_t epd_col      = (int16_t)epd_x;
    int16_t epd_row      = (int16_t)epd_y + bitmap_read.read_height - 1;
    uint32_t pixels      = 0; //read buffer
    uint32_t address     = bitmap_read.start_row_address;

    for (uint32_t r = 0; r < bitmap_read.read_height; ++r, address += bitmap_read.row_bit_size) {
        #if SIKTEC_EPD_DEBUG_BITMAP_PIXELS
            PRINT_DEBUG_BUFFER("\n Row[%d,%ld]", r, (long)bitmap_read.row_bit_size);
        #endif

        this->seekSet(address + bitmap_read.column_offset_bytes);
        
        for (uint32_t c = 0; c < bitmap_read.read_width; c += bitmap_read.pixels_per_iteration) {
            //Point file to pixel array at start of row:
            if (this->definition.info_header.bpp == 1) {
                pixels = this->read8();
                for (uint8_t p = bitmap_read.column_skip_bytes; p < bitmap_read.pixels_per_iteration; ++p) {
                    uint8_t pixel1 = (pixels & ((uint32_t)1 << (bitmap_read.pixels_per_iteration-p-1))) != 0 ? 1 : 0;
                    epd->drawPixel(
                        epd_col++, epd_row, 
                        this->pixelColorProccess(
                            //NOTE: shlomi - added this for security, in case the file (pixel array is curoptted) we want to avoid reading from undefined palette memory
                            (colorBits_t)(pixel1 < this->definition.palette_size ? this->definition.palette[pixel1] : 0xFFFF), 
                            filter
                        )
                    );
                    #if SIKTEC_EPD_DEBUG_BITMAP_PIXELS
                        PRINT_DEBUG_BUFFER(" %3d", pixel1);
                    #endif
                }
            } else if (this->definition.info_header.bpp == 4) {
                pixels = this->read8();
                for (int8_t p = bitmap_read.pixels_per_iteration - 1 - bitmap_read.column_skip_bytes; p >= 0; --p) { //Int8_t is important here uint8_t will cause an infinite loop
                    uint8_t pixel1 = (pixels >> (4 * p)) & 0xF;
                    epd->drawPixel(
                        epd_col++, epd_row, 
                        this->pixelColorProccess(
                            (colorBits_t)(pixel1 < this->definition.palette_size ? this->definition.palette[pixel1] : 0xFFFF), 
                            filter
                        )
                    );
                    #if SIKTEC_EPD_DEBUG_BITMAP_PIXELS
                        PRINT_DEBUG_BUFFER(" %3d", pixel1);
                    #endif
                }
            } else if (this->definition.info_header.bpp == 8) {
                uint8_t pixel1 = this->read8();
                epd->drawPixel(
                    epd_col++, epd_row, 
                    this->pixelColorProccess(
                        (colorBits_t)(pixel1 < this->definition.palette_size ? this->definition.palette[pixel1] : 0xFFFF), 
                        filter
                    )
                );
                #if SIKTEC_EPD_DEBUG_BITMAP_PIXELS
                    PRINT_DEBUG_BUFFER(" %3d", pixel1);
                #endif
            } else if (this->definition.info_header.bpp == 16) {
                //We assume RGB 565 as the color:                                   
                // 00000 000000 00000
                uint16_t pixel1 = this->read16();
                epd->drawPixel(
                    epd_col++, epd_row, 
                    this->pixelColorProccess(pixel1, filter)
                );
                #if SIKTEC_EPD_DEBUG_BITMAP_PIXELS
                    PRINT_DEBUG_BUFFER(" %3d", pixel1);
                #endif
            } else if (this->definition.info_header.bpp == 24) {
                //We assume RGB 888 as the color:
                // 00000000 00000000 00000000
                uint32_t pixel1 = this->read24();
                epd->drawPixel(
                    epd_col++, epd_row, 
                    this->pixelColorProccess(pixel1, filter)
                );
                #if SIKTEC_EPD_DEBUG_BITMAP_PIXELS
                    PRINT_DEBUG_BUFFER(" %3d", pixel1);
                #endif
            } else if (this->definition.info_header.bpp == 32) {
                //We assume ARGB 8888 as the color:
                // 00000000 00000000 00000000 00000000
                uint32_t pixel1 = this->read32();
                epd->drawPixel(
                    epd_col++, epd_row, 
                    this->pixelColorProccess(pixel1, filter)
                );
                #if SIKTEC_EPD_DEBUG_BITMAP_PIXELS
                    PRINT_DEBUG_BUFFER(" %3d", pixel1);
                #endif
            }
        }
        
        //FUTURE: handle flipped pixel array TOP to BOTTOM....
        //Next epd row for drawing:
        epd_row--; 
        epd_col = (int16_t)epd_x;
    }
    #if SIKTEC_EPD_DEBUG_BITMAP_PIXELS
        Serial.println();
    #endif
}

/**
 * @brief Will apply the Filter / Kernel to a parsed pixel and return the color format to use.
 * 
 * @param rgb565        - uint16_t a 565 16bit color format pixel.
 * @param filter        - the filter to apply.
 * @return colorBits_t  - pixel color bitfield
 */
colorBits_t SIKTEC_EPD_BITMAP::pixelColorProccess(uint16_t rgb565, BITMAP_FILTER_IMPLEMENTATION *filter) {
    if (bitmap_color_result == BITMAP_COLOR_MODE::COLOR565) {
        if (filter != nullptr) {
            return filter->kernel(
                rgb565 >> 11,         // R
                (rgb565 >> 5) & 0x3F, // G
                rgb565 & 0x1F         // B
            );
        }
        return (colorBits_t)rgb565;
    } else {
        return this->pixelColorProccess(this->color16to32(rgb565), filter);
    }
}

/**
 * @brief Will apply the Filter / Kernel to a parsed pixel and return the color format to use.
 * 
 * @param rgb888        - uint32_t a RGB888 color format pixel.
 * @param filter        - the filter to apply.
 * @return colorBits_t - pixel color bitfield
 */
colorBits_t SIKTEC_EPD_BITMAP::pixelColorProccess(uint32_t rgb888, BITMAP_FILTER_IMPLEMENTATION *filter) {
    if (bitmap_color_result == BITMAP_COLOR_MODE::COLOR888) {
        if (filter != nullptr) {
            return filter->kernel(
                rgb888 >> 16,          // R
                (rgb888 >> 8) & 0xFF,  // G
                rgb888 & 0xFF          // B
            );
        }
        return (colorBits_t)rgb888;
    } else {
        return this->pixelColorProccess(this->color32to16(rgb888), filter);
    }
}

/**
 * @brief Converts a 32bit color (888) to 16bit color (565).
 * 
 * @param R8 red chanel 8bit color 0-255.
 * @param G8 green chanel 8bit color 0-255.
 * @param B8 blue chanel 8bit color 0-255.
 * @return uint16_t RGB565.
 */
uint16_t SIKTEC_EPD_BITMAP::color32to16(uint8_t R8, uint8_t G8, uint8_t B8) {  // A888 => 565
    //return ((R8 & 0xF8) << 8) | ((G8 & 0xFC) << 3) | (B8 >> 3);
    return (((R8 >> 3) & 0x1f) << 11) | (((G8 >> 2) & 0x3f) << 5) | ((B8 >> 3) & 0x1f);
}

/**
 * @brief Converts a 32bit color (A888) to 16bit color (565).
 * 
 * @param C32 uint32_t ARGB8888 color.
 * @return uint16_t RGB565.
 */
uint16_t SIKTEC_EPD_BITMAP::color32to16(uint32_t C32) { // A888 => 565
    uint8_t R8 = (C32 >> 16) & 0xFF;
    uint8_t G8 = (C32 >> 8) & 0xFF;
    uint8_t B8 = C32 & 0xFF;
    return this->color32to16(R8, G8, B8);
}

/**
 * @brief Converts a 16bit color (565) to 32bit color (0888).
 * 
 * @param R5 red chanel 5bit color 0-31.
 * @param G6 green chanel 8bit color 0-63.
 * @param B5 blue chanel 8bit color 0-31.
 * @return uint32_t ARGB 0888.
 */
uint32_t SIKTEC_EPD_BITMAP::color16to32(uint8_t R5, uint8_t G6, uint8_t B5) { // 565 => A888
    uint8_t R8 = ( R5 * 527 + 23 ) >> 6;
    uint8_t G8 = ( G6 * 259 + 33 ) >> 6;
    uint8_t B8 = ( B5 * 527 + 23 ) >> 6;
    return (R8 << 16) | (G8 << 8) | B8;
}

/**
 * @brief Converts a 16bit color (565) to 32bit color (888).
 * 
 * @param C16 uint16_t RGB565 color.
 * @return uint16_t ARGB 0888.
 */
uint32_t SIKTEC_EPD_BITMAP::color16to32(uint16_t C16) {
    uint8_t R5 = C16 >> 11;
    uint8_t G6 = (C16 >> 5) & 0x3F;
    uint8_t B5 = C16 & 0x1F;
    return this->color16to32(R5, G6, B5);
}

/**
 * @brief Rewind file pointer to seek pos 0.
 * 
 * @return void
 */
void SIKTEC_EPD_BITMAP::rewind() {
    this->file.rewind();
}

/**
 * @brief Set a seek position (cursor) of the current open file.
 * 
 * @param pos uint32_t - the cursor index.
 * @return bool success - may fail on incorrect position or file not open.
 */
bool SIKTEC_EPD_BITMAP::seekSet(uint32_t pos) {
    return this->file.seekSet(pos);
}

/**
 * @brief Reads a byte from current seek position - Increments the cursor.
 * 
 * @return uint8_t 
 */
uint8_t SIKTEC_EPD_BITMAP::read8() {
    return this->file.read();
}

/**
 * @brief Reads a 16bit number (2 bytes) from current seek position - Increments the cursor.
 * 
 * @return uint16_t 
 */
uint16_t SIKTEC_EPD_BITMAP::read16() {
    #if !defined(ESP32) && !defined(ESP8266) && (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
        uint16_t result; // Read directly into result -- BMP data and variable both little-endian.
        this->file.read(&result, 2);
        return result;
    #else
        return this->file.read() | ((uint16_t)this->file.read() << 8); // Big-endian or unknown. Byte-by-byte read will perform reversal if needed.
    #endif
}

/**
 * @brief Reads a 24bit number (3 bytes) from current seek position - Increments the cursor.
 * 
 * @return uint32_t 
 */
uint32_t SIKTEC_EPD_BITMAP::read24() {
    //Todo test with esp as we are not sure this will work
    #if !defined(ESP32) && !defined(ESP8266) && (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
        uint32_t result; // Read directly into result -- BMP data and variable both little-endian.
        this->file.read(&result, 3);
        // return result >> 8;
        return result; //Test this result without shift 8 on 24bpp none esp boards
    #else
        // Big-endian or unknown. Byte-by-byte read will perform reversal if needed.
        return this->file.read() | ((uint32_t)this->file.read() << 8) | ((uint32_t)this->file.read() << 16) | 0;
    #endif
}

/**
 * @brief Reads 24bit numbers (3 bytes each) from current seek position into an array - Increments the cursor.
 * 
 * @param output uint32_t array.
 * @param length size_t how many numbers to read.
 * @return void
 */
void SIKTEC_EPD_BITMAP::read24(uint32_t *output, const size_t length) {
    for (size_t i = 0; i < length; ++i) {
        output[i] = this->read24(); 
    }
}

/**
 * @brief Reads a 32bit number (4 bytes) from current seek position - Increments the cursor.
 * 
 * @return uint32_t 
 */ 
uint32_t SIKTEC_EPD_BITMAP::read32() {
    #if !defined(ESP32) && !defined(ESP8266) && (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
        uint32_t result; // Read directly into result -- BMP data and variable both little-endian.
        this->file.read(&result, sizeof result);
        return result;
    #else
        // Big-endian or unknown. Byte-by-byte read will perform reversal if needed.
        return this->file.read() | ((uint32_t)this->file.read() << 8) | ((uint32_t)this->file.read() << 16) | ((uint32_t)this->file.read() << 24);
    #endif
}

/**
 * @brief Reads 32bit numbers (4 bytes) from current seek position into an array - Increments the cursor.
 * 
 * @param output uint32_t array.
 * @param length size_t how many numbers to read.
 * @return void
 */
void SIKTEC_EPD_BITMAP::read32(uint32_t *output, const size_t length) {
    for (size_t i = 0; i < length; ++i) {
        output[i] = this->read32(); 
    }
}

#if SIKTEC_EPD_DEBUG_BITMAP
    /**
     * @brief Debug function (prints info of the parsed bitmap)
     * @returns void
     */
    void SIKTEC_EPD_BITMAP::debug_bitmapDefinition() {
        
        PRINT_DEBUG_BUFFER("BITMAP '%s' Status[%d] Variant[%d]:\n", 
            this->name, 
            this->definition.status, 
            this->definition.variant
        );
        PRINT_DEBUG_BUFFER("    Type : %u - %#X\n    File Size : %u\n    Array Offset : %u - %#X\n",
            (unsigned int)this->definition.file_header.type, (unsigned int)this->definition.file_header.type,
            (unsigned int)this->definition.file_header.size,
            (unsigned int)this->definition.file_header.array_start, (unsigned int)this->definition.file_header.array_start
        );
        if (this->definition.variant == BMP_VARIANT::BITMAPCOREHEADER_12) {
            PRINT_DEBUG_BUFFER("    Header Size : %u\n    Dim : %u,%u\n",
                (unsigned int)this->definition.info_header.header_size,
                (unsigned int)this->definition.info_header.width, (unsigned int)this->definition.info_header.height
            );
            PRINT_DEBUG_BUFFER("    Planes : %u\n    BPP : %u\n",
                (unsigned int)this->definition.info_header.color_planes,
                (unsigned int)this->definition.info_header.bpp
            );
        } else {
            PRINT_DEBUG_BUFFER("    Header Size : %u\n    Raw Array : %u\n    Res H/V : %ld,%ld\n",
                (unsigned int)this->definition.info_header.header_size,
                (unsigned int)this->definition.info_header.raw_size,
                (long)this->definition.info_header.horizontal_res,(long)this->definition.info_header.vertical_res
            );
            PRINT_DEBUG_BUFFER("    Dim : %u,%u\n    Colors : %u\n    UColors : %u\n",
                (unsigned int)this->definition.info_header.width, (unsigned int)this->definition.info_header.height,
                (unsigned int)this->definition.info_header.colors,
                (unsigned int)this->definition.info_header.important_colors
            );
            PRINT_DEBUG_BUFFER("    Planes : %u\n    BPP : %u\n    Compression : %u\n",
                (unsigned int)this->definition.info_header.color_planes,
                (unsigned int)this->definition.info_header.bpp,
                (unsigned int)this->definition.info_header.compression
            );
            if (this->definition.variant == BMP_VARIANT::OS22XBITMAPHEADER_64) {
                PRINT_DEBUG_BUFFER("    Direction : %u\n    Halftoning : %u\n    Encoding : %u\n",
                    (unsigned int)this->definition.info_header.fill_direction,
                    (unsigned int)this->definition.info_header.halftoning_algo,
                    (unsigned int)this->definition.info_header.color_encoding
                );
            }
        }
        PRINT_DEBUG_BUFFER("    Palette Size: [%ld]\n", (long)this->definition.palette_size);
        if (this->definition.palette_size && this->definition.palette != nullptr) {
            //Colors are stored on the lib side as ARGB
            for (size_t i = 0; i < this->definition.palette_size; ++i) {
                if (bitmap_color_result == BITMAP_COLOR_MODE::COLOR565) {
                    PRINT_DEBUG_BUFFER("        [%2d] %3d, %3d, %3d 0x%08x\n",
                        i,                                          // Palette Index.
                        this->definition.palette[i] >> 11,          // R
                        (this->definition.palette[i] >> 5) & 0x3F,  // G
                        this->definition.palette[i] & 0x1F,         // B
                        this->definition.palette[i]                 // Binary view.
                    );
                } else {
                    PRINT_DEBUG_BUFFER("        [%2d] %3d, %3d, %3d, %3d 0x%08x\n",
                        i,                                                // Palette Index.
                        (this->definition.palette[i] & 0xFF000000) >> 24, // A
                        (this->definition.palette[i] & 0x00FF0000) >> 16, // R
                        (this->definition.palette[i] & 0x0000FF00) >> 8,  // G
                        this->definition.palette[i] & 0x000000FF,         // B
                        this->definition.palette[i]                       // Binary view.
                    );
                }
            }
        }
    }
#endif

#if SIKTEC_EPD_DEBUG_BITMAP_DITHER
    void SIKTEC_EPD_BITMAP::printDitherBuffer(const uint16_t buffer, int16_t *ram_buffer,  SIKTEC_EPD *epd, const uint16_t width) {
        colorBits_t bmpColorPixel;
        uint8_t color_buf[2];
        Serial.println("S---- DITHER BUF");
        for (uint16_t row = 0; row < 2; ++row) {
            for (uint16_t col = 0; col < width; ++col) {
                if (ram_buffer == nullptr) {
                    // From sram:
                    epd->getSramArrayBufferElement(buffer, col + row * width, color_buf, 1);
                    bmpColorPixel = (color_buf[0] << 8) | color_buf[1];
                } else {
                    // From ram:
                    bmpColorPixel = ram_buffer[col + row * width];
                }
                PRINT_DEBUG_BUFFER("% 7d ", bmpColorPixel);
            }
            Serial.println();
        }
        Serial.println("E-----");
    }   
#endif

}