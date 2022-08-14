/******************************************************************************/
// Created by: SIKTEC.
// Release Version : 1.0.5
// Creation Date: 2022-04-12
// Copyright 2022, SIKTEC.
// 
/******************************************************************************/

/**  @file SIKTEC_EPD_G4_inst_lut.h */
#pragma once


//------------------------------------------------------------------------//
// Ram mode definitions for EPD:
//------------------------------------------------------------------------//
#define EPD_G4_RAM_BW   0x10
#define EPD_G4_RAM_RED  0x13
#define EPD_G4_RES_HORIZONTAL 400
#define EPD_G4_RES_VERTICAL   300
//------------------------------------------------------------------------//
// EPD DRIVER COMMANDS Driver IL0398:
//------------------------------------------------------------------------//
#define IL0398_PANEL_SETTING        0x00
#define IL0398_POWER_SETTING        0x01
#define IL0398_POWER_OFF            0x02
#define IL0398_POWER_OFF_SEQUENCE   0x03
#define IL0398_POWER_ON             0x04
#define IL0398_POWER_ON_MEASURE     0x05
#define IL0398_BOOSTER_SOFT_START   0x06
#define IL0398_DEEP_SLEEP           0x07
#define IL0398_DTM1                 0x10
#define IL0398_DATA_STOP            0x11
#define IL0398_DISPLAY_REFRESH      0x12
#define IL0398_DTM2                 0x13
#define IL0398_PDTM1                0x14
#define IL0398_PDTM2                0x15
#define IL0398_PDRF                 0x16
#define IL0398_LUT1                 0x20
#define IL0398_LUTWW                0x21
#define IL0398_LUTBW                0x22
#define IL0398_LUTWB                0x23
#define IL0398_LUTBB                0x24
#define IL0398_PLL                  0x30
#define IL0398_TEMPCALIBRATE    0x40
#define IL0398_TEMPSELECT       0x41
#define IL0398_TEMPWRITE        0x42
#define IL0398_TEMPREAD         0x43
#define IL0398_VCOM             0x50
#define IL0398_LOWPOWERDETECT   0x51
#define IL0398_TCON             0x60
#define IL0398_RESOLUTION       0x61
#define IL0398_GSSTSETTING      0x65
#define IL0398_REVISION         0x70
#define IL0398_GETSTATUS        0x71
#define IL0398_AUTOVCOM         0x80
#define IL0398_READVCOM         0x81
#define IL0398_VCM_DC_SETTING   0x82
#define IL0398_PARTWINDOW       0x90
#define IL0398_PARTIALIN        0x91
#define IL0398_PARTIALOUT       0x92
#define IL0398_PROGRAMMODE      0xA0
#define IL0398_ACTIVEPROGRAM    0xA1
#define IL0398_READOTP          0xA2
#define IL0398_CASCADESET       0xE0
#define IL0398_POWERSAVING      0xE3
#define IL0398_FORCETEMP        0xE5

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
static const uint8_t il0398_default_init_code[] {
    EPD_CMD_SEQUENCE_WAIT,          50,                            // busy wait
    IL0398_BOOSTER_SOFT_START,      3,      0x17, 0x17, 0x17,
    IL0398_POWER_ON,                0,
    EPD_CMD_SEQUENCE_WAIT,          50,                            // busy wait
    IL0398_PANEL_SETTING,           2,      0x1F, 0x0D,            // 00011111 00001101 // lut from OTP & VCOM = 0v
    IL0398_VCOM,                    1,      0x97,                  // 1001 0111
    EPD_CMD_SEQUENCE_END
};

/** @brief init sequence for gray4 */
static const uint8_t ti_420t2_gray4_init_code[] {
    IL0398_POWER_SETTING,           4,      0x03, 0x00, 0x2b, 0x2b,// 0x3 frm 13 to 3
    IL0398_BOOSTER_SOFT_START,      3,      0x17, 0x17, 0x17,
    IL0398_POWER_ON,                0,
    EPD_CMD_SEQUENCE_WAIT,          200,
    IL0398_PANEL_SETTING,           1,      0x3F, //LUT from OTP BF  KWR-AF BWROTP-0f BWOTP-1f EXTERNAL-3F
    IL0398_PLL,                     1,      0x3C, // 3C 50Hz, 3A 100Hz, 29 150Hz, 39 200Hz, 31 171Hz
    IL0398_RESOLUTION,              4,      EPD_G4_RES_HORIZONTAL / 256, EPD_G4_RES_HORIZONTAL % 256, EPD_G4_RES_VERTICAL / 256, EPD_G4_RES_VERTICAL % 256,
    IL0398_VCM_DC_SETTING,          1,      0x12, // 08 -0.5V, 12 -1.0V, 1C -1.5V
    IL0398_VCOM,                    1,      0xd7, // WBmode:VBDF D7 VBDW 97 VBDB 57 - WBRmode:VBDF F7 VBDW 77 VBDB 37 VBDR B7
    EPD_CMD_SEQUENCE_END
};

//------------------------------------------------------------------------//
// LOOK UP TABLES:
//------------------------------------------------------------------------//

static const uint8_t ti_420t2_gray4_lut_code[] = {
  IL0398_LUT1, 42,
  0x00, 0x0A, 0x00, 0x00, 0x00, 0x01,
  0x60, 0x14, 0x14, 0x00, 0x00, 0x01,
  0x00, 0x14, 0x00, 0x00, 0x00, 0x01,
  0x00, 0x13, 0x0A, 0x01, 0x00, 0x01,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  IL0398_LUTWW, 42, 
  0x40, 0x0A, 0x00, 0x00, 0x00, 0x01,
  0x90, 0x14, 0x14, 0x00, 0x00, 0x01,
  0x10, 0x14, 0x0A, 0x00, 0x00, 0x01,
  0xA0, 0x13, 0x01, 0x00, 0x00, 0x01,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  IL0398_LUTBW, 42,
  0x40, 0x0A, 0x00, 0x00, 0x00, 0x01,
  0x90, 0x14, 0x14, 0x00, 0x00, 0x01,
  0x00, 0x14, 0x0A, 0x00, 0x00, 0x01,
  0x99, 0x0C, 0x01, 0x03, 0x04, 0x01,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  IL0398_LUTWB, 42,
  0x40, 0x0A, 0x00, 0x00, 0x00, 0x01,
  0x90, 0x14, 0x14, 0x00, 0x00, 0x01,
  0x00, 0x14, 0x0A, 0x00, 0x00, 0x01,
  0x99, 0x0B, 0x04, 0x04, 0x01, 0x01,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  IL0398_LUTBB, 42,
  0x80, 0x0A, 0x00, 0x00, 0x00, 0x01,
  0x90, 0x14, 0x14, 0x00, 0x00, 0x01,
  0x20, 0x14, 0x0A, 0x00, 0x00, 0x01,
  0x50, 0x13, 0x01, 0x00, 0x00, 0x01,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  EPD_CMD_SEQUENCE_END
};
