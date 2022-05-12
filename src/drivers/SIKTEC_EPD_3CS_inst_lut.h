/******************************************************************************/
// Created by: SIKTEC.
// Release Version : 1.0.2
// Creation Date: 2022-04-12
// Copyright 2022, SIKTEC.
// 
/******************************************************************************/

/**  @file SIKTEC_EPD_3CS_inst_lut.h */
#pragma once

//------------------------------------------------------------------------//
// Ram mode definitions for EPD:
//------------------------------------------------------------------------//
#define EPD_3CS_RAM_BW  0x10
#define EPD_3CS_RAM_RED 0x13


//------------------------------------------------------------------------//
// EPD DRIVER COMMANDS Driver SSD1619:
//------------------------------------------------------------------------//
#define SSD1619_DRIVER_CONTROL      0x01
#define SSD1619_GATE_VOLTAGE        0x03
#define SSD1619_SOURCE_VOLTAGE      0x04
#define SSD1619_PROGOTP_INITIAL     0x08
#define SSD1619_PROGREG_INITIAL     0x09
#define SSD1619_READREG_INITIAL     0x0A
#define SSD1619_BOOST_SOFTSTART     0x0C
#define SSD1619_DEEP_SLEEP          0x10
#define SSD1619_DEEP_SLEEP_OPTION_NORMAL    0x00
#define SSD1619_DEEP_SLEEP_OPTION_MODE1     0x01
#define SSD1619_DEEP_SLEEP_OPTION_MODE2     0x03

#define SSD1619_DATA_MODE       0x11
#define SSD1619_SW_RESET        0x12
#define SSD1619_TEMP_CONTROL    0x18
#define SSD1619_TEMP_WRITE      0x1A
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
#define SSD1619_MASTER_ACTIVATE     0x20
#define SSD1619_DISP_CTRL1          0x21
#define SSD1619_DISP_CTRL2          0x22
#define SSD1619_WRITE_RAM1          0x24
#define SSD1619_WRITE_RAM2          0x26
#define SSD1619_WRITE_VCOM          0x2C
#define SSD1619_READ_OTP            0x2D
#define SSD1619_READ_STATUS         0x2F
#define SSD1619_WRITE_LUT           0x32
#define SSD1619_WRITE_BORDER        0x3C
#define SSD1619_SET_RAMXPOS         0x44
#define SSD1619_SET_RAMYPOS         0x45
#define SSD1619_SET_RAMXCOUNT       0x4E
#define SSD1619_SET_RAMYCOUNT       0x4F
#define SSD1619_SET_ANALOGBLOCK     0x74
#define SSD1619_SET_DIGITALBLOCK    0x7E


//------------------------------------------------------------------------//
// EPD DRIVER INIT SEQUENCES:
//------------------------------------------------------------------------//

/** @brief init sequence for ssd1619 driver */
static const uint8_t ssd1619_default_init_code[] {
    SSD1619_SW_RESET,           0,                  // soft reset
    EPD_CMD_SEQUENCE_WAIT,      20,                 // busy wait
    SSD1619_SET_ANALOGBLOCK,    1,      0x54,       // set analog block control
    SSD1619_SET_DIGITALBLOCK,   1,      0x3B,       // set digital block control
    SSD1619_DRIVER_CONTROL,     3,      0x2B, 0x01, 0x00, // Set MUX as 300
    SSD1619_DATA_MODE,          1,      0x03,       // Ram data entry mode
    SSD1619_WRITE_BORDER,       1,      0x01,       // border color
    SSD1619_TEMP_CONTROL,       1,      0x80,       // Temp control
    SSD1619_DISP_CTRL2,         1,      0xB1,
    SSD1619_MASTER_ACTIVATE,    0,
    EPD_CMD_SEQUENCE_WAIT,      20,                 // busy wait
    EPD_CMD_SEQUENCE_END
};

//------------------------------------------------------------------------//
// LOOK UP TABLES:
//------------------------------------------------------------------------//


