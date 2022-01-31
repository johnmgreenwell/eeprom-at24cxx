//----------------------------------------------------------------------------
// Name        : main.cpp
// Purpose     : AT24CXX EEPROM Controller Class Test
// Description : 
//               This purpose of this test project is to test the a custom
//               AT24CXX EEPROM library with feedback via an OLED display.
//
// Platform    : Heltec Wifi Kit 32 V2
// Language    : C++
// Framework   : Arduino
// Copyright   : MIT License 2022, John Greenwell
// Requires    : External Libraries : heltecautomation/Heltec ESP32 Dev-Boards
//               Custom Libraries   : at24cxx.h
//----------------------------------------------------------------------------

// TODO: address problem with AT24CXX hanging if no chip present

#include <Arduino.h>
#include <Wire.h>
#include "heltec.h"
#include "at24cxx.h"

// Objects
PeripheralIO::AT24CXX eeprom_2k;
PeripheralIO::AT24CXX eeprom_64k;
PeripheralIO::AT24CXX eeprom_512k;

// Test strings
const char TEST_STRING_2k[] = "Testing the 2k EEPROM.....";
const char TEST_STRING_64k[] = "Testing the 64k EEPROM....";
const char TEST_STRING_512k[] = "Testing the 512k EEPROM...";

void setup(void) {

    // Initialize Heltec display
    delay(1);
    Heltec.begin(true, false, true);
    Heltec.display->init();
    Heltec.display->clear();
    Heltec.display->setTextAlignment(TEXT_ALIGN_LEFT);
    Heltec.display->setFont(ArialMT_Plain_16);
    Heltec.display->drawString(0, 0, "Initializing...");
    Heltec.display->display();
    delay(1);

    // Initialize peripheral objects
    Wire1.begin(21, 22, 0);
    eeprom_2k.begin(PeripheralIO::AT24C02, 0x00, Wire1);
    eeprom_64k.begin(PeripheralIO::AT24C64, 0x01, Wire1);
    eeprom_512k.begin(PeripheralIO::AT24C512, 0x02, Wire1);

    // Check that each chip is detected
    bool allChipsPresent = false;
    if (!eeprom_2k.isConnected());
    else if (!eeprom_64k.isConnected());
    else if (!eeprom_512k.isConnected());
    else allChipsPresent = true;
    if (!allChipsPresent) {
        Heltec.display->clear();
        Heltec.display->drawString(0, 0, "Chip error.");
        Heltec.display->drawString(0, 16, "Terminating.");
        Heltec.display->display();
        while(true) delay(1000);
    }

    // Write test strings to EEPROM
    eeprom_2k.write(3, TEST_STRING_2k, 26);
    eeprom_64k.write(62, TEST_STRING_64k, 26);
    eeprom_512k.write(510, TEST_STRING_512k, 26);

    // Read and check test strings from EEPROM
    static char str[30] = {};
    Heltec.display->clear();
    Heltec.display->drawString(0, 0, "Check 2k");
    Heltec.display->drawString(0, 16, "Check 64k");
    Heltec.display->drawString(0, 32, "Check 512k");
    Heltec.display->drawString(90, 0, ":");
    Heltec.display->drawString(90, 16, ":");
    Heltec.display->drawString(90, 32, ":");

    eeprom_2k.read(3, str, 26);
    str[26] = '\0';
    if (strcmp(str, TEST_STRING_2k) == 0)
        Heltec.display->drawString(98, 0, "OK");
    else
        Heltec.display->drawString(98, 0, "FAIL");

    eeprom_64k.read(62, str, 26);
    str[26] = '\0';
    if (strcmp(str, TEST_STRING_64k) == 0)
        Heltec.display->drawString(98, 16, "OK");
    else
        Heltec.display->drawString(98, 16, "FAIL");

    eeprom_512k.read(510, str, 26);
    str[26] = '\0';
    if (strcmp(str, TEST_STRING_512k) == 0)
        Heltec.display->drawString(98, 32, "OK");
    else
        Heltec.display->drawString(98, 32, "FAIL");

    Heltec.display->display();
}

void loop() {
    delay(1000);
}

