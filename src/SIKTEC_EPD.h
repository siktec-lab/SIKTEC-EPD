/******************************************************************************/
// Created by: SIKTEC.
// Release Version : 1.0.1
// Creation Date: 2022-04-12
// Copyright 2022, SIKTEC.
// 
/******************************************************************************/
/*****************************      NOTES       *******************************
    -> UC8276 -> IL0398 SIKTEC_EPD_G4
    -> UC8276 -> SIKTEC_EPD_3CU
    -> SSD1619 -> SIKTEC_EPD_3CS
*******************************************************************************/
/*****************************      Changelog       ****************************
1.0.1:
    -> initial release.
    -> adafruit gfx compatible.
    -> 3 drivers implemented.
    -> SRAM support - 23K256-I/SN implements SIKTEC_SRAM Library.
    -> MONO, TRICOLOR, 4GRAY Modes support.
*******************************************************************************/

/**  @file SIKTEC_EPD.h */
#pragma once

//------------------------------------------------------------------------//
// DEBUGGING FLAGS:
//------------------------------------------------------------------------//
// #define SIKTEC_EPD_DEBUG
// #define SIKTEC_EPD_DEBUG_PIXELS
// #define SIKTEC_EPD_DEBUG_SRAM_READ_WRITE 150

//------------------------------------------------------------------------//
// INCLUDES:
//------------------------------------------------------------------------//
#include <SIKTEC_SRAM.h>
#include <Adafruit_GFX.h>
#include <SIKTEC_SPI.h>

//------------------------------------------------------------------------//
// DEFAULT ADDRESSES AND CONSTANTS:
//------------------------------------------------------------------------//

#define EPD_DATA_MODE               0x1
#define EPD_COMMAND_MODE            0x0
#define EPD_CMD_SEQUENCE_WAIT_BUSY  0xFF
#define EPD_CMD_SEQUENCE_END        0xFE

namespace SIKtec {

/** @brief Supported EPD color codes  */
enum {
    EPD_WHITE,
    EPD_BLACK,
    EPD_RED,
    EPD_GRAY,
    EPD_DARK,
    EPD_LIGHT,
    EPD_NUM_COLORS
};

/** @brief types of EPD monitors - this is used to set the colors etc... */
typedef enum {
  EPD_MODE_MONO,
  EPD_MODE_TRICOLOR,
  EPD_MODE_GRAYSCALE4,
  EPD_MODE_MONO_PARTIAL
} siktecepd_mode_t;

/**
 * @brief simple swap macro function
*/
#define EPD_swap(a, b)   \
  {                      \
    int16_t t = a;       \
    a = b;               \
    b = t;               \
  }

//------------------------------------------------------------------------//
// SIKTEC_EPD
//------------------------------------------------------------------------//

/**
 * @brief  The SIKTEC EPD Class that interfaces the EPD shield with SRAM support.
 * wraps the gfx lib
*/
class SIKTEC_EPD : public Adafruit_GFX {

public:
    /** @brief  The SIKTEC EPD constructor when you you define your own SPI pins. */
    SIKTEC_EPD(
            int width, int height, 
            int8_t SID, int8_t SCLK, int8_t DC, int8_t RST, int8_t CS, int8_t SRCS, int8_t MISO, int8_t BUSY = -1
    );

    /** @brief The SIKTEC EPD constructor with the Arduino SPIClass. */
    SIKTEC_EPD(
            int width, int height, 
            int8_t DC, int8_t RST, int8_t CS, int8_t SRCS, int8_t BUSY = -1, SPIClass *spi = &SPI
    );

    /** @brief virtual clears the internal buffers and destructs SRAM object. */
    ~SIKTEC_EPD();

    //------------------------------------------------------------------------//
    // SRAM Related:
    //------------------------------------------------------------------------//

public:

    SIKTEC_SRAM *sram;
    bool is_using_sram();

protected:

    bool use_sram; ///< true if we are using an SRAM chip as a framebuffer

    //------------------------------------------------------------------------//
    // BUFFERS:
    //------------------------------------------------------------------------//

    uint32_t buffer1_size;          // size of the primary buffer
    uint32_t buffer2_size;          // size of the secondary buffer
    uint8_t *buffer1;               // the pointer to the primary buffer if using on-chip ram
    uint8_t *buffer2;               // the pointer to the secondary buffer if using on-chip ram
    uint8_t *color_buffer;          // the pointer to the color buffer if using on-chip ram
    uint8_t *black_buffer;          // the pointer to the black buffer if using on-chip ram
    uint16_t buffer1_addr;          // The SRAM address offsets for the primary buffer
    uint16_t buffer2_addr;          // The SRAM address offsets for the secondary buffer
    uint16_t colorbuffer_addr;      // The SRAM address offsets for the color buffer
    uint16_t blackbuffer_addr;      // The SRAM address offsets for the black buffer

