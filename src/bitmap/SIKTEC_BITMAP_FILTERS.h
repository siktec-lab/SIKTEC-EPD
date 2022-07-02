/******************************************************************************/
// Created by: SIKTEC.
// Release Version : 1.0.3
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
    #define BITMAP_RED_CH_MAX   255
    #define BITMAP_GREEN_CH_MAX 255
    #define BITMAP_BLUE_CH_MAX  255
#else 
    typedef uint16_t colorBits_t;
    #define BITMAP_RED_CH_MAX   31
    #define BITMAP_GREEN_CH_MAX 63
    #define BITMAP_BLUE_CH_MAX  31
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


//NOTE: shlomi implemented this one much faster sqrt() taken from quake hack...
/** 
 * @brief faster sqrt approximation 
**/
#define SQRT_MAGIC_F 0x5f3759df 
inline float  sqrt2(const float x)
{
  const float xhalf = 0.5f*x;
  union // get bits for floating value
  {
    float x;
    int i;
  } u;
  u.x = x;
  u.i = SQRT_MAGIC_F - (u.i >> 1);  // gives initial guess y0
  return x*u.x*(1.5f - xhalf*u.x*u.x);// Newton step, repeating increases accuracy 
}

//BITMAP FILTER ABSTRACTION:
class BITMAP_FILTER_IMPLEMENTATION {
    
    public:
    
    BITMAP_COLOR_MODE color_mode = bitmap_color_result; ///< the color mode we are operating at 
    colorBits_t (*color_map)[4] = nullptr; ///< color map 
    uint16_t    map_size = 0; ///< map size
    
    /**
     * @brief Set the Color Map object
     * 
     * @param map - N length of 4 colorBits_t sized elements { R, G, B, EPD_COLOR }
     * @param size - The color map size
     */
    inline  void setColorMap( colorBits_t (*map)[4],  const uint16_t size) {
        this->map_size = size;
        this->color_map = map;
    }
    
