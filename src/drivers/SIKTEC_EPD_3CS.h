/******************************************************************************/
// Created by: SIKTEC.
// Release Version : 1.0.4
// Creation Date: 2022-04-12
// Copyright 2022, SIKTEC.
// 
/******************************************************************************/
/*****************************      NOTES       *******************************
    -> driver SSD1619 -> SIKTEC_EPD_3CS
    -> Maximum SPI write speed 20MHz is the absolute max for the SSD1619
    -> Supports internal temp read.
    - SSD1619 ->VCI: 2.2 to 3.7V
*******************************************************************************/
/*****************************      Changelog       ****************************
1.0.1:
    -> initial release
1.0.2:
    -> Improved drivers layout - all init lut and instructions moved to a header file.
    -> Improved init code for 3CS board.
    -> Fixed Arduino DUE complianing and miscalculating buffer sizes.
    -> Improved debugging methods.

*******************************************************************************/

/**  @file SIKTEC_EPD_3CS.h */
#pragma once

#include "SIKTEC_EPD_3CS_inst_lut.h"

//------------------------------------------------------------------------//
// GENERAL EPD CONSTANTS:
//------------------------------------------------------------------------//
#define EPD_3CS_BUSY_DELAY       500
#define EPD_3CS_BUSY_RETRY_TIMES 75
#define EPD_3CS_REFRESH_DELAY    15000 
#define EPD_3CS_WIDTH            300
#define EPD_3CS_HEIGHT           400
#define EPD_3CS_RAM_SIZE_Kib     256

#ifndef PRINT_DEBUG_BUFFER
#define PRINT_DEBUG_BUFFER(__template, ...) \
    sprintf(debug_message, __template, __VA_ARGS__); \
    Serial.print(debug_message)
#endif

namespace SIKtec {

extern char debug_message[];
extern const int debug_message_len;

//------------------------------------------------------------------------//
// DRIVER WRAPPER Definition:
//------------------------------------------------------------------------//

/**
 * @brief  Class for interfacing with SIKTEC_EPD_3CS EPD driver And on board perephirials 
 */
class SIKTEC_EPD_3CS : public SIKTEC_EPD {

public:

