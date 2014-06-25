
#include "KeyboardLightController.h"

KeyboardLightController::KeyboardLightController(
    const unsigned char SLAVE_SELECT_PIN_
    ) :
    _SLAVE_SELECT_PIN(SLAVE_SELECT_PIN_)
{}

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

unsigned short
KeyboardLightController::disableLights(
const unsigned short BITMASK_
) {
    return set(BITMASK_, 0);
}

unsigned short
KeyboardLightController::enableLights(
const unsigned short BITMASK_
) {
    return set(BITMASK_, 1);
}

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

unsigned short
KeyboardLightController::set(
const unsigned short BITMASK_,
const unsigned char CTL_
) {
    const unsigned char *gp = 0;

    // Apply bitmask relative or absolute
    switch (CTL_) {
    case 0:
        _state &= ~BITMASK_;
        break;
    case 1:
        _state |= BITMASK_;
        break;
    default:
        _state = BITMASK_;
    }

    // Shift bits for pin mapping
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

unsigned short
KeyboardLightController::setLights(
const unsigned short BITMASK_
) {
    return set(BITMASK_, 0xFF);
}
