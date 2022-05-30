/******************************************************************************/
// Created by: SIKTEC.
// Release Version : 1.0.2
// Creation Date: 2022-04-12
// Copyright 2022, SIKTEC.
// 
/******************************************************************************/

/**  @file SIKTEC_EPD_COLOR_KERNELS.h */

#pragma once 

#include <Arduino.h>


//-----------------------------------------------------------------------------------------//
// BASIC MACROS FOR DEBUGGING:
//-----------------------------------------------------------------------------------------//

#ifndef SIKTEC_EPD_DEBUG_BITMAP 
    #define SIKTEC_EPD_DEBUG_BITMAP 0
#endif

#ifndef SIKTEC_EPD_DEBUG_BITMAP_KERNELS_PIXELS 
    #define SIKTEC_EPD_DEBUG_BITMAP_KERNELS_PIXELS 0
#endif

#ifndef PRINT_DEBUG_BUFFER
#define PRINT_DEBUG_BUFFER(__template, ...) \
    sprintf(debug_message, __template, __VA_ARGS__); \
    Serial.print(debug_message)
#endif

//-----------------------------------------------------------------------------------------//
// SET the COLOR mode we are going to use:
//-----------------------------------------------------------------------------------------//

#ifdef BITMAP_COLOR_RESULT_888 
    typedef uint32_t colorBits_t;
#else 
    typedef uint16_t colorBits_t;
#endif 


namespace SIKtec {

extern char debug_message[];
extern const int debug_message_len;

/**
 * @brief A 4 grays color kernel to support direct translation 
 *        for epd G4 displays.
 *        colors should be adjusted by the color before use 
 *        as EPDs dont take ARGB color formats.
 */
class BITMAP_G4_COLOR_KERNEL {
    
    private:
    
    static uint8_t  black_thresh;
    static uint8_t  dark_thresh;
    static uint8_t  light_thresh;
    static uint8_t  white_thresh;
    static colorBits_t black_color;
    static colorBits_t dark_color;
    static colorBits_t light_color;
    static colorBits_t white_color;

    public:

    /**
     * @brief Set the Threshold values for converting to the unique EPD grayscale.
     * 
     * @param black     - 0->41 in 565 mode or 0->255 in 888 mode
     * @param dark      - 0->41 in 565 mode or 0->255 in 888 mode
     * @param light     - 0->41 in 565 mode or 0->255 in 888 mode
     * @param white     - 0->41 in 565 mode or 0->255 in 888 mode
     * @return void
     */
    inline static void setThreshold(uint8_t black, uint8_t dark, uint8_t light, uint8_t white) {
        BITMAP_G4_COLOR_KERNEL::black_thresh   = black;
        BITMAP_G4_COLOR_KERNEL::dark_thresh    = dark;
        BITMAP_G4_COLOR_KERNEL::light_thresh   = light;
        BITMAP_G4_COLOR_KERNEL::white_thresh   = white;
    }

    /**
     * @brief Set the Desired output Colors.
     * 
     * @param black 
     * @param dark 
     * @param light 
     * @param white 
     * @return void
     */
    inline static void setDesiredColors(colorBits_t black, colorBits_t dark, colorBits_t light, colorBits_t white) {
        BITMAP_G4_COLOR_KERNEL::black_color   = black;
        BITMAP_G4_COLOR_KERNEL::dark_color    = dark;
        BITMAP_G4_COLOR_KERNEL::light_color   = light;
        BITMAP_G4_COLOR_KERNEL::white_color   = white;
    }

    /**
     * @brief The color kernel which is called on every pixel
     * 
     * @param R 
     * @param G 
     * @param B 
     * @return colorBits_t 
     */
    inline static colorBits_t kernel(const uint8_t R, const uint8_t G, const uint8_t B) {
        //Convert to grey:
        #ifdef BITMAP_COLOR_RESULT_888 
            uint8_t grey = B;
            if (R != G || G != B) {
                grey = ((uint16_t)R+G+B) / 3;
            }
        #else
            uint8_t grey;
            if (R == B && G == 63) {
                grey = 41;
            } else if (R == 0 && R == G && G == B) {
                grey = 0;
            } else {
                grey = ((uint16_t)R+G+B) / 3;
            }
        #endif
        
        #if SIKTEC_EPD_DEBUG_BITMAP_KERNELS_PIXELS
            PRINT_DEBUG_BUFFER("\nR %d G %d B %d --> %d\n", R, G, B, grey);
        #endif
        
        if (grey > BITMAP_G4_COLOR_KERNEL::white_thresh)
            return BITMAP_G4_COLOR_KERNEL::white_color;
        
        if (grey > BITMAP_G4_COLOR_KERNEL::light_thresh)
            return BITMAP_G4_COLOR_KERNEL::light_color;
        
        if (grey > BITMAP_G4_COLOR_KERNEL::dark_thresh)
            return BITMAP_G4_COLOR_KERNEL::dark_color;
        
        return BITMAP_G4_COLOR_KERNEL::black_color;
    }
};

/**
 * @brief A BW kernel to support direct translation 
 *        for epd MONO displays.
 *        colors should be adjusted by the color before use 
 *        as EPDs dont take ARGB color formats.
 */
class BITMAP_BW_COLOR_KERNEL {

    private:
    
    static uint8_t      greylevel_thresh;
    static colorBits_t  black_color;
    static colorBits_t  white_color;
    
    public:
    
    /**
     * @brief Set the Threshold values for converting to the BW.
     * 
     * @param greylevel - 0->41 in 565 mode or 0->255 in 888 mode
     * @return void
     */
    inline static void setThreshold(uint8_t greylevel) {
        BITMAP_BW_COLOR_KERNEL::greylevel_thresh = greylevel;
    }
    
