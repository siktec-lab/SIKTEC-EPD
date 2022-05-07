/******************************************************************************/
// Created by: SIKTEC.
// Release Version : 1.0.1
// Creation Date: 2022-04-12
// Copyright 2022, SIKTEC.
// 
/******************************************************************************/
/*****************************      NOTES       *******************************
    -> UC8276 -> IL0398 SIKTEC_EPD_G4
*******************************************************************************/
/*****************************      Changelog       ****************************
1.0.1:
    -> initial release
*******************************************************************************/

/**  @file SIKTEC_EPD_G4.h */
#pragma once

//------------------------------------------------------------------------//
// Ram mode definitions for EPD:
//------------------------------------------------------------------------//
#define EPD_G4_RAM_BW 0x10
#define EPD_G4_RAM_RED 0x13

//------------------------------------------------------------------------//
// GENERAL EPD CONSTANTS:
//------------------------------------------------------------------------//
#define EPD_G4_BUSY_DELAY    500
#define EPD_G4_WIDTH         300
#define EPD_G4_HEIGHT        400
#define EPD_G4_RAM_SIZE_KBIT 256

//------------------------------------------------------------------------//
// EPD DRIVER COMMANDS Driver IL0398:
//------------------------------------------------------------------------//
#define IL0398_PANEL_SETTING 0x00
#define IL0398_POWER_SETTING 0x01
#define IL0398_POWER_OFF 0x02
#define IL0398_POWER_OFF_SEQUENCE 0x03
#define IL0398_POWER_ON 0x04
#define IL0398_POWER_ON_MEASURE 0x05
#define IL0398_BOOSTER_SOFT_START 0x06
#define IL0398_DEEP_SLEEP 0x07
#define IL0398_DTM1 0x10
#define IL0398_DATA_STOP 0x11
#define IL0398_DISPLAY_REFRESH 0x12
#define IL0398_DTM2 0x13
#define IL0398_PDTM1 0x14
#define IL0398_PDTM2 0x15
#define IL0398_PDRF 0x16
#define IL0398_LUT1 0x20
#define IL0398_LUTWW 0x21
#define IL0398_LUTBW 0x22
#define IL0398_LUTWB 0x23
#define IL0398_LUTBB 0x24
#define IL0398_PLL 0x30
#define IL0398_TEMPCALIBRATE 0x40
#define IL0398_TEMPSELECT 0x41
#define IL0398_TEMPWRITE 0x42
#define IL0398_TEMPREAD 0x43
#define IL0398_VCOM 0x50
#define IL0398_LOWPOWERDETECT 0x51
#define IL0398_TCON 0x60
#define IL0398_RESOLUTION 0x61
#define IL0398_GSSTSETTING 0x65
#define IL0398_REVISION 0x70
#define IL0398_GETSTATUS 0x71
#define IL0398_AUTOVCOM 0x80
#define IL0398_READVCOM 0x81
#define IL0398_VCM_DC_SETTING 0x82
#define IL0398_PARTWINDOW 0x90
#define IL0398_PARTIALIN 0x91
#define IL0398_PARTIALOUT 0x92
#define IL0398_PROGRAMMODE 0xA0
#define IL0398_ACTIVEPROGRAM 0xA1
#define IL0398_READOTP 0xA2
#define IL0398_CASCADESET 0xE0
#define IL0398_POWERSAVING 0xE3
#define IL0398_FORCETEMP 0xE5