    /** 
     * @brief The SIKTEC EPD constructor when you you define your own SPI pins.
     * 
     * @param CS    the chip select pin to use
     * @param SRCS  the SRAM chip select pin to use
     * @param DC    the data/command pin to use
     * @param RST   the reset pin to use
     * @param BUSY  the busy pin to use
     * @param spi_clock  the SCLK pin to use
     * @param spi_miso  the MISO pin to use
     * @param spi_mosi  the SID pin to use
     * @param clock_frequency the spi bus frequency -> for sram and epd controller
    */
    inline SIKTEC_EPD_3CS(
        int8_t CS, int8_t SRCS, int8_t DC, int8_t RST, int8_t BUSY, 
        int8_t spi_clock, int8_t spi_miso,  int8_t spi_mosi, uint32_t clock_frequency = EPD_SRAM_SPEED
    ) : SIKTEC_EPD(EPD_3CS_WIDTH, EPD_3CS_HEIGHT, CS, SRCS, DC, RST, BUSY, spi_clock, spi_miso, spi_mosi, clock_frequency) {
        this->_init();
    }
    /** 
     * @brief The SIKTEC EPD constructor when you you define your own SPI pins.
     * 
     * @param pins  the epd & sram pins
     * @param spi_clock  the SCLK pin to use
     * @param spi_miso  the MISO pin to use
     * @param spi_mosi  the SID pin to use
     * @param clock_frequency the spi bus frequency -> for sram and epd controller
    */
    inline SIKTEC_EPD_3CS(
        const epd_pins_t &pins, 
        int8_t spi_clock, int8_t spi_miso,  int8_t spi_mosi, uint32_t clock_frequency = EPD_SRAM_SPEED
    ) : SIKTEC_EPD(EPD_3CS_WIDTH, EPD_3CS_HEIGHT, pins, spi_clock, spi_miso, spi_mosi, clock_frequency) {
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
     * @param clock_frequency the spi bus frequency -> for sram and epd controller
     * @param spi the SPI bus to use
    */
    inline SIKTEC_EPD_3CS(
        int8_t CS, int8_t SRCS, int8_t DC, int8_t RST, int8_t BUSY,
        uint32_t clock_frequency = EPD_SRAM_SPEED,
        SPIClass *spi = &SPI
    ) : SIKTEC_EPD(EPD_3CS_WIDTH, EPD_3CS_HEIGHT, CS, SRCS, DC, RST, BUSY, spi, clock_frequency) {
        this->_init();
    }
    
    /** 
     * @brief The SIKTEC EPD constructor when you you define the default Hardware SPI.
     * 
     * @param pins  the epd & sram pins
     * @param clock_frequency the spi bus frequency -> for sram and epd controller
     * @param spi the SPI bus to use
    */
    inline SIKTEC_EPD_3CS(
        const epd_pins_t &pins, 
        uint32_t clock_frequency = EPD_SRAM_SPEED,
        SPIClass *spi = &SPI
    ) : SIKTEC_EPD(EPD_3CS_WIDTH, EPD_3CS_HEIGHT, pins, spi, clock_frequency) {
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
        //NOTE: shlomi - added cast to uint32 to fix bugs with compilers overflowing this basically trying to result the multiplication to uint16 - observed on leonardo 
        this->buffer1_size = (uint32_t)this->fixed8_width * this->fixed8_height / 8;
        this->buffer2_size = this->buffer1_size;

        #if SIKTEC_EPD_DEBUG
            PRINT_DEBUG_BUFFER(
                "Allocating Buffer:\n   - screen -> %u,%u\n   - fixed -> %u,%u\n   - buf1 -> %u\n   - buf2 -> %u\n",
                this->epd_width, this->epd_height, this->fixed8_width, this->fixed8_height,
                this->buffer1_size, this->buffer2_size
            );
        #endif

        if (this->use_sram) {
            // We split the space to two each byte is indexed and the address is this linear index:
            this->buffer1_addr = 0;
            this->buffer2_addr = this->buffer1_size;

            #if SIKTEC_EPD_DEBUG
                Serial.println("Allocating on SRAM");
            #endif

        } else {
            this->buffer1 = (uint8_t *)malloc(this->buffer1_size); // 15,000 bytes
            this->buffer2 = (uint8_t *)malloc(this->buffer2_size); // 15,000 bytes

            #if SIKTEC_EPD_DEBUG
                Serial.println("Allocating on RAM");
            #endif
        }
    }

public:

    uint32_t epd_width    = EPD_3CS_WIDTH;  //!< the definition width.
    uint32_t epd_height   = EPD_3CS_HEIGHT; //!< the definition height.

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
        this->default_refresh_delay = EPD_3CS_REFRESH_DELAY;  //ms

        this->setInitAndLut(); // defaults to nullptr
        
        //Set colors - simple tri color:
        this->layer_colors[EPD_WHITE] = 0b00;
        this->layer_colors[EPD_BLACK] = 0b01;
        this->layer_colors[EPD_RED]   = 0b10;
        this->layer_colors[EPD_GRAY]  = 0b01;
        this->layer_colors[EPD_DARK]  = 0b01;
        this->layer_colors[EPD_LIGHT] = 0b00;

        this->setRotation(1);
        this->powerDown();
    }

    /**
     * @brief start up the display
     * power up will send command list of init code
     * 
     * @returns void
     */
    inline void powerUp() {

        #if SIKTEC_EPD_DEBUG
            Serial.println("Powering Up display");
        #endif

        //Check if we really need to powerUp:
        if (this->epdPower) {
            #if SIKTEC_EPD_DEBUG
                Serial.println(" - EPD Is allready powered on.");
            #endif
            delay(50);
            return;
        }

        #if SIKTEC_EPD_DEBUG
            Serial.println();
        #endif

        uint8_t tries = 1;

        retry_init: 

        //First hard reset:
        this->hardwareResetEPD();
        this->busy_wait();

        //Use this init sequence:
        const uint8_t *init_code = ssd1619_default_init_code_improved;
        if (this->_epd_init_code != NULL) {
            init_code = this->_epd_init_code;
        }

        //Send init and LUT:
        #if SIKTEC_EPD_DEBUG
            Serial.println("Sending Init Sequence...");
        #endif
        if (!this->EPD_commandList(init_code)) {
            tries++;
            #if SIKTEC_EPD_DEBUG
                Serial.print("Init Failed! - Retrying ");
                Serial.println(tries);
            #endif
            goto retry_init;
        }

        //Set the RAM window as defined for scanning:
        this->setRAMWindow(0, 0, this->fixed8_height - 1, this->fixed8_width - 1);
        
        #if SIKTEC_EPD_DEBUG
            Serial.println("Init Sequence Done.");
        #endif

        this->epdPower = true;

        delay(20);
    }

