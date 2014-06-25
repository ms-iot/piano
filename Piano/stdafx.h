// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//
#pragma once
#include "targetver.h"

//#include <Windows.h>
//#include <winioctl.h>
#include <stdio.h>
#include <tchar.h>
//#include <stdlib.h>
//#include <malloc.h>
//#include <memory.h>
#include <tchar.h>
#include <map>
#include <list>
#include <string>
//#include <bitset>
#include <iostream>
//#include <ole2.h>
#include "arduino.h"
#include <xmllite.h>
#include <shlwapi.h>
#include "spi.h"
#include "spi_missing.h"

#pragma warning(disable : 4127)  // conditional expression is constant
#define CHKHR(stmt)             do { hr = (stmt); if (FAILED(hr)) goto CleanUp; } while(0)
#define HR(stmt)                do { hr = (stmt); goto CleanUp; } while(0)
#define SAFE_RELEASE(I)         do { if (I){ I->Release(); } I = NULL; } while(0)

#define MAX_LOADSTRING 100
