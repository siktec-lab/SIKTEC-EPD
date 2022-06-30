
/******************************************************************************/
// Created by: SIKTEC.
// Release Version : 1.0.2
// Creation Date: 2022-03-31
// Copyright 2022, SIKTEC.
/******************************************************************************/

/** @file SIKTEC_EPD.cpp */

#include "SIKTEC_EPD.h"
#include <stdlib.h>

namespace SIKtec {

/**
 * @brief when debug flag create a message buffer for the debug output 
 */
#if SIKTEC_EPD_DEBUG || SIKTEC_EPD_DEBUG_PIXELS || SIKTEC_EPD_DEBUG_SRAM || SIKTEC_EPD_DEBUG_COMMAND_LISTS
    char debug_message[150];
    const int debug_message_len = sizeof( debug_message ) / sizeof( debug_message[0] );
#endif

bool SIKTEC_EPD::_isInTransaction = false;

/**
 * @brief constructor if using external SRAM chip and software SPI
 * 
 * @param width the width of the display in pixels
 * @param height the height of the display in pixels
 * @param CS the chip select pin to use
 * @param SRCS the SRAM chip select pin to use
 * @param DC the data/command pin to use
 * @param RST the reset pin to use
 * @param BUSY the busy pin to use
 * @param spi_clock the SCLK pin to use
 * @param spi_mosi the SID pin to use
 * @param spi_miso the MISO pin to use
 * @param clock_frequency the spi bus frequency -> for sram and epd controller
*/
SIKTEC_EPD::SIKTEC_EPD(
    int width, int height, 
    int8_t CS, int8_t SRCS, int8_t DC, int8_t RST, int8_t BUSY, 
    int8_t spi_clock, int8_t spi_miso,  int8_t spi_mosi, uint32_t clock_frequency
    ) : SIKTEC_EPD(
        width, height, 
        { CS, SRCS, DC, RST, BUSY }, 
        spi_clock, spi_miso, spi_mosi, clock_frequency
    ) {}

/**
 * @brief constructor if using external SRAM chip and software SPI
 * 
 * @param width the width of the display in pixels
 * @param height the height of the display in pixels
 * @param pins the epd & sram pins
 * @param spi_clock the SCLK pin to use
 * @param spi_mosi the SID pin to use
 * @param spi_miso the MISO pin to use
 * @param clock_frequency the spi bus frequency -> for sram and epd controller
*/
SIKTEC_EPD::SIKTEC_EPD(
    int width, int height, 
    const epd_pins_t &pins, 
    int8_t spi_clock, int8_t spi_miso,  int8_t spi_mosi, uint32_t clock_frequency
    ) : Adafruit_GFX(width, height) {
    
    this->pins = pins;

    //Save the calculated raw dim - height should be devisible by 8:
    //This save many many many calculations later mostly in address calculation and draw pixels
    this->fixed8_width  = width;
    if (height % 8 != 0) {
        this->fixed8_height += 8 - (height % 8);
    } else {
        this->fixed8_height = height;
    }

    //Initiate SRAM:
    if (this->pins.sram_cs >= 0) {
        this->sram = new SIKTEC_SRAM(spi_mosi, spi_miso, spi_clock, this->pins.sram_cs, clock_frequency); 
        this->sram->begin();
        this->use_sram = this->sram->set_mode(SRAM_MODE::SRAM_SEQ_MODE);
        //Put in seq mode and check sram is ready to go:
        if (!this->use_sram) {
            Serial.println("SRAM INIT FAILED");
            this->sram->print_status();
        }
    } else {
        this->use_sram = false;
    }

    //Set spi device:
    this->_spi = new SIKTEC_SPI(
        this->pins.epd_cs, spi_clock, spi_miso, spi_mosi, 
        clock_frequency,        // frequency
        SPI_BITORDER_MSBFIRST,  // bit order
        SPI_MODE0               // data modespi;
    );
                            
    //Set inital values:
    this->buffer1_size      = 0;
    this->buffer2_size      = 0;
    this->buffer1_addr      = 0;
    this->buffer2_addr      = 0;
    this->colorbuffer_addr  = 0;
    this->blackbuffer_addr  = 0;
    this->buffer1           = nullptr;
    this->buffer2           = nullptr;
    this->color_buffer      = nullptr;
    this->black_buffer      = nullptr;

    #if SIKTEC_EPD_DEBUG
        Serial.println("initialized SIKTEC_EPD");
    #endif
}

/**
 * @brief constructor if using on-chip SRAM and hardware SPI
 * 
 * @param width the width of the display in pixels
 * @param height the height of the display in pixels
 * @param CS the chip select pin to use
 * @param SRCS the SRAM chip select pin to use
 * @param DC the data/command pin to use
 * @param RST the reset pin to use
 * @param BUSY the busy pin to use
 * @param spi the SPI bus to use
 * @param clock_frequency the spi bus frequency -> for sram and epd controller
*/
SIKTEC_EPD::SIKTEC_EPD(
    int width, int height, 
    int8_t CS, int8_t SRCS, int8_t DC, int8_t RST, int8_t BUSY, 
    SPIClass *spi, uint32_t clock_frequency
    ) : SIKTEC_EPD(width, height, { CS, SRCS, DC, RST, BUSY }, spi, clock_frequency) {}

/**
 * @brief constructor if using on-chip SRAM and hardware SPI
 * 
 * @param width the width of the display in pixels
 * @param height the height of the display in pixels
 * @param pins the pin definition
 * @param spi the SPI bus to use
 * @param clock_frequency the spi bus frequency -> for sram and epd controller
*/
SIKTEC_EPD::SIKTEC_EPD(int width, int height, const epd_pins_t &pins, SPIClass *spi, uint32_t clock_frequency) 
    : Adafruit_GFX(width, height) {
    this->pins = pins;
    //Save the calculated raw dim - height should be devisible by 8:
    //This save many many many calculations later mostly in address calculation and draw pixels
    this->fixed8_width  = width;
    if (height % 8 != 0) {
        this->fixed8_height += 8 - (height % 8);
    } else {
        this->fixed8_height = height;
    }
    //Initiate SRAM:
    if (this->pins.sram_cs >= 0) {
        this->sram = new SIKTEC_SRAM(this->pins.sram_cs, spi, clock_frequency); 
        this->sram->begin();
        this->use_sram = this->sram->set_mode(SRAM_MODE::SRAM_SEQ_MODE);
        //Put in seq mode and check sram is ready to go:
        if (!this->use_sram) {
            Serial.println("SRAM INIT FAILED");
            this->sram->print_status();
        }
    } else {
        this->use_sram = false;
    }
    //Set spi device:
    this->_spi = new SIKTEC_SPI(
        this->pins.epd_cs,      // EPD cs
        clock_frequency,         // frequency
        SPI_BITORDER_MSBFIRST,  // bit order
        SPI_MODE0,              // data modespi;
        spi
    );
    //Set inital values:
    this->buffer1_size      = 0;
    this->buffer2_size      = 0;
    this->buffer1_addr      = 0;
    this->buffer2_addr      = 0;
    this->colorbuffer_addr  = 0;
    this->blackbuffer_addr  = 0;
    this->buffer1           = nullptr;
    this->buffer2           = nullptr;
    this->color_buffer      = nullptr;
    this->black_buffer      = nullptr;
    #if SIKTEC_EPD_DEBUG
        Serial.println("initialized SIKTEC_EPD");
    #endif
}

/**
 * @brief default destructor
*/
SIKTEC_EPD::~SIKTEC_EPD() {
    //Destruct SRAM obj:
    delete this->sram;
    //Descrutor -> free allocated memory of buffer:
    if (this->buffer1 != NULL) {
        free(this->buffer1);
        this->buffer1 = NULL;
    }
    if (this->buffer2 != NULL) {
        free(this->buffer2);
        this->buffer2 = NULL;
    }
}

/**
 * @brief check if we are using sram for with this board:
 * 
 * @returns bool 
*/
bool SIKTEC_EPD::is_using_sram() {
    return this->use_sram;
}

/**
 * @brief get additional SRAM available space in Kib (kilo-binary bits)
 * used by filters when additional buffer is required.
 * 
 * @param assumeTotalSizeKib 
 * @return epd_sram_space_t free space in Kib and Bytes and address
 */
epd_sram_space_t SIKTEC_EPD::getFreeSramSpace(uint32_t assumeTotalSizeKib)  {

    if (!this->use_sram) return {0,0,0};
    uint32_t total = assumeTotalSizeKib << 10; // bits 
    uint32_t bitsRemaining = total - ((uint32_t)this->buffer1_size * 8 + (uint32_t)this->buffer2_size * 8);
    return {
        bitsRemaining >> 10, 
        bitsRemaining / 8, 
        (uint16_t)(this->buffer2_addr + this->buffer2_size + 1)
    };
}

/**
 * @brief allocate space in extra SRAM space and get the address
 * 
 * @param num the number of elements we are want to allocate.
 * @param ele_bytes the size in bytes of one element
 * @return uint16_t the address to use when writing / reading
 * @return 0 -> mens can't allocate, not enough space or SRAM is not used
 */
uint16_t SIKTEC_EPD::allocateSramArrayBuffer(const uint16_t num, const uint16_t ele_bytes)  {
    if (this->use_sram) {
        epd_sram_space_t space = this->getFreeSramSpace();
        if (space.bytes > (num * ele_bytes)) {
            this->ram_buffer_element_size = ele_bytes;
            return space.address;
        }
    }
    return 0;
}

/**
 * @brief release / reset the the SRAM buffer allocation
 * 
 * @return epd_sram_space_t free space in Kib and Bytes and address
 */
void SIKTEC_EPD::releaseSramArrayBuffer()  {
    if (this->use_sram) {
        this->ram_buffer_element_size = 1;
    }
}

/**
 * @brief get / read an element from the allocated EXTRA sram buffer.
 * 
 * @param address - buffer start address
 * @param index - element index.
 * @param out - output buffer to write to
 * @param num - number of elements to sequentially read  
 * @return bool
 * 
 */
bool SIKTEC_EPD::getSramArrayBufferElement(const uint16_t address, const uint16_t index, uint8_t *out, const uint16_t num)  {
    if (this->use_sram) {
        for (uint16_t i = 0; i < num; ++i)
            this->sram->read(address + index * this->ram_buffer_element_size, out, this->ram_buffer_element_size);
        return true;
    }
    return false;
}

/**
 * @brief set / write an element to the allocated EXTRA sram buffer.
 * 
 * @param address - buffer start address
 * @param index - element index.
 * @param in - input buffer to read from
 * @param num - number of elements to sequentially write 
 * @return bool
 * 
 */
bool SIKTEC_EPD::setSramArrayBufferElement(const uint16_t address, const uint16_t index, uint8_t *in, const uint16_t num)  {
    if (this->use_sram) {
        for (uint16_t i = 0; i < num; ++i)
            this->sram->write(address + index * this->ram_buffer_element_size, in, this->ram_buffer_element_size);
        return true;
    }
    return false;
}

/**
 * @brief begin communication with EPD and set up the display.
 * 
 * @param reset true for epd harware reset.
 * @returns void
*/
void SIKTEC_EPD::begin(bool reset) {
    
    #if SIKTEC_EPD_DEBUG
        Serial.println("EPD Begin procedure");
    #endif

    //Set buffers:
    this->setBlackBuffer(0, true);  // black defaults to inverted
    this->setColorBuffer(1, false); // color defaults to not inverted

    //Set color layers - defaults B/W:
    this->layer_colors[EPD_WHITE] = 0b00; 
    this->layer_colors[EPD_BLACK] = 0b01; 
    this->layer_colors[EPD_RED]   = 0b10; 
    this->layer_colors[EPD_GRAY]  = 0b10; 
    this->layer_colors[EPD_DARK]  = 0b01; 
    this->layer_colors[EPD_LIGHT] = 0b00; 

    //Begin epd spi - will also set cs:
    (void)this->_spi->begin();
    // set pins
    pinMode(this->pins.dc, OUTPUT);
    //Do a harware reset:
    if (reset) {
        this->hardwareResetEPD();
    }
    //Set the epd busy pin:
    if (this->pins.busy >= 0) {
        pinMode(this->pins.busy, INPUT);
    }
}

/**
 * @brief Initiate epd hardware reset
 * 
 * @returns void
*/
void SIKTEC_EPD::hardwareResetEPD() {
    
    #if SIKTEC_EPD_DEBUG
        Serial.print("Performing Hardware Reset - ");
    #endif

    if (this->pins.rst >= 0) {
        #if SIKTEC_EPD_DEBUG
            Serial.print("Starting...");
        #endif
        // Setup reset pin direction
        pinMode(this->pins.rst, OUTPUT);
        // VDD (3.3V) goes high at start, lets just chill for a ms
        digitalWrite(this->pins.rst, HIGH);
        delay(50);
        // bring reset low
        digitalWrite(this->pins.rst, LOW);
        // wait 10ms
        delay(20);
        // bring out of reset
        digitalWrite(this->pins.rst, HIGH);
        delay(50);
    }
    #if SIKTEC_EPD_DEBUG
        else {
            Serial.print("Skipping...");
        }
    #endif
    #if SIKTEC_EPD_DEBUG
        Serial.println("Done!");
    #endif
}

/**
 * @brief draw a single pixel on the screen
 * this is the virtual override of gfx derived lib and all drawing
 * methods will call this.
 * 
 * @param x the x axis coordinate
 * @param y the y axis coordinate
 * @param color the color of the pixel
 * 
 * @returns void
*/
void SIKTEC_EPD::drawPixel(int16_t x, int16_t y, uint16_t color) {

    uint8_t  *black_pBuf;
    uint8_t  *color_pBuf;
    uint8_t black_c, color_c;

    SIKTEC_EPD::pixelAddress_t pixel = this->getPixelAddress(x, y); // will handle bounds check also
    
    if (!pixel.inBound) {
        return;
    }

    //Save the memory address of black and color to temp bufs:
    if (this->use_sram) {
        black_c = this->sram->read8(pixel.sram_black);
        black_pBuf = &black_c;
        color_c = this->sram->read8(pixel.sram_color);
        color_pBuf = &color_c;
        #if SIKTEC_EPD_DEBUG_PIXELS
            PRINT_DEBUG_BUFFER("Draw Pixel[%u,%u, C%d] -> Address[B %u:%#X, C %u::%#X] ", x, y, color, pixel.sram_black, pixel.sram_black, pixel.sram_color, pixel.sram_color);
        #endif

    } else {

        color_pBuf = pixel.ram_color;
        black_pBuf = pixel.ram_black;
        #if SIKTEC_EPD_DEBUG_PIXELS
            PRINT_DEBUG_BUFFER("Draw Pixel[%u,%u, C%d] -> Address[B %u:%#X, C %u::%#X] ", x, y, color, pixel.ram_black, pixel.ram_black, pixel.ram_color, pixel.ram_color);
        #endif
    }

    bool black_bit = this->layer_colors[color] & 0x1; //01
    bool color_bit = this->layer_colors[color] & 0x2; //10

    //Set pixel bit in the 8 bit pixel space
    if ((color_bit && this->colorInverted) || (!color_bit && !this->colorInverted)) {
        *color_pBuf &= ~(1 << (7 - pixel.ry % 8));
    } else {
        *color_pBuf |= (1 << (7 - pixel.ry % 8));
    }
    if ((black_bit && this->blackInverted) || (!black_bit && !this->blackInverted)) {
        *black_pBuf &= ~(1 << (7 - pixel.ry % 8));
    } else {
        *black_pBuf |= (1 << (7 - pixel.ry % 8));
    }

    #if SIKTEC_EPD_DEBUG_PIXELS
        PRINT_DEBUG_BUFFER("-> Write[B %u, C %u]\n", *black_pBuf, *color_pBuf);
    #endif

    //If its SRAM save it:
    if (this->use_sram) {
        this->sram->write8(pixel.sram_color, *color_pBuf);
        this->sram->write8(pixel.sram_black, *black_pBuf);
    }
}

/**
 * @brief check if pixel is in bounds of display area
 * 
 * @param x coordinate
 * @param y coordinate
 * @return bool
 */
bool SIKTEC_EPD::pixelInBounds(const int16_t x, const int16_t y) {
    return !((x < 0) || (x >= this->width()) || (y < 0) || (y >= this->height()));
}

/**
 * @brief calculates the address offset of x,y pixel coordinates
 * 
 * @param x coordinate
 * @param y coordinate
 * @return uint16_t the offset -> always positive can be 0
 */
uint16_t SIKTEC_EPD::getPixelAddressOffset(const int16_t x, const int16_t y) {
    return ((uint32_t)(this->fixed8_width - 1 - x) * (uint32_t)this->fixed8_height + y) / 8;
}

/**
 * @brief get a pixel address struct
 * 
 * @param x coordinate 
 * @param y coordinate 
 * @return SIKTEC_EPD::pixelAddress_t 
 */
SIKTEC_EPD::pixelAddress_t SIKTEC_EPD::getPixelAddress(const int16_t x, const int16_t y) {

    int16_t rx = x;
    int16_t ry = y;
    if (!this->pixelInBounds(x, y)) {
        return {
            false,
            0,
            rx,
            ry,
            0,
            0,
            nullptr,
            nullptr
        };
    }
    //Handle rotation first:
    switch (this->getRotation()) {
        case 1:
            EPD_swap(rx, ry);
            rx = this->fixed8_width - rx - 1;
            break;
        case 2:
            rx = this->fixed8_width - rx - 1;
            ry = this->fixed8_height - ry - 1;
            break;
        case 3:
            EPD_swap(rx, ry);
            ry = this->fixed8_height - ry - 1;
            break;
    }

    //Get offset:
    uint16_t offset = this->getPixelAddressOffset(rx, ry);
    if (this->use_sram) {
        return {
            true,
            offset,
            rx,
            ry,
            (uint16_t)(this->blackbuffer_addr + offset),
            (uint16_t)(this->colorbuffer_addr + offset),
            nullptr,
            nullptr
        };
    }
    return {
        true,
        offset,
        rx,
        ry,
        0x00,
        0x00,
        this->black_buffer + offset,
        this->color_buffer + offset,
    };
}


/**
 * @brief get pixel value at coordinates:
 * same as getPixelAddress
 * @param x coordinate
 * @param y coordinate
 * @return SIKTEC_EPD::pixelValue_t 
 */
SIKTEC_EPD::pixelValue_t SIKTEC_EPD::getPixel(const int16_t x, const int16_t y) {
    
    SIKTEC_EPD::pixelAddress_t pixel = this->getPixelAddress(x, y); // will handle bounds check also
    
    if (!this->use_sram) {
        return{
            pixel.inBound,                                          // bool     inBound;
            false,                                                  // bool     sram;
            (uint8_t)(pixel.ram_black != nullptr ? *pixel.ram_black : 0x0), // uint8_t black;
            (uint8_t)(pixel.ram_color != nullptr ? *pixel.ram_color : 0x0)  // uint8_t color;
        };
    }
    return {
        pixel.inBound,                                                // bool     inBound;
        true,                                                         // bool     sram;
        (uint8_t)(pixel.inBound ? this->sram->read8(pixel.sram_black) : 0x0),  // uint8_t black;
        (uint8_t)(pixel.inBound ? this->sram->read8(pixel.sram_color) : 0x0)   // uint8_t color;
    };
}

/**
 * @brief debugs a pixel -> prints what is actually saved for this pixel in both channels
 * 
 * @param x coordinate  
 * @param y coordinate 
 * @return void
 */
#if SIKTEC_EPD_DEBUG
void SIKTEC_EPD::debugPixel(const int16_t x, const int16_t y) {
    SIKTEC_EPD::pixelAddress_t pixel = this->getPixelAddress(x, y);
    char pbuf[90];
    if (pixel.inBound) {
        uint8_t black_c = 0, color_c = 0;
        if (this->use_sram) {
            black_c = this->sram->read8(pixel.sram_black);
            color_c = this->sram->read8(pixel.sram_color);
        } else {
            black_c = *pixel.ram_black;
            color_c = *pixel.ram_color;
        }
        PRINT_DEBUG_BUFFER("PIXEL[%u,%u]-TRANS[%u,%u] B %u::0x%#X C %u::0x%#X \n", x, y, pixel.rx, pixel.ry, black_c, black_c, color_c, color_c);
    } else {
        PRINT_DEBUG_BUFFER("PIXEL[%u,%u]-OUT OF BOUNDS", x, y);
    }
}
#endif

/**
 * @brief transfer the data in the buffer to epd ram:
 * 
 * @param framebuffer       the internal buffer to read from
 * @param framebuffer_size  the internal buffer size
 * @param EPDlocation       the location of the ram on EPD to write to
 * @param invertdata        should invert bytes? defaults to false -> invert is mainly done in drawpixel
 * 
 * @returns void
 */
void SIKTEC_EPD::writeRAMFramebufferToEPD(uint8_t *framebuffer, uint32_t framebuffer_size, uint8_t EPDlocation, bool invertdata) {

    //We want to control cs pin ourselfs so disable the lib auto toggling
    this->_spi->disableCsToggle();
    this->EPD_csLow();
    
    this->writeRAMCommand(EPDlocation);
    this->EPD_dc_mode(EPD_DATA_MODE);

    this->EPD_data(framebuffer, framebuffer_size, invertdata);

    this->EPD_csHigh();
    this->_spi->enableCsToggle();
}

/**
 * @brief transfer the data in the sram buffer to epd ram:
 * 
 * @param SRAM_buffer_addr the sram address to read from
 * @param buffer_size      the sram total bytes to read
 * @param EPDlocation      the location of the ram on EPD to write to
 * @param invertdata       should invert bytes? defaults to false -> invert is mainly done in drawpixel
 * 
 * @returns void
 */
void SIKTEC_EPD::writeSRAMFramebufferToEPD(uint16_t SRAM_buffer_addr, uint32_t buffer_size, uint8_t EPDlocation, bool invertdata) {

    //Single byte buffer:
    uint8_t c;

    //We want to control cs pins of both the epd and sram ourselfs so disable the lib auto toggling
    this->_spi->disableCsToggle();
    this->sram->disableCsToggle();

    //Make sure EPD is not interacting:
    this->EPD_csHigh();
    //put SRAM in read state:
    this->sram->csLow();
    //delay(10);

    //Put Sram in read mode cs low will still be high:
    this->sram->write(SRAM_buffer_addr, nullptr, 0, SIK_SRAM_READ);

    //Put the EPD in write mode c will have the first byte od data taken from Sram::
    this->EPD_csLow();
    c = this->writeRAMCommand(EPDlocation);
    
    //Epd now in data mode The SRAM also listens so this the first byte that is returned directly into the EPD 
    this->EPD_dc_mode(EPD_DATA_MODE);

    //get the transmitted data from SRAM and transfer it to epd simultaneously:
    this->_spi->beginTransaction();
    for (uint32_t i = 0; i < buffer_size; i++) {
        #if SIKTEC_EPD_DEBUG_SRAM
            if (c != 255 && c != 0 && i < SIKTEC_EPD_DEBUG_SRAM_READ_WRITE) { //Debug only none white pixels:
                PRINT_DEBUG_BUFFER("EPD SRAM Write [%u:%#X] -> %u:%#X \n", SRAM_buffer_addr + i, SRAM_buffer_addr + i, c, c);
            }
        #endif
        if (invertdata) {
            c = this->_spi->transfer(~c);
        } else {
            c = this->_spi->transfer(c);
        }
    }
    this->_spi->endTransaction();

    //Put both to high to stop listenning to spi:
    this->EPD_csHigh();
    this->sram->csHigh();

    //Reenable auto toggling:
    this->sram->enableCsToggle();
    this->_spi->enableCsToggle();
}

/**
 * @brief helper method to set Or reset init instructions and lut
 * 
 * @param code - pointer to instruction sequence
 * @param lut  - pointer to lut sequence
 * @param partial - is it in partial mode or not?
 * 
 * @returns void
 */
void SIKTEC_EPD::setInitAndLut(const uint8_t * code, const uint8_t * lut, bool partial) {
    this->setInitCode(code, partial);
    this->setLut(lut, partial);
}

/**
 * @brief helper method to set init instructions sequence
 * 
 * @param code - pointer to instruction sequence
 * @param partial - is it in partial mode or not?
 * 
 * @returns void
 */
void SIKTEC_EPD::setInitCode(const uint8_t * code, bool partial) {
    if (partial) {
        this->_epd_partial_init_code = code;
    } else {
        this->_epd_init_code = code;
    }
}

/**
 * @brief helper method to set lut sequence
 * 
 * @param lut  - pointer to lut sequence
 * @param partial - is it in partial mode or not?
 * 
 * @returns void
 */
void SIKTEC_EPD::setLut(const uint8_t * lut, bool partial) {
    if (partial) {
        this->_epd_partial_lut_code = lut;
    } else {
        this->_epd_lut_code = lut;
    }
} 

/**
 * @brief Transfer the data stored in the buffer(s) to the display 
 *        and will trigger the display (update) sequence.
 * 
 * @param sleep should we put the screen to sleep after update? defaults to false.
 * 
 * @returns void
 */
void SIKTEC_EPD::display(bool sleep) {

    #if SIKTEC_EPD_DEBUG
        Serial.println("Refreshing Display, Writing data.");
    #endif

    //First wakeup display:
    this->powerUp();

    // Set X & Y ram address: 
    this->setRAMAddress(0, 0);

    //Write buffer1 using sram or mem buffer:
    if (this->use_sram) {
        #if SIKTEC_EPD_DEBUG
            PRINT_DEBUG_BUFFER("Write SRAM buffer1 to EPD > address: %#X size: %u \n", this->buffer1_addr, this->buffer1_size);
        #endif
        this->writeSRAMFramebufferToEPD(this->buffer1_addr, this->buffer1_size, 0);
    } else {
        #if SIKTEC_EPD_DEBUG
            PRINT_DEBUG_BUFFER("Write RAM buffer1 to EPD > address: %#X size: %u \n", (uint32_t)this->buffer1, this->buffer1_size);
        #endif
        this->writeRAMFramebufferToEPD(this->buffer1, this->buffer1_size, 0);
    }

    if (this->buffer2_size != 0) {

        // we have a second buffer - transfer it:
        delay(10);

        // Set X & Y ram address: 
        this->setRAMAddress(0, 0);

        if (this->use_sram) {

            #if SIKTEC_EPD_DEBUG
                PRINT_DEBUG_BUFFER("Write SRAM buffer2 to EPD > address: %#X size: %u \n", this->buffer2_addr, this->buffer2_size);
            #endif
            this->writeSRAMFramebufferToEPD(this->buffer2_addr, this->buffer2_size, 1);

        } else {

            #if SIKTEC_EPD_DEBUG
                PRINT_DEBUG_BUFFER("Write RAM buffer2 to EPD > address: %#X size: %u \n", (uint32_t)this->buffer2, this->buffer2_size);
            #endif
            this->writeRAMFramebufferToEPD(this->buffer2, this->buffer2_size, 1);

        }
    }

    //Finished - update the screen now:
    #if SIKTEC_EPD_DEBUG
        Serial.println("Updating Screen....");
        float timer = (float)millis();
    #endif
    
    this->update();
    
    #if SIKTEC_EPD_DEBUG
        timer = ((float)millis() - timer) / 1000;
        PRINT_DEBUG_BUFFER("Update Done in %.2f seconds\n", timer);
    #endif

    //FUTURE -> remember we need to handle the partial count before full refresh
    // this->partialsSinceLastFullUpdate = 0; 
    if (sleep) {
        this->powerDown();
    }
}

/**
 * @brief Determine whether the black pixel data is the first or second buffer
 * 
 * @param index 0 or 1, for primary or secondary value
 * @param inverted Whether to invert the logical value
 * 
 * @returns void
*/
void SIKTEC_EPD::setBlackBuffer(int8_t index, bool inverted) {
    if (index == 0) {
        if (this->use_sram) {
            this->blackbuffer_addr = this->buffer1_addr;
        } else {
            this->black_buffer = this->buffer1;
        }
    }
    if (index == 1) {
        if (this->use_sram) {
            this->blackbuffer_addr = this->buffer2_addr;
        } else {
            this->black_buffer = this->buffer2;
        }
    }
    this->blackInverted = inverted;
}

/**
 * @brief Determine whether the color pixel data is the first or second buffer
 * 
 * @param index 0 or 1, for primary or secondary value
 * @param inverted Whether to invert the logical value
 * 
 * @returns void
*/
void SIKTEC_EPD::setColorBuffer(int8_t index, bool inverted) {
    if (index == 0) {
        if (this->use_sram) {
            this->colorbuffer_addr = this->buffer1_addr;
        } else {
            this->color_buffer = this->buffer1;
        }
    }
    if (index == 1) {
        if (this->use_sram) {
            this->colorbuffer_addr = this->buffer2_addr;
        } else {
            this->color_buffer = this->buffer2;
        }
    }
    this->colorInverted = inverted;
}

/**
 * @brief clear all pixels data buffers
 * 
 * @returns void
*/
void SIKTEC_EPD::clearBuffer() {
    if (this->use_sram) {
        if (this->blackInverted) {
            this->sram->erase(this->blackbuffer_addr, this->buffer1_size, 0xFF);
        } else {
            this->sram->erase(this->blackbuffer_addr, this->buffer1_size, 0x00);
        }
        #if SIKTEC_EPD_DEBUG
            Serial.println("Cleared black buffer.");
        #endif
        if (this->colorInverted) {
            this->sram->erase(this->colorbuffer_addr, this->buffer2_size, 0xFF);
        } else {
            this->sram->erase(this->colorbuffer_addr, this->buffer2_size, 0x00);
        }
        #if SIKTEC_EPD_DEBUG
            Serial.println("Cleared color buffer.");
        #endif
    } else {
        if (this->black_buffer) {
            if (this->blackInverted) {
                memset(this->black_buffer, 0xFF, this->buffer1_size);
            } else {
                memset(this->black_buffer, 0x00, this->buffer1_size);
            }
            #if SIKTEC_EPD_DEBUG
                Serial.println("Cleared black buffer.");
            #endif
        }
        if (this->color_buffer) {
            if (this->colorInverted) {
                memset(this->color_buffer, 0xFF, this->buffer2_size);
            } else {
                memset(this->color_buffer, 0x00, this->buffer2_size);
            }
            #if SIKTEC_EPD_DEBUG
                Serial.println("Cleared color buffer.");
            #endif
        }
    }
}

/**
 * @brief clear the display twice to remove any spooky ghost images
 * 
 * @returns void
*/
void SIKTEC_EPD::clearDisplay() {
    this->clearBuffer();
    this->display();
    delay(100);
    this->display();
}

/**
 * @brief Sendst a stream of commands to the epd
 *        This is mainly use for sequences make sure its terminated correctly:
 *        EPD_CMD_SEQUENCE_END         -> 0xFE -> End of commandlist
 *        EPD_CMD_SEQUENCE_WAIT        -> 0XFF -> Busy wait
 * @param init_code byte array of commands and data to send:
 * 
 * @returns void
 */
bool SIKTEC_EPD::EPD_commandList(const uint8_t *init_code) {

    //This is the command + data buffer 
    //its fixed in size and hopefully large enough lut block are usually 42 -> 57 
    uint8_t buf[64];
    bool busy_check = false;
    #if SIKTEC_EPD_DEBUG_COMMAND_LISTS
        Serial.println("Starting command sequence:");
    #endif

    while (init_code[0] != EPD_CMD_SEQUENCE_END) {
        uint8_t cmd = init_code[0];
        init_code++;
        uint8_t num_args = init_code[0];
        init_code++;
        //Busy wait instruction?
        if (cmd == EPD_CMD_SEQUENCE_WAIT) {
            if (this->busy_wait(num_args)) { // num args in this case is the ms to delay additionally to the busywait.
                continue;
            } else {
                return false;
            }
        }
        //Make sure we dont overflow:
        if (num_args > sizeof(buf)) {
            Serial.println("FATAL ERROR! - command list buffer is not large enough!");
            while (1) delay(10); //Halt the execution!
        }
        //Fill buffer
        for (int i = 0; i < num_args; i++) {
            buf[i] = init_code[0];
            init_code++;
        }

        #if SIKTEC_EPD_DEBUG_COMMAND_LISTS
            PRINT_DEBUG_BUFFER("  -> Sending EPD Command: %#X args %u \n", cmd, num_args);
        #endif
        //Send command & data:
        this->EPD_command(cmd, buf, num_args);
    }

    #if SIKTEC_EPD_DEBUG_COMMAND_LISTS
        Serial.println("Finished Sending command sequence.");
    #endif
    return true;
}

/**
 * @brief 
 * 
 * @param c    the command byte to send
 * @param buf  the buffer of data to send after the command:
 * @param len  the length of the data buffer
 * 
 * @returns void
*/
void SIKTEC_EPD::EPD_command(uint8_t c, const uint8_t *buf, uint16_t len) {

    //We first disable auto toggling to control cs ourselfs:
    this->_spi->disableCsToggle();
    this->EPD_csLow();
    
    //Write command:
    this->EPD_command(c);

    //Write the attached data:
    this->EPD_data(buf, len);
    
    this->EPD_csHigh();

    //Reenable auto toggling: 
    this->_spi->enableCsToggle();
}

/**
 * @brief send an EPD command with no data
 * 
 * @param c the command byte to send
 * 
 * @returns void
*/
void SIKTEC_EPD::EPD_command(uint8_t c) {

    this->EPD_dc_mode(EPD_COMMAND_MODE);
    
    uint8_t cmd  = c;
    
    (void)this->_spi->write(&cmd, 1);
}

/**
 * @brief send an EPD command with and get the replied byte
 * 
 * @param cmd the command byte to send
 * 
 * @returns uint8_t the replied byte:
*/
uint8_t SIKTEC_EPD::EPD_command_with_read(uint8_t cmd) {
    
    this->EPD_dc_mode(EPD_COMMAND_MODE);
    return this->_spi->write_and_read(cmd);
}

/**
 * @brief send an EPD command with and get the replied byte
 * 
 * @param cmd the command byte to send
 * @param buf output buffer
 * @param len expected read bytes length 
 * 
 * @returns void
*/
void SIKTEC_EPD::EPD_command_with_read(uint8_t cmd, uint8_t *buf, uint16_t len) {
    
    this->EPD_dc_mode(EPD_COMMAND_MODE);
    this->EPD_command(cmd);
    this->EPD_dc_mode(EPD_DATA_MODE);
    this->_spi->read(buf, len);

}

/**
 * @brief Send a stream of bytes to the EPD
 * 
 * @param buf the data buffer to send
 * @param len the length of the data buffer
 * @param invert should we invert the byte? defaults to false
 * 
 * @returns void
*/
void SIKTEC_EPD::EPD_data(const uint8_t *buf, size_t len, bool invert) {

    this->EPD_dc_mode(EPD_DATA_MODE);

    (void)this->_spi->write(buf, len, nullptr, 0, invert);
}

/**
 * @brief send a single data byte to EPD 
 * 
 * @param data the data byte to send
 * 
 * @returns void
*/
void SIKTEC_EPD::EPD_data(uint8_t data) {
    uint8_t d = data;
    this->EPD_data(&d, 1);
}

/** 
 * @brief drive EPD chip select pin high
 * 
 * @returns void 
*/
void SIKTEC_EPD::EPD_csHigh() {
    digitalWrite(this->pins.epd_cs, HIGH);
}

/** 
 * @brief drive EPD chip select pin low
 * 
 * @returns void 
*/
void SIKTEC_EPD::EPD_csLow() {
    digitalWrite(this->pins.epd_cs, LOW);
}

/** 
 * @brief set data/command pin low - LOW = COMMAND | HIGH = DATA
 * 
 * @param mode the required input mode
 * 
 * @returns void 
*/
void SIKTEC_EPD::EPD_dc_mode(uint8_t mode) {
    digitalWrite(this->pins.dc, mode);
}

/**
 * @brief checks if the Epd is powered - that only checking for a flag 
 *        that means a boolean is toggled every time power methods are called.
 * 
 * @returns true - is powered
 * @returns false - not powered
 */
bool SIKTEC_EPD::EPD_isPowered() {
    return this->epdPower;
}


/**
 * @brief DEBUG_STUFF - will print to serial the mem buffer:
 * 
 * @returns void 
*/
void SIKTEC_EPD::_print_debug_byte(uint16_t addr, uint8_t value, bool new_line, Stream *SerialPort) {
    SerialPort->print("[0x");
    SerialPort->print(addr, HEX);
    SerialPort->print("] : ");
    SerialPort->print(value, DEC);
    SerialPort->print(" | 0x");
    SerialPort->print(value, HEX);
    SerialPort->print(" | ");
    SerialPort->print(value, BIN);
    if (new_line) SerialPort->println();
}
void SIKTEC_EPD::_display_buffer(uint16_t from_addr, uint8_t cols, int length, Stream *SerialPort) {
    uint8_t current_col = 1;
    uint32_t upto_addr = from_addr + length;
    for (uint16_t i = from_addr; i < upto_addr; i++) {
        uint8_t value = this->use_sram ? this->sram->read8(i) : this->black_buffer[i];
        if (current_col < cols) {
            this->_print_debug_byte(i, value, false, SerialPort);
            SerialPort->print(",\t");
            current_col += 1;
        } else {
            this->_print_debug_byte(i, value, true, SerialPort);
            current_col = 1;
        }
    }
}


#if SIKTEC_EPD_DEBUG
    /**
     * @brief Scans the SRAM chip to try and calculate the total RAM size
     *        Will work only with SEQUENTIAL MODE.
     *        Currently wil handle up to 512 Kib scanning
     *        > this is extremely costly so use only for debugging with care
     * @param print print result or not
     * @param SerialPort the Stream interface to use for printing
     * @return uint32_t total addressable bytes
     */
    uint32_t SIKTEC_EPD::analyzeSRAMsize(const bool print, Stream *SerialPort) {
        uint8_t signature[] = { 9, 9, 9, 8, 8, 8, 100, 100 };
        uint8_t seen[8]    = { 0 };
        uint32_t signa_len = sizeof(signature) / sizeof(signature[0]);
        uint32_t actual_size;
        bool found = false;
        this->sram->write(0x0, signature, signa_len);
        for (int i = signa_len; i < 65535; i += signa_len) {
            this->sram->read(i, seen, signa_len);
            actual_size = i;
            if (memcmp(signature, seen, signa_len) == 0) {
                found = true;
                break;
            }
        }
        if (print) {
            SerialPort->print("SRAM SIZE: ");
            if (found) 
                SerialPort->println(actual_size);
            else
                SerialPort->println("UNKNOWN");
        }
        return actual_size;
    }
#endif

}