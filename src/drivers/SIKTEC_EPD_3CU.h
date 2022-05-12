/******************************************************************************/
// Created by: SIKTEC.
// Release Version : 1.0.2
// Creation Date: 2022-04-12
// Copyright 2022, SIKTEC.
// 
/******************************************************************************/
/*****************************      NOTES       *******************************
    -> driver UC8276 -> SIKTEC_EPD_3CU
*******************************************************************************/
/*****************************      Changelog       ****************************
1.0.1:
    -> initial release
1.0.2:
    -> Improved drivers layout - all init lut and instructions moved to a header file.
    -> Fixed Arduino DUE complianing and miscalculating buffer sizes.
    -> Improved debugging methods.

*******************************************************************************/

/**  @file SIKTEC_EPD_3CU.h */
#pragma once 

#include "SIKTEC_EPD_3CU_inst_lut.h"

//------------------------------------------------------------------------//
// GENERAL EPD CONSTANTS:
//------------------------------------------------------------------------//
#define EPD_3CU_BUSY_DELAY    500
#define EPD_3CU_REFRESH_DELAY 13000 
#define EPD_3CU_WIDTH         300
#define EPD_3CU_HEIGHT        400
#define EPD_3CU_RAM_SIZE_Kib 256

namespace SIKtec {

extern char debug_message[];
extern const int debug_message_len;

//------------------------------------------------------------------------//
// DRIVER WRAPPER Definition:
//------------------------------------------------------------------------//
/**
 * @brief  Class for interfacing with SIKTEC_EPD_3CU EPD driver And on board perephirials
 */
class SIKTEC_EPD_3CU : public SIKTEC_EPD {

public:

    /** 
     * @brief The SIKTEC EPD constructor when you you define your own SPI pins.
     * 
     * @param CS    the chip select pin to use
     * @param SRCS  the SRAM chip select pin to use
     * @param DC    the data/command pin to use
     * @param RST   the reset pin to use
     * @param BUSY  the busy pin to use
     * @param SCLK  the SCLK pin to use
     * @param MISO  the MISO pin to use
     * @param MOSI  the SID pin to use
    */
    inline SIKTEC_EPD_3CU(
        int8_t CS, int8_t SRCS, int8_t DC, int8_t RST, int8_t BUSY, 
        int8_t spi_clock, int8_t spi_miso,  int8_t spi_mosi
    ) : SIKTEC_EPD(EPD_3CU_WIDTH, EPD_3CU_HEIGHT, CS, SRCS, DC, RST, BUSY, spi_clock, spi_miso, spi_mosi) {
        this->_init();
    }
    
    /** 
     * @brief The SIKTEC EPD constructor when you you define your own SPI pins.
     * 
     * @param pins  the epd & sram pins
     * @param SCLK  the SCLK pin to use
     * @param MISO  the MISO pin to use
     * @param MOSI  the SID pin to use
    */
    inline SIKTEC_EPD_3CU(
        const epd_pins_t &pins, 
        int8_t spi_clock, int8_t spi_miso,  int8_t spi_mosi
    ) : SIKTEC_EPD(EPD_3CU_WIDTH, EPD_3CU_HEIGHT, pins, spi_clock, spi_miso, spi_mosi) {
        this->_init();
    }
    
    /** 
     * @brief The SIKTEC EPD constructor when you you define the default Hardware SPI.
     * 
     * @param CS    the chip select pin to use
     * @param SRCS  the SRAM chip select pin to use
     * @param DC    the data/command pin to use
     * @param RST   the reset pin to use
     * @param BUSY  the busy pin to use
    */
    inline SIKTEC_EPD_3CU(
        int8_t CS, int8_t SRCS, int8_t DC, int8_t RST, int8_t BUSY
    ) : SIKTEC_EPD(EPD_3CU_WIDTH, EPD_3CU_HEIGHT, CS, SRCS, DC, RST, BUSY) {
        this->_init();
    }
    
    /** 
     * @brief The SIKTEC EPD constructor when you you define the default Hardware SPI.
     * 
     * @param pins  the epd & sram pins
    */
    inline SIKTEC_EPD_3CU(const epd_pins_t &pins) : SIKTEC_EPD(EPD_3CU_WIDTH, EPD_3CU_HEIGHT, pins) {
        this->_init();
    }


private:

    /**
     * @brief initialize this class - use by constructors.
     * will set buffers and allocate memory if needed.
     * 
     * @returns void
     */
    inline void _init() {
        // Set buffers size: 
        //SH: added cast to uint32 to fix bugs with compilers overflowing this basically trying to result the multiplication to uint16 - observed on leonardo 
        this->buffer1_size = (uint32_t)this->fixed8_width * this->fixed8_height / 8;
        this->buffer2_size = this->buffer1_size;

        #ifdef SIKTEC_EPD_DEBUG
            sprintf(
                debug_message, 
                "Allocating Buffer:\n   - screen -> %u,%u\n   - fixed -> %u,%u\n   - buf1 -> %u\n   - buf2 -> %u\n ",
                this->epd_width, this->epd_height, this->fixed8_width, this->fixed8_height,
                this->buffer1_size, this->buffer2_size
            );
            Serial.print(debug_message);
        #endif

        if (this->use_sram) {

            // We split the space to two each byte is indexed and the address is this linear index:
            this->buffer1_addr = 0;
            this->buffer2_addr = this->buffer1_size;

            #ifdef SIKTEC_EPD_DEBUG
                Serial.println("Allocating on SRAM");
            #endif

        } else {
            this->buffer1 = (uint8_t *)malloc(this->buffer1_size); // 15,000 bytes
            this->buffer2 = (uint8_t *)malloc(this->buffer2_size); // 15,000 bytes

            #ifdef SIKTEC_EPD_DEBUG
                Serial.println("Allocating on RAM");
            #endif

        }
    }

public:

