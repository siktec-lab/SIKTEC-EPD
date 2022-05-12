# SIKTEC-EPD
 ePaper / eInk display driver to easily integrate SIKTEC displays.<br />
 Adafruit GFX compatible with optional external SRAM use.

<br/>

## Description
This library seamlessly integrates Epaper displays with SRAM support and exposes GFX drawing functions. <br />
Its mainly for using SIKTEC display boards and that features several different EPD drivers which are all included in this library. <br />
The library is designed to be simple yet very flexible and can be used for any board that is driven by one of the implemented drivers.   
<br />

<a id="drivers-epd"></a>

### **Implemented Drivers:**

| DRIVER             | Constructor         | Color                | Datasheet                                                  |
|:------------------:|:--------------------|:---------------------|:-----------------------------------------------------------|
| **IL0398**         | `SIKTEC_EPD_G4()`   | EPD_MODE_GRAYSCALE4  | [IL09398.pdf](https://github.com/siktec-lab/SIKTEC-EPD/extras/IL09398.pdf) |
| **UC8276**         | `SIKTEC_EPD_3CU()`  | EPD_MODE_TRICOLOR    | [UC8276.pdf](https://github.com/siktec-lab/SIKTEC-EPD/extras/UC8276.pdf) |
| **SSD1619**        | `SIKTEC_EPD_3CS()`  | EPD_MODE_TRICOLOR    | [SSD1619A.pdf](https://github.com/siktec-lab/SIKTEC-EPD/extras/SSD1619A.pdf) |

<br />

### **Physically tested with BOARDS:**

| BOARD    | Pins (EPD_CS, EPD_DC, EPD_RESET, EPD_BUSY, SRAM_CS)              | Tested with:                 | 
|:--------:|:-----------------------------------------------------------------|:-----------------------------|
| ESP32    | {16, 17, 4, 13, 15}                                              | Arduino IDE, PlatformIO      |
| UNO      | {9, 8, 7, 6, 5}                                                  | Arduino IDE, PlatformIO      |
| NANO     | {9, 8, 7, 6, 5}                                                  | Arduino IDE, PlatformIO      |
| MEGA     | {8, 9, 10, 11, 12}                                               | Arduino IDE, PlatformIO      |
| LEONARDO | {9, 8, 7, 6, 5}                                                  | Arduino IDE, PlatformIO      |
| DUE      | 16, 4, 13, 15, 17                                                | Arduino IDE, PlatformIO      |

<br />

### **Physically tested with SRAM:**

| CHIP           | Manufacturer | Datasheet                                                  |
|:--------------:|:-------------|:-----------------------------------------------------------|
| 23K256-I/SN    | Microchip    | [http://ww1.microchip.com/downloads/en/devicedoc/22100f.pdf](http://ww1.microchip.com/downloads/en/devicedoc/22100f.pdf) |

<br/>

<a id="table-contents"></a>

## Table of Contents:
- [Quick Installation](#installation)
- [Example Included](#examples)
- [Decalring SIKTEC_EPD](#declaring-epd)
- [Controling the EPD](#controling-epd)
- [Drawing](#drawing-epd)
- [Displaying](#displaying-epd)
- [Debugging](#debugging)

<br/>

<a id="installation"></a>

## Installation:

[ :arrow_up_small: Return](#table-contents)

You can install the library through one of the following:
1. Arduino or PlatformIO library manager: Search for "SIKTEC_EPD" and click install.
2. Download the repositories as a ZIP file and install it through the Arduino IDE by:<br/>
   `Sketch -> Include library -> Add .ZIP Library.`
3. Download the library and include it in your project folder - Then you can Include it directly:<br/>
    `#include "{path to}\SIKTEC_EPD.h"`
> :paperclip: **Dependencies** :  When manually including the library you should also import the library dependencies
> - [SIKTEC_SPI.h](https://github.com/siktec-lab/SIKTEC-SPI)
> - [SIKTEC_SRAM.h](https://github.com/siktec-lab/SIKTEC-SRAM)
> - [Adafruit_GFX.h](https://github.com/adafruit/Adafruit-GFX-Library) *and its own dependencies

<br/>

<a id="examples"></a>

## Example included:

[ :arrow_up_small: Return](#table-contents)

- **EPDHelloWorld.ino** - An example which can be compiled with or without SRAM - Will demonstrate printing text to the EPD, Drawing a color pallet, Drawing some cool circles. The code is well commented so feel free to go through the source code.
> :pushpin: To disable the SRAM Set the pin as -1 

<br/>

<a id="declaring-epd"></a>

## Declaring of 'SIKTEC_EPD' object:

[:arrow_up_small: Return](#table-contents)

Call `SIKTEC_EPD_G4 | SIKTEC_EPD_3CS | SIKTEC_EPD_3CU` with al required pins - *SpiClass is optional and if its omitted Default Arduino SPI Instance will be used.<br />
After creating the `SIKTEC_EPD` instance the `begin()` method should be called to start EPD communication and setting the COLOR mode.

```cpp

#include <Adafruit_I2CDevice.h> // Only required when compiling with platformio
#include <Adafruit_GFX.h>       // Only required when compiling with platformio
#include <SIKTEC_EPD.h>

...

using namespace SIKtec;


//Declare EPD and connected pins:
epd_pins_t epd_pins = {16, 17, 4, 13, 15}; // epd_pins_t { edp_cs, sram_cs, dc, rst, busy }
SIKTEC_EPD_3CS *board;
//SIKTEC_EPD_3CU *board;
//SIKTEC_EPD_G4 *board;

void setup() {

    ...

    //Initialize:
    board = new SIKTEC_EPD_3CS(epd_pins);
    // board = new SIKTEC_EPD_3CU(epd_pins);
    // board = new SIKTEC_EPD_G4(epd_pins);

    //Start EPD Communication:
    board->begin(EPD_MODE_TRICOLOR);
    ...
}
...
```
> :pushpin: SIKTEC_EPD wraps all required functionality - SRAM communication, EPD communication, GFX drawing.<br />
> :pushpin: When declaring pin EPD_BUSY as -1, the display will wait a fix amount of time ~13 seconds. So its a good practice to dedicate a pin for the busy signal.<br />
> :pushpin: When declaring pin EPD_RESET as -1, The EPD won't be put fully to sleep (It can't be awaken without HW reset). While Thats possible <u>it's not recommended :exclamation:</u> It may cause spooky shadow images and damage the display. 

<br/>




<a id="controling-epd"></a>

## Controling the EPD:

[ :arrow_up_small: Return](#table-contents)

There are 4 main methods used to control the EPD. Each one of the can be called several time and used anywhere except from withinn ISR (Interrupts).<br />

**`.begin(epd_mode_t mode = EPD_MODE_MONO)`** This method starts the communication with EPD and should be called at least one after intialization before doing anything with EPD.
There are several color modes - you check the [table](#drivers-epd) above to see the matching color mode for you driver.

```cpp
...

//Start EPD Communication:
board->begin(EPD_MODE_TRICOLOR);
/*
    Begin will:
    - Wake up the display.
    - Send an init sequence.
    - Set the color mode and the buffers .
*/
...
```
> :pushpin: `EPD_MODE_MONO` can be used with all board.<br />

<br />

**`.clearBuffer()`** This method clears the entire drawing buffer - It won trigger a refresh.

```cpp
...

//Clear internal drawing buffer:
board->clearBuffer();

...
```

<br />

**`.clearDisplay()`** This method clears the display - Basically a clearBuffer + Screen Update.

```cpp
...

//Clear internal drawing buffer + refresh the display:
board->clearDisplay();

...
```
> :pushpin: `clearDisplay` is a full refresh so only use it when you actually need to display an empty screen.<br />

<br/>

<a id="drawing-epd"></a>

## Drawing


[:arrow_up_small: Return](#table-contents)

The EPD class has single drawing method `drawPixel(int16_t x, int16_t y, uint16_t color)` which will set a single pixel at a specific X,Y position
This method is used by all of the GFX library drawing functions. - It can be called directly or passed to additional drawing libraries.

```cpp
    ...

    //Set a single pixel manually:
    board->drawPixel(10, 20, EPD_BLACK);
    /*
        Colors: are defined by the enum:
        enum {
            EPD_WHITE,
            EPD_BLACK,
            EPD_RED,
            EPD_GRAY,
            EPD_DARK,
            EPD_LIGHT,
            EPD_NUM_COLORS
        };
    */
    //All GFX drawing functions:
    board->drawLine(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color);
    board->drawFastVLine(uint16_t x0, uint16_t y0, uint16_t length, uint16_t color);
    board->drawFastHLine(uint8_t x0, uint8_t y0, uint8_t length, uint16_t color);
    board->drawRect(uint16_t x0, uint16_t y0, uint16_t w, uint16_t h, uint16_t color);
    board->fillRect(uint16_t x0, uint16_t y0, uint16_t w, uint16_t h, uint16_t color);
    board->drawCircle(uint16_t x0, uint16_t y0, uint16_t r, uint16_t color);
    board->fillCircle(uint16_t x0, uint16_t y0, uint16_t r, uint16_t color);
    board->drawRoundRect(uint16_t x0, uint16_t y0, uint16_t w, uint16_t h, uint16_t radius, uint16_t color);
    board->fillRoundRect(uint16_t x0, uint16_t y0, uint16_t w, uint16_t h, uint16_t radius, uint16_t color);
    board->drawTriangle(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color);
    board->fillTriangle(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color);
    board->drawChar(uint16_t x, uint16_t y, char c, uint16_t color, uint16_t bg, uint8_t size);
    board->setCursor(int16_t x0, int16_t y0);
    board->setTextColor(uint16_t color);
    board->setTextSize(uint8_t size);
    board->setTextWrap(boolean w);
    board->getTextBounds(string str, int16_t x, int16_ty, int16_t *x1, int16_t *y1, int16_t *w, int16_t *h);
    board->print(str);
    board->drawBitmap(int16_t x, int16_t y, uint8_t *bitmap, int16_t w, int16_t h, uint16_t color);
    board->fillScreen(uint16_t color);
    board->setRotation(uint8_t rotation);
    board->width();
    board->height();
    board->setFont(&FreeMonoBoldOblique12pt7b);

    ...
```
> :pushpin: For more about Adafruit GFX library and using custom fonts and more [Adafruit-GFX-Library](https://github.com/adafruit/Adafruit-GFX-Library).

<br/>

<a id="displaying-epd"></a>

## Displaying

<hr />

[:arrow_up_small: Return](#table-contents)

The `display(bool sleep)` method is used to transffer the buffered pixels to th EPD and disply the on the scree - passing true will powerdown the display when finished and put it in sleep mode. </br>
The display is powered on if its need when this method is called,

```cpp
    ...

    //Update display:
    board->display(true);

    ...
```
> :pushpin: You should alway power down the display. Not powerign down can cause damage to the display. 

<br/>

<a id="debugging"></a>

## Debugging:

<hr />

[:arrow_up_small: Return](#table-contents)

The library 3 macro define Flags that enables debugging. They should be defined BEFORE the library include.<br />
Serial monitor will get all the captured data and events.

```cpp
    ...

    #define SIKTEC_EPD_DEBUG
    #define SIKTEC_EPD_DEBUG_PIXELS
    #define SIKTEC_EPD_DEBUG_SRAM_READ_WRITE 

    ...
```

