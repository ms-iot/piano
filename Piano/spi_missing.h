// Copyright(c) Microsoft Open Technologies, Inc. All rights reserved.
// Licensed under the BSD 2 - Clause License.
// See License.txt in the project root for license information.

#include "spi.h"
#define SPI_CONTINUE 0
#define SPI_LAST     1

// Namespace to replace the missing SPI calls
namespace SPI_Missing
{
    // Calls SPI.begin after initializing the slaveSelectPin
    inline void begin(int slaveSelectPin)
    {
        if ((slaveSelectPin >= 0) &&
            (slaveSelectPin < NUM_ARDUINO_PINS) &&
            (slaveSelectPin != 11) &&
            (slaveSelectPin != 12) &&
            (slaveSelectPin != 13))
        {
            // save this is the bitmask
            // pinSelectBitmask |= (1 << slaveSelectPin);

            // bring CS pin HIGH before pinning out
            pinMode(slaveSelectPin, OUTPUT);
            digitalWrite(slaveSelectPin, HIGH);

            // if they want to use one of the analog pins, select the alternate pin function (GPIO)
            // otherwise select the default pin function (GPIO)
            pinFunction(slaveSelectPin, _IsAnalogPin(slaveSelectPin) ? 1 : 0);
        }

        SPI.begin();
    }

    // created to allow arduino compatible libraries to work as well
    inline void end(int slaveSelectPin)
    {
        // pinSelectBitmask &= ~(1 << slaveSelectPin);
    }

    // Calls SPI.transfer and toggles the slaveSelectPin to match.
    inline int transfer(int slaveSelectPin, unsigned char val, int transferMode)
    {
        int ret;

        digitalWrite(slaveSelectPin, LOW);

        ret = SPI.transfer(val);

        if (transferMode != SPI_CONTINUE)
        {
            digitalWrite(slaveSelectPin, HIGH);
        }

        return ret;
    }

    // 8-bit transfers
    inline int transfer(int slaveSelectPin, unsigned char val)
    {
        return transfer(slaveSelectPin, val, SPI_LAST);
    }

    //ULONG pinSelectBitmask;
}