    uint32_t epd_width    = EPD_3CU_WIDTH;  //!< the definition width.
    uint32_t epd_height   = EPD_3CU_HEIGHT; //!< the definition height.

    /**
     * @brief start and initialize the epd.
     * 
     * will set buffers (COLOR, BLACK) inverted modes.
     * the rotation and will powerdown the display
     * 
     * @param mode the EPD mode is also used for gfx.
     * 
     * @returns void
     */
    inline void begin(epd_mode_t mode = EPD_MODE_TRICOLOR) {
        SIKTEC_EPD::begin(true);

        this->inkmode = mode;

        this->setColorBuffer(1, false); // red defaults to un inverted
        this->setBlackBuffer(0, true);  // black defaults to inverted

        this->default_refresh_delay = EPD_3CU_REFRESH_DELAY; //ms

        this->setInitAndLut(); // defaults to nullptr

        this->layer_colors[EPD_WHITE] = 0b00; 
        this->layer_colors[EPD_BLACK] = 0b01; 
        this->layer_colors[EPD_RED]   = 0b10; 
        this->layer_colors[EPD_GRAY]  = 0b10; 
        this->layer_colors[EPD_DARK]  = 0b10; 
        this->layer_colors[EPD_LIGHT] = 0b00; 

        this->setRotation(1);
        this->powerDown();
    }

    /**
     * @brief start up the display power up will send command list of init code
     * 
     * @returns void
     */
    inline void powerUp() {

        #ifdef SIKTEC_EPD_DEBUG
            Serial.println("Powering Up display");
        #endif

        //Check if we really need to powerUp:
        if (this->epdPower) return;

        //First hard reset:
        this->hardwareResetEPD();

        const uint8_t *init_code = uc8276_default_init_code;

        if (this->_epd_init_code != NULL) {
            init_code = this->_epd_init_code;
        }
        this->EPD_commandList(init_code);
        
        this->epdPower = true;

        delay(20);
    }

    /**
     * @brief signal the display to update meaning full refresh
     * 
     * @returns void
     */
    inline void update(void) {
        this->EPD_command(UC8276_DISPLAY_REFRESH);
        delay(50);
        this->busy_wait();
        if (this->pins.busy <= -1) {
            delay(this->default_refresh_delay);
        }
    }

    /**
     * @brief power down the display - deep sleep if possible
     * 
     * @returns void
     */
    inline void powerDown() {

        #ifdef SIKTEC_EPD_DEBUG
            Serial.println("Powering Down display");
        #endif

        //Check if we really need to powerUp:
        if (!this->epdPower) return;

        uint8_t buf[1];
        buf[0] = 0xF7; // disable VCOM
        this->EPD_command(UC8276_WRITE_VCOM, buf, 1);
        this->EPD_command(UC8276_POWEROFF);
        this->busy_wait();

        // Only deep sleep if we can get out of it
        if (this->pins.rst >= 0) {
            buf[0] = 0xA5;
            this->EPD_command(UC8276_DEEPSLEEP, buf, 1);
        }

        this->epdPower = false;

        delay(100);
    }

protected:

    /**
     * @brief Send the specific command to start writing to EPD display RAM
     * 
     * This will return a byte that is read while sending the data -> its usefull when bridging two SPI devices 
     * 
     * @param index The index for which buffer to write (0 or 1 or tri-color displays) Ignored for monochrome displays.
     * 
     * @returns The byte that is read from SPI at the same time as sending the command
     */
    inline uint8_t writeRAMCommand(uint8_t index) {
        if (index == 0) {
            return this->EPD_command_with_read(UC8276_WRITE_RAM1);
        }
        if (index == 1) {
            return this->EPD_command_with_read(UC8276_WRITE_RAM2);
        }
        return 0;
    }

    /**
     * @brief Some displays require setting the RAM address pointer
     * 
     * @param x X address counter value
     * @param y Y address counter value
     * 
     * @returns void
     */
    inline void setRAMAddress(uint16_t x, uint16_t y) {
        // not used in this chip!
        (void)x;
        (void)y;
    }

    /**
     * @brief Some displays require setting the RAM address pointer
     * 
     * @param x X address counter value
     * @param y Y address counter value
     * 
     * @returns void
     */
    inline void setRAMWindow(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2) {
        // not used in this chip!
        (void)x1;
        (void)y1;
        (void)x2;
        (void)y2;
    }
    
    /**
     * @brief  wait for busy signal to end
     * 
     * busy pin is LOW while Driver is working.
     * wait for HIGH.
     * @returns void
     */
    inline void busy_wait(uint16_t moredelay = 0) {
        if (this->pins.busy >= 0) {
            while (!digitalRead(this->pins.busy)) { // wait for busy HIGH
                this->EPD_command(UC8276_GET_STATUS);
                delay(100);
            }
        } else {
            delay(EPD_3CU_BUSY_DELAY);
        }
        if (moredelay > 0) {
            delay(moredelay);
        }
    }

};

}
