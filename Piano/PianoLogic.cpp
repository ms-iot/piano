// Copyright(c) Microsoft Open Technologies, Inc.All rights reserved.Licensed under the BSD 2 - Clause License.See License.txt in the project root for license information.
//

// Hosts all of the piano specific logic

#include "PianoLogic.h"
#include "SpiMidi.h"

KeyboardLightController keyboard_lights(10);

using namespace std;

struct KeyAttribs
{
    int		MidiChannel;
    int		MidiNote;
    int		LightIndex;
    wstring FileName;
    wstring CommandName;
};

std::map<WCHAR, KeyAttribs> KeyTable;	// The Table Containing Mappings for the keys
byte note = 0;						    // The MIDI note value to be played
byte resetMIDI = 4;					    // Tied to VS1053 Reset line
byte ledPin = 13;					    // MIDI traffic inidicator
int  instrument = 0;

struct Note
{
    WCHAR key;							// L'\0' is a rest (no sound)
    DWORD duration;
};

list<Note> SongNotes;					// to hold the notes and their playtimes of a song

HANDLE monitor;		// the handle for the folder changed events

// Turns the light on for the mapped key
void TurnLightOn(WCHAR key)
{
    printf("Light On: %d\n", KeyTable[key].LightIndex);
    OutputDebugString(L"Light On\n");

    if (KeyTable[key].LightIndex > 0)
    {
        // since the lights go from 1 to 14 in the key mapping
        int i = KeyTable[key].LightIndex - 1;

        // set the bit for this light index
        unsigned short key_mask = 1;
        key_mask <<= i;

        // set the key designated in temp to on
        keyboard_lights.enableLights(key_mask);
    }
}

// Turns the light off for the mapped key
void TurnLightOff(WCHAR key)
{
    printf("Light Off: %d\n", KeyTable[key].LightIndex);
    OutputDebugString(L"Light Off\n");

    if (KeyTable[key].LightIndex > 0)
    {
        // set the bits for this light index
        unsigned short key_mask = 1;
        key_mask <<= (KeyTable[key].LightIndex - 1);

        // set the key designated in temp to off
        keyboard_lights.disableLights(key_mask);
    }
}

// Reads the Key Map Attributes and stores the mappings
HRESULT ReadKeyMapAttributes(IXmlReader* pReader)
{
    const WCHAR* pwszPrefix;		// storing the prefix
    const WCHAR* pwszLocalName;		// storing the LocalName
    const WCHAR* pwszValue;			// storing the Value
    HRESULT hr = pReader->MoveToFirstAttribute();

    if (S_FALSE == hr)
        return hr;
    if (S_OK != hr)
    {
        wprintf(L"Error moving to first attribute, error is %08.8lx\n", hr);
        return hr;
    }
    else
    {
        WCHAR key = L'\0';
        KeyAttribs a;
        while (TRUE)
        {
            if (!pReader->IsDefault())
            {
                UINT cwchPrefix;

                // handles getting the prefix, localname, and value for the attribute
                if (FAILED(hr = pReader->GetPrefix(&pwszPrefix, &cwchPrefix)))
                {
                    wprintf(L"Error getting prefix, error is %08.8lx\n", hr);
                    return hr;
                }
                if (FAILED(hr = pReader->GetLocalName(&pwszLocalName, NULL)))
                {
                    wprintf(L"Error getting local name, error is %08.8lx\n", hr);
                    return hr;
                }
                if (FAILED(hr = pReader->GetValue(&pwszValue, NULL)))
                {
                    wprintf(L"Error getting value, error is %08.8lx\n", hr);
                    return hr;
                }

                // Handles the attributes
                if (wcscmp(pwszLocalName, TEXT("key")) == 0)
                {
                    // stores the key
                    key = pwszValue[0];
                }
                else if (wcscmp(pwszLocalName, TEXT("note")) == 0)
                {
                    // stores the note
                    a.MidiNote = stoi(pwszValue);
                }
                else if (wcscmp(pwszLocalName, TEXT("command")) == 0)
                {
                    // stores the command
                    a.CommandName = pwszValue;
                }
                else if (wcscmp(pwszLocalName, TEXT("light")) == 0)
                {
                    // stores the light
                    a.LightIndex = stoi(pwszValue);
                }
                else if (wcscmp(pwszLocalName, TEXT("channel")) == 0)
                {
                    // stores the channel
                    a.MidiChannel = stoi(pwszValue);
                }
            }
            if (S_OK != pReader->MoveToNextAttribute())
                break;
        }
        if (key == L'\0')
        {
            wprintf(L"bad xml file format\n");
        }
        else
        {
            // remember the key mapping
            KeyTable[key] = a;
            printf("Key inputted into table\n");
        }
    }
    return hr;
}

