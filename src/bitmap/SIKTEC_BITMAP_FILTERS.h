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
    #define BITMAP_RED_CH_MAX 255
    #define BITMAP_GREEN_CH_MAX 255
    #define BITMAP_BLUE_CH_MAX 255
#else 
    typedef uint16_t colorBits_t;
    #define BITMAP_RED_CH_MAX 31
    #define BITMAP_GREEN_CH_MAX 63
    #define BITMAP_BLUE_CH_MAX 31
#endif 

namespace SIKtec {

extern char debug_message[];
extern const int debug_message_len;


/**
 * @brief Two drawing color modes are supported 565 and 888
 * By default 565 is used unless a -D flag 'BITMAP_COLOR_RESULT_888'
 * is used when compiling
 */
enum BITMAP_COLOR_MODE : uint8_t {
    COLOR565,
    COLOR888
};

/**
 * @brief A global constant that stores the draw color mode used for compilation
 */
#ifdef BITMAP_COLOR_RESULT_888 
    typedef uint32_t colorBits_t;
    static const BITMAP_COLOR_MODE bitmap_color_result = BITMAP_COLOR_MODE::COLOR888;
#else 
    typedef uint16_t colorBits_t;
    static const BITMAP_COLOR_MODE bitmap_color_result = BITMAP_COLOR_MODE::COLOR565;
#endif 

//BITMAP FILTER ABSTRACTION:
class BITMAP_FILTER_IMPLEMENTATION {
    
    public:
    
    BITMAP_COLOR_MODE color_mode = bitmap_color_result;
    colorBits_t (*color_map)[4] = nullptr;
    uint16_t    map_size = 0;
    
    inline  void setColorMap( colorBits_t (*map)[4],  const uint16_t size) {
        this->map_size = size;
        this->color_map = map;
    }
    
    inline uint16_t colorDistance(const uint8_t R, const uint8_t G, const uint8_t B) {
        float fit = 100000;
        uint16_t index = 0;
        for (uint16_t i = 0; i < this->map_size; ++i) {
            float dist = sqrt(
                  pow(R - this->color_map[i][0], 2) 
                + pow(G - this->color_map[i][1], 2)
                + pow(B - this->color_map[i][2], 2)
            );
            if (dist == 0) {
                return i;
            }
            if (dist < fit) {
                fit = dist;
                index = i;
            }
        }
        return index;
    }
    
    inline uint16_t colorDistance(const uint16_t color) {
        uint8_t R;
        uint8_t G;
        uint8_t B;
        float fit = 100000;
        uint16_t index = 0;

        if (this->color_mode == BITMAP_COLOR_MODE::COLOR888) {
            R = color >> 16;
            G = (color >> 8) & 0xFF;
            B = color & 0xFF;
        } else {
            R = color >> 11;
            G = (color >> 5) & 0x3F;
            B = color & 0x1F;
        }
        
        for (uint16_t i = 0; i < this->map_size; ++i) {
            float dist = sqrt(
                  pow(R - this->color_map[i][0], 2) 
                + pow(G - this->color_map[i][1], 2)
                + pow(B - this->color_map[i][2], 2)
            );
            if (dist == 0) {
                return i;
            }
            if (dist < fit) {
                fit = dist;
                index = i;
            }
        }
        return index;
    }

    virtual colorBits_t kernel(const uint8_t R, const uint8_t G, const uint8_t B) = 0;
    
    protected:
};

class BitmapFilter_BW : public BITMAP_FILTER_IMPLEMENTATION {

    uint8_t threshold;
    public:

    inline BitmapFilter_BW(const uint8_t _threshold = 40) {
        float percent = (float)_threshold / 100;
        if (this->color_mode == BITMAP_COLOR_MODE::COLOR888) {
            this->threshold = percent * 255;
        } else {
            this->threshold = percent * 41;
        }
    }

