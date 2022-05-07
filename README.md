# SIKTEC-SRAM
Library for Interfacing Microchip SRAM chips.<br />
Suitable and tested with Microchip 23K256-I/SN should work with most of the chips of the same family and others.

<br/>

## Description
This library seamlessly communcates with SPI based SRAM chips and exposes a simple and lightweight API to interface with an external SRAM chip.<br />
The library is well documented and has 2 examples - You can easily write / read / erase and debug SRAM chips. All SPI communication is managed by the library. <br />
This library can be easily ported to other chips by simply changing the instruction set used by the library. 
<br />

### **Physically tested with:**

| CHIP           | Manufacturer | Datasheet                                                  |
|:--------------:|:-------------|:-----------------------------------------------------------|
| 23K256-I/SN    | Microchip    | [http://ww1.microchip.com/downloads/en/devicedoc/22100f.pdf](http://ww1.microchip.com/downloads/en/devicedoc/22100f.pdf) |

<br/>

<a id="table-contents"></a>

## Table of Contents:
- [Quick Installation](#installation)
- [Example Included](#examples)
- [Decalring SIKTEC_SRAM](#declaring)
- [Writing to SRAM](#writing-data)
- [Reading from SRAM](#reading-data)
- [Erasing data on the SRAM](#erasing-data)
- [Debugging](#debugging)
- [Important Notes](#important-notes)

<br/>

<a id="installation"></a>

## Installation:

<hr />

[Return](#table-contents)

You can install the library through one of the following:
1. Arduino or PlatformIO library manager: Search for "SIKTEC_SRAM" and click install.
2. Download the repositories as a ZIP file and install it through the Arduino IDE by:<br/>
   `Sketch -> Include library -> Add .ZIP Library.`
3. Download the library and include it in your project folder - Then you can Include it directly:<br/>
    `#include "{path to}\SIKTEC_SRAM.h"`
> **Dependency** When manually including the library you should also import the library dependencies [SIKTEC_SPI.h](https://github.com/siktec-lab/SIKTEC-SPI) .

<br/>

<a id="examples"></a>

## Example included:

<hr />

[Return](#table-contents)

- **ReadWrite.ino** - A simple example that writes an array to SRAM, dumps raw data from the SRAM chip and Reads back the read data.
- **StoreStruct.ino** - This example demonstrates how to serialize a structure and store it on the SRAM chip - Also exposes a raw dump of the data and how to deserialize it back to the same type structure. 

<br/>

<a id="declaring"></a>

## Declaring of 'SIKTEC_SRAM' object:

<hr />

[Return](#table-contents)

Call `SIKTEC_SRAM` expects a CS (chip select) pin number to initialize with default HardWare SPI - Or pass the SPI pins to be used.<br />
After creating the `SIKTEC_SRAM` instance the `begin()` method should be called to start SRAM communication.

```cpp

#include <SIKTEC_SRAM.h>

...

//using namespace SIKtec; // Optional 

//Define SRAM CS Pin attached:
#define SRAM_CS     17

//Declare SRAM object:
SIKtec::SIKTEC_SRAM sram(SRAM_CS);

void setup() {
    
    ...
    
    //Start SRAM Communication
    sram.begin();

    //Set the SRAM chip array Mode:
    if (!sram.set_mode(SRAM_MODE::SRAM_SEQ_MODE)) {
        Serial.print("SRAM MODE FAILED");
    }
    ...
}
...
```
> - **Note**: SIKTEC_SRAM depends on SIKTEC_SPI which is an SPI wrapper that improves the SPI api.
> - **Note**: Its a good practice to add a pull-up ressistor to the CS pin.
> - **Note**: Three MODES are available - SRAM_SEQ_MODE, SRAM_PAGE_MODE, SRAM_BYTE_MODE - read more about those modes in the datasheet. 

<br/>

<a id="writing-data"></a>

## Writing to SRAM:

<hr />

[Return](#table-contents)

By default the library doesn't set any default data on the SRAM - Its full of garbage. <br />
To write data you should use the `write()` passing it a **start address**, a **buffer** of data, and the **size** to write.<br />
Address space starts at `0x0000` and ends depending on the SRAM size.

```cpp
    ...
    
    //Write a single byte:
    uint8_t single = 7;
    sram.write8(0x0000, single); // will occupy 0x0000

    //Write a uint16:
    uint16_t single = 1700;
    sram.write16(0x0001, single); // will occupy 0x0001, 0x0002

    //Array write:
    uint8_t data[] = {1, 2, 3, 4, 5};
    sram.write(0x0003, data, sizeof(data)); // will occupy 0x0003 -> 0x0007

    ...
```

> **Note:** If an overflow occurs the SRAM chip will loop back to the 0x00 and write there.

<br/>

<a id="reading-data"></a>

## Reading from SRAM

<hr />

[Return](#table-contents)

To read data you should use the object `read()` methods passing it the **starting address** and a **buffer** to write to with the requested **length**.

```cpp
    ...

    //Read a single byte:
    uint8_t single8 = sram.read8(0x0000);

    //Read a uint16:
    uint16_t single16 = sram.read16(0x0001); // will basically read 0x1 and 0x2 and returns a uint16 number

    //Read to buffer:
    uint8_t data[10];
    sram.read(0x0003, data, sizeof(data));

    ...
```
> **Note:** Even if you did not write data to those addresses SRAM will return data - It will be garbage data set by default.

<br/>

<a id="erasing-data"></a>

## Erasing data on the SRAM

<hr />

[Return](#table-contents)

There is no real data erasing - The trick is to set the data to a default value which we consider as "erased". The default value used by the method is `0x00` but you can use your own - To erase you should use the `erase()` method.

```cpp
    ...

    //Erase:
    sram.erase(0x0000, 2); // 0x0 and 0x1 will be set to 0.

    sram.erase(0x0000, 2, 0x1); // 0x0 and 0x1 will be set to 1.

    ...
```

<br/>

<a id="debugging"></a>

## Debugging:

<hr />

[Return](#table-contents)

The library has two debugging methods `print_status()`, `mem_dump()` - Those methods expects a `Stream` instance 
By default will use the `Serial` instance included by Arduino.  

```cpp
    ...

    /*  
    void mem_dump(
        uint16_t from, 
        uint16_t length, 
        bool address = true, 
        bool decimal = true, 
        bool hex = true, 
        bool binary = true, 
        Stream *serialport = &Serial
    );
    */

    sram.mem_dump(0x0000, 2);
    /*
    Will print data written on sram:
    SRAM [0x0] => 100 , 0x64 , 1100100
    SRAM [0x1] => 0 , 0x0 , 0
    */

    sram.print_status(0x0000, 2);
    /*
    Will print:
    SRAM STATUS REGISTER - 64 [1000000]
    */

    ...
```
> By default `mem_dump()` will print data in 3 formats **dec, hex, bin** - you can disable those formats with the flags passed to mem_dump.

<br />

<a id="important-notes"></a>

## Additional notes: 

<hr />

[Return](#table-contents)

1. The library has more methods which exposes a lower level that gives you control over the CS pin state - Its usefull especially when you want to bridge data between the SRAM chip to another connected SPI device directly.
2. For more advance usage and storing structs use the examples which are commented and provides more real-world usage of this library.