    //------------------------------------------------------------------------//
    // SPI related:
    //------------------------------------------------------------------------//

protected:  

    SIKTEC_SPI *_spi = NULL;
    static bool _isInTransaction; // true if SPI bus is in transfer state

    //------------------------------------------------------------------------//
    // EPD methods:
    //------------------------------------------------------------------------//

public:

    void begin(bool reset = true);

protected:
    
    int8_t _dc_pin;
    int8_t _reset_pin;
    int8_t _cs_pin;
    int8_t _busy_pin;
    uint16_t default_refresh_delay = 15000;
    uint8_t partialsSinceLastFullUpdate = 0;
    siktecepd_mode_t inkmode;                   ///< Ink mode passed to begin() from driver begin()
    bool blackInverted;                         ///< is black channel inverted
    bool colorInverted;                         ///< is red channel inverted
    uint8_t layer_colors[EPD_NUM_COLORS];
    bool epdPower = false;

public:

    void hardwareResetEPD(); ///< Perform a hardware reset with the reset pin.
    void drawPixel(int16_t x, int16_t y, uint16_t color); ///< Draw a pixel on the screen.
    void clearBuffer(); ///< Clear drawing buffer.
    void clearDisplay(); ///< Clear the EPD screen. 
    void setBlackBuffer(int8_t index, bool inverted);
    void setColorBuffer(int8_t index, bool inverted);
    void display(bool sleep = false);
    void EPD_commandList(const uint8_t *init_code);
    void EPD_command(uint8_t c, const uint8_t *buf, uint16_t len);
    void EPD_command(uint8_t c);
    uint8_t EPD_command_with_read(uint8_t cmd);
    void EPD_command_with_read(uint8_t cmd, uint8_t *buf, uint16_t len);
    void EPD_data(const uint8_t *buf, size_t len, bool invert = false); ///< Send a stream of bytes to the EPD
    void EPD_data(uint8_t data); ///< Send a byte of data to the EPD
    void EPD_csLow();   ///< Toggles the transaction SPI flag
    void EPD_csHigh(); ///< Toggles the transaction SPI flag
    void EPD_dc_mode(uint8_t mode = EPD_COMMAND_MODE);
    bool EPD_isPowered();

    //Debugging stuff:
    void _print_debug_byte(uint16_t addr, uint8_t value, bool new_line = false, Stream *SerialPort = &Serial);
    void _display_buffer(uint16_t from_addr, uint8_t cols, int length, Stream *SerialPort = &Serial);

protected:

    void writeRAMFramebufferToEPD(uint8_t *buffer, uint32_t buffer_size, uint8_t EPDlocation, bool invertdata = false);
    void writeSRAMFramebufferToEPD(uint16_t SRAM_buffer_addr, uint32_t buffer_size, uint8_t EPDlocation, bool invertdata = false);

    //------------------------------------------------------------------------//
    // BOARD Implementations required:
    //------------------------------------------------------------------------//

protected:

    const uint8_t *_epd_init_code           = NULL;
    const uint8_t *_epd_lut_code            = NULL;
    const uint8_t *_epd_partial_init_code   = NULL;
    const uint8_t *_epd_partial_lut_code    = NULL;

    /** 
     * @brief Send the specific command to start writing to EPD display RAM
     * 
     * @param index The index for which buffer to write (0 or 1 or tri-color displays) Ignored for monochrome displays.
     * 
     * @returns The byte that is read from SPI at the same time as sending the command
    */
    virtual uint8_t writeRAMCommand(uint8_t index) = 0;

    /**
     * @brief Some displays require setting the RAM address pointer
     * 
     * @param x X address counter value
     * @param y Y address counter value
    */
    virtual void setRAMAddress(uint16_t x, uint16_t y) = 0;
    virtual void busy_wait(void) = 0;

    /** @brief start up the display */
    virtual void powerUp() = 0;

    /** @brief signal the display to update */
    virtual void update(void) = 0;

    /** @brief wind down the display */
    virtual void powerDown(void) = 0;

};

}

//-----------------------------------------------------------------------------------------//
// INCLUDE DEFAULT DRIVERS:
//-----------------------------------------------------------------------------------------//
//NOTE: this should not affect the final size - the linker will drop dead code.
#include "drivers/SIKTEC_EPD_G4.h"
#include "drivers/SIKTEC_EPD_3CU.h"
#include "drivers/SIKTEC_EPD_3CS.h"



