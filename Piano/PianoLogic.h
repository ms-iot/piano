#include "stdafx.h"
#include "KeyboardLightController.h"

// special commands
#define START_RECORDING L"_StartRecording"
#define STOP_RECORDING	L"_StopRecording"
#define PLAY_SONG_FILE	L"_PlaySongFile"
#define KEY_MAP_FILE L"KeyMap.xml"
#define SONG_FILE L"Song.xml"
#define DIRECTORY_LOC L"C:\\Piano"

HRESULT Initialize();

VOID CALLBACK FolderChanged(PVOID lpParameter, BOOLEAN TimerOrWaitFired);
void ClearKeyTable();
HRESULT ProcessKeyMapXml(wchar_t * file);
bool GetLatestFile(WCHAR * pFileName);
void StartRecording();
void StopRecording();
void PlaySongFile(LPCWSTR pFileName);
void KeyDown(WCHAR key);
void KeyUp(WCHAR key);
void StartSound(WCHAR key);
void StopSound(WCHAR key);
void LightOn(WCHAR key);
void LightOff(WCHAR key);

extern KeyboardLightController keyboard_lights;  // Initialize keyboard light controller with slave select on pin 10