namespace SIKtec {
    
//------------------------------------------------------------------------//
// EPD DRIVER INIT SEQUENCES:
//------------------------------------------------------------------------//

/** @brief init sequence for mono and general use */
static const uint8_t il0398_default_init_code[] {
    0xFF, 20,                            // busy wait
    IL0398_BOOSTER_SOFT_START, 3, 0x17, 0x17, 0x17,
    IL0398_POWER_ON, 0,
    0xFF, 20,                            // busy wait
    IL0398_PANEL_SETTING, 2, 0x1F, 0x0D, // lut from OTP & VCOM = 0v
    IL0398_VCOM, 1, 0x97,
    0xFE
};

/** @brief init sequence for gray4 */
static const uint8_t ti_420t2_gray4_init_code[] {
    IL0398_POWER_SETTING, 5, 0x03, 0x00, 0x2b, 0x2b, 0x13,
    IL0398_BOOSTER_SOFT_START, 3, 0x17, 0x17, 0x17,
    IL0398_POWER_ON, 0,
    0xFF, 200,
    IL0398_PANEL_SETTING, 1, 0x3F,
    IL0398_PLL, 1, 0x3C,    
    IL0398_VCM_DC_SETTING, 1, 0x12,
    IL0398_VCOM, 1, 0x97,
    0xFE // EOM
};

//------------------------------------------------------------------------//
// LOOK UP TABLES:
//------------------------------------------------------------------------//

static const uint8_t ti_420t2_gray4_lut_code[] = {
  // const unsigned char lut_vcom[]PROGMEM =
  IL0398_LUT1, 42,
  0x00, 0x0A, 0x00, 0x00, 0x00, 0x01,
  0x60, 0x14, 0x14, 0x00, 0x00, 0x01,
  0x00, 0x14, 0x00, 0x00, 0x00, 0x01,
  0x00, 0x13, 0x0A, 0x01, 0x00, 0x01,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

  // const unsigned char lut_ww[]PROGMEM ={
  IL0398_LUTWW, 42, 
  0x40, 0x0A, 0x00, 0x00, 0x00, 0x01,
  0x90, 0x14, 0x14, 0x00, 0x00, 0x01,
  0x10, 0x14, 0x0A, 0x00, 0x00, 0x01,
  0xA0, 0x13, 0x01, 0x00, 0x00, 0x01,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

  // const unsigned char lut_bw[]PROGMEM ={
  IL0398_LUTBW, 42,
  0x40, 0x0A, 0x00, 0x00, 0x00, 0x01,
  0x90, 0x14, 0x14, 0x00, 0x00, 0x01,
  0x00, 0x14, 0x0A, 0x00, 0x00, 0x01,
  0x99, 0x0C, 0x01, 0x03, 0x04, 0x01,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

  // const unsigned char lut_wb[]PROGMEM ={
  IL0398_LUTWB, 42,
  0x40, 0x0A, 0x00, 0x00, 0x00, 0x01,
  0x90, 0x14, 0x14, 0x00, 0x00, 0x01,
  0x00, 0x14, 0x0A, 0x00, 0x00, 0x01,
  0x99, 0x0B, 0x04, 0x04, 0x01, 0x01,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

  // const unsigned char lut_bb[]PROGMEM ={
  IL0398_LUTBB, 42,
  0x80, 0x0A, 0x00, 0x00, 0x00, 0x01,
  0x90, 0x14, 0x14, 0x00, 0x00, 0x01,
  0x20, 0x14, 0x0A, 0x00, 0x00, 0x01,
  0x50, 0x13, 0x01, 0x00, 0x00, 0x01,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

//------------------------------------------------------------------------//
// DRIVER WRAPPER Definition:
//------------------------------------------------------------------------//

/**
 * @brief  Class for interfacing with SIKTEC_EPD_G4 EPD drivers And on board 
 */
class SIKTEC_EPD_G4 : public SIKTEC_EPD {

public:

    /** 
     * @brief constructor if using Optional SRAM chip and software SPI
     * 
     * @param SID the SID pin to use
     * @param SCLK the SCLK pin to use
     * @param DC the data/command pin to use
     * @param RST the reset pin to use
     * @param CS the chip select pin to use
     * @param SRCS the SRAM chip select pin to use
     * @param MISO the MISO pin to use
     * @param BUSY the busy pin to use
    */
    inline SIKTEC_EPD_G4(int8_t SID, int8_t SCLK, int8_t DC, int8_t RST, int8_t CS, int8_t SRCS, int8_t MISO, int8_t BUSY = -1) 
        : SIKTEC_EPD(EPD_G4_WIDTH, EPD_G4_HEIGHT, SID, SCLK, DC, RST, CS, SRCS, MISO, BUSY) {
        this->_init();
    }

    /** 
     * @brief constructor if using onboard SRAM and hardware SPI
     * 
     * @param DC the data/command pin to use
     * @param RST the reset pin to use
     * @param CS the chip select pin to use
     * @param SRCS the SRAM chip select pin to use
     * @param BUSY the busy pin to use
    */
    inline SIKTEC_EPD_G4(int8_t DC, int8_t RST, int8_t CS, int8_t SRCS, int8_t BUSY = -1, SPIClass *spi = &SPI) 
        : SIKTEC_EPD(EPD_G4_WIDTH, EPD_G4_HEIGHT, DC, RST, CS, SRCS, BUSY, spi) {
        this->_init();
    }

private:

    /**
     * @brief initialize this class - use by constructors.
     *        will set buffers and allocate memory if needed. 
     * 
     * @returns void
     */
    inline void _init() {
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

    uint32_t epd_width    = EPD_G4_WIDTH;  //!< the definition width.
    uint32_t epd_height   = EPD_G4_HEIGHT; //!< the definition height.

    /**
     * @brief start and initialize the epd.
     *        will set buffers (COLOR, BLACK) inverted mode.
     *        the rotation and will powerdown the display
     * @param mode the EPD mode is also used for gfx.
     * 
     * @returns void
     */
    inline void begin(siktecepd_mode_t mode = EPD_MODE_MONO) {
            
        SIKTEC_EPD::begin(true);
        
        //Set buffers:
        this->setColorBuffer(0, true);
        this->setBlackBuffer(1, true);

        this->inkmode = mode;

        if (mode == EPD_MODE_MONO) {
        
            this->_epd_init_code = NULL;
            this->_epd_lut_code = NULL;

            this->layer_colors[EPD_WHITE] = 0b00;
            this->layer_colors[EPD_BLACK] = 0b01;
            this->layer_colors[EPD_RED]   = 0b01;
            this->layer_colors[EPD_GRAY]  = 0b01;
            this->layer_colors[EPD_LIGHT] = 0b00;
            this->layer_colors[EPD_DARK]  = 0b01;
        
        } else if (mode == EPD_MODE_GRAYSCALE4) {

            this->_epd_init_code = ti_420t2_gray4_init_code;
            this->_epd_lut_code = ti_420t2_gray4_lut_code;

            this->layer_colors[EPD_WHITE] = 0b00;
            this->layer_colors[EPD_BLACK] = 0b11;
            this->layer_colors[EPD_RED]   = 0b10;
            this->layer_colors[EPD_GRAY]  = 0b10;
            this->layer_colors[EPD_LIGHT] = 0b01;
            this->layer_colors[EPD_DARK]  = 0b10;

        }

        this->default_refresh_delay = 1000;

        this->setRotation(1);
        this->powerDown();
    }

    /**
     * @brief start up the display -power up will send command list of init code and lut
     *        will also set the resoloution definition.
     * @returns void
     */
    inline void powerUp()  {

        #ifdef SIKTEC_EPD_DEBUG
            Serial.println("Powering Up display");
        #endif

        //Check if we really need to powerUp:
        if (this->epdPower) return;

        //First hard reset:
        this->hardwareResetEPD();

        //Init code default or other:
        const uint8_t *init_code = il0398_default_init_code;
        if (this->_epd_init_code != NULL) {
            init_code = this->_epd_init_code;
        }

        //Send init and LUT:
        this->EPD_commandList(init_code);
        if (this->_epd_lut_code) {
            this->EPD_commandList(this->_epd_lut_code);
        }

        //Set resolution:
        uint8_t buf[4];
        buf[0] = (this->HEIGHT >> 8) & 0xFF;
        buf[1] = this->HEIGHT & 0xFF;
        buf[2] = (this->WIDTH >> 8) & 0xFF;
        buf[3] = this->WIDTH & 0xFF;
        this->EPD_command(IL0398_RESOLUTION, buf, 4);

        this->epdPower = true;

        delay(20);
    }

    /**
     * @brief signal the display to update meaning full refresh:
     * 
     * @returns void
     */
    inline void update(void) {
        this->EPD_command(IL0398_DISPLAY_REFRESH);
        delay(50);
        this->busy_wait();
        if (this->_busy_pin <= -1) {
            delay(this->default_refresh_delay);
        }
    }
    
    /**
     * @brief power down the display
     *        power down will send command VCOM and POWEROFF
     *        will put in sleep only if reset pin is set.
     * @returns void
     */
    inline void powerDown() {
        
        #ifdef SIKTEC_EPD_DEBUG
            Serial.println("Powering Down display");
        #endif

        //Check if we really need to powerUp:
        if (!this->epdPower) return;

        //Data buffer:
        uint8_t buf[4];
        // power off
        buf[0] = 0xF7; // border floating
        this->EPD_command(IL0398_VCOM, buf, 1);
        this->EPD_command(IL0398_POWER_OFF);
        this->busy_wait();
        // Only deep sleep if we can get out of it
        if (this->_reset_pin >= 0) {
            buf[0] = 0xA5; // deep sleep
            this->EPD_command(IL0398_DEEP_SLEEP, buf, 1);
        }

        this->epdPower = false;

        delay(100);
    }

protected:

    /**
     * @brief Send the specific command to start writing to EPD display RAM.
     *        This will return a byte that is read while sending the data -> its usefull when bridging two SPI devices 
     * @param index The index for which buffer to write (0 or 1 or tri-color displays) Ignored for monochrome displays.
     * 
     * @returns The byte that is read from SPI at the same time as sending the command
     */
    inline uint8_t writeRAMCommand(uint8_t index) {
        if (index == 0) {
            return this->EPD_command_with_read(EPD_G4_RAM_BW);
        }
        if (index == 1) {
            return this->EPD_command_with_read(EPD_G4_RAM_RED);
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
        (void)x;
        (void)y;
    }

    /**
     * @brief  wait for busy signal to end - busy pin is LOW while Driver is working.
     *         wait for HIGH.
     * @returns void
     */
    inline void busy_wait() {
        if (this->_busy_pin >= 0) {
            while (!digitalRead(this->_busy_pin)) { // wait for busy HIGH
                this->EPD_command(IL0398_GETSTATUS);
                delay(100);
            }
            delay(200);
        } else {
            delay(EPD_G4_BUSY_DELAY);
        }
    }

public:

};

}
