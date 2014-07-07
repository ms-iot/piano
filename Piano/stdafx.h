// Copyright(c) Microsoft Open Technologies, Inc. All rights reserved.
// Licensed under the BSD 2 - Clause License.
// See License.txt in the project root for license information.

// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently

#pragma once
#include "targetver.h"

#include <stdio.h>
#include <tchar.h>
#include <tchar.h>
#include <map>
#include <list>
#include <string>
#include <iostream>
#include "arduino.h"
#include <xmllite.h>
#include <shlwapi.h>
#include "spi.h"
#include "spi_missing.h"

#define CHKHR(stmt)             do { hr = (stmt); if (FAILED(hr)) goto CleanUp; } while(0)
#define HR(stmt)                do { hr = (stmt); goto CleanUp; } while(0)
#define SAFE_RELEASE(I)         do { if (I){ I->Release(); } I = NULL; } while(0)

#define MAX_LOADSTRING 100
