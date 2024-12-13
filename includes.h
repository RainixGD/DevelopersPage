#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <cocos2d.h>
#include <gd.h>
#include <MinHook.h>
#include <nlohmann/json.hpp>

#include <array>
#include <fstream>

#define MEMBERBYOFFSET(type, class, offset) *reinterpret_cast<type*>(reinterpret_cast<uintptr_t>(class) + offset)
#define MBO MEMBERBYOFFSET

using namespace cocos2d;
using namespace gd;

#define WIN32CAC_ENTRY(inject) \
	DWORD WINAPI _thread__func_(void* hModule) { \
        MH_Initialize(); \
	    inject(); \
	    return true; \
	} \
	BOOL APIENTRY DllMain(HMODULE handle, DWORD reason, LPVOID reserved) { \
	    if (reason == DLL_PROCESS_ATTACH) { \
	        CreateThread(0, 0x100, _thread__func_, handle, 0, 0); \
	    } \
	    return TRUE; \
	} 