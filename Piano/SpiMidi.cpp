// Copyright(c) Microsoft Open Technologies, Inc. All rights reserved.
// Licensed under the BSD 2 - Clause License.
// See License.txt in the project root for license information.

// SPI client driver implementation functions

#include "SpiMidi.h"

// send data on Sdi
void SdiSend(void *buf, int byteCount)
{
    byte *spi_data = (byte *) buf;
    unsigned char spi_last = SPI_CONTINUE;
    for (int i = 0; i < byteCount; ++i) {
        if (i == (byteCount - 1))
        {
            spi_last = SPI_LAST;
        }
        SPI_Missing::transfer(VS1053_XDCS, spi_data[i], spi_last);
    }
}

//
// READ - 0x03
// First send READ opcode
// Then send 8-bit address
// After the address has been read in, further data on SI is ignored.
// Next, the 16-bit data will be shifted onto the SO line
//
unsigned int SciRead(VS1053_REGISTER reg)
{
    unsigned short value = 0;
    byte *value_bytes = (byte *) &value;

    // send a single 16-bit command
    SPI_Missing::transfer(VS1053_XCS, OPCODE_READ, SPI_CONTINUE);
    SPI_Missing::transfer(VS1053_XCS, reg, SPI_CONTINUE);

    // read a single 16-bit response
    value_bytes[0] = SPI_Missing::transfer(VS1053_XCS, 0x00, SPI_CONTINUE);
    value_bytes[1] = SPI_Missing::transfer(VS1053_XCS, 0x00, SPI_LAST);

    if (!value)
    {
        wprintf(L"SPI transfer failed!\n");
        return -1;
    }

    return value;
}

// only the low 16 bits of data are used
//
// WRITE - 0x02
// Send WRITE opcode
// Send 8-bit address
// Send 16-bit data
//
void SciWrite(VS1053_REGISTER reg, unsigned int data)
{
    SPI_Missing::transfer(VS1053_XCS, OPCODE_WRITE, SPI_CONTINUE);
    SPI_Missing::transfer(VS1053_XCS, reg, SPI_CONTINUE);
    SPI_Missing::transfer(VS1053_XCS, HIBYTE(data), SPI_CONTINUE);
    SPI_Missing::transfer(VS1053_XCS, LOBYTE(data), SPI_LAST);
}

// Initialize the Galileo for communications with the music shield
// Also resets the music shield. Serial communication can be done
// after calling this method.
void MusicShieldInit(bool realTimeMidiMode)
{
    wprintf(L"Initializing music shield\n");

    wprintf(L"Putting VS1053 in reset\n");
    digitalWrite(VS1053_RESET, LOW);      // reset is active low

    digitalWrite(VS1053_RX, HIGH);
    digitalWrite(VS1053_XDCS, HIGH);
    digitalWrite(VS1053_XCS, HIGH);

    // Music shield has been modified to connect IO2 of Arduino header
    // to GPIO1 of VS1053. Bringing this pin high during reset will
    // put the VS1053 in real time midi mode
    digitalWrite(VS1053_GPIO1, realTimeMidiMode);
    wprintf(L"Bringing VS1053 out of reset\n");
    // bring VS1053 out of reset
    digitalWrite(VS1053_RESET, HIGH);      // reset is active low and is tied to ground through a pulldown
    // need to bring high to power up the chip
}

//Plays a MIDI note. Doesn't check to see that cmd is greater than 127, or that data values are less than 127
void talkMIDI(BYTE cmd, BYTE data1, BYTE data2)
{
    unsigned char buf [] = { 0x00, cmd, 0x00, data1, 0x00, data2 };
    SdiSend(buf, sizeof(buf));
}

//Plays a MIDI note. Doesn't check to see that cmd is greater than 127, or that data values are less than 127
void talkMIDI(BYTE cmd, BYTE data1)
{
    unsigned char buf [] = { 0x00, cmd, 0x00, data1 };
    SdiSend(buf, sizeof(buf));
}

//Send a MIDI note-on message.  Like pressing a piano key
//channel ranges from 0-15
void noteOn(BYTE channel, BYTE note, BYTE attack_velocity) {
    talkMIDI((0x90 | channel), note, attack_velocity);
}

//Send a MIDI note-off message.  Like releasing a piano key
void noteOff(BYTE channel, BYTE note, BYTE release_velocity) {
    talkMIDI((0x80 | channel), note, release_velocity);
}