/******************************************************************************/
// Created by: SIKTEC.
// Release Version : 1.0.1
// Creation Date: 2022-04-12
// Copyright 2022, SIKTEC.
// 
/******************************************************************************/
/*****************************      NOTES       *******************************
    -> SSD1619 -> SIKTEC_EPD_3CS
    -> Maximum SPI write speed 20MHz is the absolute max for the SSD1619
    -> Supports internal temp read.
    - SSD1619 ->VCI: 2.2 to 3.7V
*******************************************************************************/
/*****************************      Changelog       ****************************
1.0.1:
    -> initial release
*******************************************************************************/

/**  @file SIKTEC_EPD_3CS.h */
#pragma once

//------------------------------------------------------------------------//
// Ram mode definitions for EPD:
//------------------------------------------------------------------------//
#define EPD_3CS_RAM_BW  0x10
#define EPD_3CS_RAM_RED 0x13

//------------------------------------------------------------------------//
// GENERAL EPD CONSTANTS:
//------------------------------------------------------------------------//
#define BUSY_DELAY            500
#define EPD_3CS_WIDTH         300
#define EPD_3CS_HEIGHT        400
#define EPD_3CS_RAM_SIZE_KBIT 256

//------------------------------------------------------------------------//
// EPD DRIVER COMMANDS Driver SSD1619:
//------------------------------------------------------------------------//
#define SSD1619_DRIVER_CONTROL 0x01
#define SSD1619_GATE_VOLTAGE 0x03
#define SSD1619_SOURCE_VOLTAGE 0x04
#define SSD1619_PROGOTP_INITIAL 0x08
#define SSD1619_PROGREG_INITIAL 0x09
#define SSD1619_READREG_INITIAL 0x0A
#define SSD1619_BOOST_SOFTSTART 0x0C
#define SSD1619_DEEP_SLEEP 0x10
#define SSD1619_DEEP_SLEEP_OPTION_NORMAL 0x00
#define SSD1619_DEEP_SLEEP_OPTION_MODE1 0x01
#define SSD1619_DEEP_SLEEP_OPTION_MODE2 0x03

#define SSD1619_DATA_MODE 0x11
#define SSD1619_SW_RESET 0x12
#define SSD1619_TEMP_CONTROL 0x18
#define SSD1619_TEMP_WRITE 0x1A
#define SSD1619_MASTER_ACTIVATE_OPTION_MODE1 0xC7 
    // ^ 1. Enable Clock Signal
    //   2. Enable ANALOG
    //   3. DISPLAY Mode 1
    //   4. Disable ANALOG
    //   5. Disable OSC
#define SSD1619_MASTER_ACTIVATE_OPTION_MODE2 0xCF
    // ^ 1. Enable Clock Signal
    //   2. Enable ANALOG
    //   3. DISPLAY Mode 2
    //   4. Disable ANALOG
    //   5. Disable OSC
#define SSD1619_MASTER_ACTIVATE 0x20
#define SSD1619_DISP_CTRL1 0x21
#define SSD1619_DISP_CTRL2 0x22
#define SSD1619_WRITE_RAM1 0x24
#define SSD1619_WRITE_RAM2 0x26
#define SSD1619_WRITE_VCOM 0x2C
#define SSD1619_READ_OTP 0x2D
#define SSD1619_READ_STATUS 0x2F
#define SSD1619_WRITE_LUT 0x32
#define SSD1619_WRITE_BORDER 0x3C
#define SSD1619_SET_RAMXPOS 0x44
#define SSD1619_SET_RAMYPOS 0x45
#define SSD1619_SET_RAMXCOUNT 0x4E
#define SSD1619_SET_RAMYCOUNT 0x4F
#define SSD1619_SET_ANALOGBLOCK 0x74
#define SSD1619_SET_DIGITALBLOCK 0x7E


//------------------------------------------------------------------------//
// EPD DRIVER INIT SEQUENCES:
//------------------------------------------------------------------------//


namespace SIKtec {


/** @brief init sequence for ssd1619 driver */

static const uint8_t ssd1619_default_init_code[] {
    SSD1619_SW_RESET, 0,                // soft reset
    EPD_CMD_SEQUENCE_WAIT_BUSY, 20,     // busy wait
    SSD1619_SET_ANALOGBLOCK, 1, 0x54,   // set analog block control
    SSD1619_SET_DIGITALBLOCK, 1, 0x3B,  // set digital block control
    SSD1619_DATA_MODE, 1, 0x03,         // Ram data entry mode
    SSD1619_WRITE_BORDER, 1, 0x01,      // border color
    SSD1619_TEMP_CONTROL, 1, 0x80,      // Temp control
    SSD1619_DISP_CTRL2, 1, 0xB1,
    SSD1619_MASTER_ACTIVATE, 0,
    EPD_CMD_SEQUENCE_WAIT_BUSY, 20,     // busy wait
    EPD_CMD_SEQUENCE_END
};

//------------------------------------------------------------------------//
// LOOK UP TABLES:
//------------------------------------------------------------------------//


//------------------------------------------------------------------------//
// DRIVER WRAPPER Definition:
//------------------------------------------------------------------------//
/**
 * @brief  Class for interfacing with SIKTEC_EPD_3CS EPD driver And on board perephirials 
 */
class SIKTEC_EPD_3CS : public SIKTEC_EPD {

public:

