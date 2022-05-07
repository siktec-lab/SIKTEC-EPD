
/******************************************************************************/
// Created by: SIKTEC.
// Release Version : 1.0.1
// Creation Date: 2022-03-31
// Copyright 2022, SIKTEC.
/******************************************************************************/

/** @file SIKTEC_EPD.cpp */

#include "SIKTEC_EPD.h"
#include <stdlib.h>

namespace SIKtec {


bool SIKTEC_EPD::_isInTransaction = false;

/**
 * @brief constructor if using external SRAM chip and software SPI
 * 
 * @param width the width of the display in pixels
 * @param height the height of the display in pixels
 * @param spi_mosi the SID pin to use
 * @param spi_clock the SCLK pin to use
 * @param DC the data/command pin to use
 * @param RST the reset pin to use
 * @param CS the chip select pin to use
 * @param SRCS the SRAM chip select pin to use
 * @param spi_miso the MISO pin to use
 * @param BUSY the busy pin to use
*/
SIKTEC_EPD::SIKTEC_EPD(int width, int height, int8_t spi_mosi, int8_t spi_clock, int8_t DC, int8_t RST, int8_t CS, int8_t SRCS, int8_t spi_miso, int8_t BUSY)
                        : Adafruit_GFX(width, height) {
    this->_cs_pin       = CS;
    this->_reset_pin    = RST;
    this->_dc_pin       = DC;
    this->_busy_pin     = BUSY;

    //Initiate SRAM:
    if (SRCS >= 0) {
        this->sram = new SIKTEC_SRAM(spi_mosi, spi_miso, spi_clock, SRCS, 4000000); 
        this->sram->begin();
        //Put in seq mode and check sram is ready to go:
        this->use_sram = this->sram->set_mode(SRAM_MODE::SRAM_SEQ_MODE);
        if (!this->use_sram) {
            Serial.print("SRAM MODE FAILED");
        }
    } else {
        this->use_sram = false;
    }

    //Set spi device:
    this->_spi = new SIKTEC_SPI(
        CS, spi_clock, spi_miso, spi_mosi, 
        4000000,                // frequency
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
    this->buffer1           = NULL;
    this->buffer2           = NULL;
    this->color_buffer      = NULL;
    this->black_buffer      = NULL;

    #ifdef SIKTEC_EPD_DEBUG
        Serial.println("initialized SIKTEC_EPD");
    #endif
}

/**
 * @brief constructor if using on-chip RAM and hardware SPI
 * 
 * @param width the width of the display in pixels
 * @param height the height of the display in pixels
 * @param DC the data/command pin to use
 * @param RST the reset pin to use
 * @param CS the chip select pin to use
 * @param SRCS the SRAM chip select pin to use
 * @param BUSY the busy pin to use
 * @param spi the SPI bus to use
*/
SIKTEC_EPD::SIKTEC_EPD(int width, int height, int8_t DC, int8_t RST, int8_t CS, int8_t SRCS, int8_t BUSY, SPIClass *spi)
            : Adafruit_GFX(width, height) {
    this->_cs_pin       = CS;
    this->_reset_pin    = RST;
    this->_dc_pin       = DC;
    this->_busy_pin     = BUSY;

    //Initiate SRAM:
    if (SRCS >= 0) {
        this->sram = new SIKTEC_SRAM(SRCS, spi); 
        this->sram->begin();
        this->use_sram = this->sram->set_mode(SRAM_MODE::SRAM_SEQ_MODE);
        //Put in seq mode and check sram is ready to go:
        if (!this->use_sram) {
            Serial.println("SRAM MODE FAILED");
            this->sram->print_status();
        }
    } else {
        this->use_sram = false;
    }

    //Set spi device:
    this->_spi = new SIKTEC_SPI(
        CS, 4000000,            // frequency
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
    this->buffer1           = NULL;
    this->buffer2           = NULL;
    this->color_buffer      = NULL;
    this->black_buffer      = NULL;

    #ifdef SIKTEC_EPD_DEBUG
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
 * @brief begin communication with EPD and set up the display.
 * 
 * @param reset if true the reset pin will be toggled.
 * 
 * @returns void
*/
void SIKTEC_EPD::begin(bool reset) {
    
    #ifdef SIKTEC_EPD_DEBUG
        Serial.println("EPD Begin procedure");
    #endif

    //Set buffers:
    this->setBlackBuffer(0, true);  // black defaults to inverted
    this->setColorBuffer(1, false); // red defaults to not inverted

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
    pinMode(this->_dc_pin, OUTPUT);
    
    //Do a harware reset:
    if (reset) {
        this->hardwareResetEPD();
    }

    //Set the epd busy pin:
    if (this->_busy_pin >= 0) {
        pinMode(_busy_pin, INPUT);
    }

}

/**
 * @brief reset Perform a hardware reset
 * 
 * @returns void
*/
void SIKTEC_EPD::hardwareResetEPD() {
    
    #ifdef SIKTEC_EPD_DEBUG
        Serial.print("Performing Hardware Reset - ");
    #endif

    if (this->_reset_pin >= 0) {
        #ifdef SIKTEC_EPD_DEBUG
            Serial.print("Starting...");
        #endif
        // Setup reset pin direction
        pinMode(this->_reset_pin, OUTPUT);

        // VDD (3.3V) goes high at start, lets just chill for a ms
        digitalWrite(this->_reset_pin, HIGH);
        delay(10);
        
        // bring reset low
        digitalWrite(this->_reset_pin, LOW);
        
        // wait 10ms
        delay(10);
        
        // bring out of reset
        digitalWrite(this->_reset_pin, HIGH);
        delay(10);
    }
    #ifdef SIKTEC_EPD_DEBUG
        else {
            Serial.print("Skipping...");
        }
    #endif
    #ifdef SIKTEC_EPD_DEBUG
        Serial.println("Done!");
    #endif
}

/**
 * @brief draw a single pixel on the screen
 * this is the virtual override of gfx derived lib and all drawing
 * methods will call this.
 * 
 * @param x the x axis position
 * @param y the y axis position
 * @param color the color of the pixel
 * 
 * @returns void
*/
void SIKTEC_EPD::drawPixel(int16_t x, int16_t y, uint16_t color) {
    
    //If pixel is outside of panel avoid:
    if ((x < 0) || (x >= this->width()) || (y < 0) || (y >= this->height()))
        return;

    uint8_t  *black_pBuf;
    uint8_t  *color_pBuf;

    // deal with non-8-bit heights
    uint16_t _HEIGHT = this->HEIGHT;
    if (_HEIGHT % 8 != 0) {
        _HEIGHT += 8 - (_HEIGHT % 8);
    }

    // check rotation, move pixel around if necessary
    switch (this->getRotation()) {
        case 1:
            EPD_swap(x, y);
            x = this->WIDTH - x - 1;
            break;
        case 2:
            x = this->WIDTH - x - 1;
            y = _HEIGHT - y - 1;
            break;
        case 3:
            EPD_swap(x, y);
            y = _HEIGHT - y - 1;
            break;
    }

    //The address is based on the coordinates and color type:
    uint16_t addr = ((uint32_t)(this->WIDTH - 1 - x) * (uint32_t)_HEIGHT + y) / 8; // 20,20 200,400: 0011 1001 0101 0010
    uint8_t black_c, color_c;

    //Save the memory address of black and color to temp bufs:
    if (this->use_sram) {
        black_c = this->sram->read8(this->blackbuffer_addr + addr);
        black_pBuf = &black_c;
        color_c = this->sram->read8(this->colorbuffer_addr + addr);
        color_pBuf = &color_c;

        #ifdef SIKTEC_EPD_DEBUG_PIXELS
            Serial.print("Got pixel reads: B ");
            Serial.print(black_c);
            Serial.print(", C ");
            Serial.print(color_c);
            Serial.print(" | to address: B ");
            Serial.print(this->blackbuffer_addr);
            Serial.print(", C ");
            Serial.print(this->colorbuffer_addr);
            Serial.print(" | to offset: ");
            Serial.print(addr);
        #endif

    } else {

        color_pBuf = this->color_buffer + addr;
        black_pBuf = this->black_buffer + addr;

        #ifdef SIKTEC_EPD_DEBUG_PIXELS
            Serial.print("Got pixel reads: B ");
            Serial.print(*black_pBuf);
            Serial.print(", C ");
            Serial.print(*color_pBuf);
            Serial.print(" | to offset: ");
            Serial.print(addr);
        #endif

    }

    bool black_bit = this->layer_colors[color] & 0x1; //01
    bool color_bit = this->layer_colors[color] & 0x2; //10

    if ((color_bit && this->colorInverted) || (!color_bit && !this->colorInverted)) {
        *color_pBuf &= ~(1 << (7 - y % 8));
    } else {
        *color_pBuf |= (1 << (7 - y % 8));
    }

    if ((black_bit && this->blackInverted) || (!black_bit && !this->blackInverted)) {
        *black_pBuf &= ~(1 << (7 - y % 8));
    } else {
        *black_pBuf |= (1 << (7 - y % 8));
    }

    #ifdef SIKTEC_EPD_DEBUG_PIXELS
        Serial.print(" | Write: B");
        Serial.print(*black_pBuf);
        Serial.print(", C ");
        Serial.println(*color_pBuf);
    #endif
    
    if (this->use_sram) {
        this->sram->write8(this->colorbuffer_addr + addr, *color_pBuf);
        this->sram->write8(this->blackbuffer_addr + addr, *black_pBuf);
    }
}

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
        #ifdef SIKTEC_EPD_DEBUG_SRAM_READ_WRITE
            if (SRAM_buffer_addr >= 15 && i < SIKTEC_EPD_DEBUG_SRAM_READ_WRITE) {
                Serial.print("write -> ");
                Serial.print(c);
            }
        #endif
        if (invertdata) {
            c = this->_spi->transfer(~c);
        } else {
            c = this->_spi->transfer(c);
        }
        #ifdef SIKTEC_EPD_DEBUG_SRAM_READ_WRITE
            if (SRAM_buffer_addr >= 15 && i < SIKTEC_EPD_DEBUG_SRAM_READ_WRITE) {
                Serial.print(" | read -> ");
                Serial.println(c);
            }
        #endif
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
 * @brief Transfer the data stored in the buffer(s) to the display 
 *        and will trigger the display (update) sequence.
 * 
 * @param sleep should we put the screen to sleep after update? defaults to false.
 * 
 * @returns void
 */
void SIKTEC_EPD::display(bool sleep) {

    #ifdef SIKTEC_EPD_DEBUG
        Serial.println("Refreshing Display, Writing data.");
    #endif

    //First wakeup display:
    this->powerUp();

    // Set X & Y ram address: 
    this->setRAMAddress(0, 0);

    //Write buffer1 using sram or mem buffer:
    if (this->use_sram) {
        #ifdef SIKTEC_EPD_DEBUG
            Serial.printf(
                " > Write SRAM buffer1 to EPD:\n     - Address: %10x | Size: %d \n",
                this->buffer1_addr, this->buffer1_size
            );
        #endif
        this->writeSRAMFramebufferToEPD(this->buffer1_addr, this->buffer1_size, 0);
    } else {
        #ifdef SIKTEC_EPD_DEBUG
            Serial.printf(
                    " > Write RAM buffer1 to EPD:\n     - Address: %p | Size: %d \n",
                    this->buffer1, this->buffer1_size
            );
        #endif
        this->writeRAMFramebufferToEPD(this->buffer1, this->buffer1_size, 0);
    }

    if (this->buffer2_size != 0) {
        // we have a second buffer - transffer it:
        delay(2);
        // Set X & Y ram address: 
        this->setRAMAddress(0, 0);
        if (this->use_sram) {
            #ifdef SIKTEC_EPD_DEBUG
                Serial.printf(
                    " > Write SRAM buffer2 to EPD:\n     - Address: %10x | Size: %d \n",
                    this->buffer2_addr, this->buffer2_size
                );
            #endif
            this->writeSRAMFramebufferToEPD(this->buffer2_addr, this->buffer2_size, 1);
        } else {
            #ifdef SIKTEC_EPD_DEBUG
                Serial.printf(
                        " > Write RAM buffer2 to EPD:\n     - Address: %p | Size: %d \n",
                        this->buffer2, this->buffer2_size
                );
            #endif
            this->writeRAMFramebufferToEPD(this->buffer2, this->buffer2_size, 1);
        }
    }

    //Finished - update the screen now:
    this->update();
    // this->partialsSinceLastFullUpdate = 0; //TODO -> remember we need to handle the partial count before full refresh

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
        #ifdef SIKTEC_EPD_DEBUG
            Serial.println("Cleared black buffer.");
        #endif
        if (this->colorInverted) {
            this->sram->erase(this->colorbuffer_addr, this->buffer2_size, 0xFF);
        } else {
            this->sram->erase(this->colorbuffer_addr, this->buffer2_size, 0x00);
        }
        #ifdef SIKTEC_EPD_DEBUG
            Serial.println("Cleared color buffer.");
        #endif
    } else {
        if (this->black_buffer) {
            if (this->blackInverted) {
                memset(this->black_buffer, 0xFF, this->buffer1_size);
            } else {
                memset(this->black_buffer, 0x00, this->buffer1_size);
            }
            #ifdef SIKTEC_EPD_DEBUG
                    Serial.println("Cleared black buffer.");
            #endif
        }
        if (this->color_buffer) {
            if (this->colorInverted) {
                memset(this->color_buffer, 0xFF, this->buffer2_size);
            } else {
                memset(this->color_buffer, 0x00, this->buffer2_size);
            }
            #ifdef SIKTEC_EPD_DEBUG
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
 * This is mainly use for sequences make sure its terminated correctly:
 * EPD_CMD_SEQUENCE_END         -> 0xFE -> End of commandlist
 * EPD_CMD_SEQUENCE_WAIT_BUSY   -> 0XFF -> Busy wait
 * 
 * @param init_code byte array of commands and data to send:
 */
void SIKTEC_EPD::EPD_commandList(const uint8_t *init_code) {

    //This is the command + data buffer 
    //its fixed in size and hopefully large enough
    uint8_t buf[64];

    while (init_code[0] != EPD_CMD_SEQUENCE_END) {
        uint8_t cmd = init_code[0];
        init_code++;
        uint8_t num_args = init_code[0];
        init_code++;
        //Busy wait instruction?
        if (cmd == EPD_CMD_SEQUENCE_WAIT_BUSY) {
            this->busy_wait();
            continue;
        }
        //Make sure we dont overflow:
        if (num_args > sizeof(buf)) {
            Serial.println("FATAL ERROR - command list buffer is not large enough!");
            while (1) delay(10); //Halt the execution!
        }
        //Fill buffer
        for (int i = 0; i < num_args; i++) {
            buf[i] = init_code[0];
            init_code++;
        }
        //Send command & data:
        this->EPD_command(cmd, buf, num_args);
    }
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
    digitalWrite(this->_cs_pin, HIGH);
}

/** 
 * @brief drive EPD chip select pin low
 * 
 * @returns void 
*/
void SIKTEC_EPD::EPD_csLow() {
    digitalWrite(this->_cs_pin, LOW);
}

/** 
 * @brief set data/command pin low - LOW = COMMAND | HIGH = DATA
 * 
 * @param mode the required input mode
 * 
 * @returns void 
*/
void SIKTEC_EPD::EPD_dc_mode(uint8_t mode) {
    digitalWrite(this->_dc_pin, mode);
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
    uint8_t value = 0;
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

}