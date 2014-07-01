// Copyright(c) Microsoft Open Technologies, Inc.All rights reserved.Licensed under the BSD 2 - Clause License.See License.txt in the project root for license information.
//

/*
KeyboardLightController Static Library

This library will control keyboard lighting
using pins 1-14 (ignoring 0 and 15) of the
MCP23S17 GPIO port expander via SPI.

SPI requires the following pins:
XX - SS - Slave select (user defined)
11 - MOSI - Master out Slave in
12 - MISO - Master in Slave out [UNUSED]
13 - SCK - Serial Clock

The circuit:
MCP23S17 - GPIO (general purpose input/output) port expander
14 N-channel MOSFETs - One for each LED, triggering 12V/1A of current.
LED Strip - RadioShack LED Waterproof Flexi Strip 60 LED 1M (White)
12V/5A power supply
*/

#include "stdafx.h"

#ifndef KEYBOARD_LIGHT_CONTROLLER
#define KEYBOARD_LIGHT_CONTROLLER

class KeyboardLightController {
    //%%%% MCP23S17 Opcodes %%%%//
    static const unsigned char SPI_SLAVE_ID = 0x40;      //0b01000000
    static const unsigned char SPI_SLAVE_ADDR = 0x00;    //0b00000000
    static const unsigned char SPI_SLAVE_WRITE = 0x00;   //0b00000000

    //%%%% MCP23S17 Control Register Addresses %%%%//
    static const unsigned char CTLREG_BK0_IODIRA = 0x00; //0b00000000
    static const unsigned char CTLREG_BK0_IODIRB = 0x01; //0b00000001
    static const unsigned char CTLREG_BK0_GPIOA = 0x12;  //0b00010010
    static const unsigned char CTLREG_BK0_GPIOB = 0x13;  //0b00010011

public:
    KeyboardLightController(
        const unsigned char SLAVE_SELECT_PIN_
        );

    void
        begin(
        void
        );

    unsigned short
        disableLights(
        const unsigned short BITMASK_
        );

    unsigned short
        enableLights(
        const unsigned short BITMASK_
        );

    void
        end(
        void
        );

    void
        heartbeat(
        void
        );

    unsigned short
        setLights(
        const unsigned short BITMASK_
        );

private:
    const unsigned char _SLAVE_SELECT_PIN;
    unsigned short _state;

    unsigned short
        set(
        const unsigned short BITMASK_,
        const unsigned char CTL_
        );

};

#endif