    inline colorBits_t kernel(const uint8_t R, const uint8_t G, const uint8_t B) {
        
        //Convert to grey:
        uint16_t match;
        uint8_t  grey = ((uint16_t)R + G + B) / 3;
        if (this->color_mode == BITMAP_COLOR_MODE::COLOR888) {
            match = grey >= this->threshold 
                    ? this->colorDistance(255, 255, 255) 
                    : this->colorDistance(0, 0, 0);
        } else {
            match = grey >= this->threshold 
                    ? this->colorDistance(31, 63, 31) 
                    : this->colorDistance(0, 0, 0);
        }

        #if SIKTEC_EPD_DEBUG_BITMAP_KERNELS_PIXELS
            PRINT_DEBUG_BUFFER("RGB: %u,%u,%u Grey %d Thresh %d Color %d : %d \n", R, G, B, grey, this->threshold, match, this->color_map[match][3]);
        #endif

        return this->color_map[match][3];
    }
};

class BitmapFilter_BWR : public BITMAP_FILTER_IMPLEMENTATION {

    uint8_t threshold;
    int16_t red_index;
    int16_t red_thresh_g;
    int16_t red_thresh_b;
    public:

    inline BitmapFilter_BWR(const uint16_t _red_index, const uint8_t _threshold = 40, const uint8_t _reddish = 30) {
        float percent = (float)_threshold / 100;
        if (this->color_mode == BITMAP_COLOR_MODE::COLOR888) {
            this->threshold = percent * 255;
        } else {
            this->threshold = percent * 41;
        }
        percent = (float)_reddish / 100;
        if (this->color_mode == BITMAP_COLOR_MODE::COLOR888) {
            this->red_thresh_g = percent * 255;
            this->red_thresh_b = percent * 255;
        } else {
            this->red_thresh_g = percent * 12;
            this->red_thresh_b = percent * 22;
        }
        this->red_index = _red_index;
    }

    inline bool reddish(const int16_t R, const int16_t G, const int16_t B) {
        bool test = ((R-G) > this->red_thresh_g) && ((R-B) > this->red_thresh_b);
        #if SIKTEC_EPD_DEBUG_BITMAP_KERNELS_PIXELS
            PRINT_DEBUG_BUFFER("RGB: %u,%u,%u R-G %d R-B %d Test %d \n", R, G, B, (R-G), (R-B), test ? 1 : 0);
        #endif
        return ((R-G) > this->red_thresh_g) && ((R-B) > this->red_thresh_b);
    }

    inline colorBits_t kernel(const uint8_t R, const uint8_t G, const uint8_t B) {
        
        //Convert to grey:
        uint16_t match;
        uint8_t  grey;
        if (this->reddish(R, G, B) == true) {
            match = this->red_index;
        } else if (this->color_mode == BITMAP_COLOR_MODE::COLOR888) {
            grey = ((uint16_t)R + G + B) / 3;
            match = grey >= this->threshold 
                    ? this->colorDistance(255, 255, 255) 
                    : this->colorDistance(0, 0, 0);
        } else {
            grey = ((uint16_t)R + G + B) / 3;
            match = grey >= this->threshold 
                    ? this->colorDistance(31, 63, 31) 
                    : this->colorDistance(0, 0, 0);
        }
        #if SIKTEC_EPD_DEBUG_BITMAP_KERNELS_PIXELS
            PRINT_DEBUG_BUFFER("RGB: %u,%u,%u Grey %d Thresh %d Color %d : %d \n", R, G, B, grey, this->threshold, match, this->color_map[match][3]);
        #endif
        return this->color_map[match][3];
    }
};

class BitmapFilter_GRAY4 : public BITMAP_FILTER_IMPLEMENTATION {

    float lighten;

    public:

    inline BitmapFilter_GRAY4(const uint8_t _lighten = 40) {
        if (_lighten > 100) {
            this->lighten = 2;
        } else {
            this->lighten = 1 + ((float)_lighten / 100);
        }
    }

