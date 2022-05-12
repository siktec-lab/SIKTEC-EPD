/******************************************************************************/
// Created by: SIKTEC.
// Release Version : 1.0.2
// Creation Date: 2022-04-12
// Copyright 2022, SIKTEC.
// 
/******************************************************************************/

/**  @file SIKTEC_EPD_G4_inst_lut.h */
#pragma once



//------------------------------------------------------------------------//
// Ram mode definitions for EPD:
//------------------------------------------------------------------------//
#define EPD_3CU_RAM_BW  0x10
#define EPD_3CU_RAM_RED 0x13

//------------------------------------------------------------------------//
// EPD DRIVER COMMANDS Driver UC8276:
//------------------------------------------------------------------------//
#define UC8276_PANELSETTING     0x00
#define UC8276_POWEROFF         0x02
#define UC8276_POWERON          0x04
#define UC8276_DEEPSLEEP        0x07
#define UC8276_DISPLAY_REFRESH  0x12
#define UC8276_WRITE_RAM1       0x10
#define UC8276_WRITE_RAM2       0x13
#define UC8276_WRITE_VCOM       0x50
#define UC8276_GET_STATUS       0x71

#ifndef EPD_CMD_SEQUENCE_END 
    #define EPD_CMD_SEQUENCE_END    0XFE
#endif
#ifndef EPD_CMD_SEQUENCE_WAIT 
    #define EPD_CMD_SEQUENCE_WAIT   0XFF
#endif
//------------------------------------------------------------------------//
// EPD DRIVER INIT SEQUENCES:
//------------------------------------------------------------------------//

/** @brief init sequence for mono and general use */
static const uint8_t uc8276_default_init_code[] {
    UC8276_POWERON,             0,              // soft reset
    EPD_CMD_SEQUENCE_WAIT,      50,             // busy wait
    UC8276_PANELSETTING,        1,      0x0f,   // LUT from OTP
    UC8276_WRITE_VCOM,          1,      0xD7,
    EPD_CMD_SEQUENCE_END
};

//------------------------------------------------------------------------//
// LOOK UP TABLES:
//------------------------------------------------------------------------//