// Processes the KeyMap file
HRESULT ProcessKeyMapXml(wchar_t * file)
{
    HRESULT hr = S_OK;
    IStream *pFileStream = NULL;
    IXmlReader *pReader = NULL;
    XmlNodeType nodeType;
    const WCHAR* pwszPrefix;
    const WCHAR* pwszLocalName;
    UINT cwchPrefix;

    //Open read-only input stream
    if (FAILED(hr = SHCreateStreamOnFile(file, STGM_READ, &pFileStream)))
    {
        printf("Error creating file reader, error is %08.8lx\n", hr);
        HR(hr);
    }

    if (FAILED(hr = CreateXmlReader(__uuidof(IXmlReader), (void**) &pReader, NULL)))
    {
        wprintf(L"Error creating xml reader, error is %08.8lx\n", hr);
        HR(hr);
    }

    if (FAILED(hr = pReader->SetProperty(XmlReaderProperty_DtdProcessing, DtdProcessing_Prohibit)))
    {
        wprintf(L"Error setting XmlReaderProperty_DtdProcessing, error is %08.8lx\n", hr);
        HR(hr);
    }

    if (FAILED(hr = pReader->SetInput(pFileStream)))
    {
        wprintf(L"Error setting input for reader, error is %08.8lx\n", hr);
        HR(hr);
    }

    //read until there are no more nodes
    while (S_OK == (hr = pReader->Read(&nodeType)))
    {
        switch (nodeType)
        {
        case XmlNodeType_Element:
            if (FAILED(hr = pReader->GetPrefix(&pwszPrefix, &cwchPrefix)))
            {
                wprintf(L"Error getting prefix, error is %08.8lx\n", hr);
                HR(hr);
            }
            if (FAILED(hr = pReader->GetLocalName(&pwszLocalName, NULL)))
            {
                wprintf(L"Error getting local name, error is %08.8lx\n", hr);
                HR(hr);
            }

            // checks to see if the element is a note, if so look at the attributes
            if (wcscmp(pwszLocalName, TEXT("note")) == 0)
            {
                if (FAILED(hr = ReadKeyMapAttributes(pReader)))
                {
                    wprintf(L"Error writing attributes, error is %08.8lx\n", hr);
                    HR(hr);
                }
            }

            if (pReader->IsEmptyElement())
            {
                wprintf(L" (empty)");
            }
            break;
        }
    }

CleanUp:
    SAFE_RELEASE(pFileStream);
    SAFE_RELEASE(pReader);
    return hr;
}

