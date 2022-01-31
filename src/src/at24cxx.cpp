//----------------------------------------------------------------------------
// Name        : at24cxx.cpp
// Purpose     : AT24CXX EEPROM Controller Class
// Description : This source file accompanies header file at24cxx.h
// Platform    : Multiple
// Framework   : Arduino
// Language    : C++
// Copyright   : MIT License 2022, John Greenwell
//----------------------------------------------------------------------------

#include <Arduino.h>
#include <Wire.h>
#include "at24cxx.h"

namespace PeripheralIO {

AT24CXX::AT24CXX()
: _chip_addr(0),
  _chip_size(0),
  _page_size(0),
  _addr_bytes(0),
  _addr_ov_bits(0),
  _addr_size(0),
  _mode(0),
  _wp_pin(-1),
  _wire(nullptr)
{ }

/*!
    @brief Initialize AT24CXX using hardware I2C
    @param chip Idendity of chip (e.g. PeripheralIO::AT24C02)
    @param chip_addr Address of EEPROM chip
*/
void AT24CXX::begin(uint32_t chip, uint8_t chip_addr, TwoWire& wire,
                    uint8_t wp_pin) {
    _chip_addr = AT24CXX_ADDR | (chip_addr & 0x07);
    _chip_size = (chip & 0x0001FFFF);
    _page_size = (uint8_t)((chip & 0x0FF00000) >> 20);
    _addr_bytes = (uint8_t)((chip & 0x30000000) >> 28);
    _addr_ov_bits = (uint8_t)((chip & 0xC0000000) >> 30);
    _wire = &wire;
    _wp_pin = wp_pin;
    if (_wp_pin != -1) {
        pinMode(_wp_pin, OUTPUT);
        digitalWrite(_wp_pin, LOW);
    }
    _mode = 1; // Active mode
}

/*!
    @brief Check whether AT24CXX is present
    @return True for successful acknowledgement from chip 
*/
bool AT24CXX::isConnected() const {
    bool acknowledged = false;
    if (_mode) {
        _wire->beginTransmission(_chip_addr);
        if (_wire->endTransmission() == 0)
            acknowledged = true;
    }
    return acknowledged;
}

/*!
    @brief Write byte to AT24CXX
    @param address Address to write byte
    @param val Byte to write
    @return False for failed to write (e.g. invalid memory regions)
*/
bool AT24CXX::write(uint16_t address, uint8_t val) const {
    uint8_t byte = val;
    return writeN(address, &byte, 1);
}

/*!
    @brief Write n bytes to AT24CXX from vals
    @param address Address to write bytes
    @param vals Pointer to array of bytes
    @param n Number of successive bytes to write
    @return False for failed to write (e.g. invalid memory regions)
*/
bool AT24CXX::write(uint16_t address, uint8_t vals[], uint8_t n) const {
    return writeN(address, vals, n);
}

/*!
    @brief Write n chars to AT24CXX from str
    @param address Address to write chars
    @param str Pointer to array of chars
    @param n Number of successive chars to write
    @return False for failed to write (e.g. invalid memory regions)
*/
bool AT24CXX::write(uint16_t address, const char str[], uint8_t n) const {
    return writeN(address, (uint8_t*)str, n);
}

/*!
    @brief Read byte from AT24CXX
    @param address Address to read byte
    @return Byte read
*/
uint8_t AT24CXX::read(uint16_t address) const {
    uint8_t byte;
    readN(address, &byte, 1);
    return byte;
}

/*!
    @brief Read n successive bytes from AT24CXX
    @param address Address to read bytes
    @param vals Pointer to array bytes will be written to
    @param n Number of successive bytes to read
    @return False for failed to read (e.g. invalid memory regions)
*/
bool AT24CXX::read(uint16_t address, uint8_t* vals, uint8_t n) const {
    return readN(address, vals, 1);
}

/*!
    @brief Read n successive chars from AT24CXX
    @param address Address to read chars
    @param str Pointer to String chars will be written to
    @param n Number of successive chars to read
    @return False for failed to read (e.g. invalid memory regions)
*/
bool AT24CXX::read(uint16_t address, char str[], uint8_t n) const {
    return readN(address, (uint8_t*)str, n);
}

/*!
    @brief Raise WP pin so that write operations may not be applied
*/
void AT24CXX::setWriteProtect() const {
    if (_wp_pin != -1) {
        digitalWrite(_wp_pin, HIGH);
    }
}

/*!
    @brief Release WP pin so that write operations may be applied
*/
void AT24CXX::clearWriteProtect() const {
    if (_wp_pin != -1) {
        digitalWrite(_wp_pin, LOW);
    }
}

// Private: Hardware I2C Write Function
bool AT24CXX::writeN(uint16_t address, uint8_t* vals, uint8_t n) const {
    bool result = false;
    if (_mode && ((uint32_t)(address + n) <= _chip_size)) {
        uint8_t page_size = _page_size;
        if ((_addr_bytes > 1) && (n > 30))
            page_size = 16; // AT24C32+ can only write 30 bytes at a time
        uint8_t j = address % page_size;
        uint8_t pages_req = (((n + j - 1) / page_size) + 1);
        uint8_t addr = _chip_addr;
        uint8_t n_sent = 0;
        for (uint8_t i = 0; i < pages_req; i++) {
            if (_addr_ov_bits)
                addr = ((uint8_t)((_chip_addr & 0xF8) |
                        (((address + n_sent) & 0x0700) >> 8)));
            _wire->beginTransmission(addr);
            if (_addr_bytes > 1)
                _wire->write((uint8_t)((address + n_sent) >> 8));
            _wire->write((uint8_t)(address + n_sent));
            while ((j++ < page_size) && (n_sent < n))
                _wire->write(vals[n_sent++]);
            _wire->endTransmission(1);
            j = 0;
            delay(EEPROM_WRITE_CYCLE_TIME_MS);
        }
        result = true;
    }
    return result;
}

// Private: Hardware I2C Read Function
bool AT24CXX::readN(uint16_t address, uint8_t* vals, uint8_t n) const {
    bool result = false;
    if (_mode && ((uint32_t)(address + n) <= _chip_size)) {
        uint8_t addr = _chip_addr;
        if (_addr_ov_bits)
            addr = ((uint8_t)((_chip_addr & 0xF8) |
                    (((address + 0) & 0x0700) >> 8)));
        _wire->beginTransmission(addr);
        if (_addr_bytes > 1) 
            _wire->write((uint8_t)((address) >> 8));
        _wire->write((uint8_t)(address));
        _wire->endTransmission(0);
        uint8_t bytes_read = 0;
        uint8_t bytes_per_cycle = 0;
        while (bytes_read < n) {
            if (I2C_READ_BUFFER_SIZE < n - bytes_read)
                bytes_per_cycle = I2C_READ_BUFFER_SIZE;
            else
                bytes_per_cycle = n - bytes_read;
            _wire->requestFrom(addr, bytes_per_cycle);
            while (_wire->available())
                vals[bytes_read++] = _wire->read();
            //(this->*_i2cEndTransmission)(1); // Final stop not necessary
        }
        result = true;
    }
    return result;
}

// Chip Selection (word size | page size | addr bytes | addr overflow bits)
const uint32_t AT24C01 = 128 | (8 << 20) | (1 << 28) | (0 << 30);
const uint32_t AT24C02 = 256 | (8 << 20) | (1 << 28) | (0 << 30);
const uint32_t AT24C04 = 512 | (16 << 20) | (1 << 28) | (1 << 30);
const uint32_t AT24C08 = 1024 | (16 << 20) | (1 << 28) | (2 << 30);
const uint32_t AT24C16 = 2048 | (16 << 20) | (1 << 28) | (3 << 30);
const uint32_t AT24C32 = 4096 | (32 << 20) | (2 << 28) | (0 << 30);
const uint32_t AT24C64 = 8192 | (32 << 20) | (2 << 28) | (0 << 30);
const uint32_t AT24C128 = 16384 | (64 << 20) | (2 << 28) | (0 << 30);
const uint32_t AT24C256 = 32768 | (64 << 20) | (2 << 28) | (0 << 30);
const uint32_t AT24C512 = 65536 | (128 << 20) | (2 << 28) | (0 << 30);

// Base Address and I2C Defines
const uint8_t AT24CXX_ADDR = 0x50; // 7-bit addr
const uint8_t I2C_READ_BUFFER_SIZE = 32; // Maximum held in Wire buffer
const uint8_t EEPROM_WRITE_CYCLE_TIME_MS = 5; // datasheet: 5ms max

}
