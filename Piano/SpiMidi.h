#include "stdafx.h"
//0x0 rw 0x4800 80 CLKI4 MODE Mode control
enum VS1053_REGISTER
{
    MODE = 0,           // Mode control
    STATUS,             // Status of VS1053
    BASS,               // Built-in bass/treble control
    CLOCKF,             // Clock freq + multiplier
    DECODE,             // TIME Decode time in seconds
    AUDATA,             // Misc. audio data
    WRAM,               // RAM write/read
    WRAMADDR,           // Base address for RAM write/read
    HDAT0,              // Stream header data 0
    HDAT1,              // Stream header data 1
    AIADDR,             // Start address of application
    VOL,                // Volume control
    AICTRL0,            // Application control register 0
    AICTRL1,            // Application control register 1
    AICTRL2,            // Application control register 2
    AICTRL3,            // Application control register 3
};

const int VS1053_XDCS = 7;
const int VS1053_XCS = 6;
const int VS1053_DREQ = 2;
const int VS1053_RX = 3;        // needs to be kept high
const int VS1053_GPIO1 = 4;
const int VS1053_RESET = 8;

const int OPCODE_READ = 0x03;
const int OPCODE_WRITE = 0x02;

#define VS1053_MODE_DEFAULT 0x4800          // this is the value of the MODE register on reset
#define VS1053_MODE_SM_TESTS 5              // bit position of test mode

void MusicShieldInit(bool realTimeMidiMode);
unsigned int SciRead(VS1053_REGISTER reg);
void talkMIDI(BYTE cmd, BYTE data1, BYTE data2);
void talkMIDI(BYTE cmd, BYTE data1);
void noteOn(BYTE channel, BYTE note, BYTE attack_velocity);
void noteOff(BYTE channel, BYTE note, BYTE release_velocity);