    /**
     * @brief signal the display to update meaning full refresh
     * 
     * @returns void
     */
    inline void update(void) {
        uint8_t buf[1];
        buf[0] = SSD1619_MASTER_ACTIVATE_OPTION_MODE1;
        this->EPD_command(SSD1619_DISP_CTRL2, buf, 1);
        this->EPD_command(SSD1619_MASTER_ACTIVATE);
        this->busy_wait();
        //If no busy pin attached wait a fixed amount of time
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

        #if SIKTEC_EPD_DEBUG
            Serial.println("Powering Down display");
        #endif

        //Check if we really need to powerDown:
        if (!this->epdPower) return;

        uint8_t buf[1];
        // Only deep sleep if we can get out of it
        if (this->pins.rst >= 0) {
            buf[0] = SSD1619_DEEP_SLEEP_OPTION_MODE1;
            this->EPD_command(SSD1619_DEEP_SLEEP, buf, 1);
        } else {
            /*
                It resets the commands and parameters to their S/W Reset default values except -Deep Sleep Mode
                During operation, BUSY pad will output high.
                Note: RAM is not effected by this command.
            */
            this->EPD_command(SSD1619_SW_RESET);
            this->busy_wait();
        }

        this->epdPower = false;
        
        delay(100);
    }

protected:

    /**
     * @brief write data to internal EPD ram
     * 
     * @param index 
     * 
     * @returns uint8_t - read byte
     */
    inline uint8_t writeRAMCommand(uint8_t index) {
        if (index == 0) {
        return this->EPD_command_with_read(SSD1619_WRITE_RAM1);
        }
        if (index == 1) {
            return this->EPD_command_with_read(SSD1619_WRITE_RAM2);
        }
        return 0;
    }

    /**
     * @brief set the Internal EPD RAM address
     * 
     * always will set to 0,0 
     * 
     * @param x - void its ignored
     * @param y - void its ignored 
     * 
     * @returns void 
     */
    inline void setRAMAddress(uint16_t x, uint16_t y) {
        (void)x;
        (void)y;

        uint8_t buf[2];

        // set RAM x address count
        buf[0] = 0x00;
        this->EPD_command(SSD1619_SET_RAMXCOUNT, buf, 1);

        // set RAM y address count
        buf[0] = 0x0;
        buf[1] = 0x0;
        this->EPD_command(SSD1619_SET_RAMYCOUNT, buf, 2);
    }

    /**
     * @brief set the internal EPD ram window - start & end 
     * 
     * @param x1  - start X
     * @param y1  - start Y 
     * @param x2  - end X 
     * @param y2  - end Y
     * 
     * @returns void
     */
    inline void setRAMWindow(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2) {
        uint8_t buf[5];
        // Set ram X start/end postion
        buf[0] = x1 / 8;
        buf[1] = x2 / 8;
        this->EPD_command(SSD1619_SET_RAMXPOS, buf, 2);
        // Set ram Y start/end postion
        buf[0] = y1;
        buf[1] = y1 >> 8;
        buf[2] = y2;
        buf[3] = y2 >> 8;
        this->EPD_command(SSD1619_SET_RAMYPOS, buf, 4);
    }

    /**
     * @brief  wait for busy signal to end
     *         busy pin is LOW while Driver is working. wait for HIGH.
     * @param moredelay - additional delay to wait
     * @returns bool
     */
    inline bool busy_wait(uint16_t moredelay = 0) {
        #if SIKTEC_EPD_DEBUG
            Serial.print("Waiting for busy signal.");
        #endif
        uint16_t test = 0;
        if (this->pins.busy >= 0) {
            while (digitalRead(this->pins.busy)) { // wait for busy low
                delay(200);
                if (test++ > EPD_3CS_BUSY_RETRY_TIMES) {
                    return false;
                }
                #if SIKTEC_EPD_DEBUG
                    Serial.print(".");
                #endif
            }
        } else {
            delay(EPD_3CS_BUSY_DELAY);
        }
        if (moredelay > 0) {
            delay(moredelay);
        }
        #if SIKTEC_EPD_DEBUG
            Serial.println("READY!");
        #endif
        return true;
    }
};

}