    /**
     * @brief returns the index of the closest color
     * 
     * @param R 
     * @param G 
     * @param B 
     * @return uint16_t - the closest color index
     */
    inline uint16_t colorDistance(const uint8_t R, const uint8_t G, const uint8_t B) {
        int16_t fit = 10000;
        uint8_t index = 0;
        int16_t dist;
        int16_t dR;
        int16_t dG;
        int16_t dB;
        for (uint16_t i = 0; i < this->map_size; ++i) {
            dR = R - this->color_map[i][0];
            dG = G - this->color_map[i][1];
            dB = B - this->color_map[i][2];
            //NOTE: shlomi implemented this one much faster then pow()...
            dist = sqrt2(dR*dR + dG*dG + dB*dB);
            // dist = sqrt(
            //       pow(R - this->color_map[i][0], 2) 
            //     + pow(G - this->color_map[i][1], 2)
            //     + pow(B - this->color_map[i][2], 2)
            // );
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
        int16_t fit = 10000;
        uint8_t index = 0;
        int16_t dist;
        int16_t dR;
        int16_t dG;
        int16_t dB;
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
            dR = R - this->color_map[i][0];
            dG = G - this->color_map[i][1];
            dB = B - this->color_map[i][2];
            //NOTE: shlomi implemented this one much faster pow()...
            dist = sqrt2(dR*dR + dG*dG + dB*dB);
            // dist = sqrt(
            //       pow(R - this->color_map[i][0], 2) 
            //     + pow(G - this->color_map[i][1], 2)
            //     + pow(B - this->color_map[i][2], 2)
            // );
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

    /**
     * @brief called on all pixels - apply the filter logic here.
     * 
     * @param R 
     * @param G 
     * @param B 
     * @return colorBits_t 
     */
    virtual colorBits_t kernel(const uint8_t R, const uint8_t G, const uint8_t B) = 0;
    
    protected:
};

/**
 * @brief B & W filter
 */
class BitmapFilter_BW : public BITMAP_FILTER_IMPLEMENTATION {

    uint8_t threshold; ///< threshold value

    /**
     * @brief black and white cached colors
     */
    struct { 
        uint8_t     has = 0;
        colorBits_t black;
        colorBits_t white;
    } cache;

    public:

    /**
     * @brief Construct a new BitmapFilter_BW object
     *  
     * @param _threshold the threshold color 0 - 100 
     */
    inline BitmapFilter_BW(const uint8_t _threshold = 40) {
        float percent = (float)_threshold / 100;
        if (this->color_mode == BITMAP_COLOR_MODE::COLOR888) {
            this->threshold = percent * 255;
        } else {
            this->threshold = percent * 41;
        }
    }

    /**
     * @brief called on all pixels - calculates the greyscale and returns the black or white color
     *        based on the threshold.
     * @param R 
     * @param G 
     * @param B 
     * @return colorBits_t 
     */
    inline colorBits_t kernel(const uint8_t R, const uint8_t G, const uint8_t B) {
        //Convert to grey:
        #if SIKTEC_EPD_DEBUG_BITMAP_KERNELS_PIXELS
        colorBits_t match;
        #endif
        uint8_t  grey = ((uint16_t)R + G + B) / 3;
        if (this->cache.has) {
            #if SIKTEC_EPD_DEBUG_BITMAP_KERNELS_PIXELS
                match = grey >= this->threshold ? cache.white : cache.black;
            #else 
                return grey >= this->threshold ? cache.white : cache.black;
            #endif
        } else if (this->color_mode == BITMAP_COLOR_MODE::COLOR888) {
            this->cache.white = this->color_map[this->colorDistance(255, 255, 255)][3];
            this->cache.black = this->color_map[this->colorDistance(0,0,0)][3];
            this->cache.has = 1;
            #if SIKTEC_EPD_DEBUG_BITMAP_KERNELS_PIXELS
                match = grey >= this->threshold ? cache.white : cache.black;
            #else 
                return grey >= this->threshold ? cache.white : cache.black;
            #endif
        } else {
            this->cache.white = this->color_map[this->colorDistance(31, 63, 31)][3];
            this->cache.black = this->color_map[this->colorDistance(0,0,0)][3];
            this->cache.has = 1;
            #if SIKTEC_EPD_DEBUG_BITMAP_KERNELS_PIXELS
                match = grey >= this->threshold ? cache.white : cache.black;
            #else 
                return grey >= this->threshold ? cache.white : cache.black;
            #endif
        }
        #if SIKTEC_EPD_DEBUG_BITMAP_KERNELS_PIXELS
            PRINT_DEBUG_BUFFER("RGB: %u,%u,%u Grey %d Thresh %d Color %d \n", R, G, B, grey, this->threshold, match);
            return match;
        #endif
    }
};

/**
 * @brief B & W & RED filter
 */
class BitmapFilter_BWR : public BITMAP_FILTER_IMPLEMENTATION {

    uint8_t threshold;      ///< threshold value for B & W
    int16_t red_index;      ///< red color index in the colormap
    int16_t red_thresh_g;   ///< threshold green channel
    int16_t red_thresh_b;   ///< threshold blue channel

    /**
     * @brief black and white cached colors
     */
    struct {
        uint8_t     has = 0;
        colorBits_t black;
        colorBits_t white;
    } cache;

    public:

    /**
     * @brief Construct a new BitmapFilter_BWR object
     * 
     * @param _red_index red color index in the colormap
     * @param _threshold threshold value for B & W
     * @param _reddish   threshold value for RED
     */
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

    /**
     * @brief checks if a color is reddish 
     * 
     * @param R 
     * @param G 
     * @param B 
     * @return bool
     */
    inline bool reddish(const int16_t R, const int16_t G, const int16_t B) {
        #if SIKTEC_EPD_DEBUG_BITMAP_KERNELS_PIXELS
            bool test = ((R-G) > this->red_thresh_g) && ((R-B) > this->red_thresh_b);
            PRINT_DEBUG_BUFFER("RGB: %u,%u,%u R-G %d R-B %d Test %d \n", R, G, B, (R-G), (R-B), test ? 1 : 0);
        #endif
        return ((R-G) > this->red_thresh_g) && ((R-B) > this->red_thresh_b);
    }

    /**
     * @brief called on all pixels - calculates the reddish and greyscale to determine Red Black or White.
     * @param R 
     * @param G 
     * @param B 
     * @return colorBits_t 
     */
    inline colorBits_t kernel(const uint8_t R, const uint8_t G, const uint8_t B) {
        
        //Convert to grey:
        colorBits_t match;
        uint8_t  grey;
        if (this->reddish(R, G, B) == true) {
            match = this->color_map[this->red_index][3];
        } else if (this->cache.has) {
            grey = ((uint16_t)R + G + B) / 3;
            match = grey >= this->threshold ? cache.white : cache.black;
        } else if (this->color_mode == BITMAP_COLOR_MODE::COLOR888) {
            grey = ((uint16_t)R + G + B) / 3;
            this->cache.white = this->color_map[this->colorDistance(255, 255, 255)][3];
            this->cache.black = this->color_map[this->colorDistance(0,0,0)][3];
            this->cache.has = 1;
            match = grey >= this->threshold ? cache.white : cache.black;
        } else {
            grey = ((uint16_t)R + G + B) / 3;
            this->cache.white = this->color_map[this->colorDistance(31, 63, 31)][3];
            this->cache.black = this->color_map[this->colorDistance(0,0,0)][3];
            this->cache.has = 1;
            match = grey >= this->threshold ? cache.white : cache.black;
        }
        #if SIKTEC_EPD_DEBUG_BITMAP_KERNELS_PIXELS
            PRINT_DEBUG_BUFFER("RGB: %u,%u,%u Grey %d Thresh %d Color %d \n", R, G, B, grey, this->threshold, match);
        #endif
        return match;
    }
};

/**
 * @brief 4 GRAY filter
 */
class BitmapFilter_GRAY4 : public BITMAP_FILTER_IMPLEMENTATION {

    float lighten; ///< lighten percent 0 - 200

    public:

    /**
     * @brief Construct a new BitmapFilter_GRAY4 object
     * 
     * @param _lighten lighten percent 0 - 200
     */
    inline BitmapFilter_GRAY4(const uint8_t _lighten = 40) {
        if (_lighten > 100) {
            this->lighten = 2;
        } else {
            this->lighten = 1 + ((float)_lighten / 100);
        }
    }

    /**
     * @brief called on all pixels - return the closest color defined in the color map based on the greyscale value.
     * @param R 
     * @param G 
     * @param B 
     * @return colorBits_t 
     */
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

/**
 * @brief QUANTIZE filter
 */
class BitmapFilter_QUANT : public BITMAP_FILTER_IMPLEMENTATION {

    public:

    /**
     * @brief called on all pixels - return the closest color defined in the color map.
     * @param R 
     * @param G 
     * @param B 
     * @return colorBits_t 
     */
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
};

/**
 * @brief DITHER 2D B&W filter
 */
class BitmapFilter_DITHER_BW : public BITMAP_DITHER_FILTER {

    private:

    float    threshold;             ///< threshold value for B & W
    int16_t  level = 128;           ///< greyscale mid 
    uint16_t color_black = 0x0000;  ///< black color 
    uint16_t color_white = 0xFFFF;  ///< white color 

    /** @brief weights vector for the error distribution */
    uint8_t  weights[4] = { 
        DITHER_WEIGHTS_VECTOR_FLOYD 
    };

    public:
    
    /**
     * @brief Construct a new BitmapFilter_DITHER_BW object
     * 
     * @param _color_black black color 
     * @param _color_white white color 
     * @param _threshold   the threshold color 0.0 - 2.0 
     */
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

    /**
     * @brief Set the Weight Vector object
     * 
     * @param right 
     * @param down_left 
     * @param down 
     * @param down_right 
     * @return void
     */
    inline  void setWeightVector(const uint8_t right, const uint8_t down_left, const uint8_t down, const uint8_t down_right) {
        this->weights[0] = right;
        this->weights[1] = down_left;
        this->weights[2] = down;
        this->weights[3] = down_right;
    }

    /**
     * @brief called on all pixels - returns the pixel greyscale.
     * @param R 
     * @param G 
     * @param B 
     * @return colorBits_t 
     */
    inline colorBits_t kernel(const uint8_t R, const uint8_t G, const uint8_t B) {
        //Convert to grey:
        return (((uint16_t)R + G + B) / 3) * this->threshold;
    }

    /**
     * @brief distributes the current pixel to the surronding pixels
     * 
     * @param pixels the pixels [current, right, bottom left, bottom, bottom right]
     */
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
        pixels[1] = pixels[1] + this->weights[0] * err_adj;
        //BOTTOM LEFT:
        pixels[2] = pixels[2] + this->weights[1] * err_adj;
        //BOTTOM:
        pixels[3] = pixels[3] + this->weights[2] * err_adj;
        //BOTTOM RIGHT:
        pixels[4] = pixels[4] + this->weights[3] * err_adj;
    }
};

/**
 * @brief DITHER 2D 4 GRAY filter
 */
class BitmapFilter_DITHER_GRAY4 : public BITMAP_DITHER_FILTER {

    private:

    float    threshold;     ///< threshold value for B & W
    int16_t  level = 128;   ///< greyscale mid 

    /** @brief weights vector for the error distribution */
    uint8_t  weights[4] = { 
        DITHER_WEIGHTS_VECTOR_FLOYD 
    };

    public:

    /**
     * @brief Construct a new BitmapFilter_DITHER_GRAY4 object
     * 
     * @param _threshold   the threshold color 0.0 - 2.0 
     */
    inline BitmapFilter_DITHER_GRAY4(const float _threshold = 1) {
        this->threshold = _threshold;
        if (this->color_mode == BITMAP_COLOR_MODE::COLOR888) {
            this->level = 128;
        } else {
            this->level = 21;
        }
    }

    /**
     * @brief Set the Weight Vector object
     * 
     * @param right 
     * @param down_left 
     * @param down 
     * @param down_right 
     * @return void
     */
    inline  void setWeightVector(const uint8_t right, const uint8_t down_left, const uint8_t down, const uint8_t down_right) {
        this->weights[0] = right;
        this->weights[1] = down_left;
        this->weights[2] = down;
        this->weights[3] = down_right;
    }

    /**
     * @brief return closest color index based on its grey value
     * 
     * @param grey 
     * @return uint16_t 
     */
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

    /**
     * @brief called on all pixels - returns the pixel greyscale.
     * @param R 
     * @param G 
     * @param B 
     * @return colorBits_t 
     */
    inline colorBits_t kernel(const uint8_t R, const uint8_t G, const uint8_t B) {
        //Convert to grey:
        return (((uint16_t)R + G + B) / 3) * this->threshold;
    }

    /**
     * @brief distributes the current pixel to the surronding pixels
     * 
     * @param pixels the pixels [current, right, bottom left, bottom, bottom right]
     */
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
        pixels[1] = pixels[1] + this->weights[0] * err_adj;
        //BOTTOM LEFT:
        pixels[2] = pixels[2] + this->weights[1] * err_adj;
        //BOTTOM:
        pixels[3] = pixels[3] + this->weights[2] * err_adj;
        //BOTTOM RIGHT:
        pixels[4] = pixels[4] + this->weights[3] * err_adj;
    }
};


}


