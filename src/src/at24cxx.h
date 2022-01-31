//----------------------------------------------------------------------------
// Name        : at24cxx.h
// Purpose     : AT24CXX EEPROM Controller Class
// Description : 
//               This class intended for control of AT24CXX EEPROM chips.
//               Though the AT24CXX family of EEPROM chips are internally
//               organized according to varied page sizes, use of this class
//               abstracts away this arrangement so that extended strings and
//               arrays of arbitrary length (up to 255) may be written to any
//               random address within the memory. Attempts to overwrite the
//               end boundary of the chip's memory space will be ignored and
//               the write()/read() method will return false.
//
//               Calls to any operational methods will perform no action if
//               an initial call to begin() has not yet been performed.
//
//               Use of write protect pin WP is optional, and calls to
//               methods setWriteProtect() and clearWriteProtect() will only
//               execute properly if wp_pin was included at call to begin().
//
// Platform    : Multiple
// Language    : C++
// Framework   : Arduino
// Copyright   : MIT License 2022, John Greenwell
// Requires    : External Libraries : N/A
//               Custom Libraries   : N/A
//----------------------------------------------------------------------------
#ifndef AT24CXX_H
#define AT24CXX_H

namespace PeripheralIO {

// Chip Selection
extern const uint32_t AT24C01; // Not tested
extern const uint32_t AT24C02;
extern const uint32_t AT24C04; // Not tested
extern const uint32_t AT24C08;
extern const uint32_t AT24C16;
extern const uint32_t AT24C32;
extern const uint32_t AT24C64;
extern const uint32_t AT24C128;
extern const uint32_t AT24C256;
extern const uint32_t AT24C512;

class AT24CXX {
public:
    AT24CXX();

    void begin(uint32_t chip, uint8_t chip_addr=0, TwoWire& wire=Wire,
                uint8_t wp_pin=-1);
    // Initilize IO to the AT24CXX chip with hardware I2C
    // Parameter chip is the chip name, e.g. PeripheralIO::AT24C02
    // Parameter chip_addr is the EEPROM external biasing (lowest bits)
    // Parameter wp_pin is the pin connected to WP on the chip

    bool isConnected() const;
    // Returns true for acknowledged communication with chip, else false
    // Calls to this method in close proximity may hang some Wire libraries
    
    bool write(uint16_t address, uint8_t val) const;
    // Write value to EEPROM address
    // Sets iterator to intended address value and proceeds
    // Returns false for attempt to write to invalid memory regions

    bool write(uint16_t address, uint8_t vals[], uint8_t n) const;
    // Write n successive values to address
    // Returns false for attempt to write to invalid memory regions

    bool write(uint16_t address, const char str[], uint8_t n) const;
    // Write string of length n to address
    // Returns false for attempt to write to invalid memory regions

    uint8_t read(uint16_t address) const;
    // Read value from specific EEPROM address
    // Returns false for attempt to read from invalid memory regions

    bool read(uint16_t address, uint8_t vals[], uint8_t n) const;
    // Read n values to location of pointer vals starting at address
    // Returns false for attempt to read from invalid memory regions

    bool read(uint16_t address, char str[], uint8_t n) const;
    // Read n chars to string str starting at address
    // Returns false for attempt to read from invalid memory regions

    void setWriteProtect() const;
    // Raise WP pin so that write operations may not be applied
    // Requires wp_pin inclusion at call to begin()

    void clearWriteProtect() const;
    // Release WP pin so that write operations may be applied
    // Requires wp_pin inclusion at call to begin()

private:
    bool writeN(uint16_t, uint8_t*, uint8_t) const;
    bool readN(uint16_t, uint8_t*, uint8_t) const;

    uint8_t _chip_addr;
    uint32_t _chip_size;
    uint8_t _page_size;
    uint8_t _addr_bytes;
    uint8_t _addr_ov_bits;
    uint8_t _addr_size;
    uint8_t _mode;
    uint8_t _wp_pin;
    TwoWire* _wire;


};

// Base Address and I2C Defines
extern const uint8_t AT24CXX_ADDR; // 7-bit addr
extern const uint8_t I2C_READ_BUFFER_SIZE; // Maximum held in Wire buffer
extern const uint8_t EEPROM_WRITE_CYCLE_TIME_MS; // datasheet: 5ms max

}

#endif