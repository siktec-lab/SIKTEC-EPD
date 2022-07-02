/******************************************************************************/
// Created by: SIKTEC.
// Release Version : 1.0.3
// Creation Date: 2022-04-12
// Copyright 2022, SIKTEC.
// 
/******************************************************************************/
/*****************************      NOTES       *******************************
 * ePaper / eInk display driver to easily integrate SIKTEC displays.
 * GFX compatible with optional external SRAM use.
 * Supported drivers / SIKTEC boards:
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
1.0.2:
    -> Improved pixel draw method.
    -> Sram now gives extra space for additional user space buffer.
    -> Improved drivers layout - all init lut and instructions moved to a header file.
    -> Improved lut for 4gray G4 board.
    -> Improved init code for 3CS board.
    -> Fixed Arduino DUE specific problems.
    -> Added pixel debug methods. 
    -> MONO, TRICOLOR, 4GRAY Modes support.
*******************************************************************************/

/**  @file SIKTEC_EPD.h */
#pragma once

//------------------------------------------------------------------------//
// DEBUGGING FLAGS:
//------------------------------------------------------------------------//

#ifndef SIKTEC_EPD_DEBUG 
    #define SIKTEC_EPD_DEBUG 0
#endif
#ifndef SIKTEC_EPD_DEBUG_COMMAND_LISTS 
    #define SIKTEC_EPD_DEBUG_COMMAND_LISTS 0
#endif
#ifndef SIKTEC_EPD_DEBUG_PIXELS 
    #define SIKTEC_EPD_DEBUG_PIXELS 0
#endif
#ifndef SIKTEC_EPD_DEBUG_SRAM 
    #define SIKTEC_EPD_DEBUG_SRAM 0
#endif
#ifndef SIKTEC_EPD_DEBUG_SRAM_READ_WRITE 
    #define SIKTEC_EPD_DEBUG_SRAM_READ_WRITE 15001
#endif


#ifndef PRINT_DEBUG_BUFFER
#define PRINT_DEBUG_BUFFER(__template, ...) \
    sprintf(debug_message, __template, __VA_ARGS__); \
    Serial.print(debug_message)
#endif

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
#define EPD_CMD_SEQUENCE_WAIT       0xFF
#define EPD_CMD_SEQUENCE_END        0xFE

//NOTE: onboard SRAM is rated 20Mhz AND EPD Drivers 20Mhz also - That said 20Mhz is not reliable best is 18 - 19. 
#if defined(ESP32) || defined(ARDUINO_AVR_MEGA) || defined(AVR_MEGA2560) || defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__) || defined(ARDUINO_SAM_DUE) || defined(SAM3X8E)
    // #define EPD_SRAM_SPEED              4000000L
    // #define EPD_SRAM_SPEED              8000000L
    // #define EPD_SRAM_SPEED              16000000L
    #define EPD_SRAM_SPEED                 18000000L 
    // #define EPD_SRAM_SPEED              20000000L
#else 
    #define EPD_SRAM_SPEED                 4000000L
    // #define EPD_SRAM_SPEED              8000000L
    // #define EPD_SRAM_SPEED              16000000L
    // #define EPD_SRAM_SPEED              18000000L
    // #define EPD_SRAM_SPEED              20000000L
#endif


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
} epd_mode_t;

/**
 * @brief simple swap macro function
*/
#define EPD_swap(a, b)   \
  {                      \
    int16_t t = a;       \
    a = b;               \
    b = t;               \
  }

/**
 * @brief required pins struct.
*/
typedef struct EPD_Pins {
    int8_t epd_cs;
    int8_t sram_cs;
    int8_t dc;
    int8_t rst;
    int8_t busy;
} epd_pins_t;

/**
 * @brief available SRAM space return struct.
*/
typedef struct EPD_SRAM_Space {
    uint32_t    kbit;
    uint32_t    bytes;
    uint16_t    address;
} epd_sram_space_t;

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
        int8_t CS, int8_t SRCS, int8_t DC, int8_t RST, int8_t BUSY, 
        int8_t spi_clock, int8_t spi_miso,  int8_t spi_mosi, uint32_t clock_frequency = EPD_SRAM_SPEED
    );
    SIKTEC_EPD(
        int width, int height, 
        const epd_pins_t &pins, 
        int8_t spi_clock, int8_t spi_miso,  int8_t spi_mosi, uint32_t clock_frequency = EPD_SRAM_SPEED
    );
    /** @brief The SIKTEC EPD constructor with the Arduino SPIClass. */
    SIKTEC_EPD(
        int width, int height, 
        int8_t CS, int8_t SRCS, int8_t DC, int8_t RST, int8_t BUSY, 
        SPIClass *spi = &SPI,
        uint32_t clock_frequency = EPD_SRAM_SPEED
    );
    SIKTEC_EPD(
        int width, int height, 
        const epd_pins_t &pins, 
        SPIClass *spi = &SPI,
        uint32_t clock_frequency = EPD_SRAM_SPEED
    );

    /** @brief virtual clears the internal buffers and destructs SRAM object. */
    ~SIKTEC_EPD();

    //------------------------------------------------------------------------//
    // SRAM Related:
    //------------------------------------------------------------------------//