// Reads the XML attributes of the song file, parses them, and stores them in a queue for playback.
HRESULT ReadSongAttributes(IXmlReader* pReader)
{
    const WCHAR* pwszPrefix;		// storing the prefix
    const WCHAR* pwszLocalName;		// storing the LocalName
    const WCHAR* pwszValue;			// storing the Value
    HRESULT hr = pReader->MoveToFirstAttribute();
    int timing = 0;

    if (S_FALSE == hr)
        return hr;
    if (S_OK != hr)
    {
        wprintf(L"Error moving to first attribute, error is %08.8lx\n", hr);
        return hr;
    }
    else
    {
        WCHAR key = L'\0';
        DWORD duration = 0;
        while (TRUE)
        {
            if (!pReader->IsDefault())
            {
                UINT cwchPrefix;
                if (FAILED(hr = pReader->GetPrefix(&pwszPrefix, &cwchPrefix)))
                {
                    wprintf(L"Error getting prefix, error is %08.8lx\n", hr);
                    return hr;
                }
                if (FAILED(hr = pReader->GetLocalName(&pwszLocalName, NULL)))
                {
                    wprintf(L"Error getting local name, error is %08.8lx\n", hr);
                    return hr;
                }
                if (FAILED(hr = pReader->GetValue(&pwszValue, NULL)))
                {
                    wprintf(L"Error getting value, error is %08.8lx\n", hr);
                    return hr;
                }

                if (wcscmp(pwszLocalName, TEXT("key")) == 0)
                {
                    // stores the key to "press"
                    key = pwszValue[0];
                }
                else  if (wcscmp(pwszLocalName, TEXT("duration")) == 0)
                {
                    // stores the duration
                    duration = _wtoi(pwszValue);
                }
            }

            if (S_OK != pReader->MoveToNextAttribute())
                break;
        }

        // If we have a proper note, queue it
        if (key != L'\0' && duration > 0)
        {
            Note n = { key, duration };
            SongNotes.push_back(n);
        }
    }
    return hr;
}

// Processes the song XML file
HRESULT ProcessSongXml(LPCWSTR file)
{
    HRESULT hr = S_OK;
    IStream *pFileStream = NULL;
    IXmlReader *pReader = NULL;
    XmlNodeType nodeType;
    const WCHAR* pwszPrefix;
    const WCHAR* pwszLocalName;
    UINT cwchPrefix;

    //Open read-only input stream
    if (FAILED(hr = SHCreateStreamOnFile(file, STGM_READ, &pFileStream)))
    {
        wprintf(L"Error creating file reader, error is %08.8lx\n", hr);
        HR(hr);
    }

    if (FAILED(hr = CreateXmlReader(__uuidof(IXmlReader), (void**) &pReader, NULL)))
    {
        wprintf(L"Error creating xml reader, error is %08.8lx\n", hr);
        HR(hr);
    }

    if (FAILED(hr = pReader->SetProperty(XmlReaderProperty_DtdProcessing, DtdProcessing_Prohibit)))
    {
        wprintf(L"Error setting XmlReaderProperty_DtdProcessing, error is %08.8lx\n", hr);
        HR(hr);
    }

    if (FAILED(hr = pReader->SetInput(pFileStream)))
    {
        wprintf(L"Error setting input for reader, error is %08.8lx\n", hr);
        HR(hr);
    }

    //read until there are no more nodes
    while (S_OK == (hr = pReader->Read(&nodeType)))
    {
        switch (nodeType)
        {
        case XmlNodeType_Element:
            if (FAILED(hr = pReader->GetPrefix(&pwszPrefix, &cwchPrefix)))
            {
                wprintf(L"Error getting prefix, error is %08.8lx\n", hr);
                HR(hr);
            }
            if (FAILED(hr = pReader->GetLocalName(&pwszLocalName, NULL)))
            {
                wprintf(L"Error getting local name, error is %08.8lx\n", hr);
                HR(hr);
            }

            // checks to see if the element is a note, if so look at the attributes
            if (wcscmp(pwszLocalName, TEXT("note")) == 0 ||
                wcscmp(pwszLocalName, TEXT("song")) == 0)
            {
                if (FAILED(hr = ReadSongAttributes(pReader)))
                {
                    wprintf(L"Error writing attributes, error is %08.8lx\n", hr);
                    HR(hr);
                }
            }

            if (pReader->IsEmptyElement())
                wprintf(L" (empty)");
            break;
        }
    }

CleanUp:
    SAFE_RELEASE(pFileStream);
    SAFE_RELEASE(pReader);
    return hr;
}

// Play the song
void PlaySongFile(LPCWSTR pFileName)
{

    SongNotes.clear();

    ProcessSongXml(pFileName);
    if (SongNotes.empty())
    {
        return;
    }

    printf("Mode: auto play\n");
    OutputDebugString(L"Mode: auto play\n");

    // play them all !
    for (list<Note>::iterator it = SongNotes.begin(); it != SongNotes.end(); ++it)
    {
        if (it->key != L'\0') {
            StartSound(it->key);
            TurnLightOn(it->key);
        }
        Sleep(it->duration);
        if (it->key != L'\0') {
            StopSound(it->key);
            TurnLightOff(it->key);
        }
    }

    printf("Mode: free play\n");
    OutputDebugString(L"Mode: free play\n");
}

