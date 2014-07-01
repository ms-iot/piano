// Copyright(c) Microsoft Open Technologies, Inc.All rights reserved.Licensed under the BSD 2 - Clause License.See License.txt in the project root for license information.
//

#include "KeyboardLightController.h"

KeyboardLightController::KeyboardLightController(
    const unsigned char SLAVE_SELECT_PIN_
    ) :
    _SLAVE_SELECT_PIN(SLAVE_SELECT_PIN_)
{}

// sets up the SPI for the Keyboard Lighting
void
KeyboardLightController::begin(
void
) {
    // Initialize SPI communication to MCP23S17
    SPI_Missing::begin(_SLAVE_SELECT_PIN);

    // Set GPA to OUTPUT
    SPI_Missing::transfer(_SLAVE_SELECT_PIN, (SPI_SLAVE_ID | SPI_SLAVE_ADDR | SPI_SLAVE_WRITE), SPI_CONTINUE);
    SPI_Missing::transfer(_SLAVE_SELECT_PIN, CTLREG_BK0_IODIRA, SPI_CONTINUE);
    SPI_Missing::transfer(_SLAVE_SELECT_PIN, 0, SPI_LAST);

    // Set GPB to OUTPUT
    SPI_Missing::transfer(_SLAVE_SELECT_PIN, (SPI_SLAVE_ID | SPI_SLAVE_ADDR | SPI_SLAVE_WRITE), SPI_CONTINUE);
    SPI_Missing::transfer(_SLAVE_SELECT_PIN, CTLREG_BK0_IODIRB, SPI_CONTINUE);
    SPI_Missing::transfer(_SLAVE_SELECT_PIN, 0, SPI_LAST);
}

// this will turn off the lights set by the bitmask
unsigned short
KeyboardLightController::disableLights(
const unsigned short BITMASK_
) {
    return set(BITMASK_, 0);
}

// this will turn on the lights set by the bitmask
unsigned short
KeyboardLightController::enableLights(
const unsigned short BITMASK_
) {
    return set(BITMASK_, 1);
}

// To be called when ending the SPI communication with the keyboard lights.
void
KeyboardLightController::end(
void
) {
    // Set GPA to INPUT
    SPI_Missing::transfer(_SLAVE_SELECT_PIN, (SPI_SLAVE_ID | SPI_SLAVE_ADDR | SPI_SLAVE_WRITE), SPI_CONTINUE);
    SPI_Missing::transfer(_SLAVE_SELECT_PIN, CTLREG_BK0_IODIRA, SPI_CONTINUE);
    SPI_Missing::transfer(_SLAVE_SELECT_PIN, 1, SPI_LAST);

    // Set GPB to INPUT
    SPI_Missing::transfer(_SLAVE_SELECT_PIN, (SPI_SLAVE_ID | SPI_SLAVE_ADDR | SPI_SLAVE_WRITE), SPI_CONTINUE);
    SPI_Missing::transfer(_SLAVE_SELECT_PIN, CTLREG_BK0_IODIRB, SPI_CONTINUE);
    SPI_Missing::transfer(_SLAVE_SELECT_PIN, 1, SPI_LAST);

    // Finalize SPI communication to MCP23S17
    SPI_Missing::end(_SLAVE_SELECT_PIN);
}

// Alternates flashing of the keyboard lights 0 and 15 to provide a heartbeat
void
KeyboardLightController::heartbeat(
void
) {
    static unsigned char bank_a = 1;

    // Transmit bytes
    SPI_Missing::transfer(_SLAVE_SELECT_PIN, (SPI_SLAVE_ID | SPI_SLAVE_ADDR | SPI_SLAVE_WRITE), SPI_CONTINUE);
    SPI_Missing::transfer(_SLAVE_SELECT_PIN, CTLREG_BK0_GPIOA, SPI_CONTINUE);

    if (bank_a) {
        SPI_Missing::transfer(_SLAVE_SELECT_PIN, 0x01, SPI_CONTINUE);
        SPI_Missing::transfer(_SLAVE_SELECT_PIN, 0x00, SPI_LAST);
    }
    else {
        SPI_Missing::transfer(_SLAVE_SELECT_PIN, 0x00, SPI_CONTINUE);
        SPI_Missing::transfer(_SLAVE_SELECT_PIN, 0x80, SPI_LAST);
    }

    // Toggle bank
    bank_a = !bank_a;
}

// Sets the lights given by the bitmask based on the controller (CTL_).
// Note: does not affect pins 0 and 15
unsigned short
KeyboardLightController::set(
const unsigned short BITMASK_,
const unsigned char CTL_
) {
    const unsigned char *gp = 0;

    // Apply bitmask relative or absolute
    switch (CTL_) {
    case 0:
        // Takes a flipped version of the bitmask. Then ANDs it with the current state
        _state &= ~BITMASK_;
        break;
    case 1:
        // Takes the bitmask and ORs it with the state
        _state |= BITMASK_;
        break;
    default:
        // state is equal to the bitmask
        _state = BITMASK_;
    }

    // Shift bits for pin mapping (0 and 15 are used for the heartbeat)
    // Note: Take this out if you want to use all the pins
    _state <<= 2;
    _state >>= 1;

    // Break into two bytes for transmission
    gp = (const unsigned char *) &_state;

    // Transmit bytes
    SPI_Missing::transfer(_SLAVE_SELECT_PIN, (SPI_SLAVE_ID | SPI_SLAVE_ADDR | SPI_SLAVE_WRITE), SPI_CONTINUE);
    SPI_Missing::transfer(_SLAVE_SELECT_PIN, CTLREG_BK0_GPIOA, SPI_CONTINUE);
    SPI_Missing::transfer(_SLAVE_SELECT_PIN, gp[0], SPI_CONTINUE);
    SPI_Missing::transfer(_SLAVE_SELECT_PIN, gp[1], SPI_LAST);

    // Shift bits for storage
    _state >>= 1;

    return _state;
}

// Sets the lights on and off as designated by the bitmask
unsigned short
KeyboardLightController::setLights(
const unsigned short BITMASK_
) {
    return set(BITMASK_, 0xFF);
}