    /**
     * @brief Set the Desired output Colors.
     * 
     * @param black 
     * @param white 
     * @return void
     */
    inline static void setDesiredColors(colorBits_t black, colorBits_t white) {
        BITMAP_BW_COLOR_KERNEL::black_color   = black;
        BITMAP_BW_COLOR_KERNEL::white_color   = white;
    }

    /**
     * @brief The color kernel which is called on every pixel
     * 
     * @param R 
     * @param G 
     * @param B 
     * @return colorBits_t 
     */
    inline static colorBits_t kernel(const uint8_t R, const uint8_t G, const uint8_t B) {
        //Convert to grey:
        #ifdef BITMAP_COLOR_RESULT_888 
            uint8_t grey = B;
            if (R != G || G != B) {
                grey = ((uint16_t)R+G+B) / 3;
            }
        #else
            uint8_t grey;
            if (R == B && G == 63) {
                grey = 41;
            } else if (R == 0 && R == G && G == B) {
                grey = 0;
            } else {
                grey = ((uint16_t)R+G+B) / 3;
            }
        #endif

        #if SIKTEC_EPD_DEBUG_BITMAP_KERNELS_PIXELS
            PRINT_DEBUG_BUFFER("\nR %d G %d B %d --> %d\n", R, G, B, grey);
        #endif
        if (grey >= BITMAP_BW_COLOR_KERNEL::greylevel_thresh)
            return BITMAP_BW_COLOR_KERNEL::white_color;
        return BITMAP_BW_COLOR_KERNEL::black_color;
    }
};

/**
 * @brief A BWR kernel to support direct translation 
 *        for epd BWR displays.
 *        colors should be adjusted by the color before use 
 *        as EPDs dont take ARGB color formats.
 */
class BITMAP_BWR_COLOR_KERNEL {

    private:

    static uint8_t      greylevel_thresh;
    static uint8_t      red_r_thresh;
    static uint8_t      red_g_thresh;
    static uint8_t      red_b_thresh;
    static colorBits_t  black_color;
    static colorBits_t  white_color;
    static colorBits_t  red_color;

    public:

    /**
     * @brief Set the Threshold values for converting to the BWR.
     * RED values is cons
     * 
     * @param greylevel - 0->41 in 565 mode or 0->255 in 888 mode
     * @param red_r     - Bigger then uint8_t (0->31 in 565 mode or 0->255 in 888 mode)
     * @param red_g     - Smaller then uint8_t (0->63 in 565 mode or 0->255 in 888 mode)
     * @param red_b     - Smaller then uint8_t (0->31 in 565 mode or 0->255 in 888 mode)
     * @return void
     */
    inline static void setThreshold(uint8_t greylevel, uint8_t red_r, uint8_t red_g, uint8_t red_b) {
        BITMAP_BWR_COLOR_KERNEL::greylevel_thresh = greylevel;
        BITMAP_BWR_COLOR_KERNEL::red_r_thresh     = red_r;
        BITMAP_BWR_COLOR_KERNEL::red_g_thresh     = red_g;
        BITMAP_BWR_COLOR_KERNEL::red_b_thresh     = red_b;
    }

    /**
     * @brief Set the Desired output Colors.
     * 
     * @param black 
     * @param white 
     * @param red 
     * @return void
     */
    inline static void setDesiredColors(colorBits_t black, colorBits_t white, colorBits_t red) {
        BITMAP_BWR_COLOR_KERNEL::black_color   = black;
        BITMAP_BWR_COLOR_KERNEL::white_color   = white;
        BITMAP_BWR_COLOR_KERNEL::red_color     = red;
    }

    /**
     * @brief The color kernel which is called on every pixel
     * 
     * @param R 
     * @param G 
     * @param B 
     * @return colorBits_t 
     */
    inline static colorBits_t kernel(const uint8_t R, const uint8_t G, const uint8_t B) {
        
        //Is in the Red Range: 
        if (
            R >= BITMAP_BWR_COLOR_KERNEL::red_r_thresh &&
            G <= BITMAP_BWR_COLOR_KERNEL::red_g_thresh &&
            B <= BITMAP_BWR_COLOR_KERNEL::red_b_thresh
        ) {
            #if SIKTEC_EPD_DEBUG_BITMAP_KERNELS_PIXELS
                PRINT_DEBUG_BUFFER("\nR %d G %d B %d --> %d\n", R, G, B, BITMAP_BWR_COLOR_KERNEL::red_color);
            #endif
            return BITMAP_BWR_COLOR_KERNEL::red_color;
        } 

        //Convert to grey For BW:
        #ifdef BITMAP_COLOR_RESULT_888 
            uint8_t grey = B;
            if (R != G || G != B) {
                grey = ((uint16_t)R+G+B) / 3;
            }
        #else
            uint8_t grey;
            if (R == B && G == 63) {
                grey = 41;
            } else if (R == 0 && R == G && G == B) {
                grey = 0;
            } else {
                grey = ((uint16_t)R+G+B) / 3;
            }
        #endif

        #if SIKTEC_EPD_DEBUG_BITMAP_KERNELS_PIXELS
            PRINT_DEBUG_BUFFER("\nR %d G %d B %d --> %d\n", R, G, B, grey);
        #endif

        if (grey >= BITMAP_BWR_COLOR_KERNEL::greylevel_thresh)
            return BITMAP_BWR_COLOR_KERNEL::white_color;
        return BITMAP_BWR_COLOR_KERNEL::black_color;
    }
};

}