    /** 
     * @brief constructor if using external SRAM chip and software SPI
     * 
     * @param SID   the SID pin to use
     * @param SCLK  the SCLK pin to use
     * @param DC    the data/command pin to use
     * @param RST   the reset pin to use
     * @param CS    the chip select pin to use
     * @param SRCS  the SRAM chip select pin to use
     * @param MISO  the MISO pin to use
     * @param BUSY  the busy pin to use
    */
    inline SIKTEC_EPD_3CS(int8_t SID, int8_t SCLK, int8_t DC, int8_t RST, int8_t CS, int8_t SRCS, int8_t MISO, int8_t BUSY = -1) 
        : SIKTEC_EPD(EPD_3CS_WIDTH, EPD_3CS_HEIGHT, SID, SCLK, DC, RST, CS, SRCS, MISO, BUSY) {
        this->_init();
    }

    /** 
     * @brief constructor if using on-chip RAM and hardware SPI
     * 
     * @param DC    the data/command pin to use
     * @param RST   the reset pin to use
     * @param CS    the chip select pin to use
     * @param SRCS  the SRAM chip select pin to use
     * @param BUSY  the busy pin to use
    */
    inline SIKTEC_EPD_3CS(int8_t DC, int8_t RST, int8_t CS, int8_t SRCS, int8_t BUSY = -1, SPIClass *spi = &SPI) 
        : SIKTEC_EPD(EPD_3CS_WIDTH, EPD_3CS_HEIGHT, DC, RST, CS, SRCS, BUSY, spi) {
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
        //Make sure its devisible by 8:
        if ((this->epd_height % 8) != 0) {
            this->epd_height += 8 - (this->epd_height % 8);
        }
        // Set buffers size:
        this->buffer1_size = this->epd_width * this->epd_height / 8;    // 15,000 -> should be
        this->buffer2_size = this->buffer1_size;                        // 15,000 -> should be

        #ifdef SIKTEC_EPD_DEBUG
            Serial.println("Allocating Buffer Size:");
            Serial.print("     - screen size -> ");
            Serial.print(this->epd_width);
            Serial.print(", ");
            Serial.println(this->epd_height);
            Serial.print("     - buf 1 -> ");
            Serial.println(this->buffer1_size);
            Serial.print("     - buf 2 -> ");
            Serial.println(this->buffer2_size);
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
                Serial.println("Allocating on Internal MEM");
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
    inline void begin(siktecepd_mode_t mode = EPD_MODE_TRICOLOR) {
        SIKTEC_EPD::begin(true);

        this->inkmode = mode;

        this->setColorBuffer(1, false); // red defaults to un inverted
        this->setBlackBuffer(0, true);  // black defaults to inverted
        this->default_refresh_delay = 13000;

        //Set colors - simple tri color:
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
     * @brief start up the display
     * power up will send command list of init code
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
        this->busy_wait();
        const uint8_t *init_code = ssd1619_default_init_code;

        if (this->_epd_init_code != NULL) {
            init_code = this->_epd_init_code;
        }

        this->EPD_commandList(init_code);

        // Set display size and driver output control
        // SSD1619 supports MUX addressing , Source gate scanning directions and rows
        // so use this one based on the width.
        uint8_t buf[3];
        buf[0] = (this->epd_width - 1); // 0 - 399, gate lines -> expected byte + 1
        buf[1] = (this->epd_width - 1) >> 8; // will complete: 110001111
        buf[2] = 0x00; 
            // 000 -> 
            // selects Gate 0 starting point, 
            // Scanning order (left and right gate interlaced)
            // scan from 0 place to max (299) in each gate 
        this->EPD_command(SSD1619_DRIVER_CONTROL, buf, 3);
        //Finally we need to set the RAM window as defined for scanning:
        this->setRAMWindow(0, 0, this->epd_height - 1, this->epd_width - 1);
        
        this->epdPower = true;

        delay(20);
    }

    /**
     * @brief signal the display to update meaning full refresh
     * 
     * @returns void
     */
    inline void update(void) {
        #ifdef SIKTEC_EPD_DEBUG
            Serial.println("EPD Update");
        #endif

        uint8_t buf[1];
        buf[0] = SSD1619_MASTER_ACTIVATE_OPTION_MODE1;
        this->EPD_command(SSD1619_DISP_CTRL2, buf, 1);
        this->EPD_command(SSD1619_MASTER_ACTIVATE);
        this->busy_wait();
        if (this->_busy_pin <= -1) {
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

        //Check if we really need to powerDown:
        if (!this->epdPower) return;

        uint8_t buf[1];
        // Only deep sleep if we can get out of it
        if (this->_reset_pin >= 0) {
            buf[0] = SSD1619_DEEP_SLEEP_OPTION_MODE1;
            this->EPD_command(SSD1619_DEEP_SLEEP, buf, 1);
        } else {
            /*
                It resets the commands and parameters to their S/W Reset default values except -Deep Sleep Mode
                During operation, BUSY pad will output high.
                Note: RAM are unaffected by this command.
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
     * 
     * busy pin is LOW while Driver is working.
     * wait for HIGH.
     * @returns void
     */
    inline void busy_wait() {
        #ifdef SIKTEC_EPD_DEBUG
            Serial.print("EPD Busy state waiting - ");
        #endif
        if (this->_busy_pin >= 0) {
            #ifdef SIKTEC_EPD_DEBUG
                Serial.print("Pin wait...");
            #endif
            while (digitalRead(this->_busy_pin)) { // wait for busy low
                delay(50);
            }
            delay(150);
        } else {
            #ifdef SIKTEC_EPD_DEBUG
                Serial.print("Constant wait...");
            #endif
            delay(BUSY_DELAY);
        }
        #ifdef SIKTEC_EPD_DEBUG
            Serial.println("DONE.");
        #endif
    }
};

}