    inline colorBits_t kernel(const uint8_t R, const uint8_t G, const uint8_t B) {
        //Convert to grey:
        uint16_t  grey;
        uint16_t match;
        if (this->color_mode == BITMAP_COLOR_MODE::COLOR888) {
            grey  = (((uint16_t)R + G + (uint16_t)B) / 3) * this->lighten;
            grey = grey > 255 ? 255 : grey;
            match = this->colorDistance(grey, grey, grey);
        } else {
            grey  = (((uint16_t)R*2 + G + (uint16_t)B*2) / 3) * this->lighten;
            grey = grey > 63 ? 63 : grey;
            match = this->colorDistance(grey / 2, grey, grey / 2);
        }
        #if SIKTEC_EPD_DEBUG_BITMAP_KERNELS_PIXELS
            PRINT_DEBUG_BUFFER("RGB: %u,%u,%u Grey %d Thresh %d Color %d : %d \n", R, G, B, grey, this->threshold, match, this->color_map[match][3]);
        #endif
        return this->color_map[match][3];
    }
};

class BitmapFilter_QUANT : public BITMAP_FILTER_IMPLEMENTATION {


    public:

    inline colorBits_t kernel(const uint8_t R, const uint8_t G, const uint8_t B) {
        //Convert to grey:
        uint16_t match = this->colorDistance(R, G, B);
        #if SIKTEC_EPD_DEBUG_BITMAP_KERNELS_PIXELS
            PRINT_DEBUG_BUFFER("RGB: %u,%u,%u Grey %d Thresh %d Color %d : %d \n", R, G, B, grey, this->threshold, match, this->color_map[match][3]);
        #endif
        return this->color_map[match][3];
    }
};


#ifndef DITHER_WEIGHTS_VECTOR_FLOYD
    #define DITHER_WEIGHTS_VECTOR_FLOYD 7, 3, 5, 1
#endif

#ifndef DITHER_WEIGHTS_VECTOR_HORIZON
    #define DITHER_WEIGHTS_VECTOR_HORIZON 5, 3, 5, 5
#endif

#ifndef DITHER_WEIGHTS_VECTOR_BALANCED
    #define DITHER_WEIGHTS_VECTOR_BALANCED 4, 4, 4, 4
#endif

class BITMAP_DITHER_FILTER : public BITMAP_FILTER_IMPLEMENTATION {

    public:
    static  bool FORCE_RAM_BUFFER;
    virtual colorBits_t kernel(const uint8_t R, const uint8_t G, const uint8_t B) = 0;
    virtual void setWeightVector(const uint8_t right, const uint8_t down_left, const uint8_t down, const uint8_t down_right) = 0;
    virtual void dither(int16_t pixels[5]) = 0;
    virtual int16_t adjust_pixel_error(const int16_t pixel, const uint8_t weight, const float err) = 0;
};

//bool BITMAP_DITHER_FILTER::FORCE_RAM_BUFFER = false;

class BitmapFilter_DITHER_BW : public BITMAP_DITHER_FILTER {

    private:

    float    threshold;
    int16_t  level = 128;
    uint8_t  weights[4] = { DITHER_WEIGHTS_VECTOR_FLOYD };
    uint16_t color_black = 0x0000;
    uint16_t color_white = 0xFFFF;

    public:
    
    inline BitmapFilter_DITHER_BW(const uint16_t _color_black, const uint16_t _color_white, const float _threshold = 1) {
        this->threshold = _threshold;
        this->color_black = _color_black;
        this->color_white = _color_white;
        if (this->color_mode == BITMAP_COLOR_MODE::COLOR888) {
            this->level = 128;
        } else {
            this->level = 21;
        }
    }

    inline  void setWeightVector(const uint8_t right, const uint8_t down_left, const uint8_t down, const uint8_t down_right) {
        this->weights[0] = right;
        this->weights[1] = down_left;
        this->weights[2] = down;
        this->weights[3] = down_right;
    }

    inline colorBits_t kernel(const uint8_t R, const uint8_t G, const uint8_t B) {
        //Convert to grey:
        return (((uint16_t)R + G + B) / 3) * this->threshold;
    }

