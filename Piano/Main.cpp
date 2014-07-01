// Copyright(c) Microsoft Open Technologies, Inc.All rights reserved.Licensed under the BSD 2 - Clause License.See License.txt in the project root for license information.
//

// Main.cpp : Defines the entry point for the console application, sets up the piano environment, and continually checks for keyboard input.

#include "PianoLogic.h"
#include "SpiMidi.h"

using namespace std;

WCHAR sz[1024]; // used for debug printing

// keeps track of all of the key's that are currently pressed
std::map<WCHAR, bool> KeyDownState;

int _tmain(int argc, _TCHAR* argv [])
{
    HANDLE			hIn;	// handle for input
    INPUT_RECORD	InRec;
    DWORD			NumRead;
    bool			exit = false;
    OVERLAPPED		Overlapped = { 0 };

    // put all the GPIO's in a known state
    wprintf(L"Initializing ... ");
    ArduinoInit();
    wprintf(L"Done\n");

    ZeroMemory(&Overlapped, sizeof(OVERLAPPED));

    unsigned int mode;

    SPI_Missing::begin(VS1053_XCS);
    SPI_Missing::begin(VS1053_XDCS);
    pinMode(VS1053_RESET, OUTPUT);
    pinMode(VS1053_RX, OUTPUT);
    pinMode(VS1053_GPIO1, OUTPUT);
    wprintf(L"Done calling SPI.begin()\n");
    
    MusicShieldInit(true);

    mode = SciRead(VS1053_REGISTER::MODE);
    printf("Midi Mode: %d\n", mode);

    talkMIDI(0xB1, 0x79);        // reset controller on channel 1
    talkMIDI(0xC1, 1);           // instrument select
    talkMIDI(0xB1, 7, 127);      // sendCC

    int instrument = 1;

    talkMIDI(0xC1, instrument); // sets instrument to 1

    printf("Welcome to Piano!\n");
    OutputDebugString(L"Welcome to Piano!\n");
    hIn = GetStdHandle(STD_INPUT_HANDLE);

    // Calling the PianoLogic Initialize Function
    Initialize();
    wprintf(L"Done with initialization\n");

    // run through command line parameters
    if (argc > 1)
    {
        for (int i = 0; i < argc; i++)
        {
            // If the auto-play flag was given, play the song
            if (_tcscmp(argv[i], _T("/a")) == 0)
            {
                wprintf(L"Auto Playing from command prompt\n");
                PlaySongFile(SONG_FILE);
            }
        }
    }

    // looping until the exit flag is given
    while (!exit)
    {
        ReadConsoleInput(
            hIn,
            &InRec,
            1,
            &NumRead);

        WCHAR ch;
        switch (InRec.EventType)
        {
        case KEY_EVENT:
            ch = InRec.Event.KeyEvent.uChar.UnicodeChar;

            // If the Key Event is a press down
            if (InRec.Event.KeyEvent.bKeyDown)
            {
                if (VK_ESCAPE == InRec.Event.KeyEvent.wVirtualKeyCode)
                {
                    printf("ESC was Pressed, now exiting\n");
                    OutputDebugString(L"ESC was Pressed, now exiting.\n");
                    exit = true;
                }
                else
                {

                    if (KeyDownState.count(ch) == 0)
                    {
                        // just in time init (to false)
                        KeyDownState[ch] = false;
                    }

                    // If this key is not currently pressed
                    if (KeyDownState[ch] == false)
                    {
                        // keep track of the key being pressed
                        KeyDownState[ch] = true;
                        swprintf(sz, sizeof(sz), L"Key Down: %c\n", ch);
                        OutputDebugString(sz); wprintf(sz);

                        // Call the PianoLogic's KeyDown Function
                        KeyDown(ch);
                    }
                }
            }
            else
            {
                // The Key Event must be a key up
                // keep track of the key being unpressed
                KeyDownState[ch] = false;
                swprintf(sz, sizeof(sz), L"Key Up: %c\n", ch);
                OutputDebugString(sz); wprintf(sz);

                // Call the PianoLogic's KeyUp Function
                KeyUp(ch);
            }
            break;
        }
    }

    wprintf(L"Done\n");
    wprintf(L"Shutting down VS1053\n");

    // put the VS1053 back in reset
    keyboard_lights.end();
    digitalWrite(VS1053_RESET, 0);
    SPI.end();
    wprintf(L"Success\n");

    return 0;
}