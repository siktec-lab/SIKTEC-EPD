/******************************************************************************/
// Created by: SIKTEC.
// Release Version : 1.0.2
// Creation Date: 2022-04-12
// Copyright 2022, SIKTEC.
/******************************************************************************/

/**  @file SIKTEC_EPD_COLOR_KERNELS.cpp */

#include "SIKTEC_EPD_COLOR_KERNELS.h"

namespace SIKtec {

#ifdef BITMAP_COLOR_RESULT_888 

    /**
     * @brief Defaults for 4GRAY color filter WHEN 888 mode compile flag is used:
     */
    //Thresh is 0 -> 255
    uint8_t  BITMAP_G4_COLOR_KERNEL::black_thresh = 0;
    uint8_t  BITMAP_G4_COLOR_KERNEL::dark_thresh  = 60;
    uint8_t  BITMAP_G4_COLOR_KERNEL::light_thresh = 160;
    uint8_t  BITMAP_G4_COLOR_KERNEL::white_thresh = 210;
    colorBits_t BITMAP_G4_COLOR_KERNEL::black_color  = 0;
    colorBits_t BITMAP_G4_COLOR_KERNEL::dark_color   = (colorBits_t)0x00404040;
    colorBits_t BITMAP_G4_COLOR_KERNEL::light_color  = (colorBits_t)0x00A0A0A0;
    colorBits_t BITMAP_G4_COLOR_KERNEL::white_color  = (colorBits_t)0x00FFFFFF;

    /**
     * @brief Defaults for BW color filter WHEN 888 mode compile flag is used:
     */
    uint8_t     BITMAP_BW_COLOR_KERNEL::greylevel_thresh   = 200;
    colorBits_t BITMAP_BW_COLOR_KERNEL::black_color        = 0;
    colorBits_t BITMAP_BW_COLOR_KERNEL::white_color        = (colorBits_t)0x00FFFFFF;

    /**
     * @brief Defaults for BWR color filter WHEN 888 mode compile flag is used:
     */
    uint8_t BITMAP_BWR_COLOR_KERNEL::greylevel_thresh   = 200;
    uint8_t BITMAP_BWR_COLOR_KERNEL::red_r_thresh       = 90; // Bigger then 150
    uint8_t BITMAP_BWR_COLOR_KERNEL::red_g_thresh       = 55;  // Smaller then 50
    uint8_t BITMAP_BWR_COLOR_KERNEL::red_b_thresh       = 55;  // Smaller then 40
    colorBits_t BITMAP_BWR_COLOR_KERNEL::black_color    = 0x00000000;
    colorBits_t BITMAP_BWR_COLOR_KERNEL::white_color    = (colorBits_t)0x00FFFFFF;
    colorBits_t BITMAP_BWR_COLOR_KERNEL::red_color      = (colorBits_t)0x00FF0000;

#else 

    /**
     * @brief Defaults for 4GRAY color filter WHEN 565 (default) mode compile flag is used:
     */
    //Thresh is 0 -> 41
    uint8_t     BITMAP_G4_COLOR_KERNEL::black_thresh = 0;
    uint8_t     BITMAP_G4_COLOR_KERNEL::dark_thresh  = 5;
    uint8_t     BITMAP_G4_COLOR_KERNEL::light_thresh = 22;
    uint8_t     BITMAP_G4_COLOR_KERNEL::white_thresh = 35;
    colorBits_t BITMAP_G4_COLOR_KERNEL::black_color  = 0;
    colorBits_t BITMAP_G4_COLOR_KERNEL::dark_color   = (colorBits_t)0x4208;
    colorBits_t BITMAP_G4_COLOR_KERNEL::light_color  = (colorBits_t)0xA514;
    colorBits_t BITMAP_G4_COLOR_KERNEL::white_color  = (colorBits_t)0xFFFF;


    /**
     * @brief Defaults for BW color filter WHEN 565 (default) mode compile flag is used:
     */
    uint8_t     BITMAP_BW_COLOR_KERNEL::greylevel_thresh   = 30;
    colorBits_t BITMAP_BW_COLOR_KERNEL::black_color        = 0;
    colorBits_t BITMAP_BW_COLOR_KERNEL::white_color        = (colorBits_t)0xFFFF;

    /**
     * @brief Defaults for BWR color filter WHEN 565 (default) mode compile flag is used:
     */
    uint8_t BITMAP_BWR_COLOR_KERNEL::greylevel_thresh   = 30;
    uint8_t BITMAP_BWR_COLOR_KERNEL::red_r_thresh       = 18;  // Bigger then 18
    uint8_t BITMAP_BWR_COLOR_KERNEL::red_g_thresh       = 13;  // Smaller then 18
    uint8_t BITMAP_BWR_COLOR_KERNEL::red_b_thresh       = 8;   // Smaller then 12
    colorBits_t BITMAP_BWR_COLOR_KERNEL::black_color    = 0x0000;
    colorBits_t BITMAP_BWR_COLOR_KERNEL::white_color    = (colorBits_t)0xFFFF;
    colorBits_t BITMAP_BWR_COLOR_KERNEL::red_color      = (colorBits_t)0xF800;

#endif

}
