/*++

Copyright (c) Microsoft Corporation.  All rights reserved.

Module Name:

SpiMidi.cpp

Abstract:

SPI client driver implementation functions

--*/

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

// Must be called directly after hardware reset
// (can read registers before calling this, but no config changes)
void VS1053SinWaveTest()
{
    unsigned int mode;

    // enter test mode: set SM_TESTS in mode register
    wprintf(L"Doing VS1053 sine wave test (1.125Khz)\n");
    mode = VS1053_MODE_DEFAULT | (1 << VS1053_MODE_SM_TESTS);
    SciWrite(VS1053_REGISTER::MODE, mode);

    // read back mode
    mode = SciRead(VS1053_REGISTER::MODE);
    wprintf(L"mode is: 0x%x\n", mode);

    // Send test command on SDI bus
    //   Enter sine test:  0x53 0xEF 0x6E n 0 0 0 0 where n = (1 << 5) | 3 to yield 1.125Khz sine wave
    wprintf(L"Sending sine test command on SDI\n");
    unsigned char enterSineTestCmd [] = { 0x53, 0xEF, 0x6E, (1 << 5) | 3, 0, 0, 0, 0 };
    SdiSend(enterSineTestCmd, sizeof(enterSineTestCmd));

    // let it run for 10 seconds
    Sleep(10000);

    wprintf(L"Sending stop test command\n");
    //   Exit sine test:: 0x45 0x78 0x69 0x74 0 0 0 0
    unsigned char exitSineTestCmd [] = { 0x45, 0x78, 0x69, 0x74, 0, 0, 0, 0 };
    SdiSend(exitSineTestCmd, sizeof(exitSineTestCmd));

    wprintf(L"Exiting test mode\n");
    // exit test mode
    SciWrite(VS1053_REGISTER::MODE, VS1053_MODE_DEFAULT);
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
    wprintf(L"size of buffer sent: %d\n",sizeof(buf));
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