#pragma once

#ifdef WINVER
#undef WINVER
#endif
#ifdef _WIN32_WINNT
#undef _WIN32_WINNT
#endif

#define WINVER 0x0A00 // Windows 10
#define _WIN32_WINNT 0x0A00
#define WIN32_LEAN_AND_MEAN

#include <Windows.h>