    inline void dither(int16_t pixels[5]) {
        // for (int i = 0; i < 5; ++i)
        //     Serial.printf("%d ", pixels[i]);
        int16_t old_c = pixels[0];
        int16_t new_c;
        int16_t quant;
        int16_t err;
        float err_adj;
        if (old_c > this->level) {
            quant = this->color_mode == BITMAP_COLOR_MODE::COLOR888 ? 255 : 41; // white
            new_c = this->color_white;
        } else {
            quant = 0; // black
            new_c = this->color_black;
        }
        err = old_c - quant;
        err_adj = (float)err / 16;
        // Serial.printf(" => Errors(%d): %d - %.3f", this->level, err, err_adj);
        // Serial.println();
        //Current pixel:
        pixels[0] = new_c; // The selected color
        //Right:
        pixels[1] = this->adjust_pixel_error(pixels[1], this->weights[0], err_adj);
        // //Down Left:
        pixels[2] = this->adjust_pixel_error(pixels[2], this->weights[1], err_adj);
        // //Down:
        pixels[3] = this->adjust_pixel_error(pixels[3], this->weights[2], err_adj);
        // //Down Right:
        pixels[4] = this->adjust_pixel_error(pixels[4], this->weights[3], err_adj);
    }

    inline int16_t adjust_pixel_error(const int16_t pixel, const uint8_t weight, const float err) {
        int adj = pixel + weight * err;
        adj = (adj < -32768) ? -32768 : adj;
        adj = (adj > 32768) ? 32768 : adj;
        //Serial.printf("        %d => Weight(%d): %d - %.3f \n", pixel, weight, adj, err);
        return  adj;
    }

};

class BitmapFilter_DITHER_GRAY4 : public BITMAP_DITHER_FILTER {

    private:

    float    threshold;
    int16_t  level = 128;
    uint8_t  weights[4] = { DITHER_WEIGHTS_VECTOR_FLOYD };

    public:
    
    inline BitmapFilter_DITHER_GRAY4(const float _threshold = 1) {
        this->threshold = _threshold;
        if (this->color_mode == BITMAP_COLOR_MODE::COLOR888) {
            this->level = 128;
        } else {
            this->level = 21;
        }
    }

    inline  void setWeightVector(const uint8_t right, const uint8_t down_left, const uint8_t down, const uint8_t down_right) {
        this->weights[0] = right;
        this->weights[1] = down_left;
        this->weights[2] = down;
        this->weights[3] = down_right;
    }

    inline uint16_t closest(const int16_t grey) {
        uint16_t index = 0;
        uint16_t close = 10000;
        uint16_t dist  = 0;
        for (uint16_t i = 0; i < this->map_size; ++i) {
            dist = abs(grey - this->color_map[i][0]);
            if (close > dist) {
                index = i;
                close = dist;
            }
        }
        return index;
    }

    inline colorBits_t kernel(const uint8_t R, const uint8_t G, const uint8_t B) {
        //Convert to grey:
        return (((uint16_t)R + G + B) / 3) * this->threshold;
    }

    inline void dither(int16_t pixels[5]) {
        //for (int i = 0; i < 5; ++i)
        //    Serial.printf("%d ", pixels[i]);
        int16_t  old_c   = pixels[0];
        uint16_t quant_i = this->closest(old_c);
        int16_t  new_c   = this->color_map[quant_i][3];
        int16_t  err     = old_c - this->color_map[quant_i][0];
        float    err_adj = (float)err / 16;
        //Serial.printf(" => Errors(%d): %d - %.3f", this->level, err, err_adj);
        //Serial.println();
        //Current pixel:
        pixels[0] = new_c; // The selected color
        //Right:
        pixels[1] = this->adjust_pixel_error(pixels[1], this->weights[0], err_adj);
        // //Down Left:
        pixels[2] = this->adjust_pixel_error(pixels[2], this->weights[1], err_adj);
        // //Down:
        pixels[3] = this->adjust_pixel_error(pixels[3], this->weights[2], err_adj);
        // //Down Right:
        pixels[4] = this->adjust_pixel_error(pixels[4], this->weights[3], err_adj);
    }

    inline int16_t adjust_pixel_error(const int16_t pixel, const uint8_t weight, const float err) {
        int adj = pixel + weight * err;
        adj = (adj < -32768) ? -32768 : adj;
        adj = (adj > 32768) ? 32768 : adj;
        // Serial.printf("        %d => Weight(%d): %d - %.3f \n", pixel, weight, adj, err);
        return  adj;
    }

};


}