// Logic for sending the start sound command to the MIDI shield
void StartSound(WCHAR key)
{
    if (KeyTable[key].LightIndex > 0)
    {
        // get the note to play
        printf("Sound On: %d:%d\n", KeyTable[key].MidiChannel, KeyTable[key].MidiNote);
        OutputDebugString(L"Sound On\n");
        noteOn(KeyTable[key].MidiChannel, KeyTable[key].MidiNote, 100);
    }
}

// Logic for sending the stop sound command to the MIDI shield
void StopSound(WCHAR key)
{
    // Using KeyLightTable to find out if this is a legitimate note
    if (KeyTable[key].LightIndex > 0)
    {
        // stop the sound
        printf("Sound Off: %d:%d\n", KeyTable[key].MidiChannel, KeyTable[key].MidiNote);
        OutputDebugString(L"Sound Off\n");
        noteOff(KeyTable[key].MidiChannel, KeyTable[key].MidiNote, 100);
    }
}

// When the containing folder is changed, it should clear the table and re-process the KeyMap
VOID CALLBACK FolderChanged(PVOID lpParameter, BOOLEAN TimerOrWaitFired)
{
    OutputDebugString(L"KeyMap was changed\n");
    if (0 == FindNextChangeNotification(monitor))
    {
        OutputDebugString(L"FindCloseChangeNotification failed\n");
    }
    else
    {
        ClearKeyTable();
        ProcessKeyMapXml(KEY_MAP_FILE);
    }
}

// Logic for when the key is pressed
void KeyDown(WCHAR key)
{
    // special command?
    if (!KeyTable[key].CommandName.empty())
    {
        // ignore special commands, KeyUp will do it
    }
    else
    {
        // Sound on! light it up!
        TurnLightOn(key);
        StartSound(key);
    }
}

// Logic for when the key is unpressed
void KeyUp(WCHAR key)
{
    // special command?
    if (!KeyTable[key].CommandName.empty())
    {
        // special command?
        if (wcscmp(KeyTable[key].CommandName.c_str(), PLAY_SONG_FILE) == 0)
        {
            PlaySongFile(SONG_FILE);
        }
    }
    else
    {
        // Turn the sound and light off.
        StopSound(key);
        TurnLightOff(key);
    }
}

// Clears the stored keymappings
void ClearKeyTable()
{
    KeyTable.clear();
}

// Initializes the key mapping, folder monitoring, and keyboard lights
HRESULT Initialize()
{
    HRESULT hr = S_OK;

    // load our key mapping configuration
    ProcessKeyMapXml(KEY_MAP_FILE);

    // initialize the keyboard lights and twinkle the lights
    wprintf(L"Keyboard Light Initialization\n");
    OutputDebugString(L"Keyboard Light Initialization\n");
    keyboard_lights.begin();
    for (int i = 1; i < 8193; i <<= 1) {
        keyboard_lights.enableLights(i);
        delay(100);
    }
    wprintf(L"Resetting Lights\n");
    OutputDebugString(L"Resetting Lights\n");
    for (int i = 1; i < 8193; i <<= 1) {
        keyboard_lights.disableLights(i);
        delay(100);
    }

    // Starts the Monitoring on the Folder for the changes
    monitor = FindFirstChangeNotification(DIRECTORY_LOC, FALSE, FILE_NOTIFY_CHANGE_LAST_WRITE);
    if (INVALID_HANDLE_VALUE == monitor)
    {
        OutputDebugString(L"FindFirstChangeNotification failed: Invalid Handle\n");
    }

    HANDLE newWaitObject;
    RegisterWaitForSingleObject(&newWaitObject, monitor, FolderChanged, monitor, INFINITE, WT_EXECUTEDEFAULT);

    //end:
    return hr;
}