private:

    uint16_t ram_buffer_element_size = 1;

public:

    SIKTEC_SRAM *sram;

    bool is_using_sram();

    epd_sram_space_t getFreeSramSpace(uint32_t assumeTotalSizeKib = 256);

    uint16_t allocateSramArrayBuffer(const uint16_t num, const uint16_t ele_bytes);

    void releaseSramArrayBuffer();

    bool getSramArrayBufferElement(const uint16_t address, const uint16_t index, uint8_t *out, const uint16_t num = 1);

    bool setSramArrayBufferElement(const uint16_t address, const uint16_t index, uint8_t *in, const uint16_t num = 1);

    #if SIKTEC_EPD_DEBUG
        uint32_t analyzeSRAMsize(const bool print, Stream *SerialPort = &Serial);
    #endif

protected:

    bool use_sram; ///< true if we are using an SRAM chip as a framebuffer

    //------------------------------------------------------------------------//
    // BUFFERS:
    //------------------------------------------------------------------------//

    uint32_t buffer1_size = 0;          // size of the primary buffer
    uint32_t buffer2_size = 0;          // size of the secondary buffer
    uint8_t *buffer1;               // the pointer to the primary buffer if using ram
    uint8_t *buffer2;               // the pointer to the secondary buffer if using ram
    uint8_t *color_buffer;          // the pointer to the color buffer if using ram
    uint8_t *black_buffer;          // the pointer to the black buffer if using ram
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
    epd_mode_t  inkmode;                            ///< Ink mode passed to begin() from driver begin()

protected:
    
    epd_pins_t  pins;
    uint16_t    fixed8_width = 0;
    uint16_t    fixed8_height = 0;
    uint16_t    default_refresh_delay = 15000;
    uint8_t     partialsSinceLastFullUpdate = 0;
    bool        blackInverted;                      ///< is black channel inverted
    bool        colorInverted;                      ///< is red channel inverted
    uint8_t     layer_colors[EPD_NUM_COLORS];
    bool        epdPower = false;

public:

    /** @brief a struct used to address pixels*/
    typedef struct PixelAddress {
        bool inBound;
        uint16_t offset;
        int16_t rx;
        int16_t ry;
        uint16_t sram_black;
        uint16_t sram_color;
        uint8_t *ram_black;
        uint8_t *ram_color;
    } pixelAddress_t;

    /** @brief a struct used to address pixels*/
    typedef struct PixelValue {
        bool     inBound;
        bool     sram;
        uint8_t black;
        uint8_t color;
    } pixelValue_t;

    void hardwareResetEPD(); ///< Perform a hardware reset with the reset pin.
    pixelValue_t getPixel(const int16_t x, const int16_t y);
    void drawPixel(int16_t x, int16_t y, uint16_t color); ///< Draw a pixel on the screen.
    void clearBuffer(); ///< Clear drawing buffer.
    void clearDisplay(); ///< Clear the EPD screen. 
    void setBlackBuffer(int8_t index, bool inverted);
    void setColorBuffer(int8_t index, bool inverted);
    void display(bool sleep = false);
    bool EPD_commandList(const uint8_t *init_code);
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
    #if SIKTEC_EPD_DEBUG
        void debugPixel(const int16_t x, const int16_t y);
    #endif
    void _print_debug_byte(uint16_t addr, uint8_t value, bool new_line = false, Stream *SerialPort = &Serial);
    void _display_buffer(uint16_t from_addr, uint8_t cols, int length, Stream *SerialPort = &Serial);

protected:

    bool pixelInBounds(const int16_t x, const int16_t y);
    uint16_t getPixelAddressOffset(const int16_t x, const int16_t y);
    pixelAddress_t getPixelAddress(const int16_t x, const int16_t y);
    
    void writeRAMFramebufferToEPD(uint8_t *buffer, uint32_t buffer_size, uint8_t EPDlocation, bool invertdata = false);
    void writeSRAMFramebufferToEPD(uint16_t SRAM_buffer_addr, uint32_t buffer_size, uint8_t EPDlocation, bool invertdata = false);

    //------------------------------------------------------------------------//
    // BOARD Implementations required:
    //------------------------------------------------------------------------//

public:

    void setInitAndLut(const uint8_t * code = nullptr, const uint8_t * lut = nullptr, bool partial = false);
    void setInitCode(const uint8_t * code, bool partial = false);
    void setLut(const uint8_t * lut, bool partial = false);

protected:

    const uint8_t *_epd_init_code           = nullptr;
    const uint8_t *_epd_lut_code            = nullptr;
    const uint8_t *_epd_partial_init_code   = nullptr;
    const uint8_t *_epd_partial_lut_code    = nullptr;


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
    virtual bool busy_wait(uint16_t moredelay = 0) = 0;

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
//NOTE: this should not affect the final size - the linker will drop dead code, hopefully.
#include "drivers/SIKTEC_EPD_G4.h"
#include "drivers/SIKTEC_EPD_3CU.h"
#include "drivers/SIKTEC_EPD_3CS.h"

//-----------------------------------------------------------------------------------------//
// INCLUDE Extensions: should be removed by the linker if not addressed.
//-----------------------------------------------------------------------------------------//
#include "bitmap/SIKTEC_EPD_BITMAP.h"



